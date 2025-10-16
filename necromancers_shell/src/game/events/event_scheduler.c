/**
 * @file event_scheduler.c
 * @brief Event scheduling system implementation
 */

#include "event_scheduler.h"
#include "../game_state.h"
#include "../resources/resources.h"
#include "../resources/corruption.h"
#include "../../utils/logger.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MAX_EVENTS 256
#define MAX_FLAGS 128

/**
 * @brief Flag structure for game state tracking
 */
typedef struct {
    char name[64];
    bool set;
} GameFlag;

/**
 * @brief Event scheduler structure
 */
struct EventScheduler {
    ScheduledEvent events[MAX_EVENTS];
    size_t event_count;

    GameFlag flags[MAX_FLAGS];
    size_t flag_count;

    uint32_t last_check_day;
    uint8_t last_check_corruption;
    uint32_t last_check_location;
};

EventScheduler* event_scheduler_create(void) {
    EventScheduler* scheduler = malloc(sizeof(EventScheduler));
    if (!scheduler) {
        LOG_ERROR("Failed to allocate EventScheduler");
        return NULL;
    }

    scheduler->event_count = 0;
    scheduler->flag_count = 0;
    scheduler->last_check_day = 0;
    scheduler->last_check_corruption = 0;
    scheduler->last_check_location = 0;

    memset(scheduler->events, 0, sizeof(scheduler->events));
    memset(scheduler->flags, 0, sizeof(scheduler->flags));

    LOG_DEBUG("EventScheduler created");
    return scheduler;
}

void event_scheduler_destroy(EventScheduler* scheduler) {
    if (scheduler) {
        LOG_DEBUG("EventScheduler destroyed");
        free(scheduler);
    }
}

bool event_scheduler_register(EventScheduler* scheduler, ScheduledEvent event) {
    if (!scheduler) {
        return false;
    }

    if (scheduler->event_count >= MAX_EVENTS) {
        LOG_ERROR("EventScheduler event list is full");
        return false;
    }

    scheduler->events[scheduler->event_count] = event;
    scheduler->event_count++;

    LOG_DEBUG("Event registered: %s (ID: %u, trigger: %d, value: %u)",
               event.name, event.id, event.trigger_type, event.trigger_value);

    return true;
}

/**
 * @brief Check if event's trigger conditions are met
 *
 * @param event Event to check
 * @param state Game state
 * @param scheduler Scheduler (for flag checking)
 * @return true if conditions met, false otherwise
 */
static bool event_check_conditions(const ScheduledEvent* event, const GameState* state,
                                  const EventScheduler* scheduler) {
    /* Check if already triggered */
    if (event->triggered) {
        return false;
    }

    /* Check day range */
    if (event->min_day > 0 && state->resources.day_count < event->min_day) {
        return false;
    }
    if (event->max_day > 0 && state->resources.day_count > event->max_day) {
        return false;
    }

    /* Check required flag */
    if (event->requires_flag) {
        if (!event_scheduler_has_flag(scheduler, event->required_flag)) {
            return false;
        }
    }

    /* Check trigger-specific conditions */
    switch (event->trigger_type) {
        case EVENT_TRIGGER_DAY:
            return state->resources.day_count == event->trigger_value;

        case EVENT_TRIGGER_CORRUPTION:
            return state->corruption.corruption >= event->trigger_value;

        case EVENT_TRIGGER_LOCATION:
            return state->current_location_id == event->trigger_value;

        case EVENT_TRIGGER_FLAG:
            /* Flag name is stored in required_flag field */
            return event_scheduler_has_flag(scheduler, event->required_flag);

        case EVENT_TRIGGER_QUEST:
            /* Quest completion checking would go here */
            /* For now, always false until quest system integration */
            return false;

        default:
            return false;
    }
}

/**
 * @brief Compare events for priority sorting
 */
static int event_compare_priority(const void* a, const void* b) {
    const ScheduledEvent* event_a = (const ScheduledEvent*)a;
    const ScheduledEvent* event_b = (const ScheduledEvent*)b;

    /* Higher priority first */
    if (event_a->priority > event_b->priority) {
        return -1;
    } else if (event_a->priority < event_b->priority) {
        return 1;
    }

    /* If same priority, sort by ID */
    if (event_a->id < event_b->id) {
        return -1;
    } else if (event_a->id > event_b->id) {
        return 1;
    }

    return 0;
}

uint32_t event_scheduler_check_triggers(EventScheduler* scheduler, GameState* state) {
    if (!scheduler || !state) {
        return 0;
    }

    uint32_t triggered_count = 0;
    ScheduledEvent* triggered_events[MAX_EVENTS];
    size_t triggered_events_count = 0;

    /* Check all events for trigger conditions */
    for (size_t i = 0; i < scheduler->event_count; i++) {
        ScheduledEvent* event = &scheduler->events[i];

        if (event_check_conditions(event, state, scheduler)) {
            triggered_events[triggered_events_count++] = event;
        }
    }

    /* Sort triggered events by priority */
    if (triggered_events_count > 0) {
        qsort(triggered_events, triggered_events_count, sizeof(ScheduledEvent*),
              event_compare_priority);

        /* Execute triggered events in priority order */
        for (size_t i = 0; i < triggered_events_count; i++) {
            ScheduledEvent* event = triggered_events[i];

            LOG_INFO("Triggering event: %s (Day %u)",
                      event->name, state->resources.day_count);

            /* Mark as triggered */
            event->triggered = true;

            /* Execute callback if present */
            if (event->callback) {
                bool success = event->callback(state, event->id);
                event->completed = success;

                if (success) {
                    LOG_DEBUG("Event %s completed successfully", event->name);
                    triggered_count++;
                } else {
                    LOG_WARN("Event %s callback failed", event->name);
                }
            } else {
                /* No callback, just mark as completed */
                event->completed = true;
                triggered_count++;
            }
        }
    }

    /* Update last check values */
    scheduler->last_check_day = state->resources.day_count;
    scheduler->last_check_corruption = state->corruption.corruption;
    scheduler->last_check_location = state->current_location_id;

    return triggered_count;
}

bool event_scheduler_was_triggered(const EventScheduler* scheduler, uint32_t event_id) {
    if (!scheduler) {
        return false;
    }

    for (size_t i = 0; i < scheduler->event_count; i++) {
        if (scheduler->events[i].id == event_id) {
            return scheduler->events[i].triggered;
        }
    }

    return false;
}

bool event_scheduler_was_completed(const EventScheduler* scheduler, uint32_t event_id) {
    if (!scheduler) {
        return false;
    }

    for (size_t i = 0; i < scheduler->event_count; i++) {
        if (scheduler->events[i].id == event_id) {
            return scheduler->events[i].completed;
        }
    }

    return false;
}

const ScheduledEvent* event_scheduler_get_event(const EventScheduler* scheduler, uint32_t event_id) {
    if (!scheduler) {
        return NULL;
    }

    for (size_t i = 0; i < scheduler->event_count; i++) {
        if (scheduler->events[i].id == event_id) {
            return &scheduler->events[i];
        }
    }

    return NULL;
}

const ScheduledEvent** event_scheduler_get_upcoming(const EventScheduler* scheduler, size_t* count_out) {
    if (!scheduler || !count_out) {
        return NULL;
    }

    static const ScheduledEvent* upcoming[MAX_EVENTS];
    size_t count = 0;

    for (size_t i = 0; i < scheduler->event_count; i++) {
        if (!scheduler->events[i].triggered) {
            upcoming[count++] = &scheduler->events[i];
        }
    }

    *count_out = count;
    return upcoming;
}

uint32_t event_scheduler_load_from_file(EventScheduler* scheduler, const char* filepath) {
    if (!scheduler || !filepath) {
        return 0;
    }

    /* TODO: Implement data file loading */
    /* For now, return 0 - will be implemented with data loader */
    LOG_WARN("Event loading from file not yet implemented: %s", filepath);
    return 0;
}

bool event_scheduler_force_trigger(EventScheduler* scheduler, uint32_t event_id, GameState* state) {
    if (!scheduler || !state) {
        return false;
    }

    for (size_t i = 0; i < scheduler->event_count; i++) {
        ScheduledEvent* event = &scheduler->events[i];

        if (event->id == event_id) {
            LOG_INFO("Forcing event trigger: %s", event->name);

            event->triggered = true;

            if (event->callback) {
                bool success = event->callback(state, event->id);
                event->completed = success;
                return success;
            } else {
                event->completed = true;
                return true;
            }
        }
    }

    LOG_WARN("Event ID %u not found for forced trigger", event_id);
    return false;
}

bool event_scheduler_reset_event(EventScheduler* scheduler, uint32_t event_id) {
    if (!scheduler) {
        return false;
    }

    for (size_t i = 0; i < scheduler->event_count; i++) {
        ScheduledEvent* event = &scheduler->events[i];

        if (event->id == event_id) {
            if (!event->repeatable) {
                LOG_WARN("Cannot reset non-repeatable event: %s", event->name);
                return false;
            }

            event->triggered = false;
            event->completed = false;
            LOG_DEBUG("Event reset: %s", event->name);
            return true;
        }
    }

    return false;
}

bool event_scheduler_set_flag(EventScheduler* scheduler, const char* flag_name) {
    if (!scheduler || !flag_name) {
        return false;
    }

    /* Check if flag already exists */
    for (size_t i = 0; i < scheduler->flag_count; i++) {
        if (strcmp(scheduler->flags[i].name, flag_name) == 0) {
            scheduler->flags[i].set = true;
            LOG_DEBUG("Flag set: %s", flag_name);
            return true;
        }
    }

    /* Add new flag */
    if (scheduler->flag_count >= MAX_FLAGS) {
        LOG_ERROR("Flag list is full");
        return false;
    }

    snprintf(scheduler->flags[scheduler->flag_count].name, sizeof(scheduler->flags[scheduler->flag_count].name), "%s", flag_name);
    scheduler->flags[scheduler->flag_count].set = true;
    scheduler->flag_count++;

    LOG_DEBUG("Flag created and set: %s", flag_name);
    return true;
}

bool event_scheduler_has_flag(const EventScheduler* scheduler, const char* flag_name) {
    if (!scheduler || !flag_name) {
        return false;
    }

    for (size_t i = 0; i < scheduler->flag_count; i++) {
        if (strcmp(scheduler->flags[i].name, flag_name) == 0) {
            return scheduler->flags[i].set;
        }
    }

    return false;
}

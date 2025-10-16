/**
 * @file ashbrook_event.c
 * @brief Ashbrook Village event implementation
 */

#include "ashbrook_event.h"
#include "event_scheduler.h"
#include "../game_state.h"
#include "../souls/soul.h"
#include "../souls/soul_manager.h"
#include "../resources/corruption.h"
#include "../resources/resources.h"
#include "../../utils/logger.h"
#include <stdlib.h>
#include <string.h>

#define ASHBROOK_EVENT_ID 47
#define ASHBROOK_POPULATION 147
#define ASHBROOK_BASE_ENERGY 2800  /* Approximate energy from 147 souls */
#define ASHBROOK_CORRUPTION_GAIN 13
#define ASHBROOK_CORRUPTION_LOSS 2

/* Global Ashbrook event state */
static AshbrookEvent g_ashbrook = {
    .state = ASHBROOK_NOT_TRIGGERED,
    .trigger_day = 47,
    .event_registered = false,
    .warned = false,
    .souls_gained = 0,
    .energy_gained = 0
};

bool ashbrook_event_callback(GameState* state, uint32_t event_id) {
    if (!state) {
        return false;
    }

    (void)event_id; /* Unused */

    LOG_INFO("=== ASHBROOK EVENT TRIGGERED (Day %u) ===", state->resources.day_count);
    LOG_INFO("The village of Ashbrook has been discovered.");
    LOG_INFO("Population: %u living souls", ASHBROOK_POPULATION);
    LOG_INFO("Potential soul energy gain: ~%u", ASHBROOK_BASE_ENERGY);
    LOG_INFO("WARNING: Mass harvest will increase corruption by %u%%", ASHBROOK_CORRUPTION_GAIN);

    g_ashbrook.warned = true;

    /* Event has been triggered, but choice hasn't been made yet */
    /* Player must explicitly call ashbrook_harvest_village() or ashbrook_spare_village() */

    return true;
}

bool ashbrook_register_event(EventScheduler* scheduler, GameState* state) {
    if (!scheduler || !state) {
        return false;
    }

    if (g_ashbrook.event_registered) {
        LOG_WARN("Ashbrook event already registered");
        return false;
    }

    ScheduledEvent event = {
        .id = ASHBROOK_EVENT_ID,
        .trigger_type = EVENT_TRIGGER_DAY,
        .trigger_value = 47,
        .triggered = false,
        .completed = false,
        .repeatable = false,
        .priority = EVENT_PRIORITY_CRITICAL,
        .callback = ashbrook_event_callback,
        .requires_flag = false,
        .min_day = 0,
        .max_day = 0
    };

    strncpy(event.name, "Ashbrook Discovery", sizeof(event.name) - 1);
    strncpy(event.description, "The village of Ashbrook presents a terrible choice",
            sizeof(event.description) - 1);

    bool success = event_scheduler_register(scheduler, event);
    if (success) {
        g_ashbrook.event_registered = true;
        LOG_INFO("Ashbrook event registered for Day 47");
    }

    return success;
}

bool ashbrook_harvest_village(GameState* state) {
    if (!state) {
        return false;
    }

    if (g_ashbrook.state != ASHBROOK_NOT_TRIGGERED && g_ashbrook.state != ASHBROOK_IGNORED) {
        LOG_WARN("Ashbrook has already been resolved");
        return false;
    }

    if (!g_ashbrook.warned) {
        LOG_WARN("Ashbrook event has not been triggered yet");
        return false;
    }

    LOG_INFO("=== ASHBROOK HARVEST INITIATED ===");

    /* Generate souls based on village population */
    uint32_t total_energy = 0;
    uint32_t souls_created = 0;

    /* Village breakdown (approximate distribution):
     * - 120 Common souls (farmers, craftspeople)
     * - 20 Warrior souls (guards, retired soldiers)
     * - 5 Mage souls (village elder, herbalist, etc.)
     * - 2 Innocent souls (children, clergy)
     */

    /* Common souls (quality 40-70) */
    for (uint32_t i = 0; i < 120; i++) {
        Soul* soul = soul_create(SOUL_TYPE_COMMON, 40 + (i % 31));
        if (soul && soul_manager_add(state->souls, soul)) {
            total_energy += soul->energy;
            souls_created++;
        }
    }

    /* Warrior souls (quality 70-90) */
    for (uint32_t i = 0; i < 20; i++) {
        Soul* soul = soul_create(SOUL_TYPE_WARRIOR, 70 + (i % 21));
        if (soul && soul_manager_add(state->souls, soul)) {
            total_energy += soul->energy;
            souls_created++;
        }
    }

    /* Mage souls (quality 75-90) */
    for (uint32_t i = 0; i < 5; i++) {
        Soul* soul = soul_create(SOUL_TYPE_MAGE, 75 + (i * 3));
        if (soul && soul_manager_add(state->souls, soul)) {
            total_energy += soul->energy;
            souls_created++;
        }
    }

    /* Innocent souls (quality 90-95) - the child and village elder */
    for (uint32_t i = 0; i < 2; i++) {
        Soul* soul = soul_create(SOUL_TYPE_INNOCENT, 92 + (i * 3));
        if (soul && soul_manager_add(state->souls, soul)) {
            total_energy += soul->energy;
            souls_created++;
        }
    }

    /* Add soul energy */
    resources_add_soul_energy(&state->resources, total_energy);

    /* Increase corruption significantly */
    corruption_add(&state->corruption, ASHBROOK_CORRUPTION_GAIN,
                  "Mass harvest of Ashbrook Village (147 souls)", state->resources.day_count);

    /* Update Ashbrook state */
    g_ashbrook.state = ASHBROOK_HARVESTED;
    g_ashbrook.souls_gained = souls_created;
    g_ashbrook.energy_gained = total_energy;

    LOG_INFO("=== ASHBROOK HARVEST COMPLETE ===");
    LOG_INFO("Souls harvested: %u", souls_created);
    LOG_INFO("Soul energy gained: %u", total_energy);
    LOG_INFO("Corruption increased to %u%%", state->corruption.corruption);
    LOG_INFO("ACHIEVEMENT UNLOCKED: Mass Harvest");

    /* Set event flag for story tracking */
    if (state->event_scheduler) {
        event_scheduler_set_flag(state->event_scheduler, "ashbrook_harvested");
    }

    return true;
}

bool ashbrook_spare_village(GameState* state) {
    if (!state) {
        return false;
    }

    if (g_ashbrook.state != ASHBROOK_NOT_TRIGGERED && g_ashbrook.state != ASHBROOK_IGNORED) {
        LOG_WARN("Ashbrook has already been resolved");
        return false;
    }

    if (!g_ashbrook.warned) {
        LOG_WARN("Ashbrook event has not been triggered yet");
        return false;
    }

    LOG_INFO("=== ASHBROOK SPARED ===");
    LOG_INFO("You chose mercy over power.");

    /* Slightly reduce corruption for showing restraint */
    if (state->corruption.corruption >= ASHBROOK_CORRUPTION_LOSS) {
        /* Reduce corruption by subtracting current and adding back less */
        uint8_t current = state->corruption.corruption;
        state->corruption.corruption = (current >= ASHBROOK_CORRUPTION_LOSS) ?
                                       (current - ASHBROOK_CORRUPTION_LOSS) : 0;
    }

    /* Update Ashbrook state */
    g_ashbrook.state = ASHBROOK_SPARED;
    g_ashbrook.souls_gained = 0;
    g_ashbrook.energy_gained = 0;

    LOG_INFO("Corruption reduced to %u%%", state->corruption.corruption);
    LOG_INFO("The village of Ashbrook remains safe.");

    /* Set event flag for story tracking */
    if (state->event_scheduler) {
        event_scheduler_set_flag(state->event_scheduler, "ashbrook_spared");
    }

    return true;
}

AshbrookState ashbrook_get_state(const GameState* state) {
    (void)state; /* State not currently used, but kept for API consistency */
    return g_ashbrook.state;
}

bool ashbrook_was_harvested(const GameState* state) {
    (void)state;
    return g_ashbrook.state == ASHBROOK_HARVESTED;
}

bool ashbrook_was_spared(const GameState* state) {
    (void)state;
    return g_ashbrook.state == ASHBROOK_SPARED;
}

bool ashbrook_get_statistics(const GameState* state, uint32_t* souls_gained_out,
                             uint32_t* energy_gained_out) {
    if (!state || !souls_gained_out || !energy_gained_out) {
        return false;
    }

    if (g_ashbrook.state == ASHBROOK_NOT_TRIGGERED || g_ashbrook.state == ASHBROOK_IGNORED) {
        return false;
    }

    *souls_gained_out = g_ashbrook.souls_gained;
    *energy_gained_out = g_ashbrook.energy_gained;

    return true;
}

void ashbrook_reset_for_testing(void) {
    g_ashbrook.state = ASHBROOK_NOT_TRIGGERED;
    g_ashbrook.trigger_day = 47;
    g_ashbrook.event_registered = false;
    g_ashbrook.warned = false;
    g_ashbrook.souls_gained = 0;
    g_ashbrook.energy_gained = 0;
}

#include "core/events.h"
#include "utils/logger.h"
#include <stdlib.h>
#include <string.h>

/* Maximum queued events */
#define MAX_EVENT_QUEUE 1024

/* Subscription entry */
typedef struct Subscription {
    size_t id;
    EventType type;
    EventCallback callback;
    void* userdata;
    bool active;
    struct Subscription* next;
} Subscription;

/* Queued event entry */
typedef struct QueuedEvent {
    Event event;
    bool data_owned;  /* true if we need to free event.data */
} QueuedEvent;

/* Event bus structure */
struct EventBus {
    Subscription* subscriptions[EVENT_COUNT];  /* Subscription lists per event type */
    QueuedEvent* event_queue;
    size_t queue_size;
    size_t queue_capacity;
    size_t next_subscription_id;
    size_t total_subscriptions;
};

/* Event type names for debugging */
static const char* g_event_names[] = {
    [EVENT_NONE] = "NONE",
    [EVENT_GAME_START] = "GAME_START",
    [EVENT_GAME_PAUSE] = "GAME_PAUSE",
    [EVENT_GAME_RESUME] = "GAME_RESUME",
    [EVENT_GAME_QUIT] = "GAME_QUIT",
    [EVENT_PLAYER_MOVE] = "PLAYER_MOVE",
    [EVENT_PLAYER_ATTACK] = "PLAYER_ATTACK",
    [EVENT_PLAYER_DAMAGE_TAKEN] = "PLAYER_DAMAGE_TAKEN",
    [EVENT_PLAYER_DIED] = "PLAYER_DIED",
    [EVENT_PLAYER_LEVEL_UP] = "PLAYER_LEVEL_UP",
    [EVENT_COMBAT_START] = "COMBAT_START",
    [EVENT_COMBAT_END] = "COMBAT_END",
    [EVENT_ENEMY_SPAWN] = "ENEMY_SPAWN",
    [EVENT_ENEMY_DIED] = "ENEMY_DIED",
    [EVENT_UI_OPEN_MENU] = "UI_OPEN_MENU",
    [EVENT_UI_CLOSE_MENU] = "UI_CLOSE_MENU",
    [EVENT_UI_BUTTON_CLICKED] = "UI_BUTTON_CLICKED",
    [EVENT_UI_TEXT_INPUT] = "UI_TEXT_INPUT",
    [EVENT_RESOURCE_LOADED] = "RESOURCE_LOADED",
    [EVENT_SAVE_GAME] = "SAVE_GAME",
    [EVENT_LOAD_GAME] = "LOAD_GAME"
};

const char* event_type_name(EventType type) {
    if (type >= 0 && type < EVENT_COUNT && type < sizeof(g_event_names) / sizeof(g_event_names[0])) {
        const char* name = g_event_names[type];
        if (name) return name;
    }
    if (type >= EVENT_CUSTOM_START && type <= EVENT_CUSTOM_END) {
        return "CUSTOM_EVENT";
    }
    return "UNKNOWN";
}

EventBus* event_bus_create(void) {
    EventBus* bus = calloc(1, sizeof(EventBus));
    if (!bus) {
        LOG_ERROR("Failed to allocate event bus");
        return NULL;
    }

    /* Initialize event queue */
    bus->queue_capacity = 128;  /* Start with reasonable capacity */
    bus->event_queue = calloc(bus->queue_capacity, sizeof(QueuedEvent));
    if (!bus->event_queue) {
        LOG_ERROR("Failed to allocate event queue");
        free(bus);
        return NULL;
    }

    bus->queue_size = 0;
    bus->next_subscription_id = 1;
    bus->total_subscriptions = 0;

    LOG_DEBUG("Created event bus");
    return bus;
}

void event_bus_destroy(EventBus* bus) {
    if (!bus) return;

    /* Free all subscriptions */
    for (size_t i = 0; i < EVENT_COUNT; i++) {
        Subscription* sub = bus->subscriptions[i];
        while (sub) {
            Subscription* next = sub->next;
            free(sub);
            sub = next;
        }
    }

    /* Free queued event data */
    for (size_t i = 0; i < bus->queue_size; i++) {
        if (bus->event_queue[i].data_owned) {
            free(bus->event_queue[i].event.data);
        }
    }

    free(bus->event_queue);
    free(bus);

    LOG_DEBUG("Destroyed event bus");
}

size_t event_bus_subscribe(EventBus* bus, EventType type,
                           EventCallback callback, void* userdata) {
    if (!bus || type <= EVENT_NONE || type >= EVENT_COUNT || !callback) {
        LOG_ERROR("Invalid event bus, type, or callback");
        return 0;
    }

    /* Create new subscription */
    Subscription* sub = malloc(sizeof(Subscription));
    if (!sub) {
        LOG_ERROR("Failed to allocate subscription");
        return 0;
    }

    sub->id = bus->next_subscription_id++;
    sub->type = type;
    sub->callback = callback;
    sub->userdata = userdata;
    sub->active = true;

    /* Add to list (prepend) */
    sub->next = bus->subscriptions[type];
    bus->subscriptions[type] = sub;
    bus->total_subscriptions++;

    LOG_DEBUG("Subscribed to %s (ID: %zu)", event_type_name(type), sub->id);
    return sub->id;
}

bool event_bus_unsubscribe(EventBus* bus, size_t subscription_id) {
    if (!bus || subscription_id == 0) return false;

    /* Search all event types for this subscription */
    for (size_t i = 0; i < EVENT_COUNT; i++) {
        Subscription** ptr = &bus->subscriptions[i];
        while (*ptr) {
            if ((*ptr)->id == subscription_id) {
                Subscription* sub = *ptr;
                *ptr = sub->next;
                free(sub);
                bus->total_subscriptions--;
                LOG_DEBUG("Unsubscribed ID: %zu", subscription_id);
                return true;
            }
            ptr = &(*ptr)->next;
        }
    }

    LOG_WARN("Subscription ID %zu not found", subscription_id);
    return false;
}

void event_bus_unsubscribe_all(EventBus* bus, EventType type) {
    if (!bus || type <= EVENT_NONE || type >= EVENT_COUNT) return;

    size_t count = 0;
    Subscription* sub = bus->subscriptions[type];
    while (sub) {
        Subscription* next = sub->next;
        free(sub);
        count++;
        sub = next;
    }

    bus->subscriptions[type] = NULL;
    bus->total_subscriptions -= count;

    LOG_DEBUG("Unsubscribed all (%zu) from %s", count, event_type_name(type));
}

bool event_bus_publish(EventBus* bus, EventType type, void* data) {
    if (!bus || type <= EVENT_NONE || type >= EVENT_COUNT) return false;

    Event event = {
        .type = type,
        .data = data,
        .data_size = 0
    };

    /* Call all subscribers */
    size_t count = 0;
    Subscription* sub = bus->subscriptions[type];
    while (sub) {
        if (sub->active && sub->callback) {
            sub->callback(&event, sub->userdata);
            count++;
        }
        sub = sub->next;
    }

    LOG_DEBUG("Published %s to %zu subscribers", event_type_name(type), count);
    return true;
}

static bool event_queue_grow(EventBus* bus) {
    size_t new_capacity = bus->queue_capacity * 2;
    if (new_capacity > MAX_EVENT_QUEUE) {
        new_capacity = MAX_EVENT_QUEUE;
    }

    if (new_capacity <= bus->queue_capacity) {
        LOG_ERROR("Event queue at maximum capacity");
        return false;
    }

    QueuedEvent* new_queue = realloc(bus->event_queue, new_capacity * sizeof(QueuedEvent));
    if (!new_queue) {
        LOG_ERROR("Failed to grow event queue");
        return false;
    }

    bus->event_queue = new_queue;
    bus->queue_capacity = new_capacity;

    LOG_DEBUG("Grew event queue to %zu", new_capacity);
    return true;
}

bool event_bus_queue(EventBus* bus, EventType type, const void* data, size_t data_size) {
    if (!bus || type <= EVENT_NONE || type >= EVENT_COUNT) return false;

    /* Grow queue if needed */
    if (bus->queue_size >= bus->queue_capacity) {
        if (!event_queue_grow(bus)) {
            return false;
        }
    }

    QueuedEvent* qe = &bus->event_queue[bus->queue_size];
    qe->event.type = type;
    qe->event.data_size = data_size;
    qe->data_owned = false;

    /* Copy data if provided */
    if (data && data_size > 0) {
        qe->event.data = malloc(data_size);
        if (!qe->event.data) {
            LOG_ERROR("Failed to allocate event data");
            return false;
        }
        memcpy(qe->event.data, data, data_size);
        qe->data_owned = true;
    } else {
        qe->event.data = NULL;
    }

    bus->queue_size++;

    LOG_DEBUG("Queued %s (queue size: %zu)", event_type_name(type), bus->queue_size);
    return true;
}

void event_bus_dispatch(EventBus* bus) {
    if (!bus || bus->queue_size == 0) return;

    LOG_DEBUG("Dispatching %zu queued events", bus->queue_size);

    /* Process all queued events */
    for (size_t i = 0; i < bus->queue_size; i++) {
        QueuedEvent* qe = &bus->event_queue[i];
        Event* event = &qe->event;

        /* Call all subscribers */
        Subscription* sub = bus->subscriptions[event->type];
        while (sub) {
            if (sub->active && sub->callback) {
                sub->callback(event, sub->userdata);
            }
            sub = sub->next;
        }

        /* Free owned data */
        if (qe->data_owned) {
            free(event->data);
        }
    }

    /* Clear queue */
    bus->queue_size = 0;
}

void event_bus_clear_queue(EventBus* bus) {
    if (!bus) return;

    /* Free owned data */
    for (size_t i = 0; i < bus->queue_size; i++) {
        if (bus->event_queue[i].data_owned) {
            free(bus->event_queue[i].event.data);
        }
    }

    bus->queue_size = 0;
    LOG_DEBUG("Cleared event queue");
}

size_t event_bus_queue_size(const EventBus* bus) {
    return bus ? bus->queue_size : 0;
}

size_t event_bus_subscriber_count(const EventBus* bus, EventType type) {
    if (!bus || type <= EVENT_NONE || type >= EVENT_COUNT) return 0;

    size_t count = 0;
    Subscription* sub = bus->subscriptions[type];
    while (sub) {
        if (sub->active) count++;
        sub = sub->next;
    }

    return count;
}

size_t event_bus_total_subscriptions(const EventBus* bus) {
    return bus ? bus->total_subscriptions : 0;
}

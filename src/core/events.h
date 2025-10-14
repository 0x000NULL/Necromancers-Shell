#ifndef EVENTS_H
#define EVENTS_H

#include <stddef.h>
#include <stdbool.h>

/**
 * Event Bus - Pub/Sub Event System
 *
 * Decoupled event-driven architecture for game systems.
 * Supports multiple subscribers per event type and event queuing.
 *
 * Usage:
 *   EventBus* bus = event_bus_create();
 *   event_bus_subscribe(bus, EVENT_DAMAGE_TAKEN, on_damage, userdata);
 *   event_bus_publish(bus, EVENT_DAMAGE_TAKEN, &damage_data);
 *   event_bus_dispatch(bus);  // Process queued events
 *   event_bus_destroy(bus);
 */

/* Event types */
typedef enum {
    EVENT_NONE = 0,

    /* Game events */
    EVENT_GAME_START,
    EVENT_GAME_PAUSE,
    EVENT_GAME_RESUME,
    EVENT_GAME_QUIT,

    /* Player events */
    EVENT_PLAYER_MOVE,
    EVENT_PLAYER_ATTACK,
    EVENT_PLAYER_DAMAGE_TAKEN,
    EVENT_PLAYER_DIED,
    EVENT_PLAYER_LEVEL_UP,

    /* Combat events */
    EVENT_COMBAT_START,
    EVENT_COMBAT_END,
    EVENT_ENEMY_SPAWN,
    EVENT_ENEMY_DIED,

    /* UI events */
    EVENT_UI_OPEN_MENU,
    EVENT_UI_CLOSE_MENU,
    EVENT_UI_BUTTON_CLICKED,
    EVENT_UI_TEXT_INPUT,

    /* System events */
    EVENT_RESOURCE_LOADED,
    EVENT_SAVE_GAME,
    EVENT_LOAD_GAME,

    /* Custom event range */
    EVENT_CUSTOM_START = 1000,
    EVENT_CUSTOM_END = 9999,

    EVENT_COUNT = 10000
} EventType;

/* Forward declarations */
typedef struct EventBus EventBus;
typedef struct Event Event;

/* Event data */
struct Event {
    EventType type;
    void* data;         /* Event-specific data */
    size_t data_size;   /* Size of data (for copying) */
};

/* Event callback function */
typedef void (*EventCallback)(const Event* event, void* userdata);

/**
 * Create event bus
 *
 * @return Event bus pointer or NULL on failure
 */
EventBus* event_bus_create(void);

/**
 * Destroy event bus
 * Cleans up all subscriptions and queued events
 *
 * @param bus Event bus
 */
void event_bus_destroy(EventBus* bus);

/**
 * Subscribe to an event type
 * Multiple subscribers can listen to the same event
 *
 * @param bus Event bus
 * @param type Event type to listen for
 * @param callback Callback function
 * @param userdata User data passed to callback
 * @return Subscription ID or 0 on failure
 */
size_t event_bus_subscribe(EventBus* bus, EventType type,
                           EventCallback callback, void* userdata);

/**
 * Unsubscribe from events
 *
 * @param bus Event bus
 * @param subscription_id Subscription ID from event_bus_subscribe
 * @return true on success
 */
bool event_bus_unsubscribe(EventBus* bus, size_t subscription_id);

/**
 * Unsubscribe all callbacks for a specific event type
 *
 * @param bus Event bus
 * @param type Event type
 */
void event_bus_unsubscribe_all(EventBus* bus, EventType type);

/**
 * Publish an event immediately (synchronous)
 * All subscribers are called immediately
 *
 * @param bus Event bus
 * @param type Event type
 * @param data Event data (can be NULL)
 * @return true on success
 */
bool event_bus_publish(EventBus* bus, EventType type, void* data);

/**
 * Queue an event for later dispatch (asynchronous)
 * Event data is copied and will be freed after dispatch
 *
 * @param bus Event bus
 * @param type Event type
 * @param data Event data (will be copied, can be NULL)
 * @param data_size Size of data to copy
 * @return true on success
 */
bool event_bus_queue(EventBus* bus, EventType type, const void* data, size_t data_size);

/**
 * Dispatch all queued events
 * Processes and clears the event queue
 *
 * @param bus Event bus
 */
void event_bus_dispatch(EventBus* bus);

/**
 * Clear all queued events without dispatching
 *
 * @param bus Event bus
 */
void event_bus_clear_queue(EventBus* bus);

/**
 * Get number of queued events
 *
 * @param bus Event bus
 * @return Queue size
 */
size_t event_bus_queue_size(const EventBus* bus);

/**
 * Get number of subscribers for an event type
 *
 * @param bus Event bus
 * @param type Event type
 * @return Subscriber count
 */
size_t event_bus_subscriber_count(const EventBus* bus, EventType type);

/**
 * Get total number of active subscriptions
 *
 * @param bus Event bus
 * @return Total subscription count
 */
size_t event_bus_total_subscriptions(const EventBus* bus);

/**
 * Get event type name (for debugging)
 *
 * @param type Event type
 * @return Event name string
 */
const char* event_type_name(EventType type);

/* Helper macros for creating events with data */

/**
 * Publish event with data
 */
#define EVENT_PUBLISH_DATA(bus, type, data_ptr) \
    event_bus_publish((bus), (type), (data_ptr))

/**
 * Queue event with data (copies data)
 */
#define EVENT_QUEUE_DATA(bus, type, data_ptr) \
    event_bus_queue((bus), (type), (data_ptr), sizeof(*(data_ptr)))

/**
 * Publish event without data
 */
#define EVENT_PUBLISH_SIMPLE(bus, type) \
    event_bus_publish((bus), (type), NULL)

/**
 * Queue event without data
 */
#define EVENT_QUEUE_SIMPLE(bus, type) \
    event_bus_queue((bus), (type), NULL, 0)

#endif /* EVENTS_H */

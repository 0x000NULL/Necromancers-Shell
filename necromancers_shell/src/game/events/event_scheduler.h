/**
 * @file event_scheduler.h
 * @brief Event scheduling system for story beats
 *
 * Manages time-based, condition-based, and location-based story events.
 * Triggers events on specific days (Day 7, 47, 162), corruption thresholds,
 * location changes, and quest completions.
 */

#ifndef NECROMANCER_EVENT_SCHEDULER_H
#define NECROMANCER_EVENT_SCHEDULER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/*
 * Forward declarations
 * GameState is fully defined in game_state.h - we just need the name here for pointers
 */
typedef struct GameState GameState;

/**
 * @brief Event trigger types
 */
typedef enum {
    EVENT_TRIGGER_DAY,          /**< Triggers on specific day */
    EVENT_TRIGGER_CORRUPTION,   /**< Triggers at corruption threshold */
    EVENT_TRIGGER_LOCATION,     /**< Triggers at specific location */
    EVENT_TRIGGER_QUEST,        /**< Triggers on quest completion */
    EVENT_TRIGGER_FLAG          /**< Triggers when game flag is set */
} EventTriggerType;

/**
 * @brief Event priority levels
 */
typedef enum {
    EVENT_PRIORITY_LOW = 0,
    EVENT_PRIORITY_NORMAL = 1,
    EVENT_PRIORITY_HIGH = 2,
    EVENT_PRIORITY_CRITICAL = 3
} EventPriority;

/**
 * @brief Event callback function type
 *
 * @param state Game state
 * @param event_id Event identifier
 * @return true if event executed successfully, false otherwise
 */
typedef bool (*EventCallback)(GameState* state, uint32_t event_id);

/**
 * @brief Scheduled event structure
 */
typedef struct {
    uint32_t id;                    /**< Unique event ID */
    char name[64];                  /**< Event name */
    char description[256];          /**< Event description */

    EventTriggerType trigger_type;  /**< How this event triggers */
    uint32_t trigger_value;         /**< Day number, corruption %, location ID, etc. */

    bool triggered;                 /**< Has event been triggered? */
    bool completed;                 /**< Has event completed successfully? */
    bool repeatable;                /**< Can trigger multiple times? */

    EventPriority priority;         /**< Event priority (for sorting) */

    EventCallback callback;         /**< Function to call when triggered */

    /* Optional conditions */
    bool requires_flag;             /**< Requires a flag to be set? */
    char required_flag[64];         /**< Name of required flag */
    uint32_t min_day;               /**< Minimum day (0 = no minimum) */
    uint32_t max_day;               /**< Maximum day (0 = no maximum) */
} ScheduledEvent;

/**
 * @brief Event scheduler structure
 */
typedef struct EventScheduler EventScheduler;

/**
 * @brief Create event scheduler
 *
 * @return Newly allocated EventScheduler or NULL on failure
 */
EventScheduler* event_scheduler_create(void);

/**
 * @brief Destroy event scheduler and free memory
 *
 * @param scheduler Scheduler to destroy (can be NULL)
 */
void event_scheduler_destroy(EventScheduler* scheduler);

/**
 * @brief Register an event with the scheduler
 *
 * @param scheduler Event scheduler
 * @param event Event to register
 * @return true on success, false if scheduler is NULL or event list is full
 */
bool event_scheduler_register(EventScheduler* scheduler, ScheduledEvent event);

/**
 * @brief Check for triggered events and execute them
 *
 * Called from game_state_advance_time() and other state change functions.
 * Executes all triggered events in priority order.
 *
 * @param scheduler Event scheduler
 * @param state Game state
 * @return Number of events triggered
 */
uint32_t event_scheduler_check_triggers(EventScheduler* scheduler, GameState* state);

/**
 * @brief Check if specific event has been triggered
 *
 * @param scheduler Event scheduler
 * @param event_id Event ID to check
 * @return true if triggered, false otherwise
 */
bool event_scheduler_was_triggered(const EventScheduler* scheduler, uint32_t event_id);

/**
 * @brief Check if specific event has completed
 *
 * @param scheduler Event scheduler
 * @param event_id Event ID to check
 * @return true if completed, false otherwise
 */
bool event_scheduler_was_completed(const EventScheduler* scheduler, uint32_t event_id);

/**
 * @brief Get event by ID
 *
 * @param scheduler Event scheduler
 * @param event_id Event ID
 * @return Pointer to event or NULL if not found
 */
const ScheduledEvent* event_scheduler_get_event(const EventScheduler* scheduler, uint32_t event_id);

/**
 * @brief Get all upcoming events
 *
 * Returns events that haven't triggered yet.
 *
 * @param scheduler Event scheduler
 * @param count_out Output: number of upcoming events
 * @return Array of upcoming events (do not free)
 */
const ScheduledEvent** event_scheduler_get_upcoming(const EventScheduler* scheduler, size_t* count_out);

/**
 * @brief Load events from data file
 *
 * @param scheduler Event scheduler
 * @param filepath Path to events.dat file
 * @return Number of events loaded, or 0 on error
 */
uint32_t event_scheduler_load_from_file(EventScheduler* scheduler, const char* filepath);

/**
 * @brief Manually trigger an event (for debugging/testing)
 *
 * @param scheduler Event scheduler
 * @param event_id Event ID to trigger
 * @param state Game state
 * @return true if triggered successfully, false otherwise
 */
bool event_scheduler_force_trigger(EventScheduler* scheduler, uint32_t event_id, GameState* state);

/**
 * @brief Reset an event to untriggered state (for repeatable events)
 *
 * @param scheduler Event scheduler
 * @param event_id Event ID to reset
 * @return true on success, false if event not found or not repeatable
 */
bool event_scheduler_reset_event(EventScheduler* scheduler, uint32_t event_id);

/**
 * @brief Set a game flag (for EVENT_TRIGGER_FLAG)
 *
 * @param scheduler Event scheduler
 * @param flag_name Flag name
 * @return true on success
 */
bool event_scheduler_set_flag(EventScheduler* scheduler, const char* flag_name);

/**
 * @brief Check if a game flag is set
 *
 * @param scheduler Event scheduler
 * @param flag_name Flag name
 * @return true if flag is set, false otherwise
 */
bool event_scheduler_has_flag(const EventScheduler* scheduler, const char* flag_name);

#endif /* NECROMANCER_EVENT_SCHEDULER_H */

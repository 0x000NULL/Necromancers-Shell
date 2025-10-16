/**
 * @file ashbrook_event.h
 * @brief Ashbrook Village mass harvest event (Day 47)
 *
 * Major story beat where player faces moral choice:
 * - Harvest 147 living villagers for massive soul energy gain
 * - Spare the village and maintain lower corruption
 *
 * This event significantly impacts corruption and unlocks new content.
 */

#ifndef NECROMANCER_ASHBROOK_EVENT_H
#define NECROMANCER_ASHBROOK_EVENT_H

#include <stdint.h>
#include <stdbool.h>

/* Forward declaration */
typedef struct GameState GameState;
typedef struct EventScheduler EventScheduler;

/**
 * @brief Ashbrook event state
 */
typedef enum {
    ASHBROOK_NOT_TRIGGERED,     /**< Event hasn't occurred yet */
    ASHBROOK_HARVESTED,         /**< Village was destroyed */
    ASHBROOK_SPARED,            /**< Village was spared */
    ASHBROOK_IGNORED            /**< Player didn't make a choice */
} AshbrookState;

/**
 * @brief Ashbrook event data
 */
typedef struct {
    AshbrookState state;        /**< Current state of the event */
    uint32_t trigger_day;       /**< Day event was triggered (47) */
    bool event_registered;      /**< Has event been registered with scheduler? */
    bool warned;                /**< Has player been warned about consequences? */
    uint32_t souls_gained;      /**< Souls gained if harvested (147) */
    uint32_t energy_gained;     /**< Energy gained if harvested (~2800) */
} AshbrookEvent;

/**
 * @brief Register Ashbrook event with event scheduler
 *
 * Should be called during game initialization to set up the Day 47 trigger.
 *
 * @param scheduler Event scheduler
 * @param state Game state
 * @return true on success
 */
bool ashbrook_register_event(EventScheduler* scheduler, GameState* state);

/**
 * @brief Event callback for Ashbrook trigger (Day 47)
 *
 * Called automatically by event scheduler on Day 47.
 *
 * @param state Game state
 * @param event_id Event ID
 * @return true on success
 */
bool ashbrook_event_callback(GameState* state, uint32_t event_id);

/**
 * @brief Execute Ashbrook harvest (destroy village)
 *
 * Kills all 147 villagers, harvests their souls, grants massive energy.
 * Increases corruption significantly (+13%).
 *
 * @param state Game state
 * @return true on success
 */
bool ashbrook_harvest_village(GameState* state);

/**
 * @brief Spare Ashbrook village
 *
 * Choose not to harvest the village. Grants small reputation boost.
 * Slightly reduces corruption (-2%).
 *
 * @param state Game state
 * @return true on success
 */
bool ashbrook_spare_village(GameState* state);

/**
 * @brief Get Ashbrook event state
 *
 * @param state Game state
 * @return Current Ashbrook state
 */
AshbrookState ashbrook_get_state(const GameState* state);

/**
 * @brief Check if Ashbrook has been harvested
 *
 * @param state Game state
 * @return true if village was destroyed
 */
bool ashbrook_was_harvested(const GameState* state);

/**
 * @brief Check if Ashbrook was spared
 *
 * @param state Game state
 * @return true if village was spared
 */
bool ashbrook_was_spared(const GameState* state);

/**
 * @brief Get Ashbrook statistics
 *
 * @param state Game state
 * @param souls_gained_out Output: souls gained (if harvested)
 * @param energy_gained_out Output: energy gained (if harvested)
 * @return true if event has occurred
 */
bool ashbrook_get_statistics(const GameState* state, uint32_t* souls_gained_out,
                             uint32_t* energy_gained_out);

/**
 * @brief Reset Ashbrook event state (for testing)
 *
 * Resets the global Ashbrook event state to initial values.
 * Should only be used in unit tests.
 */
void ashbrook_reset_for_testing(void);

#endif /* NECROMANCER_ASHBROOK_EVENT_H */

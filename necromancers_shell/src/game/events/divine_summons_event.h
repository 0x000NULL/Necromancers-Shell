/**
 * @file divine_summons_event.h
 * @brief Divine summons event (Day 155) - Council calls player to judgment
 *
 * Major story beat where the Seven Divine Architects summon the player
 * to stand trial after observing their necromantic activities. This event
 * triggers the Archon trial sequence for players pursuing the Archon path.
 *
 * This event triggers on Day 155 if player has met Thessara and expressed
 * interest in the Archon path (or if corruption is in Archon range 30-60%).
 */

#ifndef NECROMANCER_DIVINE_SUMMONS_EVENT_H
#define NECROMANCER_DIVINE_SUMMONS_EVENT_H

#include <stdint.h>
#include <stdbool.h>

/* Forward declarations */
typedef struct GameState GameState;
typedef struct EventScheduler EventScheduler;

/**
 * @brief Divine summons event state
 */
typedef enum {
    SUMMONS_NOT_RECEIVED,        /**< Event hasn't occurred */
    SUMMONS_RECEIVED,            /**< Called before Divine Council */
    SUMMONS_ACKNOWLEDGED,        /**< Player accepted summons */
    SUMMONS_IGNORED              /**< Player ignored summons (path blocked) */
} DivineSummonsState;

/**
 * @brief Divine summons event data
 */
typedef struct {
    DivineSummonsState state;
    uint32_t trigger_day;        /**< Day 155 */
    bool event_registered;
    bool trials_unlocked;        /**< Trial 1 unlocked */
    uint32_t response_deadline;  /**< Day player must respond by (Day 162) */
} DivineSummonsEvent;

/**
 * @brief Register Divine summons event (Day 155)
 *
 * Should be called during game initialization to set up the Day 155 trigger.
 * Requires Thessara paths revelation (flag: "thessara_paths_revealed").
 *
 * @param scheduler Event scheduler
 * @param state Game state
 * @return true on success
 */
bool divine_summons_register_event(EventScheduler* scheduler, GameState* state);

/**
 * @brief Event callback for Divine summons (Day 155)
 *
 * Called automatically by event scheduler on Day 155 (if conditions met).
 * Displays summons from Keldrin (god of judgment) and unlocks trial sequence.
 *
 * @param state Game state
 * @param event_id Event ID
 * @return true on success
 */
bool divine_summons_event_callback(GameState* state, uint32_t event_id);

/**
 * @brief Player acknowledges summons
 *
 * Triggered by player using "invoke divine_council" after summons received.
 * Formally accepts the trial challenge and unlocks Trial 1.
 *
 * @param state Game state
 * @return true on success
 */
bool divine_summons_acknowledge(GameState* state);

/**
 * @brief Check if player ignored summons deadline
 *
 * If player doesn't respond by Day 162 (7 days after summons), the Archon
 * path is blocked and Fourth Purge proceeds without intervention.
 *
 * @param state Game state
 * @return true if deadline passed without acknowledgment
 */
bool divine_summons_is_ignored(const GameState* state);

/**
 * @brief Get Divine summons state
 *
 * @param state Game state
 * @return Current summons state
 */
DivineSummonsState divine_summons_get_state(const GameState* state);

/**
 * @brief Check if summons has been received
 *
 * @param state Game state
 * @return true if event has occurred
 */
bool divine_summons_was_received(const GameState* state);

/**
 * @brief Check if trials have been unlocked
 *
 * @param state Game state
 * @return true if Trial 1 is available
 */
bool divine_summons_trials_unlocked(const GameState* state);

/**
 * @brief Reset for testing
 *
 * Resets the global Divine summons event state to initial values.
 * Should only be used in unit tests.
 */
void divine_summons_reset_for_testing(void);

#endif /* NECROMANCER_DIVINE_SUMMONS_EVENT_H */

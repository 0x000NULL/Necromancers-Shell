/**
 * @file thessara_contact_event.h
 * @brief Thessara contact event (Day 50) - Ghost in the machine reaches out
 *
 * Major story beat where player meets Thessara in null space after Ashbrook.
 * Thessara reveals the six paths and becomes the player's mentor.
 *
 * This event triggers 3 days after Ashbrook resolution (Day 50).
 */

#ifndef NECROMANCER_THESSARA_CONTACT_EVENT_H
#define NECROMANCER_THESSARA_CONTACT_EVENT_H

#include <stdint.h>
#include <stdbool.h>

/* Forward declarations */
typedef struct GameState GameState;
typedef struct EventScheduler EventScheduler;

/**
 * @brief Thessara contact event state
 */
typedef enum {
    THESSARA_NOT_CONTACTED,      /**< Event hasn't occurred */
    THESSARA_CONTACTED,          /**< Met in null space */
    THESSARA_PATHS_REVEALED,     /**< Six paths explained */
    THESSARA_TRUST_ESTABLISHED   /**< Trust system active */
} ThessaraContactState;

/**
 * @brief Thessara contact event data
 */
typedef struct {
    ThessaraContactState state;
    uint32_t trigger_day;        /**< Day 50 */
    bool event_registered;
    bool null_space_discovered;  /**< Player found null space location */
    uint8_t trust_level;         /**< 0-100 trust */
} ThessaraContactEvent;

/**
 * @brief Register Thessara contact event (Day 50)
 *
 * Should be called during game initialization to set up the Day 50 trigger.
 * Requires Ashbrook event to be resolved first (flag: "ashbrook_resolved").
 *
 * @param scheduler Event scheduler
 * @param state Game state
 * @return true on success
 */
bool thessara_contact_register_event(EventScheduler* scheduler, GameState* state);

/**
 * @brief Event callback for Thessara contact (Day 50)
 *
 * Called automatically by event scheduler on Day 50 (if Ashbrook resolved).
 * Displays message from Thessara and unlocks null space location.
 *
 * @param state Game state
 * @param event_id Event ID
 * @return true on success
 */
bool thessara_contact_event_callback(GameState* state, uint32_t event_id);

/**
 * @brief Initiate conversation with Thessara
 *
 * Triggered by player using "dialogue thessara" after event fires.
 * Reveals six paths and establishes mentor relationship.
 *
 * @param state Game state
 * @return true on success
 */
bool thessara_initiate_conversation(GameState* state);

/**
 * @brief Accept Thessara's guidance
 *
 * Player chooses to trust Thessara and accept her help.
 *
 * @param state Game state
 * @return true on success
 */
bool thessara_accept_guidance(GameState* state);

/**
 * @brief Reject Thessara's guidance
 *
 * Player chooses not to trust Thessara (can change mind later).
 *
 * @param state Game state
 * @return true on success
 */
bool thessara_reject_guidance(GameState* state);

/**
 * @brief Get Thessara contact state
 *
 * @param state Game state
 * @return Current Thessara contact state
 */
ThessaraContactState thessara_contact_get_state(const GameState* state);

/**
 * @brief Check if Thessara has been contacted
 *
 * @param state Game state
 * @return true if event has occurred
 */
bool thessara_was_contacted(const GameState* state);

/**
 * @brief Check if paths have been revealed
 *
 * @param state Game state
 * @return true if Thessara revealed the six paths
 */
bool thessara_paths_revealed(const GameState* state);

/**
 * @brief Reset for testing
 *
 * Resets the global Thessara contact event state to initial values.
 * Should only be used in unit tests.
 */
void thessara_contact_reset_for_testing(void);

#endif /* NECROMANCER_THESSARA_CONTACT_EVENT_H */

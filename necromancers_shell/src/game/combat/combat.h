#ifndef COMBAT_H
#define COMBAT_H

#include "combatant.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @file combat.h
 * @brief Combat state machine and management
 *
 * Manages turn-based combat encounters with state machine logic.
 * Handles turn order, combat log, and victory/defeat conditions.
 */

/**
 * @brief Combat phases
 */
typedef enum {
    COMBAT_PHASE_INIT,        /**< Combat initialization */
    COMBAT_PHASE_PLAYER_TURN, /**< Player's turn to issue commands */
    COMBAT_PHASE_ENEMY_TURN,  /**< Enemy AI making decisions */
    COMBAT_PHASE_RESOLUTION,  /**< Apply effects, check win/loss */
    COMBAT_PHASE_END          /**< Combat ending, cleanup */
} CombatPhase;

/**
 * @brief Combat outcome
 */
typedef enum {
    COMBAT_OUTCOME_NONE,      /**< Combat still ongoing */
    COMBAT_OUTCOME_VICTORY,   /**< Player won */
    COMBAT_OUTCOME_DEFEAT,    /**< Player lost */
    COMBAT_OUTCOME_FLED       /**< Player successfully fled */
} CombatOutcome;

#define COMBAT_MAX_COMBATANTS 32
#define COMBAT_LOG_SIZE 100
#define COMBAT_LOG_MESSAGE_SIZE 256

/**
 * @brief Combat state structure
 *
 * Manages all state for a single combat encounter.
 */
typedef struct CombatState {
    CombatPhase phase;                                /**< Current combat phase */
    uint32_t turn_number;                             /**< Current turn number (starts at 1) */

    /* Forces */
    Combatant* player_forces[COMBAT_MAX_COMBATANTS];  /**< Player-controlled units */
    uint8_t player_force_count;                       /**< Number of player units */

    Combatant* enemy_forces[COMBAT_MAX_COMBATANTS];   /**< Enemy units */
    uint8_t enemy_force_count;                        /**< Number of enemy units */

    /* Turn order */
    Combatant* turn_order[COMBAT_MAX_COMBATANTS * 2]; /**< Initiative-sorted order */
    uint8_t turn_order_count;                         /**< Total combatants */
    uint8_t current_turn_index;                       /**< Index in turn_order */

    /* Combat log */
    char log_messages[COMBAT_LOG_SIZE][COMBAT_LOG_MESSAGE_SIZE];
    uint8_t log_count;                                /**< Number of messages */
    uint8_t log_head;                                 /**< Circular buffer head */

    /* Metadata */
    uint64_t combat_start_time;                       /**< When combat began (game time) */
    CombatOutcome outcome;                            /**< Combat result */
    bool player_can_act;                              /**< If true, player can issue commands */
} CombatState;

/**
 * @brief Create a new combat state
 *
 * Allocates and initializes a combat state structure.
 *
 * @return Newly allocated CombatState, or NULL on failure
 */
CombatState* combat_state_create(void);

/**
 * @brief Destroy a combat state
 *
 * Frees all combatants and the combat state itself.
 *
 * @param combat Combat state to destroy (can be NULL)
 */
void combat_state_destroy(CombatState* combat);

/**
 * @brief Add a combatant to player forces
 *
 * @param combat Combat state
 * @param combatant Combatant to add (takes ownership)
 * @return true on success, false if forces are full
 */
bool combat_add_player_combatant(CombatState* combat, Combatant* combatant);

/**
 * @brief Add a combatant to enemy forces
 *
 * @param combat Combat state
 * @param combatant Combatant to add (takes ownership)
 * @return true on success, false if forces are full
 */
bool combat_add_enemy_combatant(CombatState* combat, Combatant* combatant);

/**
 * @brief Initialize combat (calculate turn order, etc.)
 *
 * Must be called after all combatants are added and before combat starts.
 * Rolls initiative for all combatants and sorts turn order.
 *
 * @param combat Combat state
 * @return true on success, false on error
 */
bool combat_initialize(CombatState* combat);

/**
 * @brief Calculate turn order based on initiative
 *
 * Sorts all combatants by initiative (highest first).
 *
 * @param combat Combat state
 */
void combat_calculate_turn_order(CombatState* combat);

/**
 * @brief Update combat state (called each frame)
 *
 * Advances combat through phases, processes AI turns, checks win/loss.
 *
 * @param combat Combat state
 * @param delta_time Time since last update (seconds)
 */
void combat_update(CombatState* combat, double delta_time);

/**
 * @brief Advance to next turn
 *
 * Moves to next combatant in turn order or starts new round.
 *
 * @param combat Combat state
 */
void combat_advance_turn(CombatState* combat);

/**
 * @brief Check if player has won
 *
 * @param combat Combat state
 * @return true if all enemies are dead
 */
bool combat_check_victory(const CombatState* combat);

/**
 * @brief Check if player has lost
 *
 * @param combat Combat state
 * @return true if all player forces are dead
 */
bool combat_check_defeat(const CombatState* combat);

/**
 * @brief Add a message to the combat log
 *
 * Uses circular buffer to maintain last N messages.
 *
 * @param combat Combat state
 * @param format Printf-style format string
 * @param ... Format arguments
 */
void combat_log_message(CombatState* combat, const char* format, ...);

/**
 * @brief Get the most recent log messages
 *
 * Returns pointers to the last N messages (max COMBAT_LOG_SIZE).
 *
 * @param combat Combat state
 * @param count Maximum number of messages to retrieve
 * @param out_messages Array to fill with message pointers
 * @return Number of messages retrieved
 */
size_t combat_get_log_messages(const CombatState* combat, size_t count, const char** out_messages);

/**
 * @brief Find combatant by ID
 *
 * @param combat Combat state
 * @param id Combatant ID to search for
 * @return Pointer to combatant, or NULL if not found
 */
Combatant* combat_find_combatant(const CombatState* combat, const char* id);

/**
 * @brief Get current active combatant (whose turn it is)
 *
 * @param combat Combat state
 * @return Pointer to active combatant, or NULL if none
 */
Combatant* combat_get_active_combatant(const CombatState* combat);

/**
 * @brief Get count of living combatants in player forces
 *
 * @param combat Combat state
 * @return Number of alive player units
 */
uint8_t combat_count_living_player_forces(const CombatState* combat);

/**
 * @brief Get count of living combatants in enemy forces
 *
 * @param combat Combat state
 * @return Number of alive enemy units
 */
uint8_t combat_count_living_enemy_forces(const CombatState* combat);

/**
 * @brief Process AI turn for current combatant
 *
 * Calls the combatant's AI function to make a decision.
 *
 * @param combat Combat state
 */
void combat_process_ai_turn(CombatState* combat);

/**
 * @brief End combat and clean up
 *
 * Sets phase to END and records outcome.
 *
 * @param combat Combat state
 * @param outcome Combat outcome
 */
void combat_end(CombatState* combat, CombatOutcome outcome);

#endif /* COMBAT_H */

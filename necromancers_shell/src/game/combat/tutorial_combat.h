/**
 * @file tutorial_combat.h
 * @brief Tutorial combat encounter
 */

#ifndef TUTORIAL_COMBAT_H
#define TUTORIAL_COMBAT_H

#include <stdbool.h>

/* Forward declarations */
typedef struct CombatState CombatState;

/* Include game state for GameState struct */
#include "../game_state.h"

/**
 * @brief Create tutorial combat encounter
 *
 * A simple 1v1 fight to teach combat basics:
 * - Player has 1 zombie minion
 * - Enemy is 1 weak villager
 * - Guided prompts for each command
 *
 * @param game_state Game state to set up combat
 * @return Newly created combat state, or NULL on failure
 */
CombatState* tutorial_combat_create(GameState* game_state);

/**
 * @brief Check if player has completed tutorial
 *
 * @param game_state Game state
 * @return true if tutorial completed
 */
bool tutorial_combat_is_completed(const GameState* game_state);

/**
 * @brief Mark tutorial as completed
 *
 * @param game_state Game state
 */
void tutorial_combat_mark_completed(GameState* game_state);

/**
 * @brief Get tutorial welcome message
 *
 * @return Welcome message string
 */
const char* tutorial_combat_get_welcome_message(void);

/**
 * @brief Get tutorial instructions
 *
 * @return Instructions string
 */
const char* tutorial_combat_get_instructions(void);

/**
 * @brief Award tutorial completion bonus
 *
 * @param game_state Game state
 */
void tutorial_combat_award_bonus(GameState* game_state);

#endif /* TUTORIAL_COMBAT_H */

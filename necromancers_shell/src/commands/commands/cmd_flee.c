/**
 * @file cmd_flee.c
 * @brief Flee command implementation for combat
 */

#include "commands.h"
#include "../executor.h"
#include "../../game/game_state.h"
#include "../../game/combat/combat.h"
#include "../../game/combat/combatant.h"
#include "../../game/combat/damage.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Forward declarations */
typedef struct StateManager StateManager;
extern bool state_manager_pop(StateManager* manager);

/* External game state and state manager */
extern GameState* g_game_state;
extern StateManager* g_state_manager;

/**
 * @brief Calculate flee success chance
 *
 * Base: 50%
 * +10% per dead enemy
 * -20% if any ally HP < 30%
 */
static float calculate_flee_chance(CombatState* combat) {
    float chance = 0.50f;  /* Base 50% */

    /* Count dead enemies (+10% each) */
    uint8_t dead_enemies = 0;
    for (uint8_t i = 0; i < combat->enemy_force_count; i++) {
        if (!combatant_is_alive(combat->enemy_forces[i])) {
            dead_enemies++;
        }
    }
    chance += (dead_enemies * 0.10f);

    /* Check for low HP allies (-20% if any below 30%) */
    for (uint8_t i = 0; i < combat->player_force_count; i++) {
        Combatant* ally = combat->player_forces[i];
        if (combatant_is_alive(ally)) {
            float hp_percent = (float)ally->health / (float)ally->health_max;
            if (hp_percent < 0.30f) {
                chance -= 0.20f;
                break;  /* Only apply once */
            }
        }
    }

    /* Cap between 10% and 95% */
    if (chance < 0.10f) chance = 0.10f;
    if (chance > 0.95f) chance = 0.95f;

    return chance;
}

/**
 * @brief Execute flee command
 *
 * Usage: flee
 */
CommandResult cmd_flee(ParsedCommand* cmd) {
    if (!cmd || !g_game_state || !g_state_manager) {
        return command_result_error(EXEC_ERROR_COMMAND_FAILED, "Invalid state");
    }

    /* Check if in combat */
    if (!g_game_state->combat) {
        return command_result_error(EXEC_ERROR_COMMAND_FAILED, "Not in combat. Use this command during combat encounters.");
    }

    CombatState* combat = g_game_state->combat;

    /* Check if it's player's turn */
    if (combat->phase != COMBAT_PHASE_PLAYER_TURN) {
        return command_result_error(EXEC_ERROR_COMMAND_FAILED, "Not your turn. Wait for player turn phase.");
    }

    /* Check if player can act */
    if (!combat->player_can_act) {
        return command_result_error(EXEC_ERROR_COMMAND_FAILED, "Cannot act this turn.");
    }

    /* Calculate flee chance */
    float flee_chance = calculate_flee_chance(combat);
    int flee_percent = (int)(flee_chance * 100.0f);

    /* Roll for success */
    float roll = (float)rand() / (float)RAND_MAX;
    bool success = (roll < flee_chance);

    char msg[2048];  /* Increased buffer for multiple attack messages */
    size_t offset = 0;

    if (success) {
        /* Successful flee */
        combat_log_message(combat, "Successfully fled from combat!");

        offset += snprintf(msg + offset, sizeof(msg) - offset,
            "You flee from combat! (Success chance: %d%%)\n"
            "\n"
            "Your forces retreat to safety.",
            flee_percent);

        /* End combat */
        combat_end(combat, COMBAT_OUTCOME_FLED);

        /* Exit combat state */
        state_manager_pop(g_state_manager);

        /* Clean up combat state */
        combat_state_destroy(combat);
        g_game_state->combat = NULL;

    } else {
        /* Failed flee - enemies get free attacks */
        combat_log_message(combat, "Failed to flee! Enemies attack while you retreat!");

        offset += snprintf(msg + offset, sizeof(msg) - offset,
            "Failed to flee! (Success chance: %d%%)\n"
            "\n"
            "Enemies take advantage of your retreat!\n",
            flee_percent);

        /* Each living enemy gets a free attack on a random player force */
        for (uint8_t i = 0; i < combat->enemy_force_count; i++) {
            Combatant* enemy = combat->enemy_forces[i];
            if (!combatant_is_alive(enemy)) {
                continue;
            }

            /* Find random living player force */
            uint8_t living_count = 0;
            Combatant* living_allies[32];

            for (uint8_t j = 0; j < combat->player_force_count; j++) {
                if (combatant_is_alive(combat->player_forces[j])) {
                    living_allies[living_count++] = combat->player_forces[j];
                }
            }

            if (living_count == 0) {
                break;  /* All allies dead */
            }

            /* Pick random target */
            Combatant* target = living_allies[rand() % living_count];

            /* Attack */
            AttackResult result = damage_calculate_attack(enemy, target, DAMAGE_TYPE_PHYSICAL);
            combat_log_message(combat, "%s attacks %s during retreat!", enemy->name, target->name);

            bool alive = damage_apply_attack(combat, target, &result);

            offset += snprintf(msg + offset, sizeof(msg) - offset,
                "\n%s hits %s for %u damage!%s",
                enemy->name, target->name, result.damage_dealt,
                alive ? "" : " SLAIN!");
        }

        /* Check if all player forces died */
        if (combat_check_defeat(combat)) {
            combat_end(combat, COMBAT_OUTCOME_DEFEAT);
            offset += snprintf(msg + offset, sizeof(msg) - offset, "\n\nDEFEAT! All your forces have fallen!");
        } else {
            /* Continue combat - transition to enemy turn */
            combat->phase = COMBAT_PHASE_ENEMY_TURN;
            combat_log_message(combat, "\n--- Enemy Turn ---");

            /* Process remaining enemy turns */
            combat_process_ai_turn(combat);

            /* Check again for defeat */
            if (combat_check_defeat(combat)) {
                combat_end(combat, COMBAT_OUTCOME_DEFEAT);
                offset += snprintf(msg + offset, sizeof(msg) - offset, "\n\nDEFEAT! All your forces have fallen!");
            } else {
                /* Start new turn */
                combat->turn_number++;
                combat->phase = COMBAT_PHASE_PLAYER_TURN;

                /* Reset turn flags */
                for (uint8_t i = 0; i < combat->turn_order_count; i++) {
                    combatant_reset_turn_flags(combat->turn_order[i]);
                }

                combat_log_message(combat, "\n--- Turn %u - Player Turn ---", combat->turn_number);

                offset += snprintf(msg + offset, sizeof(msg) - offset, "\n\nTurn %u begins!", combat->turn_number);
            }
        }
    }

    return command_result_success(msg);
}

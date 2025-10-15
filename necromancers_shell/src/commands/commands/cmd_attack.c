/**
 * @file cmd_attack.c
 * @brief Attack command implementation for combat
 */

#include "commands.h"
#include "../executor.h"
#include "../../game/game_state.h"
#include "../../game/combat/combat.h"
#include "../../game/combat/combatant.h"
#include "../../game/combat/damage.h"
#include <stdio.h>
#include <string.h>

/* External game state */
extern GameState* g_game_state;

/**
 * @brief Execute attack command
 *
 * Usage: attack <target_id>
 * Example: attack E1
 */
CommandResult cmd_attack(ParsedCommand* cmd) {
    if (!cmd || !g_game_state) {
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

    /* Get target argument */
    const char* target_id = parsed_command_get_arg(cmd, 0);
    if (!target_id) {
        return command_result_error(EXEC_ERROR_COMMAND_FAILED, "Usage: attack <target_id>\nExample: attack E1");
    }

    /* Get active combatant (current turn) */
    Combatant* active = combat_get_active_combatant(combat);
    if (!active) {
        return command_result_error(EXEC_ERROR_COMMAND_FAILED, "No active combatant.");
    }

    /* Verify active combatant is player-controlled */
    if (!active->is_player_controlled) {
        return command_result_error(EXEC_ERROR_COMMAND_FAILED, "Current combatant is not player-controlled.");
    }

    /* Check if already acted this turn */
    if (active->has_acted_this_turn) {
        char err[256];
        snprintf(err, sizeof(err), "%s has already acted this turn.", active->name);
        return command_result_error(EXEC_ERROR_COMMAND_FAILED, err);
    }

    /* Find target combatant */
    Combatant* target = combat_find_combatant(combat, target_id);
    if (!target) {
        char err[256];
        snprintf(err, sizeof(err), "Target '%s' not found. Use 'status' to see valid targets.", target_id);
        return command_result_error(EXEC_ERROR_COMMAND_FAILED, err);
    }

    /* Verify target is an enemy */
    if (target->is_player_controlled) {
        return command_result_error(EXEC_ERROR_COMMAND_FAILED, "Cannot attack your own forces!");
    }

    /* Verify target is alive */
    if (!combatant_is_alive(target)) {
        char err[256];
        snprintf(err, sizeof(err), "%s is already dead.", target->name);
        return command_result_error(EXEC_ERROR_COMMAND_FAILED, err);
    }

    /* Calculate damage */
    AttackResult result = damage_calculate_attack(active, target, DAMAGE_TYPE_PHYSICAL);

    /* Log the attack */
    combat_log_message(combat, "%s attacks %s!", active->name, target->name);

    /* Apply damage */
    bool target_alive = damage_apply_attack(combat, target, &result);

    /* Mark as acted */
    active->has_acted_this_turn = true;

    /* Build result message */
    char msg[512];
    if (result.is_critical) {
        snprintf(msg, sizeof(msg),
            "CRITICAL HIT! %s deals %u damage to %s!",
            active->name, result.damage_dealt, target->name);
    } else {
        snprintf(msg, sizeof(msg),
            "%s deals %u damage to %s (%u base - %u mitigated)",
            active->name, result.damage_dealt, target->name,
            result.base_damage, result.mitigated_damage);
    }

    if (!target_alive) {
        char death_msg[256];
        snprintf(death_msg, sizeof(death_msg), "\n%s has been slain!", target->name);
        strncat(msg, death_msg, sizeof(msg) - strlen(msg) - 1);
    } else {
        char hp_msg[256];
        snprintf(hp_msg, sizeof(hp_msg), "\n%s: %u/%u HP",
            target->name, target->health, target->health_max);
        strncat(msg, hp_msg, sizeof(msg) - strlen(msg) - 1);
    }

    /* Check if all player forces have acted */
    bool all_acted = true;
    for (uint8_t i = 0; i < combat->player_force_count; i++) {
        Combatant* c = combat->player_forces[i];
        if (c->is_player_controlled && combatant_is_alive(c) && !c->has_acted_this_turn) {
            all_acted = false;
            break;
        }
    }

    if (all_acted) {
        /* Transition to enemy turn */
        combat->phase = COMBAT_PHASE_ENEMY_TURN;
        combat_log_message(combat, "\n--- Enemy Turn ---");

        /* Process all enemy turns */
        combat_process_ai_turn(combat);

        /* Check for victory/defeat */
        if (combat_check_victory(combat)) {
            combat_end(combat, COMBAT_OUTCOME_VICTORY);
            strncat(msg, "\n\nVICTORY! All enemies defeated!", sizeof(msg) - strlen(msg) - 1);
        } else if (combat_check_defeat(combat)) {
            combat_end(combat, COMBAT_OUTCOME_DEFEAT);
            strncat(msg, "\n\nDEFEAT! All your forces have fallen!", sizeof(msg) - strlen(msg) - 1);
        } else {
            /* Start new turn */
            combat->turn_number++;
            combat->phase = COMBAT_PHASE_PLAYER_TURN;

            /* Reset turn flags for all combatants */
            for (uint8_t i = 0; i < combat->turn_order_count; i++) {
                combatant_reset_turn_flags(combat->turn_order[i]);
            }

            combat_log_message(combat, "\n--- Turn %u - Player Turn ---", combat->turn_number);

            char turn_msg[64];
            snprintf(turn_msg, sizeof(turn_msg), "\n\nTurn %u begins!", combat->turn_number);
            strncat(msg, turn_msg, sizeof(msg) - strlen(msg) - 1);
        }
    }

    return command_result_success(msg);
}

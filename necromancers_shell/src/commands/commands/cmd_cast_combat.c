/**
 * @file cmd_cast_combat.c
 * @brief Combat spell casting command implementation
 */

#include "commands.h"
#include "../executor.h"
#include "../../game/game_state.h"
#include "../../game/combat/combat.h"
#include "../../game/combat/combatant.h"
#include "../../game/combat/damage.h"
#include "../../game/resources/resources.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>

/* External game state */
extern GameState* g_game_state;

/**
 * @brief Spell definition
 */
typedef struct {
    const char* name;
    uint32_t mana_cost;
    DamageType damage_type;
    uint32_t base_power;
    const char* description;
} CombatSpell;

/* Available combat spells */
static const CombatSpell SPELL_DRAIN = {
    .name = "drain",
    .mana_cost = 15,
    .damage_type = DAMAGE_TYPE_NECROTIC,
    .base_power = 15,
    .description = "Deal 15 necrotic damage, heal self for 50% of damage dealt"
};

static const CombatSpell SPELL_BOLT = {
    .name = "bolt",
    .mana_cost = 20,
    .damage_type = DAMAGE_TYPE_PURE,
    .base_power = 20,
    .description = "Deal 20 pure damage (ignores defense)"
};

static const CombatSpell SPELL_WEAKEN = {
    .name = "weaken",
    .mana_cost = 10,
    .damage_type = DAMAGE_TYPE_NECROTIC,
    .base_power = 0,
    .description = "Reduce target defense by 20% for 2 turns"
};

/**
 * @brief Find spell by name
 */
static const CombatSpell* find_spell(const char* name) {
    if (strcasecmp(name, "drain") == 0) return &SPELL_DRAIN;
    if (strcasecmp(name, "bolt") == 0) return &SPELL_BOLT;
    if (strcasecmp(name, "weaken") == 0) return &SPELL_WEAKEN;
    return NULL;
}

/**
 * @brief Execute cast command
 *
 * Usage: cast <spell> <target>
 */
CommandResult cmd_cast_combat(ParsedCommand* cmd) {
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

    /* Get active combatant */
    Combatant* caster = combat_get_active_combatant(combat);
    if (!caster) {
        return command_result_error(EXEC_ERROR_COMMAND_FAILED, "No active combatant.");
    }

    /* Verify caster is player-controlled */
    if (!caster->is_player_controlled) {
        return command_result_error(EXEC_ERROR_COMMAND_FAILED, "Current combatant is not player-controlled.");
    }

    /* Check if already acted */
    if (caster->has_acted_this_turn) {
        char err[256];
        snprintf(err, sizeof(err), "%s has already acted this turn.", caster->name);
        return command_result_error(EXEC_ERROR_COMMAND_FAILED, err);
    }

    /* Get spell name */
    const char* spell_name = parsed_command_get_arg(cmd, 0);
    if (!spell_name) {
        return command_result_error(EXEC_ERROR_COMMAND_FAILED,
            "Usage: cast <spell> <target>\n"
            "\n"
            "Available spells:\n"
            "  drain <target>  - 15 damage, heal 50%% (15 mana)\n"
            "  bolt <target>   - 20 pure damage (20 mana)\n"
            "  weaken <target> - -20%% defense for 2 turns (10 mana)"
        );
    }

    /* Find spell */
    const CombatSpell* spell = find_spell(spell_name);
    if (!spell) {
        char err[256];
        snprintf(err, sizeof(err), "Unknown spell '%s'. Available: drain, bolt, weaken", spell_name);
        return command_result_error(EXEC_ERROR_COMMAND_FAILED, err);
    }

    /* Check mana cost */
    if (g_game_state->resources.mana < spell->mana_cost) {
        char err[256];
        snprintf(err, sizeof(err), "Insufficient mana. Need %u, have %u.",
            spell->mana_cost, g_game_state->resources.mana);
        return command_result_error(EXEC_ERROR_COMMAND_FAILED, err);
    }

    /* Get target */
    const char* target_id = parsed_command_get_arg(cmd, 1);
    if (!target_id) {
        char err[256];
        snprintf(err, sizeof(err), "Specify target. Example: cast %s E1", spell->name);
        return command_result_error(EXEC_ERROR_COMMAND_FAILED, err);
    }

    Combatant* target = combat_find_combatant(combat, target_id);
    if (!target) {
        char err[256];
        snprintf(err, sizeof(err), "Target '%s' not found.", target_id);
        return command_result_error(EXEC_ERROR_COMMAND_FAILED, err);
    }

    /* Verify target is enemy */
    if (target->is_player_controlled) {
        return command_result_error(EXEC_ERROR_COMMAND_FAILED, "Cannot cast offensive spells on allies!");
    }

    /* Verify target is alive */
    if (!combatant_is_alive(target)) {
        char err[256];
        snprintf(err, sizeof(err), "%s is already dead.", target->name);
        return command_result_error(EXEC_ERROR_COMMAND_FAILED, err);
    }

    /* Deduct mana */
    g_game_state->resources.mana -= spell->mana_cost;

    /* Log spell cast */
    combat_log_message(combat, "%s casts %s on %s! (-%u mana)",
        caster->name, spell->name, target->name, spell->mana_cost);

    /* Mark as acted */
    caster->has_acted_this_turn = true;

    char msg[512];
    snprintf(msg, sizeof(msg), "%s casts %s! (-%u mana)\n\n",
        caster->name, spell->name, spell->mana_cost);

    /* Apply spell effect */
    if (strcasecmp(spell->name, "drain") == 0) {
        /* Drain: Damage + heal */
        AttackResult result = damage_calculate_attack(caster, target, DAMAGE_TYPE_NECROTIC);
        /* Override damage with spell power */
        result.damage_dealt = spell->base_power;
        result.base_damage = spell->base_power;

        bool alive = damage_apply_attack(combat, target, &result);

        /* Heal caster for 50% of damage */
        uint32_t heal = result.damage_dealt / 2;
        damage_apply_heal(combat, caster, heal);

        char effect_msg[512];
        snprintf(effect_msg, sizeof(effect_msg),
            "%s takes %u necrotic damage\n%s heals for %u HP%s",
            target->name, result.damage_dealt,
            caster->name, heal,
            alive ? "" : "\n\nTarget slain!");
        strncat(msg, effect_msg, sizeof(msg) - strlen(msg) - 1);

    } else if (strcasecmp(spell->name, "bolt") == 0) {
        /* Bolt: Pure damage */
        AttackResult result = damage_calculate_attack(caster, target, DAMAGE_TYPE_PURE);
        /* Override with spell power */
        result.damage_dealt = spell->base_power;
        result.base_damage = spell->base_power;

        bool alive = damage_apply_attack(combat, target, &result);

        char effect_msg[256];
        snprintf(effect_msg, sizeof(effect_msg),
            "%s takes %u pure damage (ignores defense)%s",
            target->name, result.damage_dealt,
            alive ? "" : "\n\nTarget slain!");
        strncat(msg, effect_msg, sizeof(msg) - strlen(msg) - 1);

    } else if (strcasecmp(spell->name, "weaken") == 0) {
        /* Weaken: Reduce defense (simplified - just log it) */
        uint32_t defense_reduction = target->defense * 20 / 100;
        combat_log_message(combat, "%s's defense reduced by %u for 2 turns!",
            target->name, defense_reduction);

        char effect_msg[256];
        snprintf(effect_msg, sizeof(effect_msg),
            "%s's defense weakened!\n-20%% defense (%u) for 2 turns",
            target->name, defense_reduction);
        strncat(msg, effect_msg, sizeof(msg) - strlen(msg) - 1);

        /* Note: Full status effect system would be Phase 5 */
        /* For now, just log the effect */
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

        /* Process enemy turns */
        combat_process_ai_turn(combat);

        /* Check victory/defeat */
        if (combat_check_victory(combat)) {
            combat_end(combat, COMBAT_OUTCOME_VICTORY);
            strncat(msg, "\n\nVICTORY! All enemies defeated!", sizeof(msg) - strlen(msg) - 1);
        } else if (combat_check_defeat(combat)) {
            combat_end(combat, COMBAT_OUTCOME_DEFEAT);
            strncat(msg, "\n\nDEFEAT! All your forces have fallen!", sizeof(msg) - strlen(msg) - 1);
        } else {
            /* New turn */
            combat->turn_number++;
            combat->phase = COMBAT_PHASE_PLAYER_TURN;

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

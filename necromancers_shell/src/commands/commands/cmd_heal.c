/**
 * @file cmd_heal.c
 * @brief Heal command implementation
 *
 * Heal damaged minions using soul energy or mana.
 */

#define _GNU_SOURCE
#include "commands.h"
#include "../../game/game_state.h"
#include "../../game/game_globals.h"
#include "../../game/minions/minion_manager.h"
#include "../../game/minions/minion.h"
#include "../../game/resources/resources.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

CommandResult cmd_heal(ParsedCommand* cmd) {
    if (!g_game_state || !g_game_state->minions) {
        return command_result_error(EXEC_ERROR_INTERNAL,
                                     "Game state not initialized");
    }

    /* Get minion ID argument */
    if (cmd->arg_count < 1) {
        return command_result_error(EXEC_ERROR_INVALID_COMMAND,
            "Missing minion ID. Usage: heal <minion_id> [--amount <hp>] [--use-mana]\n"
            "Use 'minions' command to list your army.\n"
            "Default: heal to full HP using soul energy.\n"
            "Use --use-mana to spend mana instead (more efficient).");
    }

    const char* minion_id_str = parsed_command_get_arg(cmd, 0);
    uint32_t minion_id = (uint32_t)atoi(minion_id_str);

    /* Get optional amount */
    uint32_t amount = 0;
    bool has_amount = false;
    if (parsed_command_has_flag(cmd, "amount")) {
        const ArgumentValue* amount_arg = parsed_command_get_flag(cmd, "amount");
        if (amount_arg && amount_arg->type == ARG_TYPE_INT) {
            amount = (uint32_t)amount_arg->value.int_value;
            has_amount = true;
        }
    }

    /* Check if using mana */
    bool use_mana = parsed_command_has_flag(cmd, "use-mana");

    /* Get minion */
    Minion* minion = minion_manager_get(g_game_state->minions, minion_id);
    if (!minion) {
        char error_msg[128];
        snprintf(error_msg, sizeof(error_msg), "Minion ID %u not found", minion_id);
        return command_result_error(EXEC_ERROR_COMMAND_FAILED, error_msg);
    }

    /* Build output */
    char* output = NULL;
    size_t output_size = 0;
    FILE* stream = open_memstream(&output, &output_size);
    if (!stream) {
        return command_result_error(EXEC_ERROR_INTERNAL,
                                     "Failed to allocate output buffer");
    }

    fprintf(stream, "=== Heal Minion ===\n\n");
    fprintf(stream, "Minion: %s '%s' (ID: %u)\n",
            minion_type_name(minion->type), minion->name, minion->id);
    fprintf(stream, "Current HP: %u/%u\n", minion->stats.health, minion->stats.health_max);

    /* Check if healing needed */
    if (minion->stats.health >= minion->stats.health_max) {
        fprintf(stream, "\n[UNNECESSARY] Minion is already at full health.\n");
        fclose(stream);
        CommandResult result = command_result_success(output);
        free(output);
        return result;
    }

    /* Calculate healing amount */
    uint32_t hp_missing = minion->stats.health_max - minion->stats.health;
    uint32_t heal_amount = has_amount ? amount : hp_missing;

    if (heal_amount > hp_missing) {
        heal_amount = hp_missing;
    }

    /* Calculate cost */
    uint32_t cost;
    if (use_mana) {
        cost = heal_amount / 2; /* Mana is more efficient: 2 HP per 1 mana */
        if (cost < 1) cost = 1;

        if (!resources_has_mana(&g_game_state->resources, cost)) {
            fprintf(stream, "\n[FAILED] Insufficient mana!\n");
            fprintf(stream, "Required: %u\n", cost);
            fprintf(stream, "Available: %u\n", g_game_state->resources.mana);
            fclose(stream);
            CommandResult result = command_result_success(output);
            free(output);
            return result;
        }

        /* Spend mana and heal */
        resources_spend_mana(&g_game_state->resources, cost);
        minion->stats.health += heal_amount;
        if (minion->stats.health > minion->stats.health_max) {
            minion->stats.health = minion->stats.health_max;
        }

        fprintf(stream, "\n[SUCCESS] Minion healed!\n\n");
        fprintf(stream, "Arcane energy flows into the undead flesh.\n");
        fprintf(stream, "Wounds close. Bones mend.\n\n");
        fprintf(stream, "HP restored: +%u\n", heal_amount);
        fprintf(stream, "New HP: %u/%u\n", minion->stats.health, minion->stats.health_max);
        fprintf(stream, "Mana spent: -%u (remaining: %u)\n",
                cost, g_game_state->resources.mana);

    } else {
        cost = heal_amount; /* Soul energy: 1:1 ratio */

        if (!resources_has_soul_energy(&g_game_state->resources, cost)) {
            fprintf(stream, "\n[FAILED] Insufficient soul energy!\n");
            fprintf(stream, "Required: %u\n", cost);
            fprintf(stream, "Available: %u\n", g_game_state->resources.soul_energy);
            fprintf(stream, "\nTip: Use --use-mana for more efficient healing (2 HP per 1 mana)\n");
            fclose(stream);
            CommandResult result = command_result_success(output);
            free(output);
            return result;
        }

        /* Spend energy and heal */
        resources_spend_soul_energy(&g_game_state->resources, cost);
        minion->stats.health += heal_amount;
        if (minion->stats.health > minion->stats.health_max) {
            minion->stats.health = minion->stats.health_max;
        }

        fprintf(stream, "\n[SUCCESS] Minion healed!\n\n");
        fprintf(stream, "Soul energy binds the damaged flesh.\n");
        fprintf(stream, "Death's grip tightens. Undeath reinforced.\n\n");
        fprintf(stream, "HP restored: +%u\n", heal_amount);
        fprintf(stream, "New HP: %u/%u\n", minion->stats.health, minion->stats.health_max);
        fprintf(stream, "Soul energy spent: -%u (remaining: %u)\n",
                cost, g_game_state->resources.soul_energy);
    }

    fprintf(stream, "\n--- Minion Status ---\n");
    fprintf(stream, "Health: %u/%u (%.1f%%)\n",
            minion->stats.health, minion->stats.health_max,
            (minion->stats.health * 100.0f) / minion->stats.health_max);
    fprintf(stream, "Loyalty: %u%%\n", minion->stats.loyalty);
    fprintf(stream, "Level: %u\n", minion->level);

    fclose(stream);

    CommandResult result = command_result_success(output);
    free(output);
    return result;
}

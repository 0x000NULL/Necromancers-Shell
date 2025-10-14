/**
 * @file cmd_bind.c
 * @brief Bind command implementation
 *
 * Bind a soul to a minion for stat bonuses.
 */

#define _GNU_SOURCE
#include "commands.h"
#include "../../game/game_state.h"
#include "../../game/minions/minion_manager.h"
#include "../../game/minions/minion.h"
#include "../../game/souls/soul_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern GameState* g_game_state;

CommandResult cmd_bind(ParsedCommand* cmd) {
    if (!g_game_state || !g_game_state->minions || !g_game_state->souls) {
        return command_result_error(EXEC_ERROR_INTERNAL,
                                     "Game state not initialized");
    }

    /* Get minion ID and soul ID arguments */
    if (cmd->arg_count < 2) {
        return command_result_error(EXEC_ERROR_INVALID_COMMAND,
            "Usage: bind <minion_id> <soul_id>");
    }

    const char* minion_arg_str = parsed_command_get_arg(cmd, 0);
    const char* soul_arg_str = parsed_command_get_arg(cmd, 1);

    /* Parse IDs */
    char* endptr = NULL;
    long minion_id_long = strtol(minion_arg_str, &endptr, 10);
    if (endptr == minion_arg_str || *endptr != '\0' || minion_id_long <= 0) {
        return command_result_error(EXEC_ERROR_INVALID_COMMAND,
            "Minion ID must be a positive integer");
    }

    long soul_id_long = strtol(soul_arg_str, &endptr, 10);
    if (endptr == soul_arg_str || *endptr != '\0' || soul_id_long <= 0) {
        return command_result_error(EXEC_ERROR_INVALID_COMMAND,
            "Soul ID must be a positive integer");
    }

    uint32_t minion_id = (uint32_t)minion_id_long;
    uint32_t soul_id = (uint32_t)soul_id_long;

    /* Validate minion exists */
    Minion* minion = minion_manager_get(g_game_state->minions, minion_id);
    if (!minion) {
        char error_msg[128];
        snprintf(error_msg, sizeof(error_msg), "Minion ID %u not found", minion_id);
        return command_result_error(EXEC_ERROR_COMMAND_FAILED, error_msg);
    }

    /* Check if minion already has a soul */
    if (minion->bound_soul_id != 0) {
        char error_msg[128];
        snprintf(error_msg, sizeof(error_msg),
            "Minion %u (%s) already has soul %u bound",
            minion_id, minion->name, minion->bound_soul_id);
        return command_result_error(EXEC_ERROR_COMMAND_FAILED, error_msg);
    }

    /* Validate soul exists and is unbound */
    Soul* soul = soul_manager_get(g_game_state->souls, soul_id);
    if (!soul) {
        char error_msg[128];
        snprintf(error_msg, sizeof(error_msg), "Soul ID %u not found", soul_id);
        return command_result_error(EXEC_ERROR_COMMAND_FAILED, error_msg);
    }
    if (soul->bound) {
        char error_msg[128];
        snprintf(error_msg, sizeof(error_msg),
            "Soul %u is already bound to minion %u",
            soul_id, soul->bound_minion_id);
        return command_result_error(EXEC_ERROR_COMMAND_FAILED, error_msg);
    }

    /* Store stats before binding for comparison */
    MinionStats before = minion->stats;

    /* Bind soul */
    soul_bind(soul, minion_id);
    minion_bind_soul(minion, soul_id);

    /* Calculate stat improvements (10% bonus from soul quality) */
    float bonus = 1.0f + (soul->quality / 1000.0f);
    minion->stats.attack = (uint32_t)(before.attack * bonus);
    minion->stats.defense = (uint32_t)(before.defense * bonus);
    minion->stats.speed = (uint32_t)(before.speed * bonus);
    minion->stats.loyalty = before.loyalty + (soul->quality / 10);
    if (minion->stats.loyalty > 100) minion->stats.loyalty = 100;

    /* Build output showing improvements */
    char* output = NULL;
    size_t output_size = 0;
    FILE* stream = open_memstream(&output, &output_size);
    if (!stream) {
        return command_result_error(EXEC_ERROR_INTERNAL,
                                     "Failed to allocate output buffer");
    }

    fprintf(stream, "=== Soul Binding Complete ===\n\n");
    fprintf(stream, "Bound %s soul (ID: %u, Quality: %u) to %s '%s' (ID: %u)\n\n",
            soul_type_name(soul->type), soul_id, soul->quality,
            minion_type_name(minion->type), minion->name, minion_id);

    fprintf(stream, "Stat Changes:\n");
    fprintf(stream, "  Attack:  %u → %u", before.attack, minion->stats.attack);
    if (minion->stats.attack > before.attack) {
        fprintf(stream, " (+%u)", minion->stats.attack - before.attack);
    }
    fprintf(stream, "\n");

    fprintf(stream, "  Defense: %u → %u", before.defense, minion->stats.defense);
    if (minion->stats.defense > before.defense) {
        fprintf(stream, " (+%u)", minion->stats.defense - before.defense);
    }
    fprintf(stream, "\n");

    fprintf(stream, "  Speed:   %u → %u", before.speed, minion->stats.speed);
    if (minion->stats.speed > before.speed) {
        fprintf(stream, " (+%u)", minion->stats.speed - before.speed);
    }
    fprintf(stream, "\n");

    fprintf(stream, "  Loyalty: %u → %u", before.loyalty, minion->stats.loyalty);
    if (minion->stats.loyalty > before.loyalty) {
        fprintf(stream, " (+%u)", minion->stats.loyalty - before.loyalty);
    }
    fprintf(stream, "\n");

    fclose(stream);

    CommandResult result = command_result_success(output);
    free(output);
    return result;
}

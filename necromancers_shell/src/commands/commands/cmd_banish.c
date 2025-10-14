/**
 * @file cmd_banish.c
 * @brief Banish command implementation
 *
 * Banish (destroy) a minion and return its soul to the collection.
 */

#define _GNU_SOURCE
#include "commands.h"
#include "../../game/game_state.h"
#include "../../game/game_globals.h"
#include "../../game/minions/minion_manager.h"
#include "../../game/minions/minion.h"
#include "../../game/souls/soul_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CommandResult cmd_banish(ParsedCommand* cmd) {
    if (!g_game_state || !g_game_state->minions) {
        return command_result_error(EXEC_ERROR_INTERNAL,
                                     "Game state not initialized");
    }

    /* Get minion ID argument */
    if (cmd->arg_count < 1) {
        return command_result_error(EXEC_ERROR_INVALID_COMMAND,
            "Usage: banish <minion_id>");
    }

    const char* arg_str = parsed_command_get_arg(cmd, 0);

    /* Parse minion ID */
    char* endptr = NULL;
    long minion_id_long = strtol(arg_str, &endptr, 10);
    if (endptr == arg_str || *endptr != '\0' || minion_id_long <= 0) {
        return command_result_error(EXEC_ERROR_INVALID_COMMAND,
            "Minion ID must be a positive integer");
    }

    uint32_t minion_id = (uint32_t)minion_id_long;

    /* Get minion */
    Minion* minion = minion_manager_get(g_game_state->minions, minion_id);
    if (!minion) {
        char error_msg[128];
        snprintf(error_msg, sizeof(error_msg), "Minion ID %u not found", minion_id);
        return command_result_error(EXEC_ERROR_COMMAND_FAILED, error_msg);
    }

    /* Store info for message */
    char minion_name[64];
    strncpy(minion_name, minion->name, sizeof(minion_name) - 1);
    minion_name[sizeof(minion_name) - 1] = '\0';
    MinionType type = minion->type;
    uint32_t bound_soul_id = minion->bound_soul_id;

    /* Unbind soul if any */
    if (bound_soul_id != 0) {
        Soul* soul = soul_manager_get(g_game_state->souls, bound_soul_id);
        if (soul) {
            soul_unbind(soul);
        }
    }

    /* Remove from manager (transfers ownership) */
    Minion* removed = minion_manager_remove(g_game_state->minions, minion_id);
    if (!removed) {
        return command_result_error(EXEC_ERROR_INTERNAL,
            "Failed to remove minion from army");
    }

    /* Destroy minion */
    minion_destroy(removed);

    /* Build message */
    char* output = NULL;
    size_t output_size = 0;
    FILE* stream = open_memstream(&output, &output_size);
    if (!stream) {
        return command_result_error(EXEC_ERROR_INTERNAL,
                                     "Failed to allocate output buffer");
    }

    fprintf(stream, "=== Minion Banished ===\n\n");
    fprintf(stream, "Banished %s '%s' (ID: %u)\n",
            minion_type_name(type), minion_name, minion_id);

    if (bound_soul_id != 0) {
        fprintf(stream, "\nSoul %u has been returned to your collection\n", bound_soul_id);
    }

    fprintf(stream, "\nRemaining minions: %zu\n", minion_manager_count(g_game_state->minions));

    fclose(stream);

    CommandResult result = command_result_success(output);
    free(output);
    return result;
}

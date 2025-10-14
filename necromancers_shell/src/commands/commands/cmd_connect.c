/**
 * @file cmd_connect.c
 * @brief Connect command implementation
 *
 * Travel to a connected location.
 */

#define _GNU_SOURCE
#include "commands.h"
#include "../../game/game_state.h"
#include "../../game/world/territory.h"
#include "../../game/world/location.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

extern GameState* g_game_state;

CommandResult cmd_connect(ParsedCommand* cmd) {
    if (!g_game_state || !g_game_state->territory) {
        return command_result_error(EXEC_ERROR_INTERNAL,
                                     "Game state not initialized");
    }

    /* Require location argument */
    if (cmd->arg_count < 1) {
        return command_result_error(EXEC_ERROR_INVALID_COMMAND,
                                     "Usage: connect <location_id_or_name>");
    }

    const char* target_arg = parsed_command_get_arg(cmd, 0);

    /* Try to parse as ID first */
    Location* target = NULL;
    char* endptr = NULL;
    long id = strtol(target_arg, &endptr, 10);
    if (endptr != target_arg && *endptr == '\0' && id > 0) {
        /* Parsed as valid ID */
        target = territory_manager_get_location(g_game_state->territory, (uint32_t)id);
    }

    /* If not found by ID, try by name */
    if (!target) {
        target = territory_manager_get_location_by_name(g_game_state->territory, target_arg);
    }

    if (!target) {
        char error_msg[128];
        snprintf(error_msg, sizeof(error_msg), "Location not found: %s", target_arg);
        return command_result_error(EXEC_ERROR_INVALID_COMMAND, error_msg);
    }

    /* Get current location */
    Location* current = game_state_get_current_location(g_game_state);
    if (!current) {
        return command_result_error(EXEC_ERROR_INTERNAL, "No current location");
    }

    /* Check if already at target */
    if (current->id == target->id) {
        char error_msg[128];
        snprintf(error_msg, sizeof(error_msg), "You are already at %s", target->name);
        return command_result_error(EXEC_ERROR_COMMAND_FAILED, error_msg);
    }

    /* Check if connected */
    if (!location_is_connected(current, target->id)) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg),
                 "%s is not connected to %s", target->name, current->name);
        return command_result_error(EXEC_ERROR_COMMAND_FAILED, error_msg);
    }

    /* Check if discovered */
    if (!target->discovered) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg),
                 "%s has not been discovered yet. Use 'probe %s' first.",
                 target->name, target_arg);
        return command_result_error(EXEC_ERROR_COMMAND_FAILED, error_msg);
    }

    /* Move to location */
    if (!game_state_move_to_location(g_game_state, target->id)) {
        return command_result_error(EXEC_ERROR_INTERNAL,
                                     "Failed to move to location");
    }

    /* Advance time (1-3 hours based on random) */
    srand((unsigned int)time(NULL));
    uint32_t travel_time = 1 + (rand() % 3); /* 1-3 hours */
    game_state_advance_time(g_game_state, travel_time);

    /* Build output string */
    char* output = NULL;
    size_t output_size = 0;
    FILE* stream = open_memstream(&output, &output_size);
    if (!stream) {
        return command_result_error(EXEC_ERROR_INTERNAL,
                                     "Failed to allocate output buffer");
    }

    fprintf(stream, "=== Travel Complete ===\n\n");
    fprintf(stream, "You travel from %s to %s...\n", current->name, target->name);
    fprintf(stream, "Time elapsed: %u hour(s)\n\n", travel_time);
    fprintf(stream, "You arrive at %s.\n\n", target->name);
    fprintf(stream, "%s\n\n", target->description);

    fprintf(stream, "Corpses available: %u\n", target->corpse_count);
    fprintf(stream, "Control level: %u%%\n", target->control_level);

    if (target->status == LOCATION_STATUS_HOSTILE) {
        fprintf(stream, "\n*** WARNING: This location is HOSTILE ***\n");
    }

    fclose(stream);

    CommandResult result = command_result_success(output);
    free(output);
    return result;
}

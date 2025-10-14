/**
 * @file cmd_probe.c
 * @brief Probe command implementation
 *
 * Get detailed information about a specific location and optionally discover it.
 */

#define _GNU_SOURCE
#include "commands.h"
#include "../../game/game_state.h"
#include "../../game/game_globals.h"
#include "../../game/world/territory.h"
#include "../../game/world/location.h"
#include "../../game/resources/resources.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CommandResult cmd_probe(ParsedCommand* cmd) {
    if (!g_game_state || !g_game_state->territory) {
        return command_result_error(EXEC_ERROR_INTERNAL,
                                     "Game state not initialized");
    }

    /* Require location argument */
    if (cmd->arg_count < 1) {
        return command_result_error(EXEC_ERROR_INVALID_COMMAND,
                                     "Usage: probe <location_id_or_name>");
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

    /* Check if target is connected to current location */
    Location* current = game_state_get_current_location(g_game_state);
    if (current && target->id != current->id &&
        !location_is_connected(current, target->id)) {
        char error_msg[128];
        snprintf(error_msg, sizeof(error_msg),
                 "%s is not connected to your current location", target->name);
        return command_result_error(EXEC_ERROR_COMMAND_FAILED, error_msg);
    }

    /* Discover location if undiscovered */
    bool newly_discovered = false;
    if (!target->discovered) {
        location_discover(target, g_game_state->resources.time_hours);
        newly_discovered = true;
    }

    /* Build output string */
    char* output = NULL;
    size_t output_size = 0;
    FILE* stream = open_memstream(&output, &output_size);
    if (!stream) {
        return command_result_error(EXEC_ERROR_INTERNAL,
                                     "Failed to allocate output buffer");
    }

    if (newly_discovered) {
        fprintf(stream, "=== New Location Discovered! ===\n\n");
    } else {
        fprintf(stream, "=== Location Analysis ===\n\n");
    }

    fprintf(stream, "Name: %s\n", target->name);
    fprintf(stream, "Type: %s\n", location_type_name(target->type));
    fprintf(stream, "Status: %s\n\n", location_status_name(target->status));

    fprintf(stream, "Description:\n%s\n\n", target->description);

    fprintf(stream, "Resources:\n");
    fprintf(stream, "  Corpses: %u\n", target->corpse_count);
    fprintf(stream, "  Soul Quality (avg): %u\n", target->soul_quality_avg);
    fprintf(stream, "  Control Level: %u%%\n", target->control_level);
    fprintf(stream, "  Defense Strength: %u\n\n", target->defense_strength);

    /* Show connections */
    if (target->connection_count > 0) {
        fprintf(stream, "Connected to %zu location(s):\n", target->connection_count);
        for (size_t i = 0; i < target->connection_count; i++) {
            Location* conn = territory_manager_get_location(g_game_state->territory,
                                                             target->connected_ids[i]);
            if (conn) {
                if (conn->discovered) {
                    fprintf(stream, "  - %s\n", conn->name);
                } else {
                    fprintf(stream, "  - [Unknown]\n");
                }
            }
        }
    } else {
        fprintf(stream, "No connections detected.\n");
    }

    fclose(stream);

    CommandResult result = command_result_success(output);
    free(output);
    return result;
}

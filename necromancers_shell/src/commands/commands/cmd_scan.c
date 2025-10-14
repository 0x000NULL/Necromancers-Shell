/**
 * @file cmd_scan.c
 * @brief Scan command implementation
 *
 * Show connected locations from current location.
 */

#define _GNU_SOURCE
#include "commands.h"
#include "../../game/game_state.h"
#include "../../game/game_globals.h"
#include "../../game/world/territory.h"
#include "../../game/world/location.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CommandResult cmd_scan(ParsedCommand* cmd) {
    (void)cmd; /* No arguments */

    if (!g_game_state || !g_game_state->territory) {
        return command_result_error(EXEC_ERROR_INTERNAL,
                                     "Game state not initialized");
    }

    /* Get current location */
    Location* current = game_state_get_current_location(g_game_state);
    if (!current) {
        return command_result_error(EXEC_ERROR_INTERNAL,
                                     "No current location");
    }

    /* Build output string */
    char* output = NULL;
    size_t output_size = 0;
    FILE* stream = open_memstream(&output, &output_size);
    if (!stream) {
        return command_result_error(EXEC_ERROR_INTERNAL,
                                     "Failed to allocate output buffer");
    }

    fprintf(stream, "=== Location Scan ===\n\n");
    fprintf(stream, "Current Location: %s (%s)\n",
            current->name, location_type_name(current->type));
    fprintf(stream, "Control: %u%%\n\n", current->control_level);

    if (current->connection_count == 0) {
        fprintf(stream, "No connected locations detected.\n");
    } else {
        fprintf(stream, "Connected Locations:\n\n");

        for (size_t i = 0; i < current->connection_count; i++) {
            uint32_t conn_id = current->connected_ids[i];
            Location* conn = territory_manager_get_location(g_game_state->territory, conn_id);

            if (!conn) {
                continue; /* Skip invalid connections */
            }

            if (conn->discovered) {
                fprintf(stream, "  [%u] %s (%s)\n",
                        conn->id, conn->name, location_type_name(conn->type));
                fprintf(stream, "      Status: %s\n",
                        location_status_name(conn->status));
                fprintf(stream, "      Corpses: %u\n",
                        conn->corpse_count);
                fprintf(stream, "      Control: %u%%\n",
                        conn->control_level);
            } else {
                fprintf(stream, "  [%u] Unknown Location (%s)\n",
                        conn->id, location_type_name(conn->type));
                fprintf(stream, "      Status: Undiscovered\n");
                fprintf(stream, "      Use 'probe %u' to investigate\n", conn->id);
            }

            fprintf(stream, "\n");
        }
    }

    fprintf(stream, "Use 'probe <location>' for detailed information.\n");
    fprintf(stream, "Use 'connect <location>' to travel.\n");

    fclose(stream);

    CommandResult result = command_result_success(output);
    free(output);
    return result;
}

/**
 * @file cmd_save.c
 * @brief Save command implementation
 */

#include "commands.h"
#include "../../data/save_load.h"
#include "../../game/game_state.h"
#include "../../utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CommandResult cmd_save(ParsedCommand* cmd) {
    GameState* state = game_state_get_instance();
    if (!state || !state->initialized) {
        return command_result_error(EXEC_ERROR_COMMAND_FAILED, "No game in progress.");
    }

    /* Get optional filepath argument */
    const char* filepath = parsed_command_get_arg(cmd, 0);

    /* Perform save */
    if (save_game(state, filepath)) {
        /* Also save JSON metadata for quick inspection */
        if (filepath) {
            size_t len = strlen(filepath) + 6;
            char* json_path = malloc(len);
            if (json_path) {
                snprintf(json_path, len, "%s.json", filepath);
                save_metadata_json(state, json_path);
                free(json_path);
            }
        } else {
            save_metadata_json(state, NULL);
        }

        if (filepath) {
            char msg[512];
            snprintf(msg, sizeof(msg), "Game saved successfully to %s", filepath);
            return command_result_success(msg);
        } else {
            return command_result_success("Game saved successfully.");
        }
    } else {
        LOG_ERROR("Failed to save game");
        return command_result_error(EXEC_ERROR_COMMAND_FAILED, "Failed to save game. Check logs for details.");
    }
}

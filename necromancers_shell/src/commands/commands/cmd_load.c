/**
 * @file cmd_load.c
 * @brief Load command implementation
 */

#include "commands.h"
#include "../../data/save_load.h"
#include "../../game/game_state.h"
#include "../../utils/logger.h"
#include <stdio.h>
#include <string.h>

CommandResult cmd_load(ParsedCommand* cmd) {
    /* Get optional filepath argument */
    const char* filepath = parsed_command_get_arg(cmd, 0);

    /* Check if save file exists */
    if (!save_file_exists(filepath)) {
        return command_result_error(EXEC_ERROR_COMMAND_FAILED, "Save file not found.");
    }

    /* Validate save file before loading */
    if (!validate_save_file(filepath)) {
        return command_result_error(EXEC_ERROR_COMMAND_FAILED, "Invalid or corrupted save file.");
    }

    /* Load game state */
    char error[256];
    GameState* loaded = load_game(filepath, error, sizeof(error));

    if (!loaded) {
        char msg[512];
        snprintf(msg, sizeof(msg), "Failed to load game: %s", error);
        LOG_ERROR("%s", msg);
        return command_result_error(EXEC_ERROR_COMMAND_FAILED, msg);
    }

    /* WARNING: This replaces the current game state!
     * Get the old state first so we can destroy it */
    GameState* old_state = game_state_get_instance();

    /* The loaded state has souls and minions, but needs territory and other
     * subsystems re-initialized. For now, we'll note this limitation.
     * A full implementation would need to either:
     * 1. Save/load ALL subsystems (territory, graph, etc.)
     * 2. Re-create subsystems and sync them with loaded data
     *
     * For this MVP, we'll assume the user loads into a freshly initialized
     * game or we provide a warning about partial state.
     */

    /* TODO: Complete subsystem initialization for loaded state
     * For now, mark as uninitialized so caller knows to complete setup */
    loaded->initialized = false;

    /* Set new global state */
    game_state_set_instance(loaded);

    /* Destroy old state if it existed */
    if (old_state) {
        game_state_destroy(old_state);
    }

    /* Return success with warning about partial load */
    return command_result_success(
        "Game loaded successfully. Note: Some subsystems may need reinitialization.");
}

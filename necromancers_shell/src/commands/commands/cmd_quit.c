#include "commands.h"
#include "../../data/save_load.h"
#include "../../game/game_state.h"
#include "../../utils/logger.h"
#include <stdio.h>
#include <stdlib.h>

CommandResult cmd_quit(ParsedCommand* cmd) {
    (void)cmd; /* Unused */

    /* Auto-save before exiting */
    GameState* state = game_state_get_instance();
    if (state && state->initialized) {
        LOG_INFO("Auto-saving game before exit...");
        if (save_game(state, NULL)) {
            /* Also save metadata */
            save_metadata_json(state, NULL);
            return command_result_exit("\nGame saved. Farewell, Necromancer...\n");
        } else {
            LOG_WARN("Auto-save failed on quit");
            return command_result_exit("\nWarning: Could not save game. Farewell...\n");
        }
    }

    const char* farewell = "\nFarewell, Necromancer. The shadows await your return...\n";
    return command_result_exit(farewell);
}

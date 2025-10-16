/**
 * @file cmd_spare.c
 * @brief Spare command implementation
 *
 * Spare villages/NPCs during interactive story events.
 */

#include "commands.h"
#include "../command_system.h"
#include "../../game/game_state.h"
#include "../../game/game_globals.h"
#include "../../game/events/ashbrook_event.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>

CommandResult cmd_spare(ParsedCommand* cmd) {
    if (!g_game_state) {
        return command_result_error(EXEC_ERROR_INTERNAL,
                                     "Game state not initialized");
    }

    /* Get target argument */
    if (cmd->arg_count < 1) {
        return command_result_error(EXEC_ERROR_INVALID_COMMAND,
            "Missing target. Usage: spare <target>\n"
            "Targets: ashbrook");
    }

    const char* target = parsed_command_get_arg(cmd, 0);

    /* Handle Ashbrook village */
    if (strcasecmp(target, "ashbrook") == 0) {
        if (ashbrook_spare_village(g_game_state)) {
            return command_result_success("");
        } else {
            return command_result_error(EXEC_ERROR_COMMAND_FAILED,
                "Cannot spare Ashbrook at this time.");
        }
    }

    /* Unknown target */
    char error_msg[128];
    snprintf(error_msg, sizeof(error_msg),
            "Unknown target: %s\nAvailable: ashbrook", target);
    return command_result_error(EXEC_ERROR_INVALID_COMMAND, error_msg);
}

/**
 * @brief Register spare command
 */
void register_spare_command(void) {
    CommandInfo spare_cmd = {
        .name = "spare",
        .description = "Spare villages or NPCs during story events",
        .usage = "spare <target>",
        .help_text =
            "Choose mercy during interactive story events:\n"
            "  spare ashbrook    - Spare Ashbrook village\n"
            "\n"
            "Sparing reduces corruption but may cost resources or opportunities.",
        .function = cmd_spare,
        .flags = NULL,
        .flag_count = 0,
        .min_args = 1,
        .max_args = 1,
        .hidden = false
    };

    command_system_register_command(&spare_cmd);
}

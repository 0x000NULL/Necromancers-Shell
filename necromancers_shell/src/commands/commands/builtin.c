#include "commands.h"
#include "../registry.h"
#include <stdlib.h>

int register_builtin_commands(CommandRegistry* registry) {
    if (!registry) return 0;

    int registered = 0;

    /* Help command */
    {
        CommandInfo info = {
            .name = "help",
            .description = "Display help information",
            .usage = "help [command]",
            .help_text = "Shows help for all commands or a specific command.\n"
                        "Without arguments, lists all available commands.\n"
                        "With a command name, shows detailed help for that command.",
            .function = cmd_help,
            .flags = NULL,
            .flag_count = 0,
            .min_args = 0,
            .max_args = 1,
            .hidden = false
        };
        if (command_registry_register(registry, &info)) {
            registered++;
        }
    }

    /* Status command */
    {
        static FlagDefinition status_flags[] = {
            {
                .name = "verbose",
                .short_name = 'v',
                .type = ARG_TYPE_BOOL,
                .required = false,
                .description = "Show detailed status information"
            }
        };

        CommandInfo info = {
            .name = "status",
            .description = "Show game state and statistics",
            .usage = "status [--verbose]",
            .help_text = "Displays current game state, player stats, and system information.\n"
                        "Use --verbose or -v for detailed information.",
            .function = cmd_status,
            .flags = status_flags,
            .flag_count = 1,
            .min_args = 0,
            .max_args = 0,
            .hidden = false
        };
        if (command_registry_register(registry, &info)) {
            registered++;
        }
    }

    /* Quit command */
    {
        CommandInfo info = {
            .name = "quit",
            .description = "Exit the game",
            .usage = "quit",
            .help_text = "Exits the game gracefully, saving any necessary state.",
            .function = cmd_quit,
            .flags = NULL,
            .flag_count = 0,
            .min_args = 0,
            .max_args = 0,
            .hidden = false
        };
        if (command_registry_register(registry, &info)) {
            registered++;
        }
    }

    /* Exit command (alias for quit) */
    {
        CommandInfo info = {
            .name = "exit",
            .description = "Exit the game",
            .usage = "exit",
            .help_text = "Exits the game gracefully, saving any necessary state.",
            .function = cmd_quit,
            .flags = NULL,
            .flag_count = 0,
            .min_args = 0,
            .max_args = 0,
            .hidden = false
        };
        if (command_registry_register(registry, &info)) {
            registered++;
        }
    }

    /* Clear command */
    {
        CommandInfo info = {
            .name = "clear",
            .description = "Clear the terminal screen",
            .usage = "clear",
            .help_text = "Clears the terminal screen using ANSI escape codes.",
            .function = cmd_clear,
            .flags = NULL,
            .flag_count = 0,
            .min_args = 0,
            .max_args = 0,
            .hidden = false
        };
        if (command_registry_register(registry, &info)) {
            registered++;
        }
    }

    /* Log command */
    {
        static FlagDefinition log_flags[] = {
            {
                .name = "file",
                .short_name = 'f',
                .type = ARG_TYPE_STRING,
                .required = false,
                .description = "Set log file path"
            }
        };

        CommandInfo info = {
            .name = "log",
            .description = "Manage logging settings",
            .usage = "log <level> [--file <path>]",
            .help_text = "Changes the logging level and optionally the log file.\n"
                        "Levels: trace, debug, info, warn, error, fatal\n"
                        "Without arguments, shows current log level.",
            .function = cmd_log,
            .flags = log_flags,
            .flag_count = 1,
            .min_args = 0,
            .max_args = 1,
            .hidden = false
        };
        if (command_registry_register(registry, &info)) {
            registered++;
        }
    }

    return registered;
}

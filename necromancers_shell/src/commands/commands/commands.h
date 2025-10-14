#ifndef COMMANDS_H
#define COMMANDS_H

#include "../executor.h"
#include "../parser.h"

/**
 * Built-in Commands
 *
 * Standard commands available in Necromancer's Shell:
 * - help: Display help information
 * - status: Show game state and statistics
 * - quit/exit: Exit the game
 * - clear: Clear the terminal screen
 * - log: Manage logging settings
 */

/**
 * Help Command
 * Usage: help [command]
 *
 * Displays help information. Without arguments, shows all commands.
 * With argument, shows detailed help for specific command.
 */
CommandResult cmd_help(ParsedCommand* cmd);

/**
 * Status Command
 * Usage: status [--verbose]
 *
 * Shows current game state, player stats, and system information.
 */
CommandResult cmd_status(ParsedCommand* cmd);

/**
 * Quit Command
 * Usage: quit
 *
 * Exits the game gracefully.
 */
CommandResult cmd_quit(ParsedCommand* cmd);

/**
 * Clear Command
 * Usage: clear
 *
 * Clears the terminal screen.
 */
CommandResult cmd_clear(ParsedCommand* cmd);

/**
 * Log Command
 * Usage: log <level> [--file <path>]
 *
 * Manages logging settings. Can set log level and log file.
 */
CommandResult cmd_log(ParsedCommand* cmd);

/**
 * Register all built-in commands
 *
 * @param registry Command registry to register commands to
 * @return Number of commands registered
 */
int register_builtin_commands(struct CommandRegistry* registry);

#endif /* COMMANDS_H */

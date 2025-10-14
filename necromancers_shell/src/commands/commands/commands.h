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

/**
 * Game Commands (Phase 2)
 */

/**
 * Souls Command
 * Usage: souls [--type <type>] [--quality-min <n>] [--quality-max <n>] [--bound] [--free] [--sort <criteria>]
 *
 * Display soul inventory with filtering and sorting.
 */
CommandResult cmd_souls(ParsedCommand* cmd);

/**
 * Harvest Command
 * Usage: harvest [--count <n>]
 *
 * Harvest souls from corpses at the current location.
 */
CommandResult cmd_harvest(ParsedCommand* cmd);

/**
 * Scan Command
 * Usage: scan
 *
 * Show connected locations from current location.
 */
CommandResult cmd_scan(ParsedCommand* cmd);

/**
 * Probe Command
 * Usage: probe <location>
 *
 * Get detailed information about a location.
 */
CommandResult cmd_probe(ParsedCommand* cmd);

/**
 * Connect Command
 * Usage: connect <location>
 *
 * Travel to a connected location.
 */
CommandResult cmd_connect(ParsedCommand* cmd);

/**
 * Raise Command
 * Usage: raise <type> [name] [--soul <id>]
 *
 * Raise an undead minion from corpses.
 */
CommandResult cmd_raise(ParsedCommand* cmd);

/**
 * Bind Command
 * Usage: bind <minion_id> <soul_id>
 *
 * Bind a soul to a minion for stat bonuses.
 */
CommandResult cmd_bind(ParsedCommand* cmd);

/**
 * Banish Command
 * Usage: banish <minion_id>
 *
 * Banish (destroy) a minion and return its soul.
 */
CommandResult cmd_banish(ParsedCommand* cmd);

/**
 * Minions Command
 * Usage: minions
 *
 * Display all minions in the army.
 */
CommandResult cmd_minions(ParsedCommand* cmd);

#endif /* COMMANDS_H */

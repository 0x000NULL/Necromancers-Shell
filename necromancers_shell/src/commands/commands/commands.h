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

/**
 * Map Command
 * Usage: map [--width <n>] [--height <n>] [--no-legend] [--show-all]
 *
 * Display ASCII world map with current location.
 */
CommandResult cmd_map(ParsedCommand* cmd);

/**
 * Route Command
 * Usage: route <location_name|location_id> [--show-map]
 *
 * Plot pathfinding route to destination.
 */
CommandResult cmd_route(ParsedCommand* cmd);

/**
 * Game Commands (Phase 3 - Progression)
 */

/**
 * Research Command
 * Usage: research [info|start|current|cancel|completed] [<project_id>]
 *
 * Manage research projects. View available projects, start research, check progress.
 */
CommandResult cmd_research(ParsedCommand* cmd);

/**
 * Upgrade Command
 * Usage: upgrade [info|unlock|branch|unlocked|reset] [<skill_id>|<branch_name>]
 *
 * Manage skill tree. View and unlock skills, browse branches.
 */
CommandResult cmd_upgrade(ParsedCommand* cmd);

/**
 * Skills Command
 * Usage: skills [bonuses|abilities|branch <name>]
 *
 * View active skills and bonuses. Display stat bonuses and unlocked abilities.
 */
CommandResult cmd_skills(ParsedCommand* cmd);

/**
 * Game Commands (Phase 5 - Narrative)
 */

/**
 * Memory Command
 * Usage: memory [view <id>|stats]
 *
 * View discovered memory fragments. Shows your past memories and lore.
 */
CommandResult cmd_memory(ParsedCommand* cmd);

/**
 * Game Commands (Phase 6 - Story Foundation)
 */

/**
 * Message Command
 * Usage: message <npc_id> <message>
 *
 * Send messages to NPCs (Regional Council, Thessara, gods).
 */
CommandResult cmd_message(ParsedCommand* cmd);

/**
 * Invoke Command
 * Usage: invoke <god_name> [--offering <amount>]
 *
 * Invoke Divine Architects or perform divine summoning rituals.
 */
CommandResult cmd_invoke(ParsedCommand* cmd);

/**
 * Ritual Command
 * Usage: ritual <type> [options]
 *
 * Perform necromantic rituals (phylactery, trial, offering, purification).
 */
CommandResult cmd_ritual(ParsedCommand* cmd);

/**
 * Free Command
 * Usage: free <soul_id> [--permanent]
 *
 * Release bound souls from minions or free souls to afterlife.
 */
CommandResult cmd_free(ParsedCommand* cmd);

/**
 * Heal Command
 * Usage: heal <minion_id> [--amount <hp>] [--use-mana]
 *
 * Heal damaged minions using soul energy or mana.
 */
CommandResult cmd_heal(ParsedCommand* cmd);

/**
 * Game Commands (Phase 4 - Combat)
 */

/**
 * Attack Command
 * Usage: attack <target_id>
 *
 * Attack an enemy combatant during combat.
 */
CommandResult cmd_attack(ParsedCommand* cmd);

/**
 * Defend Command
 * Usage: defend
 *
 * Take a defensive stance in combat (+50% defense).
 */
CommandResult cmd_defend(ParsedCommand* cmd);

/**
 * Flee Command
 * Usage: flee
 *
 * Attempt to escape from combat.
 */
CommandResult cmd_flee(ParsedCommand* cmd);

/**
 * Cast Command
 * Usage: cast <spell> <target>
 *
 * Cast a combat spell on a target.
 */
CommandResult cmd_cast_combat(ParsedCommand* cmd);

/**
 * Register combat commands
 *
 * @param registry Command registry to register commands to
 * @return Number of commands registered
 */
int register_combat_commands(struct CommandRegistry* registry);

/**
 * Game Commands (Sprint 3 - Narrative Integration)
 */

/**
 * Dialogue Command
 * Usage: dialogue [npc_id] [choice]
 *
 * Talk to NPCs and manage conversations.
 */
CommandResult cmd_dialogue(ParsedCommand* cmd);

/**
 * Quest Command
 * Usage: quest [subcommand] [quest_id]
 *
 * Manage quests and objectives.
 */
CommandResult cmd_quest(ParsedCommand* cmd);

/**
 * Lore Command
 * Usage: lore [memory_id] [research]
 *
 * View lore and research memories.
 */
CommandResult cmd_lore(ParsedCommand* cmd);

/**
 * Council Command
 * Usage: council [god_id|summon]
 *
 * View Divine Council and god favor.
 */
CommandResult cmd_council(ParsedCommand* cmd);

/**
 * Path Command
 * Usage: path [path_id] [choose]
 *
 * View and select transformation paths.
 */
CommandResult cmd_path(ParsedCommand* cmd);

/**
 * Register new command functions
 */
void register_dialogue_command(void);
void register_quest_command(void);
void register_lore_command(void);
void register_council_command(void);
void register_path_command(void);

#endif /* COMMANDS_H */

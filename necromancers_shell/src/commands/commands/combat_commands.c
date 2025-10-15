/**
 * @file combat_commands.c
 * @brief Registration for all combat commands
 */

#include "commands.h"
#include "../registry.h"

/**
 * @brief Register all combat commands
 */
int register_combat_commands(CommandRegistry* registry) {
    if (!registry) {
        return 0;
    }

    int registered = 0;

    /* Attack command */
    {
        CommandInfo info = {
            .name = "attack",
            .description = "Attack an enemy in combat",
            .usage = "attack <target_id>",
            .help_text =
                "Attack an enemy combatant during your turn.\n"
                "\n"
                "Target ID format: E1, E2, E3, etc.\n"
                "Use 'status' to see available targets.\n"
                "\n"
                "Example: attack E1",
            .function = cmd_attack,
            .flags = NULL,
            .flag_count = 0,
            .min_args = 1,
            .max_args = 1,
            .hidden = false
        };
        if (command_registry_register(registry, &info)) {
            registered++;
        }
    }

    /* Defend command */
    {
        CommandInfo info = {
            .name = "defend",
            .description = "Take a defensive stance in combat",
            .usage = "defend",
            .help_text =
                "Enter a defensive stance for the current turn.\n"
                "\n"
                "Effects:\n"
                "  • Defense increased by 50%%\n"
                "  • Lasts until start of next turn\n"
                "  • Reduces incoming damage\n"
                "\n"
                "Use when low on HP or expecting strong attacks.\n"
                "\n"
                "Example: defend",
            .function = cmd_defend,
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

    /* Flee command */
    {
        CommandInfo info = {
            .name = "flee",
            .description = "Attempt to escape from combat",
            .usage = "flee",
            .help_text =
                "Attempt to flee from combat and return to the world.\n"
                "\n"
                "Success Chance:\n"
                "  • Base: 50%%\n"
                "  • +10%% per dead enemy\n"
                "  • -20%% if any ally HP < 30%%\n"
                "\n"
                "Success: Escape combat, return to world\n"
                "Failure: Enemies get free attacks, combat continues\n"
                "\n"
                "Use when overwhelmed or low on resources.\n"
                "\n"
                "Example: flee",
            .function = cmd_flee,
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

    /* Cast command */
    {
        CommandInfo info = {
            .name = "cast",
            .description = "Cast a combat spell",
            .usage = "cast <spell> <target>",
            .help_text =
                "Cast a spell in combat.\n"
                "\n"
                "Available Spells:\n"
                "  drain <target>  - Deal 15 necrotic damage, heal self for 50%%\n"
                "                    Cost: 15 mana\n"
                "\n"
                "  bolt <target>   - Deal 20 pure damage (ignores defense)\n"
                "                    Cost: 20 mana\n"
                "\n"
                "  weaken <target> - Reduce target defense by 20%% for 2 turns\n"
                "                    Cost: 10 mana\n"
                "\n"
                "Examples:\n"
                "  cast drain E1\n"
                "  cast bolt E2\n"
                "  cast weaken E1",
            .function = cmd_cast_combat,
            .flags = NULL,
            .flag_count = 0,
            .min_args = 2,
            .max_args = 2,
            .hidden = false
        };
        if (command_registry_register(registry, &info)) {
            registered++;
        }
    }

    return registered;
}

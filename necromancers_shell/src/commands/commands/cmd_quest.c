/**
 * @file cmd_quest.c
 * @brief Quest command implementation
 *
 * Handles quest management and tracking.
 */

#include "../../game/game_state.h"
#include "../../terminal/ui_feedback.h"
#include "../command_system.h"
#include "../parser.h"
#include <stdio.h>
#include <string.h>

/**
 * @brief Manage quests and objectives
 *
 * Usage:
 *   quest                    - List active quests
 *   quest <quest_id>         - View quest details
 *   quest available          - Show available quests
 *   quest completed          - Show completed quests
 *   quest track <quest_id>   - Track/untrack quest
 */
CommandResult cmd_quest(ParsedCommand* cmd) {
    extern GameState* g_game_state;

    if (!g_game_state) {
        return command_result_error(EXEC_ERROR_INTERNAL, "Game state not initialized");
    }

    size_t arg_count = 0;
    for (size_t i = 0; i < 10; i++) {
        if (parsed_command_get_arg(cmd, i) != NULL) {
            arg_count++;
        } else {
            break;
        }
    }

    /* No arguments - list active quests */
    if (arg_count == 0) {
        printf("\n");printf("═══════════════════════════════════════════════════════\n");printf("            Active Quests\n");printf("═══════════════════════════════════════════════════════\n");printf("\n");

        /* TODO: Load quest data and show active quests */
        printf("Main Story:\n");
        printf("  - [ACTIVE] Stabilize Consciousness (70%% required)\n");
        printf("        └─ Current: %.1f%%\n", g_game_state->consciousness.stability);

        printf("\n");
        printf("Use: quest <quest_id> for details\n");
        printf("Use: quest available to see new quests\n");

        return command_result_success("");
    }

    const char* arg1 = parsed_command_get_arg(cmd, 0);

    /* Show available quests */
    if (strcmp(arg1, "available") == 0) {
        printf("\n");printf("═══════════════════════════════════════════════════════\n");printf("            Available Quests\n");printf("═══════════════════════════════════════════════════════\n");printf("\n");

        /* TODO: Load and filter available quests */
        printf("New quests you can start:\n");
        printf("  - first_harvest - The First Harvest\n");
        printf("        └─ Harvest 5 souls from Forgotten Graveyard\n");

        printf("\n");
        printf("Quest system integration pending.\n");

        return command_result_success("");
    }

    /* Show completed quests */
    if (strcmp(arg1, "completed") == 0) {
        printf("\n");printf("═══════════════════════════════════════════════════════\n");printf("            Completed Quests\n");printf("═══════════════════════════════════════════════════════\n");printf("\n");

        /* TODO: Load and show completed quests */
        printf("No quests completed yet.\n");
        printf("\n");
        printf("Complete quests to build your legacy.\n");

        return command_result_success("");
    }

    /* Track/untrack quest */
    if (strcmp(arg1, "track") == 0) {
        if (arg_count < 2) {
            return command_result_error(EXEC_ERROR_COMMAND_FAILED, "Usage: quest track <quest_id>");
        }

        const char* quest_id = parsed_command_get_arg(cmd, 1);
        printf("Tracking quest: %s", quest_id);

        /* TODO: Implement quest tracking */
        printf("Quest tracking integration pending.\n");

        return command_result_success("");
    }

    /* Show specific quest details */
    const char* quest_id = arg1;

    printf("\n");printf("═══════════════════════════════════════════════════════\n");printf("            Quest Details\n");printf("═══════════════════════════════════════════════════════\n");printf("\n");
    printf("Quest: %s", quest_id);
    printf("\n");

    /* TODO: Load quest data and display details */
    printf("[Quest details will be loaded from data/quests.dat]\n");
    printf("\n");
    printf("Quest system integration coming in next sprint.\n");

    return command_result_success("");
}

/**
 * @brief Register quest command
 */
void register_quest_command(void) {
    static FlagDefinition quest_flags[] = {
        {
            .name = "verbose",
            .short_name = 'v',
            .type = ARG_TYPE_BOOL,
            .required = false,
            .description = "Show detailed quest information"
        },
        {
            .name = "type",
            .short_name = 't',
            .type = ARG_TYPE_STRING,
            .required = false,
            .description = "Filter by quest type (main, redemption, trial, etc.)"
        }
    };

    CommandInfo quest_cmd = {
        .name = "quest",
        .description = "Manage quests and objectives",
        .usage = "quest [subcommand] [quest_id]",
        .help_text =
            "Quest management:\n"
            "  quest                  - List active quests\n"
            "  quest <quest_id>       - View quest details\n"
            "  quest available        - Show available quests\n"
            "  quest completed        - Show completed quests\n"
            "  quest track <id>       - Track/untrack quest\n"
            "\n"
            "Quest Types:\n"
            "  main_story    - Main storyline quests\n"
            "  redemption    - Redemption path quests\n"
            "  archon_trial  - Archon trial quests\n"
            "  lich_lord     - Lich Lord path quests\n"
            "  reaper        - Reaper path quests\n"
            "\n"
            "Examples:\n"
            "  quest                      - Show active quests\n"
            "  quest first_harvest        - View harvest quest details\n"
            "  quest --type=redemption    - Show redemption quests",
        .function = cmd_quest,
        .flags = quest_flags,
        .flag_count = 2,
        .min_args = 0,
        .max_args = 2,
        .hidden = false
    };

    command_system_register_command(&quest_cmd);
}

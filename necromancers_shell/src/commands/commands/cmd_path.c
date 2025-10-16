/**
 * @file cmd_path.c
 * @brief Transformation path command implementation
 *
 * Handles path selection and progression tracking.
 */

#include "../../game/game_state.h"
#include "../../terminal/ui_feedback.h"
#include "../command_system.h"
#include "../parser.h"
#include <stdio.h>
#include <string.h>

/**
 * @brief View and select transformation paths
 *
 * Usage:
 *   path                    - Show available paths
 *   path <path_id>          - View path details
 *   path choose <path_id>   - Select your path
 *   path progress           - View progress on current path
 */
CommandResult cmd_path(ParsedCommand* cmd) {
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
    double corruption = (double)g_game_state->corruption.corruption;

    /* No arguments - show available paths */
    if (arg_count == 0) {
        printf("\n");printf("═══════════════════════════════════════════════════════\n");printf("            Transformation Paths\n");printf("═══════════════════════════════════════════════════════\n");printf("\n");
        printf("Six paths diverge. Your corruption level determines availability.\n");
        printf("\n");

        printf("Current Corruption: %.1f%%", corruption);
        printf("\n");

        /* Revenant Path */
        if (corruption < 70) {
            printf("[AVAILABLE] Revenant - Return to Life\n");
            printf("            └─ Redemption through sustained ethics\n");
            printf("            └─ Requires: <30%% corruption, 5 redemption quests\n");
        } else {
            printf("[LOCKED] Revenant - Corruption too high (>70%%)\n");
        }
        printf("\n");

        /* Lich Lord Path */
        if (corruption > 50) {
            printf("[AVAILABLE] Lich Lord - Embrace Undeath\n");
            printf("            └─ Immortal power through corruption\n");
            printf("            └─ Requires: >50%% corruption, 100 minions\n");
        } else {
            printf("[UNAVAILABLE] Lich Lord - Requires >50%% corruption\n");
        }
        printf("\n");

        /* Reaper Path */
        if (corruption >= 40 && corruption <= 69) {
            printf("[AVAILABLE] Reaper - Eternal Service\n");
            printf("            └─ Purpose in endless duty\n");
            printf("            └─ Requires: 40-69%% corruption, guide 1000 souls\n");
        } else {
            printf("[UNAVAILABLE] Reaper - Requires 40-69%% corruption\n");
        }
        printf("\n");

        /* Hidden paths hint */
        printf("[HIDDEN] Three additional paths exist...\n");
        printf("         Discover them through exploration and lore\n");

        printf("\n");
        printf("Use: path <path_id> for details\n");
        printf("Use: path choose <path_id> to commit\n");

        return command_result_success("");
    }

    const char* arg1 = parsed_command_get_arg(cmd, 0);

    /* Choose path */
    if (strcmp(arg1, "choose") == 0) {
        if (arg_count < 2) {
            return command_result_error(EXEC_ERROR_COMMAND_FAILED, "Usage: path choose <path_id>");
        }

        const char* path_id = parsed_command_get_arg(cmd, 1);

        /* TODO: Implement path selection system */
        printf("Choosing path: %s", path_id);
        printf("\n");
        printf("This is a permanent decision. You cannot change paths once chosen.\n");
        printf("\n");
        printf("Path selection system integration pending.\n");

        return command_result_success("");
    }

    /* Show progress */
    if (strcmp(arg1, "progress") == 0) {
        printf("\n");printf("═══════════════════════════════════════════════════════\n");printf("            Path Progress\n");printf("═══════════════════════════════════════════════════════\n");printf("\n");

        /* TODO: Load current path and show progress */
        printf("No path selected yet.\n");
        printf("\n");
        printf("Choose a path to begin your transformation.\n");

        return command_result_success("");
    }

    /* Show specific path details */
    const char* path_id = arg1;

    printf("\n");printf("═══════════════════════════════════════════════════════\n");printf("            Path Details\n");printf("═══════════════════════════════════════════════════════\n");printf("\n");

    /* TODO: Load path data from data/paths.dat */
    if (strcmp(path_id, "revenant") == 0) {
        printf("The Revenant Route\n");
        printf("Subtitle: Redemption\n");
        printf("\n");

        printf("Return to life. Reclaim mortality. Escape undeath and live\n");
        printf("again as a human being.\n");
        printf("\n");

        printf("Requirements:\n");
        printf("  - Corruption: <30%% (current: %.1f%%)\n", corruption);
        printf("  - Soul Energy: 15,000\n");
        printf("  - Consciousness: >90%% (current: %.1f%%)\n",
                        g_game_state->consciousness.stability);
        printf("  - Complete: 5 redemption quests\n");
        printf("  - Research: Resurrection Protocol\n");

        printf("\n");
        printf("Outcome:\n");
        printf("You wake up. Breathing. Heart beating. Mortal again.\n");
        printf("37 years of life remaining. Make them count.\n");

        printf("\n");
        printf("Difficulty: Moderate\n");
        printf("Moral Alignment: Good\n");

    } else if (strcmp(path_id, "lich_lord") == 0) {
        printf("The Lich Lord Route\n");
        printf("Subtitle: Apotheosis Through Power\n");
        printf("\n");

        printf("Embrace undeath completely. Become immortal Lich Lord.\n");
        printf("Perfect efficiency. Perfect emptiness. Forever.\n");
        printf("\n");

        printf("Requirements:\n");
        printf("  - Corruption: >50%% (current: %.1f%%)\n", corruption);
        printf("  - Soul Energy: 20,000\n");
        printf("  - Raise: 100+ minions\n");
        printf("  - Conquer: 10+ territories\n");
        printf("  - Create: Phylactery (10 ancient souls)\n");

        printf("\n");
        printf("Outcome:\n");
        printf("Humanity permanently lost. Emotions die completely.\n");
        printf("Immortal undeath. Eventually sealed by gods.\n");

        printf("\n");
        printf("This is considered the 'bad ending'\n");
        printf("Difficulty: Moderate\n");
        printf("Moral Alignment: Evil\n");

    } else if (strcmp(path_id, "reaper") == 0) {
        printf("The Reaper Route\n");
        printf("Subtitle: Service Without End\n");
        printf("\n");

        printf("Become an eternal psychopomp. Official Death Network\n");
        printf("administrator. Guide souls forever. Peace or prison?\n");
        printf("\n");

        printf("Requirements:\n");
        printf("  - Corruption: 40-69%% (current: %.1f%%)\n", corruption);
        printf("  - Soul Energy: 25,000\n");
        printf("  - Guide: 1,000 souls without harvesting\n");
        printf("  - Complete: All lore research\n");
        printf("  - Pass: Reaper trials\n");

        printf("\n");
        printf("Outcome:\n");
        printf("Eternal duty. Purpose in endless service.\n");
        printf("Constrained freedom. Meaning forever.\n");

        printf("\n");
        printf("Difficulty: Hard\n");
        printf("Moral Alignment: Lawful Neutral\n");

    } else {
        printf("[Path details will be loaded from data/paths.dat]\n");
        printf("\n");
        printf("Path system integration coming in next sprint.\n");
    }

    return command_result_success("");
}

/**
 * @brief Register path command
 */
void register_path_command(void) {
    static FlagDefinition path_flags[] = {
        {
            .name = "verbose",
            .short_name = 'v',
            .type = ARG_TYPE_BOOL,
            .required = false,
            .description = "Show detailed path information"
        },
        {
            .name = "all",
            .short_name = 'a',
            .type = ARG_TYPE_BOOL,
            .required = false,
            .description = "Show all paths including hidden ones"
        }
    };

    CommandInfo path_cmd = {
        .name = "path",
        .description = "View and select transformation paths",
        .usage = "path [path_id] [choose]",
        .help_text =
            "Transformation path management:\n"
            "  path                  - Show available paths\n"
            "  path <path_id>        - View path details\n"
            "  path choose <id>      - Select your path\n"
            "  path progress         - View current progress\n"
            "\n"
            "Six Paths:\n"
            "  revenant     - Return to mortal life\n"
            "  lich_lord    - Immortal undeath\n"
            "  reaper       - Eternal service\n"
            "  archon       - Revolutionary (HIDDEN)\n"
            "  wraith       - Distributed freedom (HIDDEN)\n"
            "  morningstar  - Divine ascension (HIDDEN)\n"
            "\n"
            "Path Requirements:\n"
            "  - Corruption level determines availability\n"
            "  - Each path has unique prerequisites\n"
            "  - Choice is permanent once committed\n"
            "\n"
            "Examples:\n"
            "  path                     - Show available paths\n"
            "  path revenant            - View Revenant details\n"
            "  path choose revenant     - Commit to Revenant path\n"
            "  path --all               - Show hidden paths",
        .function = cmd_path,
        .flags = path_flags,
        .flag_count = 2,
        .min_args = 0,
        .max_args = 2,
        .hidden = false
    };

    command_system_register_command(&path_cmd);
}

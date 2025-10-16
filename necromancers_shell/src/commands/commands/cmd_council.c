/**
 * @file cmd_council.c
 * @brief Divine Council command implementation
 *
 * Handles Divine Council status and god favor.
 */

#include "commands.h"
#include "../../game/game_state.h"
#include "../../terminal/ui_feedback.h"
#include "../command_system.h"
#include "../parser.h"
#include <stdio.h>
#include <string.h>

/* Include god headers from parent src directory */
#include "game/narrative/gods/god.h"
#include "game/narrative/gods/divine_council.h"

/**
 * @brief View Divine Council status and god favor
 *
 * Usage:
 *   council                 - Show all gods and favor
 *   council <god_id>        - View specific god details
 *   council summon          - Check if summons is available
 */
CommandResult cmd_council(ParsedCommand* cmd) {
    extern GameState* g_game_state;

    if (!g_game_state || !g_game_state->divine_council) {
        return command_result_error(EXEC_ERROR_INTERNAL, "Divine Council not initialized");
    }

    DivineCouncil* council = g_game_state->divine_council;
    size_t arg_count = 0;

    /* Count arguments manually */
    for (size_t i = 0; i < 10; i++) {
        if (parsed_command_get_arg(cmd, i) != NULL) {
            arg_count++;
        } else {
            break;
        }
    }

    /* No arguments - show all gods */
    if (arg_count == 0) {
        printf("\n");
        printf("═══════════════════════════════════════════════════════\n");
        printf("            THE DIVINE COUNCIL\n");
        printf("═══════════════════════════════════════════════════════\n");
        printf("\n");
        printf("The Seven Divine Architects\n\n");

        /* Display each god */
        for (size_t i = 0; i < council->god_count; i++) {
            const God* god = council->gods[i];
            if (!god) continue;

            const char* favor_desc = god_get_favor_description(god);
            const char* domain = god_domain_name(god->domain);

            printf("%-15s (%s)\n", god->name, domain);
            printf("  Favor: %+3d - %s\n\n", god->favor, favor_desc);
        }

        /* Average favor */
        printf("Average Favor: %d\n", (int)council->average_favor);
        printf("Total Interactions: %u\n", council->total_interactions);

        /* Summon status */
        printf("\n");
        if (g_game_state->resources.day_count >= 162) {
            printf("Divine Council summons available!\n");
            printf("    Use: council summon for details\n");
        } else {
            printf("Summons available after day 162\n");
            printf("Current day: %u\n", g_game_state->resources.day_count);
        }

        return command_result_success("");
    }

    const char* arg1 = parsed_command_get_arg(cmd, 0);

    /* Check summon status */
    if (strcmp(arg1, "summon") == 0) {
        printf("\n");
        printf("═══════════════════════════════════════════════════════\n");
        printf("            DIVINE SUMMONS\n");
        printf("═══════════════════════════════════════════════════════\n");
        printf("\n");

        if (g_game_state->resources.day_count < 162) {
            printf("The Divine Council has not yet noticed you.\n");
            printf("Day %u of 162 required\n", g_game_state->resources.day_count);
            return command_result_success("");
        }

        printf("The Divine Council is ready to summon you!\n\n");
        printf("This will trigger a major story event where the Seven\n");
        printf("Architects will judge your actions and determine your fate.\n\n");
        printf("Your current favor levels will determine the verdict.\n");
        printf("Use 'dialogue keldrin' to accept or decline the summons.\n");

        return command_result_success("");
    }

    /* Show specific god details */
    const char* god_id = arg1;
    const God* god = divine_council_find_god(council, god_id);

    if (!god) {
        return command_result_error(EXEC_ERROR_COMMAND_FAILED, "God not found");
    }

    printf("\n");
    printf("═══════════════════════════════════════════════════════\n");
    printf("            %s\n", god->name);
    printf("═══════════════════════════════════════════════════════\n");
    printf("\n");
    printf("%s, %s\n\n", god->title, god_domain_name(god->domain));

    /* Favor status */
    const char* favor_desc = god_get_favor_description(god);
    printf("Favor: %+3d - %s\n\n", god->favor, favor_desc);

    printf("Power Level: %s\n", god_power_level_name(god->power_level));
    printf("Interactions: %u\n", god->interactions);

    /* Description */
    printf("\n─────────────────────────────────────────────────────────\n");
    printf("%s\n", god->description);
    printf("─────────────────────────────────────────────────────────\n");

    return command_result_success("");
}

/**
 * @brief Register council command
 */
void register_council_command(void) {
    CommandInfo council_cmd = {
        .name = "council",
        .description = "View Divine Council and god favor",
        .usage = "council [god_id|summon]",
        .help_text =
            "Divine Council management:\n"
            "  council              - Show all Seven Architects\n"
            "  council <god_id>     - View specific god details\n"
            "  council summon       - Check summon status\n"
            "\n"
            "The Seven Divine Architects:\n"
            "  anara     - Goddess of Life\n"
            "  keldrin   - God of Order\n"
            "  theros    - God of Time\n"
            "  myrith    - Goddess of Souls\n"
            "  vorathos  - God of Entropy\n"
            "  seraph    - Goddess of Boundaries\n"
            "  nexus     - God of Networks\n"
            "\n"
            "Favor affects:\n"
            "  - Divine judgment verdict\n"
            "  - Archon path accessibility\n"
            "  - Divine intervention in crises\n"
            "\n"
            "Examples:\n"
            "  council             - Show all gods\n"
            "  council anara       - View Anara details\n"
            "  council summon      - Check if summons ready",
        .function = cmd_council,
        .flags = NULL,
        .flag_count = 0,
        .min_args = 0,
        .max_args = 1,
        .hidden = false
    };

    command_system_register_command(&council_cmd);
}

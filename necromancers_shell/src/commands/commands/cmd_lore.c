/**
 * @file cmd_lore.c
 * @brief Lore command implementation
 *
 * Handles memory fragments and lore discovery.
 */

#include "../../game/game_state.h"
#include "../../terminal/ui_feedback.h"
#include "../command_system.h"
#include "../parser.h"
#include <stdio.h>
#include <string.h>

/**
 * @brief View discovered lore and research new memories
 *
 * Usage:
 *   lore                    - List discovered lore
 *   lore <memory_id>        - Read specific memory
 *   lore research           - Show researchable memories
 *   lore research <id>      - Research new memory fragment
 */
CommandResult cmd_lore(ParsedCommand* cmd) {
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

    /* No arguments - list discovered lore */
    if (arg_count == 0) {
        printf("\n");printf("═══════════════════════════════════════════════════════\n");printf("            Discovered Lore\n");printf("═══════════════════════════════════════════════════════\n");printf("\n");

        /* TODO: Load memory data and show discovered entries */
        printf("Memories Unlocked:\n");
        printf("  - [PERSONAL] player_death - Terminal Before Death\n");
        printf("  - [PERSONAL] first_fear - Consciousness Fragmentation Terror\n");

        printf("\n");
        printf("Use: lore <memory_id> to read\n");
        printf("Use: lore research to find new memories\n");

        return command_result_success("");
    }

    const char* arg1 = parsed_command_get_arg(cmd, 0);

    /* Show researchable memories */
    if (strcmp(arg1, "research") == 0) {
        /* Research new memory if ID provided */
        if (arg_count >= 2) {
            const char* memory_id = parsed_command_get_arg(cmd, 1);

            /* TODO: Check requirements and cost */
            printf("Researching: %s", memory_id);
            printf("\n");

            printf("Cost: 50 soul energy, 12 hours\n");
            printf("This will unlock new lore and insights.\n");
            printf("\n");

            /* TODO: Implement research system */
            printf("Lore research system integration pending.\n");

            return command_result_success("");
        }

        /* List available research */
        printf("\n");printf("═══════════════════════════════════════════════════════\n");printf("            Researchable Memories\n");printf("═══════════════════════════════════════════════════════\n");printf("\n");

        printf("Historical Lore:\n");
        printf("  - first_death - The First Death (50 energy, 12 hours)\n");
        printf("        └─ Learn how the Death Network was created\n");

        printf("\n");
        printf("  - thessara_origin - The First Necromancer (100 energy, 24 hours)\n");
        printf("        └─ Thessara's story and how she became a ghost\n");

        printf("\n");
        printf("Use: lore research <memory_id> to unlock\n");

        return command_result_success("");
    }

    /* Read specific memory */
    const char* memory_id = arg1;

    printf("\n");printf("═══════════════════════════════════════════════════════\n");printf("            Memory Fragment\n");printf("═══════════════════════════════════════════════════════\n");printf("\n");
    printf("Memory: %s", memory_id);
    printf("\n");

    /* TODO: Load memory data and display content */
    if (strcmp(memory_id, "player_death") == 0) {
        printf("═══════════════════════════════════════════════════════\n");
        printf("\n");
        printf("Terminal Before Death\n");
        printf("\n");
        printf("You remember dying.\n");
        printf("\n");
        printf("It wasn't dramatic. You were at your desk. Terminal open.\n");
        printf("Code review in progress.\n");
        printf("\n");
        printf("Sudden headache. Cerebral hemorrhage. Dead before you hit the floor.\n");
        printf("\n");
        printf("Last conscious thought: \"I haven't merged that pull request.\"\n");
        printf("\n");
        printf("Then you woke up in the Death Network. With an administrative interface.\n");
        printf("\n");
        printf("═══════════════════════════════════════════════════════\n");
        printf("\n");

        return command_result_success("");
    }

    printf("[Memory content will be loaded from data/memories.dat]\n");
    printf("\n");
    printf("Lore system integration coming in next sprint.\n");

    return command_result_success("");
}

/**
 * @brief Register lore command
 */
void register_lore_command(void) {
    static FlagDefinition lore_flags[] = {
        {
            .name = "type",
            .short_name = 't',
            .type = ARG_TYPE_STRING,
            .required = false,
            .description = "Filter by type (personal, historical, technical, philosophical)"
        },
        {
            .name = "all",
            .short_name = 'a',
            .type = ARG_TYPE_BOOL,
            .required = false,
            .description = "Show all memories including locked ones"
        }
    };

    CommandInfo lore_cmd = {
        .name = "lore",
        .description = "View lore and research memories",
        .usage = "lore [memory_id] [research]",
        .help_text =
            "Lore and memory management:\n"
            "  lore                  - List discovered lore\n"
            "  lore <memory_id>      - Read specific memory\n"
            "  lore research         - Show researchable memories\n"
            "  lore research <id>    - Research new memory\n"
            "\n"
            "Memory Types:\n"
            "  personal      - Your backstory and experiences\n"
            "  historical    - Ancient events and history\n"
            "  technical     - Death Network technical info\n"
            "  philosophical - Deep thoughts on consciousness\n"
            "\n"
            "Examples:\n"
            "  lore                        - Show discovered lore\n"
            "  lore player_death           - Read your death memory\n"
            "  lore research first_death   - Research First Death lore\n"
            "  lore --type=historical      - Show historical memories",
        .function = cmd_lore,
        .flags = lore_flags,
        .flag_count = 2,
        .min_args = 0,
        .max_args = 2,
        .hidden = false
    };

    command_system_register_command(&lore_cmd);
}

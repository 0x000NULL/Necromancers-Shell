/**
 * @file cmd_dialogue.c
 * @brief Dialogue command implementation
 *
 * Handles NPC conversations and dialogue trees.
 */

#include "../../game/game_state.h"
#include "../../terminal/ui_feedback.h"
#include "../command_system.h"
#include "../parser.h"
#include <stdio.h>
#include <string.h>

/**
 * @brief Talk to an NPC or view available conversations
 *
 * Usage:
 *   dialogue                    - List available NPCs to talk to
 *   dialogue <npc_id>           - Start conversation with NPC
 *   dialogue history            - View past conversations
 *   dialogue <npc_id> <choice>  - Make dialogue choice
 */
CommandResult cmd_dialogue(ParsedCommand* cmd) {
    extern GameState* g_game_state;

    if (!g_game_state) {
        return command_result_error(EXEC_ERROR_INTERNAL, "Game state not initialized");
    }

    /* Count arguments manually */
    size_t arg_count = 0;
    for (size_t i = 0; i < 10; i++) {
        if (parsed_command_get_arg(cmd, i) != NULL) {
            arg_count++;
        } else {
            break;
        }
    }

    /* No arguments - list available NPCs */
    if (arg_count == 0) {
        printf("\n");
        printf("═══════════════════════════════════════════════════════\n");
        printf("            Available Conversations\n");
        printf("═══════════════════════════════════════════════════════\n");
        printf("\n");
        printf("NPCs you can talk to:\n");
        printf("  - thessara: Your ghostly mentor in the Death Network\n");
        printf("  - vorgath: The Undying, powerful necromancer\n");
        printf("  - seraphine: The Pale, scholar necromancer\n");
        printf("\n");
        printf("Use: dialogue <npc_id> to start a conversation\n");

        return command_result_success("");
    }

    const char* arg1 = parsed_command_get_arg(cmd, 0);

    /* Show conversation history */
    if (strcmp(arg1, "history") == 0) {
        printf("\n");
        printf("═══════════════════════════════════════════════════════\n");
        printf("            Conversation History\n");
        printf("═══════════════════════════════════════════════════════\n");
        printf("\n");
        printf("No conversations recorded yet.\n");

        return command_result_success("");
    }

    /* Start conversation with specific NPC */
    const char* npc_id = arg1;

    printf("\n");
    printf("═══════════════════════════════════════════════════════\n");
    printf("            Conversation\n");
    printf("═══════════════════════════════════════════════════════\n");
    printf("\n");
    printf("Talking to: %s\n", npc_id);
    printf("\n");

    /* Handle dialogue choice if provided */
    if (arg_count >= 2) {
        const char* choice = parsed_command_get_arg(cmd, 1);
        printf("You chose: %s\n", choice);
        printf("\n");
        printf("Dialogue system integration pending.\n");

        return command_result_success("Choice recorded");
    }

    /* Start new conversation */
    printf("[Placeholder conversation with %s]\n", npc_id);
    printf("\n");
    printf("This will show the NPC's dialogue and available response choices.\n");
    printf("Dialogue system integration coming in next sprint.\n");

    return command_result_success("");
}

/**
 * @brief Register dialogue command
 */
void register_dialogue_command(void) {
    static FlagDefinition dialogue_flags[] = {
        {
            .name = "verbose",
            .short_name = 'v',
            .type = ARG_TYPE_BOOL,
            .required = false,
            .description = "Show detailed conversation information"
        }
    };

    CommandInfo dialogue_cmd = {
        .name = "dialogue",
        .description = "Talk to NPCs and manage conversations",
        .usage = "dialogue [npc_id] [choice]",
        .help_text =
            "Manage NPC conversations:\n"
            "  dialogue              - List available NPCs\n"
            "  dialogue <npc_id>     - Talk to specific NPC\n"
            "  dialogue history      - View past conversations\n"
            "  dialogue <npc> <num>  - Make dialogue choice\n"
            "\n"
            "Examples:\n"
            "  dialogue thessara     - Talk to Thessara\n"
            "  dialogue thessara 1   - Choose option 1 in conversation",
        .function = cmd_dialogue,
        .flags = dialogue_flags,
        .flag_count = 1,
        .min_args = 0,
        .max_args = 2,
        .hidden = false
    };

    command_system_register_command(&dialogue_cmd);
}

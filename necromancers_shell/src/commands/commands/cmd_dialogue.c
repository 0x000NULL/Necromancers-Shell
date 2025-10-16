/**
 * @file cmd_dialogue.c
 * @brief Dialogue command implementation
 *
 * Handles NPC conversations and dialogue trees.
 */

#include "../../game/game_state.h"
#include "../../game/narrative/dialogue/dialogue_manager.h"
#include "../../game/narrative/dialogue/dialogue_tree.h"
#include "../../game/events/thessara_contact_event.h"
#include "../../terminal/ui_feedback.h"
#include "../command_system.h"
#include "../parser.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * @brief Talk to an NPC or view available conversations
 *
 * Usage:
 *   dialogue                    - List available NPCs to talk to
 *   dialogue <npc_id>           - Start conversation with NPC
 *   dialogue history            - View past conversations
 *   dialogue <choice>           - Make dialogue choice (if in conversation)
 */
CommandResult cmd_dialogue(ParsedCommand* cmd) {
    extern GameState* g_game_state;

    if (!g_game_state) {
        return command_result_error(EXEC_ERROR_INTERNAL, "Game state not initialized");
    }

    if (!g_game_state->dialogues) {
        return command_result_error(EXEC_ERROR_INTERNAL, "Dialogue system not initialized");
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

    /* No arguments - show current dialogue or list NPCs */
    if (arg_count == 0) {
        /* If in active dialogue, show current state */
        if (dialogue_manager_is_active(g_game_state->dialogues)) {
            DialogueNode* current = dialogue_manager_get_current_node(g_game_state->dialogues);
            if (current) {
                printf("\n");
                printf("═══════════════════════════════════════════════════════\n");
                printf("            Current Conversation\n");
                printf("═══════════════════════════════════════════════════════\n");
                printf("\n");
                printf("%s: \"%s\"\n", current->speaker, current->text);
                printf("\n");

                if (current->choice_count > 0) {
                    printf("Choose your response:\n");
                    for (size_t i = 0; i < current->choice_count; i++) {
                        printf("  [%zu] %s\n", i + 1, current->choices[i].text);
                    }
                    printf("\nUse: dialogue <choice_number>\n");
                } else {
                    printf("[Conversation ended]\n");
                    dialogue_manager_end_dialogue(g_game_state->dialogues);
                }

                return command_result_success("");
            }
        }

        /* No active dialogue - list available NPCs */
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
        printf("(History tracking coming soon)\n");

        return command_result_success("");
    }

    /* If in active dialogue, treat argument as choice number */
    if (dialogue_manager_is_active(g_game_state->dialogues)) {
        char* endptr;
        long choice_num = strtol(arg1, &endptr, 10);

        if (*endptr == '\0' && choice_num > 0) {
            /* Valid choice number */
            int choice_index = (int)choice_num - 1;

            if (dialogue_manager_choose(g_game_state->dialogues, choice_index)) {
                /* Choice successful, show next dialogue node */
                DialogueNode* current = dialogue_manager_get_current_node(g_game_state->dialogues);
                if (current) {
                    printf("\n");
                    printf("%s: \"%s\"\n", current->speaker, current->text);
                    printf("\n");

                    if (current->choice_count > 0) {
                        printf("Choose your response:\n");
                        for (size_t i = 0; i < current->choice_count; i++) {
                            printf("  [%zu] %s\n", i + 1, current->choices[i].text);
                        }
                        printf("\nUse: dialogue <choice_number>\n");
                    } else {
                        printf("[Conversation ended]\n");
                        dialogue_manager_end_dialogue(g_game_state->dialogues);
                    }

                    return command_result_success("");
                } else {
                    dialogue_manager_end_dialogue(g_game_state->dialogues);
                    return command_result_success("Conversation completed.");
                }
            } else {
                return command_result_error(EXEC_ERROR_COMMAND_FAILED,
                    "Invalid choice. Use 'dialogue' to see available options.");
            }
        }

        /* Not a valid choice number, treat as end dialogue command */
        if (strcmp(arg1, "end") == 0) {
            dialogue_manager_end_dialogue(g_game_state->dialogues);
            return command_result_success("Ended conversation.");
        }

        return command_result_error(EXEC_ERROR_COMMAND_FAILED,
            "Already in conversation. Use dialogue <choice_number> or 'dialogue end'.");
    }

    /* Start conversation with specific NPC */
    const char* npc_id = arg1;

    /* Special handling for Thessara (event-driven conversation) */
    if (strcmp(npc_id, "thessara") == 0) {
        /* Check if player wants to accept or reject guidance */
        const char* action = parsed_command_get_arg(cmd, 1);

        if (action != NULL) {
            if (strcmp(action, "accept") == 0) {
                if (thessara_accept_guidance(g_game_state)) {
                    return command_result_success("");
                } else {
                    return command_result_error(EXEC_ERROR_COMMAND_FAILED,
                        "Cannot accept guidance at this time.");
                }
            } else if (strcmp(action, "reject") == 0) {
                if (thessara_reject_guidance(g_game_state)) {
                    return command_result_success("");
                } else {
                    return command_result_error(EXEC_ERROR_COMMAND_FAILED,
                        "Cannot reject guidance at this time.");
                }
            } else {
                return command_result_error(EXEC_ERROR_COMMAND_FAILED,
                    "Invalid action. Use 'dialogue thessara accept' or 'dialogue thessara reject'");
            }
        }

        /* No action specified - initiate conversation */
        if (thessara_initiate_conversation(g_game_state)) {
            return command_result_success("");
        } else {
            return command_result_error(EXEC_ERROR_COMMAND_FAILED,
                "Cannot talk to Thessara at this time.");
        }
    }

    /* Try to find and start dialogue tree for this NPC */
    size_t tree_count = 0;
    DialogueTree** trees = dialogue_manager_get_by_npc(g_game_state->dialogues, npc_id, &tree_count);

    if (trees && tree_count > 0) {
        /* Start first available tree for this NPC */
        if (dialogue_manager_start_dialogue(g_game_state->dialogues, trees[0]->id)) {
            DialogueNode* current = dialogue_manager_get_current_node(g_game_state->dialogues);
            free(trees);

            if (current) {
                printf("\n");
                printf("═══════════════════════════════════════════════════════\n");
                printf("            Conversation with %s\n", npc_id);
                printf("═══════════════════════════════════════════════════════\n");
                printf("\n");
                printf("%s: \"%s\"\n", current->speaker, current->text);
                printf("\n");

                if (current->choice_count > 0) {
                    printf("Choose your response:\n");
                    for (size_t i = 0; i < current->choice_count; i++) {
                        printf("  [%zu] %s\n", i + 1, current->choices[i].text);
                    }
                    printf("\nUse: dialogue <choice_number>\n");
                }

                return command_result_success("");
            }
        }
        free(trees);
    }

    /* Fallback: NPC not found or no dialogue trees */
    printf("\n");
    printf("═══════════════════════════════════════════════════════\n");
    printf("            Conversation with %s\n", npc_id);
    printf("═══════════════════════════════════════════════════════\n");
    printf("\n");
    printf("No dialogue available for %s yet.\n", npc_id);
    printf("(Dialogue content will be added in future updates)\n");

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

/**
 * @file cmd_memory.c
 * @brief Command for viewing discovered memory fragments
 */

#include "../command_system.h"
#include "../../game/game_state.h"
#include "../../game/narrative/memory/memory_manager.h"
#include "../../game/narrative/memory/memory_fragment.h"
#include "../../terminal/ui_feedback.h"
#include "../../utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void display_memory_list(const GameState* state) {
    if (!state || !state->memories) {
        ui_feedback_error("Memory system not initialized");
        return;
    }

    /* Get all discovered fragments */
    size_t count = 0;
    MemoryFragment** fragments = memory_manager_get_discovered(state->memories, &count);
    if (!fragments || count == 0) {
        ui_feedback_info("No memory fragments discovered yet.");
        printf("Explore the world to uncover fragments of your past...\n");
        return;
    }

    printf("=== Memory Fragments ===\n");
    printf("Discovered: %zu\n\n", count);

    /* Display each fragment */
    for (size_t i = 0; i < count; i++) {
        const MemoryFragment* frag = fragments[i];
        if (!frag) continue;

        /* Fragment header */
        printf("[%s] %s\n", frag->id, frag->title);

        /* Category */
        printf("  Category: %s", frag->category);
        if (frag->key_memory) {
            printf(" [KEY MEMORY]");
        }
        printf("\n");

        /* Content preview (first 100 chars) */
        char preview[105];
        snprintf(preview, sizeof(preview), "%.100s", frag->content);
        if (strlen(frag->content) > 100) {
            strcat(preview, "...");
        }
        printf("  %s\n", preview);

        /* Related NPCs */
        if (frag->npc_count > 0) {
            printf("  Related NPCs: ");
            for (size_t j = 0; j < frag->npc_count && j < MAX_FRAGMENT_CROSS_REFS; j++) {
                printf("%s", frag->related_npcs[j]);
                if (j < frag->npc_count - 1) {
                    printf(", ");
                }
            }
            printf("\n");
        }

        printf("\n");
    }

    free(fragments);
    printf("Use 'memory view <id>' to read full memory fragment\n");
}

static void display_memory_detail(const GameState* state, const char* memory_id) {
    if (!state || !state->memories || !memory_id) {
        ui_feedback_error("Invalid parameters");
        return;
    }

    MemoryFragment* frag = memory_manager_get_fragment(state->memories, memory_id);
    if (!frag) {
        ui_feedback_error("Memory fragment not found");
        return;
    }

    if (!frag->discovered) {
        ui_feedback_error("Memory fragment has not been discovered yet");
        return;
    }

    /* Display full memory */
    printf("=== Memory Fragment ===\n\n");

    printf("%s\n\n", frag->title);

    /* Full content */
    printf("%s\n\n", frag->content);

    /* Metadata */
    printf("Category: %s\n", frag->category);
    printf("Chronological Order: %d\n", frag->chronological_order);

    if (frag->key_memory) {
        printf("[KEY MEMORY - Critical to Main Story]\n");
    }

    /* Discovery info */
    printf("\n");
    printf("Discovered at: %s\n", frag->discovery_location);
    printf("Discovery method: %s\n", frag->discovery_method);

    /* Related NPCs */
    if (frag->npc_count > 0) {
        printf("\n");
        printf("Related NPCs:\n");
        for (size_t i = 0; i < frag->npc_count && i < MAX_FRAGMENT_CROSS_REFS; i++) {
            printf("  - %s\n", frag->related_npcs[i]);
        }
    }

    /* Related Locations */
    if (frag->location_count > 0) {
        printf("\n");
        printf("Related Locations:\n");
        for (size_t i = 0; i < frag->location_count && i < MAX_FRAGMENT_CROSS_REFS; i++) {
            printf("  - %s\n", frag->related_locations[i]);
        }
    }

    /* Related Fragments */
    if (frag->related_count > 0) {
        printf("\n");
        printf("Related Memories:\n");
        for (size_t i = 0; i < frag->related_count && i < MAX_FRAGMENT_CROSS_REFS; i++) {
            printf("  - %s\n", frag->related_fragments[i]);
        }
    }
}

static void display_memory_stats(const GameState* state) {
    if (!state || !state->memories) {
        ui_feedback_error("Memory system not initialized");
        return;
    }

    printf("=== Memory Fragment Statistics ===\n\n");

    size_t discovered_count = 0;
    MemoryFragment** discovered = memory_manager_get_discovered(state->memories, &discovered_count);

    printf("Discovered Fragments: %zu\n", discovered_count);

    if (discovered) {
        free(discovered);
    }
}

CommandResult cmd_memory(ParsedCommand* cmd) {
    GameState* state = game_state_get_instance();
    if (!state) {
        return command_result_error(EXEC_ERROR_INTERNAL, "Game state not initialized");
    }

    /* No subcommand - show list */
    if (cmd->arg_count == 0) {
        display_memory_list(state);
        return command_result_success(NULL);
    }

    const char* subcommand = parsed_command_get_arg(cmd, 0);

    /* View specific memory */
    if (strcmp(subcommand, "view") == 0) {
        if (cmd->arg_count < 2) {
            return command_result_error(EXEC_ERROR_INVALID_COMMAND, "Usage: memory view <id>");
        }
        const char* memory_id = parsed_command_get_arg(cmd, 1);
        display_memory_detail(state, memory_id);
        return command_result_success(NULL);
    }

    /* Show statistics */
    if (strcmp(subcommand, "stats") == 0) {
        display_memory_stats(state);
        return command_result_success(NULL);
    }

    /* Unknown subcommand */
    char error_msg[256];
    snprintf(error_msg, sizeof(error_msg),
             "Unknown subcommand '%s'. Use: memory, memory view <id>, memory stats", subcommand);
    return command_result_error(EXEC_ERROR_INVALID_COMMAND, error_msg);
}

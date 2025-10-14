/**
 * @file cmd_minions.c
 * @brief Minions command implementation
 *
 * Display all minions in the player's army.
 */

#define _GNU_SOURCE
#include "commands.h"
#include "../../game/game_state.h"
#include "../../game/minions/minion_manager.h"
#include "../../game/minions/minion.h"
#include "../../game/souls/soul_manager.h"
#include <stdio.h>
#include <stdlib.h>

extern GameState* g_game_state;

CommandResult cmd_minions(ParsedCommand* cmd) {
    (void)cmd; /* Unused parameter */

    if (!g_game_state || !g_game_state->minions) {
        return command_result_error(EXEC_ERROR_INTERNAL,
                                     "Game state not initialized");
    }

    size_t count = minion_manager_count(g_game_state->minions);

    char* output = NULL;
    size_t output_size = 0;
    FILE* stream = open_memstream(&output, &output_size);
    if (!stream) {
        return command_result_error(EXEC_ERROR_INTERNAL,
                                     "Failed to allocate output buffer");
    }

    fprintf(stream, "\n=== Minion Army ===\n\n");
    fprintf(stream, "Total Minions: %zu\n\n", count);

    if (count == 0) {
        fprintf(stream, "No minions raised yet.\n");
        fprintf(stream, "Use 'raise <type> [name]' to raise a minion.\n");
        fprintf(stream, "Available types: zombie, skeleton, ghoul, wraith, wight, revenant\n");
    } else {
        fprintf(stream, "%-4s %-20s %-10s %-5s %-10s %-6s %-7s %-6s\n",
                "ID", "Name", "Type", "Lvl", "HP", "Atk", "Def", "Loyal");
        fprintf(stream, "%-4s %-20s %-10s %-5s %-10s %-6s %-7s %-6s\n",
                "----", "--------------------", "----------", "-----", "----------",
                "------", "-------", "------");

        for (size_t i = 0; i < count; i++) {
            Minion* minion = minion_manager_get_at(g_game_state->minions, i);
            if (!minion) continue;

            char hp_str[16];
            snprintf(hp_str, sizeof(hp_str), "%u/%u",
                     minion->stats.health, minion->stats.health_max);

            fprintf(stream, "%-4u %-20s %-10s %-5u %-10s %-6u %-7u %-6u%%\n",
                    minion->id,
                    minion->name,
                    minion_type_name(minion->type),
                    minion->level,
                    hp_str,
                    minion->stats.attack,
                    minion->stats.defense,
                    minion->stats.loyalty);
        }

        fprintf(stream, "\nType distribution:\n");
        for (int type = 0; type < MINION_TYPE_COUNT; type++) {
            size_t type_count = minion_manager_count_by_type(g_game_state->minions, (MinionType)type);
            if (type_count > 0) {
                fprintf(stream, "  %s: %zu\n", minion_type_name((MinionType)type), type_count);
            }
        }

        fprintf(stream, "\nTotal bound souls: ");
        size_t bound_count = 0;
        for (size_t i = 0; i < count; i++) {
            Minion* minion = minion_manager_get_at(g_game_state->minions, i);
            if (minion && minion->bound_soul_id != 0) {
                bound_count++;
            }
        }
        fprintf(stream, "%zu / %zu\n", bound_count, count);
    }

    fclose(stream);

    CommandResult result = command_result_success(output);
    free(output);
    return result;
}

/**
 * @file cmd_free.c
 * @brief Free command implementation
 *
 * Release bound souls from minions or free souls entirely.
 */

#define _GNU_SOURCE
#include "commands.h"
#include "../../game/game_state.h"
#include "../../game/game_globals.h"
#include "../../game/souls/soul_manager.h"
#include "../../game/souls/soul.h"
#include "../../game/minions/minion_manager.h"
#include "../../game/minions/minion.h"
#include "../../game/resources/corruption.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

CommandResult cmd_free(ParsedCommand* cmd) {
    if (!g_game_state || !g_game_state->souls) {
        return command_result_error(EXEC_ERROR_INTERNAL,
                                     "Game state not initialized");
    }

    /* Get soul ID argument */
    if (cmd->arg_count < 1) {
        return command_result_error(EXEC_ERROR_INVALID_COMMAND,
            "Missing soul ID. Usage: free <soul_id> [--permanent]\n"
            "Use 'souls' command to list available souls.\n"
            "Use --permanent to completely release soul to afterlife (reduces corruption).");
    }

    const char* soul_id_str = parsed_command_get_arg(cmd, 0);
    uint32_t soul_id = (uint32_t)atoi(soul_id_str);

    /* Check if permanent release */
    bool permanent = parsed_command_has_flag(cmd, "permanent");

    /* Get soul */
    Soul* soul = soul_manager_get(g_game_state->souls, soul_id);
    if (!soul) {
        char error_msg[128];
        snprintf(error_msg, sizeof(error_msg), "Soul ID %u not found", soul_id);
        return command_result_error(EXEC_ERROR_COMMAND_FAILED, error_msg);
    }

    /* Build output */
    char* output = NULL;
    size_t output_size = 0;
    FILE* stream = open_memstream(&output, &output_size);
    if (!stream) {
        return command_result_error(EXEC_ERROR_INTERNAL,
                                     "Failed to allocate output buffer");
    }

    fprintf(stream, "=== Free Soul ===\n\n");
    fprintf(stream, "Soul: %s (ID: %u)\n", soul_type_name(soul->type), soul_id);
    fprintf(stream, "Quality: %u\n", soul->quality);

    if (permanent) {
        /* Permanent release - soul goes to afterlife */
        fprintf(stream, "\nPerforming permanent release ritual...\n\n");

        /* If soul is bound to minion, unbind it first */
        if (soul->bound) {
            Minion* minion = minion_manager_get(g_game_state->minions, soul->bound_minion_id);
            if (minion) {
                fprintf(stream, "Unbinding from minion '%s' (ID: %u)...\n",
                        minion->name, minion->id);
                minion_unbind_soul(minion);
            }
            soul_unbind(soul);
        }

        /* Remove soul from collection */
        if (soul_manager_remove(g_game_state->souls, soul_id)) {
            /* Reduce corruption for releasing soul */
            uint8_t corruption_reduction = soul->quality / 20; /* Higher quality = more reduction */
            if (corruption_reduction < 1) corruption_reduction = 1;
            if (corruption_reduction > 10) corruption_reduction = 10;

            if (g_game_state->corruption.corruption >= corruption_reduction) {
                g_game_state->corruption.corruption -= corruption_reduction;
            } else {
                g_game_state->corruption.corruption = 0;
            }

            fprintf(stream, "\n[SUCCESS] Soul released to the afterlife.\n\n");
            fprintf(stream, "The soul dissipates into light.\n");
            fprintf(stream, "You feel a weight lift from your consciousness.\n\n");
            fprintf(stream, "Corruption reduced: -%u%% (now: %u%%)\n",
                    corruption_reduction, g_game_state->corruption.corruption);
            fprintf(stream, "\nThis act of mercy is noted by the gods.\n");
            fprintf(stream, "Divine favor: +%u (not yet implemented)\n",
                    corruption_reduction);
        } else {
            fprintf(stream, "\n[FAILED] Could not release soul.\n");
        }

    } else {
        /* Temporary unbind - soul remains in collection */
        if (!soul->bound) {
            fprintf(stream, "\nSoul is not currently bound to any minion.\n");
            fprintf(stream, "Use --permanent to release it to the afterlife.\n");
        } else {
            Minion* minion = minion_manager_get(g_game_state->minions, soul->bound_minion_id);
            if (minion) {
                fprintf(stream, "\nUnbinding from minion '%s' (ID: %u)...\n\n",
                        minion->name, minion->id);

                /* Unbind soul */
                minion_unbind_soul(minion);
                soul_unbind(soul);

                fprintf(stream, "[SUCCESS] Soul unbound from minion.\n\n");
                fprintf(stream, "The soul returns to your collection.\n");
                fprintf(stream, "Minion stats reduced (lost soul quality bonus).\n");
                fprintf(stream, "\nSoul remains in your inventory.\n");
                fprintf(stream, "Use --permanent to fully release it.\n");
            } else {
                fprintf(stream, "\n[ERROR] Bound minion not found.\n");
                soul_unbind(soul);
            }
        }
    }

    fprintf(stream, "\n--- Current State ---\n");
    fprintf(stream, "Total souls: %zu\n", soul_manager_count(g_game_state->souls));
    fprintf(stream, "Corruption: %u%%\n", g_game_state->corruption.corruption);
    fprintf(stream, "Consciousness: %u%%\n", g_game_state->consciousness.level);

    fclose(stream);

    CommandResult result = command_result_success(output);
    free(output);
    return result;
}

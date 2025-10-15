/**
 * @file cmd_invoke.c
 * @brief Invoke command implementation
 *
 * Invoke Divine Architects or perform divine summoning rituals.
 */

#define _GNU_SOURCE
#include "commands.h"
#include "../../game/game_state.h"
#include "../../game/game_globals.h"
#include "../../game/resources/resources.h"
#include "../../game/resources/corruption.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

CommandResult cmd_invoke(ParsedCommand* cmd) {
    if (!g_game_state) {
        return command_result_error(EXEC_ERROR_INTERNAL,
                                     "Game state not initialized");
    }

    /* Get god name argument */
    if (cmd->arg_count < 1) {
        return command_result_error(EXEC_ERROR_INVALID_COMMAND,
            "Missing god name. Usage: invoke <god_name> [--offering <amount>]\n"
            "Gods: anara, keldrin, theros, myrith, vorathos, seraph, nexus");
    }

    const char* god_name = parsed_command_get_arg(cmd, 0);

    /* Get optional offering amount */
    uint32_t offering = 0;
    if (parsed_command_has_flag(cmd, "offering")) {
        const ArgumentValue* offering_arg = parsed_command_get_flag(cmd, "offering");
        if (offering_arg && offering_arg->type == ARG_TYPE_INT) {
            offering = (uint32_t)offering_arg->value.int_value;
        }
    }

    /* Validate offering if provided */
    if (offering > 0) {
        if (!resources_has_soul_energy(&g_game_state->resources, offering)) {
            char error_msg[128];
            snprintf(error_msg, sizeof(error_msg),
                "Insufficient soul energy for offering (need %u, have %u)",
                offering, g_game_state->resources.soul_energy);
            return command_result_error(EXEC_ERROR_COMMAND_FAILED, error_msg);
        }
    }

    /* Build output */
    char* output = NULL;
    size_t output_size = 0;
    FILE* stream = open_memstream(&output, &output_size);
    if (!stream) {
        return command_result_error(EXEC_ERROR_INTERNAL,
                                     "Failed to allocate output buffer");
    }

    fprintf(stream, "=== Divine Invocation ===\n\n");

    /* Simulate invocation based on god */
    if (strcasecmp(god_name, "anara") == 0) {
        fprintf(stream, "You invoke Anara, Goddess of Life...\n\n");
        fprintf(stream, "[A warm golden light fills the void]\n\n");
        fprintf(stream, "\"You dare call upon Life while wielding death's power?\n");
        fprintf(stream, "Your corruption is %u%%. I sense %s within you.\"\n\n",
                g_game_state->corruption.corruption,
                g_game_state->corruption.corruption < 30 ? "hope" : "darkness");
        if (offering > 0) {
            fprintf(stream, "Your offering of %u soul energy is noted.\n", offering);
            fprintf(stream, "Favor impact: +%u (not yet implemented)\n", offering / 10);
        }
        fprintf(stream, "\nDomain: Life\n");
        fprintf(stream, "Current favor: Not yet implemented\n");
    } else if (strcasecmp(god_name, "keldrin") == 0) {
        fprintf(stream, "You invoke Keldrin, God of Order...\n\n");
        fprintf(stream, "[Geometric patterns crystallize in the air]\n\n");
        fprintf(stream, "\"Necromancy = chaos introduction into stable system.\n");
        fprintf(stream, "Acceptable IFF: system maintains equilibrium.\n");
        fprintf(stream, "Your current corruption coefficient: %u%%\"\n\n",
                g_game_state->corruption.corruption);
        if (offering > 0) {
            fprintf(stream, "Offering received: %u energy units.\n", offering);
            fprintf(stream, "Logical favor adjustment: +%u\n", offering / 10);
        }
        fprintf(stream, "\nDomain: Order\n");
        fprintf(stream, "Current favor: Not yet implemented\n");
    } else if (strcasecmp(god_name, "vorathos") == 0) {
        fprintf(stream, "You invoke Vorathos, God of Entropy...\n\n");
        fprintf(stream, "[The void itself seems to laugh]\n\n");
        fprintf(stream, "\"EXCELLENT. More chaos. More corruption.\n");
        fprintf(stream, "You are %u%% of the way to oblivion.\n",
                g_game_state->corruption.corruption);
        fprintf(stream, "When you are ready, come fight me in the Void Breach.\n");
        fprintf(stream, "I will enjoy destroying you personally.\"\n\n");
        if (offering > 0) {
            fprintf(stream, "Vorathos consumes your offering of %u energy with glee.\n", offering);
            fprintf(stream, "\"MORE!\"\n");
        }
        fprintf(stream, "\nDomain: Entropy\n");
        fprintf(stream, "WARNING: Vorathos is hostile to all necromancers\n");
        fprintf(stream, "Current favor: Not yet implemented\n");
    } else if (strcasecmp(god_name, "nexus") == 0) {
        fprintf(stream, "You invoke Nexus, God of Networks...\n\n");
        fprintf(stream, "[The Death Network hums with increased activity]\n\n");
        fprintf(stream, "\"Query received. You have harvested %u souls to date.\n",
                g_game_state->total_souls_harvested);
        fprintf(stream, "Network efficiency: %u%% optimal.\n",
                100 - g_game_state->corruption.corruption / 2);
        fprintf(stream, "Continue optimizing soul flow protocols.\"\n\n");
        if (offering > 0) {
            fprintf(stream, "Network routing improvement: +%u nodes\n", offering / 5);
        }
        fprintf(stream, "\nDomain: Networks\n");
        fprintf(stream, "Current favor: Not yet implemented\n");
    } else {
        fprintf(stream, "You attempt to invoke '%s'...\n\n", god_name);
        fprintf(stream, "[Silence. No divine presence manifests.]\n\n");
        fprintf(stream, "Unknown god. Available Divine Architects:\n");
        fprintf(stream, "  anara   - Goddess of Life\n");
        fprintf(stream, "  keldrin - God of Order\n");
        fprintf(stream, "  theros  - God of Time\n");
        fprintf(stream, "  myrith  - Goddess of Souls\n");
        fprintf(stream, "  vorathos- God of Entropy (hostile)\n");
        fprintf(stream, "  seraph  - God of Boundaries\n");
        fprintf(stream, "  nexus   - God of Networks\n");
        fprintf(stream, "\nNote: Full god system integration pending.\n");
    }

    /* Spend offering if provided */
    if (offering > 0 && strcasecmp(god_name, "anara") == 0 ||
        strcasecmp(god_name, "keldrin") == 0 ||
        strcasecmp(god_name, "vorathos") == 0 ||
        strcasecmp(god_name, "nexus") == 0) {
        resources_spend_soul_energy(&g_game_state->resources, offering);
        fprintf(stream, "\nOffering consumed: -%u soul energy\n", offering);
        fprintf(stream, "Remaining: %u\n", g_game_state->resources.soul_energy);
    }

    fprintf(stream, "\nDay: %u\n", g_game_state->resources.day_count);
    fprintf(stream, "Corruption: %u%%\n", g_game_state->corruption.corruption);

    fclose(stream);

    CommandResult result = command_result_success(output);
    free(output);
    return result;
}

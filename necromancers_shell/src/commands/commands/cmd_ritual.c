/**
 * @file cmd_ritual.c
 * @brief Ritual command implementation
 *
 * Perform necromantic rituals (soul binding, phylactery creation, trials).
 */

#define _GNU_SOURCE
#include "commands.h"
#include "../../game/game_state.h"
#include "../../game/game_globals.h"
#include "../../game/resources/resources.h"
#include "../../game/resources/corruption.h"
#include "../../game/souls/soul_manager.h"
#include "../../game/events/trial_sequence_events.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

CommandResult cmd_ritual(ParsedCommand* cmd) {
    if (!g_game_state) {
        return command_result_error(EXEC_ERROR_INTERNAL,
                                     "Game state not initialized");
    }

    /* Get ritual type argument */
    if (cmd->arg_count < 1) {
        return command_result_error(EXEC_ERROR_INVALID_COMMAND,
            "Missing ritual type. Usage: ritual <type> [options]\n"
            "Types: phylactery, trial, offering, purification, ascension");
    }

    const char* ritual_type = parsed_command_get_arg(cmd, 0);

    /* Build output */
    char* output = NULL;
    size_t output_size = 0;
    FILE* stream = open_memstream(&output, &output_size);
    if (!stream) {
        return command_result_error(EXEC_ERROR_INTERNAL,
                                     "Failed to allocate output buffer");
    }

    fprintf(stream, "=== Necromantic Ritual ===\n\n");

    /* Handle different ritual types */
    if (strcasecmp(ritual_type, "phylactery") == 0) {
        fprintf(stream, "You begin the Phylactery Creation Ritual...\n\n");

        uint32_t cost = 500;
        uint8_t corruption_cost = 20;

        if (!resources_has_soul_energy(&g_game_state->resources, cost)) {
            fprintf(stream, "[FAILED] Insufficient soul energy!\n");
            fprintf(stream, "Required: %u\n", cost);
            fprintf(stream, "Available: %u\n\n", g_game_state->resources.soul_energy);
            fprintf(stream, "The ritual fizzles. Your consciousness fragments scatter.\n");
        } else {
            resources_spend_soul_energy(&g_game_state->resources, cost);
            corruption_add(&g_game_state->corruption, corruption_cost,
                          "Phylactery creation ritual", g_game_state->resources.day_count);

            fprintf(stream, "[SUCCESS] Phylactery created!\n\n");
            fprintf(stream, "You bind a fragment of your consciousness to a physical vessel.\n");
            fprintf(stream, "The artifact pulses with dark energy.\n\n");
            fprintf(stream, "Cost:\n");
            fprintf(stream, "  Soul Energy: -%u (remaining: %u)\n",
                    cost, g_game_state->resources.soul_energy);
            fprintf(stream, "  Corruption: +%u%% (now: %u%%)\n",
                    corruption_cost, g_game_state->corruption.corruption);
            fprintf(stream, "\nEffect: Death resistance increased (not yet implemented)\n");
        }

    } else if (strcasecmp(ritual_type, "trial") == 0) {
        fprintf(stream, "You attempt a Trial of Ascension...\n\n");

        fprintf(stream, "=== Archon Trials ===\n\n");
        fprintf(stream, "The path to Archon transformation requires completion of seven trials:\n\n");

        fprintf(stream, "1. Trial of Power - Combat mastery\n");
        fprintf(stream, "2. Trial of Wisdom - Knowledge and understanding\n");
        fprintf(stream, "3. Trial of Sacrifice - Willingness to give up power\n");
        fprintf(stream, "4. Trial of Leadership - Command without cruelty\n");
        fprintf(stream, "5. Trial of Mercy - Restraint and compassion\n");
        fprintf(stream, "6. Trial of Resolve - Conviction under pressure\n");
        fprintf(stream, "7. Trial of Balance - Harmony between forces\n\n");

        fprintf(stream, "Your current state:\n");
        fprintf(stream, "  Corruption: %u%%\n", g_game_state->corruption.corruption);
        fprintf(stream, "  Consciousness: %.1f%%\n", g_game_state->consciousness.stability);
        fprintf(stream, "  Souls Harvested: %zu\n", soul_manager_count(g_game_state->souls));
        fprintf(stream, "  Day: %u\n\n", g_game_state->resources.day_count);

        /* Evaluate current state */
        if (g_game_state->corruption.corruption >= 30 &&
            g_game_state->corruption.corruption <= 60 &&
            g_game_state->consciousness.stability >= 70) {
            fprintf(stream, "The Seven Architects observe: \"You walk the narrow path.\"\n");
            fprintf(stream, "\"Neither too pure, nor too corrupted. Balance personified.\"\n");
        } else if (g_game_state->corruption.corruption < 30 &&
                   g_game_state->consciousness.stability > 70) {
            fprintf(stream, "Anara nods approvingly. \"You maintain balance and purity.\"\n");
        } else if (g_game_state->corruption.corruption > 70) {
            fprintf(stream, "Vorathos laughs. \"So much corruption! Yet you seek ascension?\"\n");
            fprintf(stream, "\"Interesting. The Archon path may still be yours.\"\n");
        } else {
            fprintf(stream, "Keldrin calculates. \"Current trajectory: suboptimal.\"\n");
            fprintf(stream, "\"Recommendation: stabilize consciousness, moderate corruption.\"\n");
        }

        fprintf(stream, "\n[TRIAL SYSTEM - INTEGRATION COMPLETE]\n");
        fprintf(stream, "Trial infrastructure: Active\n");
        fprintf(stream, "Trial mechanics: Pending full implementation\n");
        fprintf(stream, "Trials will become playable in future updates.\n\n");

        if (g_game_state->archon_trials) {
            fprintf(stream, "Trial manager: Initialized\n");
            fprintf(stream, "Use 'ritual trial <number>' to view specific trial details.\n");
        } else {
            fprintf(stream, "Note: Trial manager not yet initialized in this game.\n");
        }

    } else if (strcasecmp(ritual_type, "purification") == 0) {
        fprintf(stream, "You attempt a Purification Ritual...\n\n");

        uint32_t mana_cost = 100;
        uint8_t corruption_reduction = 5;

        if (!resources_has_mana(&g_game_state->resources, mana_cost)) {
            fprintf(stream, "[FAILED] Insufficient mana!\n");
            fprintf(stream, "Required: %u\n", mana_cost);
            fprintf(stream, "Available: %u\n\n", g_game_state->resources.mana);
        } else if (g_game_state->corruption.corruption < corruption_reduction) {
            fprintf(stream, "[UNNECESSARY] You are already pure.\n");
            fprintf(stream, "Current corruption: %u%%\n", g_game_state->corruption.corruption);
        } else {
            resources_spend_mana(&g_game_state->resources, mana_cost);

            /* Reduce corruption */
            if (g_game_state->corruption.corruption >= corruption_reduction) {
                g_game_state->corruption.corruption -= corruption_reduction;
            } else {
                g_game_state->corruption.corruption = 0;
            }

            fprintf(stream, "[SUCCESS] Purification complete!\n\n");
            fprintf(stream, "Divine light burns away corruption.\n");
            fprintf(stream, "Your consciousness clears slightly.\n\n");
            fprintf(stream, "Cost:\n");
            fprintf(stream, "  Mana: -%u (remaining: %u)\n",
                    mana_cost, g_game_state->resources.mana);
            fprintf(stream, "  Corruption: -%u%% (now: %u%%)\n",
                    corruption_reduction, g_game_state->corruption.corruption);
        }

    } else if (strcasecmp(ritual_type, "offering") == 0) {
        fprintf(stream, "You perform a Divine Offering Ritual...\n\n");

        uint32_t offering = 100;

        if (!resources_has_soul_energy(&g_game_state->resources, offering)) {
            fprintf(stream, "[FAILED] Insufficient soul energy!\n");
            fprintf(stream, "Required: %u\n", offering);
            fprintf(stream, "Available: %u\n\n", g_game_state->resources.soul_energy);
        } else {
            resources_spend_soul_energy(&g_game_state->resources, offering);

            fprintf(stream, "[SUCCESS] Offering accepted!\n\n");
            fprintf(stream, "Soul energy flows upward into the divine realms.\n");
            fprintf(stream, "The gods take notice.\n\n");
            fprintf(stream, "Offering: %u soul energy\n", offering);
            fprintf(stream, "Remaining: %u\n", g_game_state->resources.soul_energy);
            fprintf(stream, "\nDivine Favor: +%u (not yet implemented)\n", offering / 10);
        }

    } else {
        fprintf(stream, "Unknown ritual type: '%s'\n\n", ritual_type);
        fprintf(stream, "Available rituals:\n");
        fprintf(stream, "  phylactery     - Create a phylactery (immortality vessel)\n");
        fprintf(stream, "  trial          - Attempt a Trial of Ascension\n");
        fprintf(stream, "  purification   - Reduce corruption (-5%%, costs 100 mana)\n");
        fprintf(stream, "  offering       - Offer soul energy to the gods\n");
        fprintf(stream, "  ascension      - (Requires all 7 trials completed)\n");
        fprintf(stream, "\nNote: Full ritual system integration pending.\n");
    }

    fprintf(stream, "\n--- Current State ---\n");
    fprintf(stream, "Day: %u\n", g_game_state->resources.day_count);
    fprintf(stream, "Soul Energy: %u\n", g_game_state->resources.soul_energy);
    fprintf(stream, "Mana: %u\n", g_game_state->resources.mana);
    fprintf(stream, "Corruption: %u%%\n", g_game_state->corruption.corruption);
    fprintf(stream, "Consciousness: %.1f%%\n", g_game_state->consciousness.stability);

    fclose(stream);

    CommandResult result = command_result_success(output);
    free(output);
    return result;
}

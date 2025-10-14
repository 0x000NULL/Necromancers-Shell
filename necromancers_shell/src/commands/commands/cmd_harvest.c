/**
 * @file cmd_harvest.c
 * @brief Harvest command implementation
 *
 * Harvest souls from corpses at the current location.
 */

#define _GNU_SOURCE
#include "commands.h"
#include "../../game/game_state.h"
#include "../../game/souls/soul_manager.h"
#include "../../game/world/location.h"
#include "../../game/resources/resources.h"
#include "../../game/resources/corruption.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

extern GameState* g_game_state;

/**
 * Determine soul type based on location type and randomness
 */
static SoulType determine_soul_type_from_location(LocationType loc_type) {
    int rand_val = rand() % 100;

    switch (loc_type) {
        case LOCATION_TYPE_GRAVEYARD:
            /* Mostly common, some innocent */
            if (rand_val < 70) return SOUL_TYPE_COMMON;
            else if (rand_val < 90) return SOUL_TYPE_INNOCENT;
            else return SOUL_TYPE_ANCIENT;

        case LOCATION_TYPE_BATTLEFIELD:
            /* Mostly warriors, some corrupted */
            if (rand_val < 60) return SOUL_TYPE_WARRIOR;
            else if (rand_val < 85) return SOUL_TYPE_COMMON;
            else return SOUL_TYPE_CORRUPTED;

        case LOCATION_TYPE_VILLAGE:
            /* Mostly innocent */
            if (rand_val < 80) return SOUL_TYPE_INNOCENT;
            else return SOUL_TYPE_COMMON;

        case LOCATION_TYPE_CRYPT:
            /* Ancient and mage souls */
            if (rand_val < 40) return SOUL_TYPE_ANCIENT;
            else if (rand_val < 70) return SOUL_TYPE_MAGE;
            else return SOUL_TYPE_WARRIOR;

        case LOCATION_TYPE_RITUAL_SITE:
            /* Corrupted and mage souls */
            if (rand_val < 50) return SOUL_TYPE_CORRUPTED;
            else if (rand_val < 80) return SOUL_TYPE_MAGE;
            else return SOUL_TYPE_ANCIENT;

        default:
            return SOUL_TYPE_COMMON;
    }
}

CommandResult cmd_harvest(ParsedCommand* cmd) {
    if (!g_game_state) {
        return command_result_error(EXEC_ERROR_INTERNAL,
                                     "Game state not initialized");
    }

    /* Get current location */
    Location* loc = game_state_get_current_location(g_game_state);
    if (!loc) {
        return command_result_error(EXEC_ERROR_INTERNAL,
                                     "No current location");
    }

    /* Parse --count flag (default 10) */
    int count = 10;
    if (parsed_command_has_flag(cmd, "count")) {
        const ArgumentValue* count_arg = parsed_command_get_flag(cmd, "count");
        if (count_arg && count_arg->type == ARG_TYPE_INT) {
            count = count_arg->value.int_value;
            if (count <= 0) {
                return command_result_error(EXEC_ERROR_INVALID_COMMAND,
                                             "Count must be positive");
            }
            if (count > 100) {
                count = 100; /* Cap at 100 */
            }
        }
    }

    /* Check if location has corpses */
    if (loc->corpse_count == 0) {
        char error_msg[128];
        snprintf(error_msg, sizeof(error_msg), "No corpses available at %s", loc->name);
        return command_result_error(EXEC_ERROR_COMMAND_FAILED, error_msg);
    }

    /* Seed random number generator */
    srand((unsigned int)time(NULL));

    /* Harvest corpses */
    uint32_t harvested = location_harvest_corpses(loc, (uint32_t)count);

    /* Create souls and track statistics */
    uint64_t total_energy = 0;
    uint32_t type_counts[SOUL_TYPE_COUNT] = {0};
    uint32_t corruption_gain = 0;

    for (uint32_t i = 0; i < harvested; i++) {
        /* Determine soul type based on location */
        SoulType type = determine_soul_type_from_location(loc->type);

        /* Quality varies around location average (±20) */
        int quality_variance = (rand() % 41) - 20; /* -20 to +20 */
        int quality = (int)loc->soul_quality_avg + quality_variance;
        if (quality < 0) quality = 0;
        if (quality > 100) quality = 100;

        /* Create soul */
        Soul* soul = soul_create(type, (uint8_t)quality);
        if (!soul) {
            continue; /* Skip on allocation failure */
        }

        soul->id = game_state_next_soul_id(g_game_state);
        soul_manager_add(g_game_state->souls, soul);

        /* Add energy to resources */
        resources_add_soul_energy(&g_game_state->resources, soul->energy);
        total_energy += soul->energy;

        /* Track statistics */
        type_counts[type]++;

        /* Add corruption for harvesting innocent souls */
        if (type == SOUL_TYPE_INNOCENT) {
            corruption_gain += 5;
        }
        /* Minor corruption for other souls */
        else {
            corruption_gain += 1;
        }
    }

    /* Apply corruption */
    if (corruption_gain > 0) {
        corruption_add(&g_game_state->corruption, (uint8_t)corruption_gain,
                      "Harvesting souls from corpses", g_game_state->resources.day_count);
    }

    /* Build output string */
    char* output = NULL;
    size_t output_size = 0;
    FILE* stream = open_memstream(&output, &output_size);
    if (!stream) {
        return command_result_error(EXEC_ERROR_INTERNAL,
                                     "Failed to allocate output buffer");
    }

    fprintf(stream, "=== Harvest Complete ===\n\n");
    fprintf(stream, "Location: %s\n", loc->name);
    fprintf(stream, "Corpses harvested: %u\n\n", harvested);

    fprintf(stream, "Souls gained:\n");
    for (int i = 0; i < SOUL_TYPE_COUNT; i++) {
        if (type_counts[i] > 0) {
            fprintf(stream, "  %s: %u\n", soul_type_name((SoulType)i), type_counts[i]);
        }
    }

    fprintf(stream, "\nTotal energy gained: %u\n", (uint32_t)total_energy);
    fprintf(stream, "Current soul energy: %u\n", g_game_state->resources.soul_energy);
    fprintf(stream, "Total souls: %zu\n", soul_manager_count(g_game_state->souls));

    if (corruption_gain > 0) {
        fprintf(stream, "\nCorruption increased by %u%% (now %u%%)\n",
                corruption_gain, g_game_state->corruption.corruption);
    }

    if (loc->corpse_count > 0) {
        fprintf(stream, "\nRemaining corpses at %s: %u\n", loc->name, loc->corpse_count);
    } else {
        fprintf(stream, "\nNo more corpses remain at %s.\n", loc->name);
    }

    fclose(stream);

    CommandResult result = command_result_success(output);
    free(output);
    return result;
}

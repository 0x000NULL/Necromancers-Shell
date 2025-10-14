/**
 * @file cmd_raise.c
 * @brief Raise command implementation
 *
 * Raise undead minions from corpses.
 */

#define _GNU_SOURCE
#include "commands.h"
#include "../../game/game_state.h"
#include "../../game/game_globals.h"
#include "../../game/minions/minion_manager.h"
#include "../../game/minions/minion.h"
#include "../../game/souls/soul_manager.h"
#include "../../game/resources/resources.h"
#include "../../game/resources/corruption.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

/**
 * Parse minion type from string
 */
static MinionType parse_minion_type(const char* type_str) {
    if (!type_str) {
        return MINION_TYPE_COUNT; /* Invalid */
    }

    if (strcasecmp(type_str, "zombie") == 0) return MINION_TYPE_ZOMBIE;
    if (strcasecmp(type_str, "skeleton") == 0) return MINION_TYPE_SKELETON;
    if (strcasecmp(type_str, "ghoul") == 0) return MINION_TYPE_GHOUL;
    if (strcasecmp(type_str, "wraith") == 0) return MINION_TYPE_WRAITH;
    if (strcasecmp(type_str, "wight") == 0) return MINION_TYPE_WIGHT;
    if (strcasecmp(type_str, "revenant") == 0) return MINION_TYPE_REVENANT;

    return MINION_TYPE_COUNT; /* Invalid */
}

CommandResult cmd_raise(ParsedCommand* cmd) {
    if (!g_game_state || !g_game_state->minions) {
        return command_result_error(EXEC_ERROR_INTERNAL,
                                     "Game state not initialized");
    }

    /* Get type argument */
    if (cmd->arg_count < 1) {
        return command_result_error(EXEC_ERROR_INVALID_COMMAND,
            "Missing minion type. Usage: raise <type> [name] [--soul <id>]\n"
            "Types: zombie, skeleton, ghoul, wraith, wight, revenant");
    }

    const char* type_str = parsed_command_get_arg(cmd, 0);

    /* Parse minion type */
    MinionType type = parse_minion_type(type_str);
    if (type >= MINION_TYPE_COUNT) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg),
            "Invalid minion type '%s'. Valid types: zombie, skeleton, ghoul, wraith, wight, revenant",
            type_str);
        return command_result_error(EXEC_ERROR_INVALID_COMMAND, error_msg);
    }

    /* Get optional name argument */
    const char* name = NULL;
    if (cmd->arg_count >= 2) {
        name = parsed_command_get_arg(cmd, 1);
    }

    /* Get optional soul to bind */
    uint32_t soul_id = 0;
    if (parsed_command_has_flag(cmd, "soul")) {
        const ArgumentValue* soul_arg = parsed_command_get_flag(cmd, "soul");
        if (soul_arg && soul_arg->type == ARG_TYPE_INT) {
            soul_id = (uint32_t)soul_arg->value.int_value;

            /* Validate soul exists and is unbound */
            Soul* soul = soul_manager_get(g_game_state->souls, soul_id);
            if (!soul) {
                char error_msg[128];
                snprintf(error_msg, sizeof(error_msg), "Soul ID %u not found", soul_id);
                return command_result_error(EXEC_ERROR_COMMAND_FAILED, error_msg);
            }
            if (soul->bound) {
                char error_msg[128];
                snprintf(error_msg, sizeof(error_msg),
                    "Soul %u is already bound to minion %u", soul_id, soul->bound_minion_id);
                return command_result_error(EXEC_ERROR_COMMAND_FAILED, error_msg);
            }
        }
    }

    /* Calculate cost */
    uint32_t cost = minion_calculate_raise_cost(type);

    /* Check if player has enough energy */
    if (!resources_has_soul_energy(&g_game_state->resources, cost)) {
        char error_msg[128];
        snprintf(error_msg, sizeof(error_msg),
            "Insufficient soul energy (need %u, have %u)",
            cost, g_game_state->resources.soul_energy);
        return command_result_error(EXEC_ERROR_COMMAND_FAILED, error_msg);
    }

    /* Create minion */
    Minion* minion = minion_create(type, name, soul_id);
    if (!minion) {
        return command_result_error(EXEC_ERROR_INTERNAL,
            "Failed to create minion");
    }

    minion->id = game_state_next_minion_id(g_game_state);
    minion->location_id = g_game_state->current_location_id;

    /* Bind soul if provided */
    if (soul_id > 0) {
        Soul* soul = soul_manager_get(g_game_state->souls, soul_id);
        soul_bind(soul, minion->id);
        minion_bind_soul(minion, soul_id);

        /* Apply soul quality bonus to stats (10% bonus from soul quality) */
        float bonus = 1.0f + (soul->quality / 1000.0f);
        minion->stats.attack = (uint32_t)(minion->stats.attack * bonus);
        minion->stats.defense = (uint32_t)(minion->stats.defense * bonus);
        minion->stats.speed = (uint32_t)(minion->stats.speed * bonus);
        minion->stats.loyalty += (soul->quality / 10);
        if (minion->stats.loyalty > 100) minion->stats.loyalty = 100;
    }

    /* Add to manager */
    if (!minion_manager_add(g_game_state->minions, minion)) {
        minion_destroy(minion);
        return command_result_error(EXEC_ERROR_INTERNAL,
            "Failed to add minion to army");
    }

    /* Spend energy */
    resources_spend_soul_energy(&g_game_state->resources, cost);

    /* Add corruption (raising undead is corrupting) */
    corruption_add(&g_game_state->corruption, 10,
        "Raising undead minion", g_game_state->resources.day_count);

    /* Build success message */
    char* output = NULL;
    size_t output_size = 0;
    FILE* stream = open_memstream(&output, &output_size);
    if (!stream) {
        return command_result_error(EXEC_ERROR_INTERNAL,
                                     "Failed to allocate output buffer");
    }

    fprintf(stream, "=== Minion Raised ===\n\n");
    fprintf(stream, "Raised %s '%s' (ID: %u)\n",
            minion_type_name(type), minion->name, minion->id);
    fprintf(stream, "Cost: %u soul energy\n\n", cost);

    fprintf(stream, "Stats:\n");
    fprintf(stream, "  HP: %u/%u\n", minion->stats.health, minion->stats.health_max);
    fprintf(stream, "  Attack: %u\n", minion->stats.attack);
    fprintf(stream, "  Defense: %u\n", minion->stats.defense);
    fprintf(stream, "  Speed: %u\n", minion->stats.speed);
    fprintf(stream, "  Loyalty: %u%%\n", minion->stats.loyalty);

    if (soul_id > 0) {
        Soul* soul = soul_manager_get(g_game_state->souls, soul_id);
        fprintf(stream, "\nBound Soul: %s (ID: %u, Quality: %u)\n",
                soul_type_name(soul->type), soul_id, soul->quality);
    }

    fprintf(stream, "\nRemaining soul energy: %u\n", g_game_state->resources.soul_energy);
    fprintf(stream, "Total minions: %zu\n", minion_manager_count(g_game_state->minions));
    fprintf(stream, "Corruption: %u%%\n", g_game_state->corruption.corruption);

    fclose(stream);

    CommandResult result = command_result_success(output);
    free(output);
    return result;
}

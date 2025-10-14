/**
 * @file game_state.c
 * @brief Implementation of central game state
 */

#include "game_state.h"
#include "minions/minion_manager.h"
#include "../utils/logger.h"
#include <stdlib.h>
#include <string.h>

GameState* game_state_create(void) {
    GameState* state = calloc(1, sizeof(GameState));
    if (!state) {
        LOG_ERROR("Failed to allocate game state");
        return NULL;
    }

    /* Initialize soul manager */
    state->souls = soul_manager_create();
    if (!state->souls) {
        LOG_ERROR("Failed to create soul manager");
        free(state);
        return NULL;
    }

    /* Initialize territory manager */
    state->territory = territory_manager_create();
    if (!state->territory) {
        LOG_ERROR("Failed to create territory manager");
        soul_manager_destroy(state->souls);
        free(state);
        return NULL;
    }

    /* Load locations (hardcoded for now) */
    size_t loaded = territory_manager_load_from_file(state->territory, NULL);
    if (loaded == 0) {
        LOG_ERROR("Failed to load any locations");
        territory_manager_destroy(state->territory);
        soul_manager_destroy(state->souls);
        free(state);
        return NULL;
    }
    LOG_INFO("Loaded %zu locations", loaded);

    /* Initialize minion manager */
    state->minions = minion_manager_create(50);
    if (!state->minions) {
        LOG_ERROR("Failed to create minion manager");
        territory_manager_destroy(state->territory);
        soul_manager_destroy(state->souls);
        free(state);
        return NULL;
    }

    /* Initialize resources */
    resources_init(&state->resources);

    /* Initialize corruption */
    corruption_init(&state->corruption);

    /* Set starting location (ID 1: Forgotten Graveyard) */
    state->current_location_id = 1;

    /* Initialize player stats */
    state->player_level = 1;
    state->player_experience = 0;

    /* Initialize ID counters */
    state->next_soul_id = 1;
    state->next_minion_id = 1;

    state->initialized = true;
    LOG_INFO("Game state initialized successfully");

    return state;
}

void game_state_destroy(GameState* state) {
    if (!state) {
        return;
    }

    soul_manager_destroy(state->souls);
    territory_manager_destroy(state->territory);
    minion_manager_destroy(state->minions);

    free(state);
    LOG_INFO("Game state destroyed");
}

uint32_t game_state_next_soul_id(GameState* state) {
    if (!state) {
        return 0;
    }
    return state->next_soul_id++;
}

uint32_t game_state_next_minion_id(GameState* state) {
    if (!state) {
        return 0;
    }
    return state->next_minion_id++;
}

Location* game_state_get_current_location(const GameState* state) {
    if (!state || !state->territory) {
        return NULL;
    }
    return territory_manager_get_location(state->territory, state->current_location_id);
}

bool game_state_move_to_location(GameState* state, uint32_t location_id) {
    if (!state || !state->territory) {
        return false;
    }

    /* Get current and target locations */
    Location* current = game_state_get_current_location(state);
    Location* target = territory_manager_get_location(state->territory, location_id);

    if (!target) {
        LOG_WARN("Target location %u not found", location_id);
        return false;
    }

    /* Check if moving from starting location or to connected location */
    if (current && !location_is_connected(current, location_id)) {
        LOG_WARN("Location %u is not connected to current location", location_id);
        return false;
    }

    /* Target must be discovered */
    if (!target->discovered) {
        LOG_WARN("Location %u has not been discovered yet", location_id);
        return false;
    }

    /* Move to location */
    state->current_location_id = location_id;
    LOG_INFO("Moved to location %u: %s", location_id, target->name);

    return true;
}

void game_state_advance_time(GameState* state, uint32_t hours) {
    if (!state) {
        return;
    }

    /* Advance time */
    resources_advance_time(&state->resources, hours);

    /* Regenerate mana (10 per hour) */
    uint32_t mana_regen = hours * 10;
    resources_add_mana(&state->resources, mana_regen);

    LOG_DEBUG("Advanced time by %u hours (mana regen: %u)", hours, mana_regen);
}

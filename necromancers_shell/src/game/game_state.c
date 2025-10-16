/**
 * @file game_state.c
 * @brief Implementation of central game state
 */

#include "game_state.h"
#include "minions/minion_manager.h"
#include "events/event_scheduler.h"
#include "endings/ending_system.h"
#include "../data/data_loader.h"
#include "../data/location_data.h"
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

    /* Load locations from data file */
    DataFile* location_data = data_file_load("data/locations.dat");
    size_t loaded = 0;
    if (location_data) {
        loaded = location_data_load_all(state->territory, location_data);
        if (loaded > 0) {
            LOG_INFO("Loaded %zu locations from data/locations.dat", loaded);
        } else {
            LOG_WARN("No locations loaded from data file, using fallback");
        }
    } else {
        LOG_WARN("Could not load data/locations.dat, using fallback");
    }

    /* Fallback: Load hardcoded locations if data file failed */
    if (loaded == 0) {
        loaded = territory_manager_load_from_file(state->territory, NULL);
        if (loaded == 0) {
            LOG_ERROR("Failed to load any locations");
            if (location_data) data_file_destroy(location_data);
            territory_manager_destroy(state->territory);
            soul_manager_destroy(state->souls);
            free(state);
            return NULL;
        }
        LOG_INFO("Loaded %zu fallback locations", loaded);
    }

    /* Initialize location graph */
    state->location_graph = location_graph_create();
    if (!state->location_graph) {
        LOG_ERROR("Failed to create location graph");
        if (location_data) data_file_destroy(location_data);
        territory_manager_destroy(state->territory);
        soul_manager_destroy(state->souls);
        free(state);
        return NULL;
    }

    /* Build graph connections from data file */
    size_t connections = 0;
    if (location_data) {
        connections = location_data_build_connections(state->territory, location_data);
        if (connections > 0) {
            LOG_INFO("Built %zu location connections from data file", connections);
        }
        data_file_destroy(location_data);
        location_data = NULL;
    }

    /* Fallback: Create default connections if none were loaded */
    if (connections == 0) {
        LOG_WARN("No connections loaded from data file, creating fallback connections");
        location_graph_add_bidirectional(state->location_graph, 1, 2, 2, 10);
        location_graph_add_bidirectional(state->location_graph, 2, 3, 3, 15);
        location_graph_add_bidirectional(state->location_graph, 2, 4, 1, 5);
        location_graph_add_bidirectional(state->location_graph, 3, 5, 2, 20);
        location_graph_add_bidirectional(state->location_graph, 4, 5, 2, 12);
    }

    /* Initialize world map */
    state->world_map = world_map_create(state->territory, state->location_graph);
    if (!state->world_map) {
        LOG_ERROR("Failed to create world map");
        location_graph_destroy(state->location_graph);
        territory_manager_destroy(state->territory);
        soul_manager_destroy(state->souls);
        free(state);
        return NULL;
    }

    /* Auto-layout locations on map */
    world_map_auto_layout(state->world_map, 100);

    /* Initialize territory status manager */
    state->territory_status = territory_status_create();
    if (!state->territory_status) {
        LOG_ERROR("Failed to create territory status manager");
        world_map_destroy(state->world_map);
        location_graph_destroy(state->location_graph);
        territory_manager_destroy(state->territory);
        soul_manager_destroy(state->souls);
        free(state);
        return NULL;
    }

    /* Initialize Death Network */
    state->death_network = death_network_create();
    if (!state->death_network) {
        LOG_ERROR("Failed to create death network");
        territory_status_destroy(state->territory_status);
        world_map_destroy(state->world_map);
        location_graph_destroy(state->location_graph);
        territory_manager_destroy(state->territory);
        soul_manager_destroy(state->souls);
        free(state);
        return NULL;
    }

    /* Populate Death Network with all locations */
    size_t location_count = territory_manager_count(state->territory);
    LOG_INFO("Populating Death Network with %zu locations", location_count);
    for (uint32_t loc_id = 1; loc_id <= location_count; loc_id++) {
        Location* loc = territory_manager_get_location(state->territory, loc_id);
        if (loc) {
            /* Set death signature based on location type */
            uint8_t base_sig = 40;  /* Default moderate */
            uint32_t max_corpses = 20;
            uint8_t regen = 2;

            switch (loc->type) {
                case LOCATION_TYPE_GRAVEYARD:
                    base_sig = 60; max_corpses = 30; regen = 3;
                    break;
                case LOCATION_TYPE_BATTLEFIELD:
                    base_sig = 80; max_corpses = 50; regen = 5;
                    break;
                case LOCATION_TYPE_VILLAGE:
                    base_sig = 30; max_corpses = 15; regen = 2;
                    break;
                case LOCATION_TYPE_CRYPT:
                    base_sig = 70; max_corpses = 40; regen = 4;
                    break;
                case LOCATION_TYPE_RITUAL_SITE:
                    base_sig = 50; max_corpses = 25; regen = 3;
                    break;
                default:
                    break;
            }

            death_network_add_location(state->death_network, loc_id, base_sig, max_corpses, regen);

            /* Set quality distribution based on location type */
            if (loc->type == LOCATION_TYPE_BATTLEFIELD) {
                /* Battlefields have higher quality */
                death_network_set_quality_distribution(state->death_network, loc_id,
                    30,  /* poor */
                    35,  /* average */
                    25,  /* good */
                    8,   /* excellent */
                    2    /* legendary */
                );
            } else if (loc->type == LOCATION_TYPE_CRYPT) {
                /* Crypts have ancient souls */
                death_network_set_quality_distribution(state->death_network, loc_id,
                    20,  /* poor */
                    30,  /* average */
                    30,  /* good */
                    15,  /* excellent */
                    5    /* legendary */
                );
            }
            /* Others use default distribution set by death_network_add_location */
        }
    }

    /* Initialize minion manager */
    state->minions = minion_manager_create(50);
    if (!state->minions) {
        LOG_ERROR("Failed to create minion manager");
        death_network_destroy(state->death_network);
        territory_status_destroy(state->territory_status);
        world_map_destroy(state->world_map);
        location_graph_destroy(state->location_graph);
        territory_manager_destroy(state->territory);
        soul_manager_destroy(state->souls);
        free(state);
        return NULL;
    }

    /* Initialize narrative systems */
    state->memories = memory_manager_create();
    if (!state->memories) {
        LOG_ERROR("Failed to create memory manager");
        minion_manager_destroy(state->minions);
        death_network_destroy(state->death_network);
        territory_status_destroy(state->territory_status);
        world_map_destroy(state->world_map);
        location_graph_destroy(state->location_graph);
        territory_manager_destroy(state->territory);
        soul_manager_destroy(state->souls);
        free(state);
        return NULL;
    }

    state->npcs = npc_manager_create();
    if (!state->npcs) {
        LOG_ERROR("Failed to create NPC manager");
        memory_manager_destroy(state->memories);
        minion_manager_destroy(state->minions);
        death_network_destroy(state->death_network);
        territory_status_destroy(state->territory_status);
        world_map_destroy(state->world_map);
        location_graph_destroy(state->location_graph);
        territory_manager_destroy(state->territory);
        soul_manager_destroy(state->souls);
        free(state);
        return NULL;
    }

    state->relationships = relationship_manager_create();
    if (!state->relationships) {
        LOG_ERROR("Failed to create relationship manager");
        npc_manager_destroy(state->npcs);
        memory_manager_destroy(state->memories);
        minion_manager_destroy(state->minions);
        death_network_destroy(state->death_network);
        territory_status_destroy(state->territory_status);
        world_map_destroy(state->world_map);
        location_graph_destroy(state->location_graph);
        territory_manager_destroy(state->territory);
        soul_manager_destroy(state->souls);
        free(state);
        return NULL;
    }

    state->quests = quest_manager_create();
    if (!state->quests) {
        LOG_ERROR("Failed to create quest manager");
        relationship_manager_destroy(state->relationships);
        npc_manager_destroy(state->npcs);
        memory_manager_destroy(state->memories);
        minion_manager_destroy(state->minions);
        death_network_destroy(state->death_network);
        territory_status_destroy(state->territory_status);
        world_map_destroy(state->world_map);
        location_graph_destroy(state->location_graph);
        territory_manager_destroy(state->territory);
        soul_manager_destroy(state->souls);
        free(state);
        return NULL;
    }

    state->dialogues = dialogue_manager_create();
    if (!state->dialogues) {
        LOG_ERROR("Failed to create dialogue manager");
        quest_manager_destroy(state->quests);
        relationship_manager_destroy(state->relationships);
        npc_manager_destroy(state->npcs);
        memory_manager_destroy(state->memories);
        minion_manager_destroy(state->minions);
        death_network_destroy(state->death_network);
        territory_status_destroy(state->territory_status);
        world_map_destroy(state->world_map);
        location_graph_destroy(state->location_graph);
        territory_manager_destroy(state->territory);
        soul_manager_destroy(state->souls);
        free(state);
        return NULL;
    }

    /* Initialize Thessara ghost system */
    state->thessara = thessara_create();
    if (!state->thessara) {
        LOG_ERROR("Failed to create Thessara system");
        dialogue_manager_destroy(state->dialogues);
        quest_manager_destroy(state->quests);
        relationship_manager_destroy(state->relationships);
        npc_manager_destroy(state->npcs);
        memory_manager_destroy(state->memories);
        minion_manager_destroy(state->minions);
        death_network_destroy(state->death_network);
        territory_status_destroy(state->territory_status);
        world_map_destroy(state->world_map);
        location_graph_destroy(state->location_graph);
        territory_manager_destroy(state->territory);
        soul_manager_destroy(state->souls);
        free(state);
        return NULL;
    }

    /* Initialize null space location system */
    state->null_space = null_space_create();
    if (!state->null_space) {
        LOG_ERROR("Failed to create null space system");
        thessara_destroy(state->thessara);
        dialogue_manager_destroy(state->dialogues);
        quest_manager_destroy(state->quests);
        relationship_manager_destroy(state->relationships);
        npc_manager_destroy(state->npcs);
        memory_manager_destroy(state->memories);
        minion_manager_destroy(state->minions);
        death_network_destroy(state->death_network);
        territory_status_destroy(state->territory_status);
        world_map_destroy(state->world_map);
        location_graph_destroy(state->location_graph);
        territory_manager_destroy(state->territory);
        soul_manager_destroy(state->souls);
        free(state);
        return NULL;
    }

    /* Initialize Divine Council (Seven Architects) */
    extern DivineCouncil* divine_favor_initialize_council(void);
    state->divine_council = divine_favor_initialize_council();
    if (!state->divine_council) {
        LOG_ERROR("Failed to create Divine Council");
        null_space_destroy(state->null_space);
        thessara_destroy(state->thessara);
        dialogue_manager_destroy(state->dialogues);
        quest_manager_destroy(state->quests);
        relationship_manager_destroy(state->relationships);
        npc_manager_destroy(state->npcs);
        memory_manager_destroy(state->memories);
        minion_manager_destroy(state->minions);
        death_network_destroy(state->death_network);
        territory_status_destroy(state->territory_status);
        world_map_destroy(state->world_map);
        location_graph_destroy(state->location_graph);
        territory_manager_destroy(state->territory);
        soul_manager_destroy(state->souls);
        free(state);
        return NULL;
    }

    /* Initialize event scheduler */
    state->event_scheduler = event_scheduler_create();
    if (!state->event_scheduler) {
        LOG_ERROR("Failed to create event scheduler");
        dialogue_manager_destroy(state->dialogues);
        quest_manager_destroy(state->quests);
        relationship_manager_destroy(state->relationships);
        npc_manager_destroy(state->npcs);
        memory_manager_destroy(state->memories);
        minion_manager_destroy(state->minions);
        death_network_destroy(state->death_network);
        territory_status_destroy(state->territory_status);
        world_map_destroy(state->world_map);
        location_graph_destroy(state->location_graph);
        territory_manager_destroy(state->territory);
        soul_manager_destroy(state->souls);
        free(state);
        return NULL;
    }

    /* Register all story events */
    extern uint32_t register_all_story_events(EventScheduler*, GameState*);
    uint32_t events_registered = register_all_story_events(state->event_scheduler, state);
    if (events_registered == 0) {
        LOG_ERROR("Failed to register any story events - game may not function correctly");
    } else {
        LOG_INFO("Successfully registered %u story event(s)", events_registered);
    }

    /* Initialize ending system */
    state->ending_system = ending_system_create();
    if (!state->ending_system) {
        LOG_ERROR("Failed to create ending system");
        event_scheduler_destroy(state->event_scheduler);
        dialogue_manager_destroy(state->dialogues);
        quest_manager_destroy(state->quests);
        relationship_manager_destroy(state->relationships);
        npc_manager_destroy(state->npcs);
        memory_manager_destroy(state->memories);
        minion_manager_destroy(state->minions);
        death_network_destroy(state->death_network);
        territory_status_destroy(state->territory_status);
        world_map_destroy(state->world_map);
        location_graph_destroy(state->location_graph);
        territory_manager_destroy(state->territory);
        soul_manager_destroy(state->souls);
        free(state);
        return NULL;
    }

    /* Initialize Archon trial system */
    extern ArchonTrialManager* archon_trial_manager_create(void);
    extern bool archon_trial_load_from_file(ArchonTrialManager*, const char*);
    state->archon_trials = archon_trial_manager_create();
    if (!state->archon_trials) {
        LOG_ERROR("Failed to create Archon trial manager");
        ending_system_destroy(state->ending_system);
        event_scheduler_destroy(state->event_scheduler);
        dialogue_manager_destroy(state->dialogues);
        quest_manager_destroy(state->quests);
        relationship_manager_destroy(state->relationships);
        npc_manager_destroy(state->npcs);
        memory_manager_destroy(state->memories);
        minion_manager_destroy(state->minions);
        death_network_destroy(state->death_network);
        territory_status_destroy(state->territory_status);
        world_map_destroy(state->world_map);
        location_graph_destroy(state->location_graph);
        territory_manager_destroy(state->territory);
        soul_manager_destroy(state->souls);
        free(state);
        return NULL;
    }

    /* Load trial definitions from data file */
    if (!archon_trial_load_from_file(state->archon_trials,
                                      "data/trials/archon_trials.dat")) {
        LOG_WARN("Failed to load Archon trials from data file");
    } else {
        LOG_INFO("Loaded Archon trial definitions successfully");
    }

    /* Initialize Week 35-37 Archon Path systems */
    extern DivineJudgmentState* divine_judgment_create(void);
    extern NetworkPatchingState* network_patching_create(void);
    extern SplitRoutingManager* split_routing_manager_create(void);
    extern PurgeState* purge_system_create(void);
    extern ArchonState* archon_state_create(void);
    extern ReformationProgram* reformation_program_create(void);

    state->divine_judgment = divine_judgment_create();
    state->network_patching = network_patching_create();
    state->split_routing = split_routing_manager_create();
    state->purge_state = purge_system_create();
    state->archon_state = archon_state_create();
    state->reformation_program = reformation_program_create();

    if (!state->divine_judgment || !state->network_patching ||
        !state->split_routing || !state->purge_state ||
        !state->archon_state || !state->reformation_program) {
        LOG_ERROR("Failed to create Archon Path systems");
        /* Cleanup will be handled by game_state_destroy */
    } else {
        LOG_INFO("Archon Path systems initialized successfully");
    }

    LOG_INFO("Narrative systems initialized successfully");

    /* Initialize resources */
    resources_init(&state->resources);

    /* Initialize corruption */
    corruption_init(&state->corruption);

    /* Initialize consciousness */
    consciousness_init(&state->consciousness);

    /* Initialize combat (NULL - not in combat) */
    state->combat = NULL;

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

    /* Destroy combat state if active */
    if (state->combat) {
        /* Forward declaration - will include combat.h when building */
        extern void combat_state_destroy(void*);
        combat_state_destroy(state->combat);
    }

    /* Destroy narrative systems */
    extern void archon_trial_manager_destroy(ArchonTrialManager*);
    extern void divine_judgment_destroy(DivineJudgmentState*);
    extern void network_patching_destroy(NetworkPatchingState*);
    extern void split_routing_manager_destroy(SplitRoutingManager*);
    extern void purge_system_destroy(PurgeState*);
    extern void archon_state_destroy(ArchonState*);
    extern void reformation_program_destroy(ReformationProgram*);

    reformation_program_destroy(state->reformation_program);
    archon_state_destroy(state->archon_state);
    purge_system_destroy(state->purge_state);
    split_routing_manager_destroy(state->split_routing);
    network_patching_destroy(state->network_patching);
    divine_judgment_destroy(state->divine_judgment);
    archon_trial_manager_destroy(state->archon_trials);
    ending_system_destroy(state->ending_system);
    event_scheduler_destroy(state->event_scheduler);

    /* Destroy divine council */
    extern void divine_council_destroy(DivineCouncil*);
    divine_council_destroy(state->divine_council);

    null_space_destroy(state->null_space);
    thessara_destroy(state->thessara);
    dialogue_manager_destroy(state->dialogues);
    quest_manager_destroy(state->quests);
    relationship_manager_destroy(state->relationships);
    npc_manager_destroy(state->npcs);
    memory_manager_destroy(state->memories);

    /* Destroy core systems */
    soul_manager_destroy(state->souls);
    minion_manager_destroy(state->minions);
    death_network_destroy(state->death_network);
    territory_status_destroy(state->territory_status);
    world_map_destroy(state->world_map);
    location_graph_destroy(state->location_graph);
    territory_manager_destroy(state->territory);

    free(state);
    LOG_INFO("Game state destroyed");
}

GameState* game_state_get_instance(void) {
    extern GameState* g_game_state;
    return g_game_state;
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

    /* Record previous month for consciousness decay tracking */
    uint32_t previous_month = resources_get_months_elapsed(&state->resources);

    /* Advance time */
    resources_advance_time(&state->resources, hours);

    /* Check if we crossed a month boundary */
    uint32_t current_month = resources_get_months_elapsed(&state->resources);
    if (current_month > previous_month) {
        /* Apply consciousness decay for the new month */
        consciousness_apply_decay(&state->consciousness, current_month);
        LOG_DEBUG("Month boundary crossed (%u -> %u), consciousness decayed to %.1f%%",
                 previous_month, current_month, state->consciousness.stability);
    }

    /* Regenerate mana (10 per hour) */
    uint32_t mana_regen = hours * 10;
    resources_add_mana(&state->resources, mana_regen);

    /* Update Death Network (regenerate corpses, decay signatures, random events) */
    if (state->death_network) {
        death_network_update(state->death_network, hours);
    }

    /* Check for triggered events */
    if (state->event_scheduler) {
        uint32_t triggered = event_scheduler_check_triggers(state->event_scheduler, state);
        if (triggered > 0) {
            LOG_INFO("Triggered %u event(s) on day %u", triggered, state->resources.day_count);
        }
    }

    LOG_DEBUG("Advanced time by %u hours (mana regen: %u)", hours, mana_regen);
}

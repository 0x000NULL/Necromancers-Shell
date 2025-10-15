/**
 * @file npc_manager.c
 * @brief NPC manager implementation
 */

#include "npc_manager.h"
#include "../../../data/data_loader.h"
#include "../../../utils/logger.h"
#include <stdlib.h>
#include <string.h>

NPCManager* npc_manager_create(void) {
    NPCManager* manager = malloc(sizeof(NPCManager));
    if (!manager) {
        LOG_ERROR("npc_manager_create: malloc failed");
        return NULL;
    }

    manager->npc_capacity = 16;
    manager->npc_count = 0;
    manager->npcs = calloc(manager->npc_capacity, sizeof(NPC*));

    if (!manager->npcs) {
        LOG_ERROR("npc_manager_create: calloc failed");
        free(manager);
        return NULL;
    }

    LOG_DEBUG("NPC manager created");
    return manager;
}

void npc_manager_destroy(NPCManager* manager) {
    if (!manager) return;

    /* Destroy all NPCs */
    for (size_t i = 0; i < manager->npc_count; i++) {
        npc_destroy(manager->npcs[i]);
    }

    free(manager->npcs);
    free(manager);

    LOG_DEBUG("NPC manager destroyed");
}

void npc_manager_add_npc(NPCManager* manager, NPC* npc) {
    if (!manager || !npc) return;

    /* Resize if needed */
    if (manager->npc_count >= manager->npc_capacity) {
        size_t new_capacity = manager->npc_capacity * 2;
        NPC** new_npcs = realloc(manager->npcs, new_capacity * sizeof(NPC*));
        if (!new_npcs) {
            LOG_ERROR("npc_manager_add_npc: realloc failed");
            return;
        }

        manager->npcs = new_npcs;
        manager->npc_capacity = new_capacity;

        /* Zero new memory */
        for (size_t i = manager->npc_count; i < new_capacity; i++) {
            manager->npcs[i] = NULL;
        }
    }

    manager->npcs[manager->npc_count++] = npc;
    LOG_DEBUG("Added NPC: %s", npc->id);
}

NPC* npc_manager_get_npc(const NPCManager* manager, const char* npc_id) {
    if (!manager || !npc_id) return NULL;

    for (size_t i = 0; i < manager->npc_count; i++) {
        if (strcmp(manager->npcs[i]->id, npc_id) == 0) {
            return manager->npcs[i];
        }
    }

    return NULL;
}

NPC** npc_manager_get_discovered(const NPCManager* manager, size_t* count_out) {
    if (!manager || !count_out) {
        if (count_out) *count_out = 0;
        return NULL;
    }

    /* Count discovered NPCs */
    size_t count = 0;
    for (size_t i = 0; i < manager->npc_count; i++) {
        if (manager->npcs[i]->discovered) {
            count++;
        }
    }

    if (count == 0) {
        *count_out = 0;
        return NULL;
    }

    /* Allocate result array */
    NPC** result = malloc(count * sizeof(NPC*));
    if (!result) {
        LOG_ERROR("npc_manager_get_discovered: malloc failed");
        *count_out = 0;
        return NULL;
    }

    /* Fill array */
    size_t idx = 0;
    for (size_t i = 0; i < manager->npc_count; i++) {
        if (manager->npcs[i]->discovered) {
            result[idx++] = manager->npcs[i];
        }
    }

    *count_out = count;
    return result;
}

NPC** npc_manager_get_at_location(const NPCManager* manager, const char* location, size_t* count_out) {
    if (!manager || !location || !count_out) {
        if (count_out) *count_out = 0;
        return NULL;
    }

    /* Count matching NPCs */
    size_t count = 0;
    for (size_t i = 0; i < manager->npc_count; i++) {
        if (strcmp(manager->npcs[i]->current_location, location) == 0) {
            count++;
        }
    }

    if (count == 0) {
        *count_out = 0;
        return NULL;
    }

    /* Allocate result array */
    NPC** result = malloc(count * sizeof(NPC*));
    if (!result) {
        LOG_ERROR("npc_manager_get_at_location: malloc failed");
        *count_out = 0;
        return NULL;
    }

    /* Fill array */
    size_t idx = 0;
    for (size_t i = 0; i < manager->npc_count; i++) {
        if (strcmp(manager->npcs[i]->current_location, location) == 0) {
            result[idx++] = manager->npcs[i];
        }
    }

    *count_out = count;
    return result;
}

NPC** npc_manager_get_available(const NPCManager* manager, size_t* count_out) {
    if (!manager || !count_out) {
        if (count_out) *count_out = 0;
        return NULL;
    }

    /* Count available NPCs */
    size_t count = 0;
    for (size_t i = 0; i < manager->npc_count; i++) {
        if (npc_is_available(manager->npcs[i])) {
            count++;
        }
    }

    if (count == 0) {
        *count_out = 0;
        return NULL;
    }

    /* Allocate result array */
    NPC** result = malloc(count * sizeof(NPC*));
    if (!result) {
        LOG_ERROR("npc_manager_get_available: malloc failed");
        *count_out = 0;
        return NULL;
    }

    /* Fill array */
    size_t idx = 0;
    for (size_t i = 0; i < manager->npc_count; i++) {
        if (npc_is_available(manager->npcs[i])) {
            result[idx++] = manager->npcs[i];
        }
    }

    *count_out = count;
    return result;
}

NPC** npc_manager_get_by_archetype(const NPCManager* manager, NPCArchetype archetype, size_t* count_out) {
    if (!manager || !count_out) {
        if (count_out) *count_out = 0;
        return NULL;
    }

    /* Count matching NPCs */
    size_t count = 0;
    for (size_t i = 0; i < manager->npc_count; i++) {
        if (manager->npcs[i]->archetype == archetype) {
            count++;
        }
    }

    if (count == 0) {
        *count_out = 0;
        return NULL;
    }

    /* Allocate result array */
    NPC** result = malloc(count * sizeof(NPC*));
    if (!result) {
        LOG_ERROR("npc_manager_get_by_archetype: malloc failed");
        *count_out = 0;
        return NULL;
    }

    /* Fill array */
    size_t idx = 0;
    for (size_t i = 0; i < manager->npc_count; i++) {
        if (manager->npcs[i]->archetype == archetype) {
            result[idx++] = manager->npcs[i];
        }
    }

    *count_out = count;
    return result;
}

NPC** npc_manager_get_by_faction(const NPCManager* manager, const char* faction, size_t* count_out) {
    if (!manager || !faction || !count_out) {
        if (count_out) *count_out = 0;
        return NULL;
    }

    /* Count matching NPCs */
    size_t count = 0;
    for (size_t i = 0; i < manager->npc_count; i++) {
        if (strcmp(manager->npcs[i]->faction, faction) == 0) {
            count++;
        }
    }

    if (count == 0) {
        *count_out = 0;
        return NULL;
    }

    /* Allocate result array */
    NPC** result = malloc(count * sizeof(NPC*));
    if (!result) {
        LOG_ERROR("npc_manager_get_by_faction: malloc failed");
        *count_out = 0;
        return NULL;
    }

    /* Fill array */
    size_t idx = 0;
    for (size_t i = 0; i < manager->npc_count; i++) {
        if (strcmp(manager->npcs[i]->faction, faction) == 0) {
            result[idx++] = manager->npcs[i];
        }
    }

    *count_out = count;
    return result;
}

NPC** npc_manager_get_with_active_quests(const NPCManager* manager, size_t* count_out) {
    if (!manager || !count_out) {
        if (count_out) *count_out = 0;
        return NULL;
    }

    /* Count NPCs with active quests */
    size_t count = 0;
    for (size_t i = 0; i < manager->npc_count; i++) {
        if (manager->npcs[i]->active_quest_count > 0) {
            count++;
        }
    }

    if (count == 0) {
        *count_out = 0;
        return NULL;
    }

    /* Allocate result array */
    NPC** result = malloc(count * sizeof(NPC*));
    if (!result) {
        LOG_ERROR("npc_manager_get_with_active_quests: malloc failed");
        *count_out = 0;
        return NULL;
    }

    /* Fill array */
    size_t idx = 0;
    for (size_t i = 0; i < manager->npc_count; i++) {
        if (manager->npcs[i]->active_quest_count > 0) {
            result[idx++] = manager->npcs[i];
        }
    }

    *count_out = count;
    return result;
}

void npc_manager_discover_npc(NPCManager* manager, const char* npc_id, const char* location) {
    if (!manager || !npc_id) return;

    NPC* npc = npc_manager_get_npc(manager, npc_id);
    if (!npc) {
        LOG_WARN("Cannot discover unknown NPC: %s", npc_id);
        return;
    }

    npc_discover(npc, location);
}

bool npc_manager_load_from_file(NPCManager* manager, const char* filepath) {
    if (!manager || !filepath) {
        LOG_ERROR("npc_manager_load_from_file: NULL parameters");
        return false;
    }

    DataFile* file = data_file_load(filepath);
    if (!file) {
        LOG_ERROR("Failed to load NPCs: %s", filepath);
        return false;
    }

    /* Get all NPC sections */
    size_t section_count;
    const DataSection** sections = data_file_get_sections(file, "NPC", &section_count);

    if (!sections) {
        LOG_WARN("No NPC sections found in %s", filepath);
        data_file_destroy(file);
        return true; /* Not an error, just empty */
    }

    /* Parse each NPC */
    for (size_t i = 0; i < section_count; i++) {
        const DataSection* section = sections[i];

        /* Extract NPC data */
        const char* id = section->section_id;
        const char* name = data_value_get_string(data_section_get(section, "name"), "Unnamed");
        const char* archetype_str = data_value_get_string(data_section_get(section, "archetype"), "neutral");

        /* Parse archetype */
        NPCArchetype archetype = NPC_ARCHETYPE_NEUTRAL;
        if (strcmp(archetype_str, "mentor") == 0) archetype = NPC_ARCHETYPE_MENTOR;
        else if (strcmp(archetype_str, "rival") == 0) archetype = NPC_ARCHETYPE_RIVAL;
        else if (strcmp(archetype_str, "ally") == 0) archetype = NPC_ARCHETYPE_ALLY;
        else if (strcmp(archetype_str, "antagonist") == 0) archetype = NPC_ARCHETYPE_ANTAGONIST;
        else if (strcmp(archetype_str, "mysterious") == 0) archetype = NPC_ARCHETYPE_MYSTERIOUS;

        /* Create NPC */
        NPC* npc = npc_create(id, name, archetype);
        if (!npc) {
            LOG_WARN("Failed to create NPC: %s", id);
            continue;
        }

        /* Set additional properties */
        const char* title = data_value_get_string(data_section_get(section, "title"), "");
        if (title[0]) {
            strncpy(npc->title, title, sizeof(npc->title) - 1);
        }

        const char* description = data_value_get_string(data_section_get(section, "description"), "");
        if (description[0]) {
            strncpy(npc->description, description, sizeof(npc->description) - 1);
        }

        const char* faction = data_value_get_string(data_section_get(section, "faction"), "");
        if (faction[0]) {
            strncpy(npc->faction, faction, sizeof(npc->faction) - 1);
        }

        /* Location settings */
        const char* location_type_str = data_value_get_string(data_section_get(section, "location_type"), "unknown");
        if (strcmp(location_type_str, "fixed") == 0) {
            npc->location_type = NPC_LOCATION_FIXED;
        } else if (strcmp(location_type_str, "mobile") == 0) {
            npc->location_type = NPC_LOCATION_MOBILE;
        } else if (strcmp(location_type_str, "quest_based") == 0) {
            npc->location_type = NPC_LOCATION_QUEST_BASED;
        }

        const char* home_location = data_value_get_string(data_section_get(section, "home_location"), "");
        if (home_location[0]) {
            strncpy(npc->home_location, home_location, sizeof(npc->home_location) - 1);
            strncpy(npc->current_location, home_location, sizeof(npc->current_location) - 1);
        }

        /* Flags */
        npc->available = data_value_get_bool(data_section_get(section, "available"), true);
        npc->is_hostile = data_value_get_bool(data_section_get(section, "hostile"), false);
        npc->is_hidden = data_value_get_bool(data_section_get(section, "hidden"), false);

        /* Auto-discover if not hidden */
        if (!npc->is_hidden) {
            npc->discovered = true;
            npc->first_met_time = 0; /* Will be set when actually met */
        }

        /* Parse dialogue states */
        size_t state_count = 0;
        const char** states = data_value_get_array(data_section_get(section, "dialogue_state"), &state_count);
        for (size_t j = 0; j < state_count && j < MAX_NPC_DIALOGUE_STATES; j++) {
            npc_add_dialogue_state(npc, states[j]);
        }

        /* Parse unlockable memories */
        size_t memory_count = 0;
        const char** memories = data_value_get_array(data_section_get(section, "unlockable_memory"), &memory_count);
        for (size_t j = 0; j < memory_count && j < MAX_NPC_MEMORIES; j++) {
            npc_add_unlockable_memory(npc, memories[j]);
        }

        /* Add to manager */
        npc_manager_add_npc(manager, npc);
    }

    free(sections);
    data_file_destroy(file);

    LOG_INFO("Loaded %zu NPCs from %s", section_count, filepath);
    return true;
}

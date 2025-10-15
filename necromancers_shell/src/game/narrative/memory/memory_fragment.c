/**
 * @file memory_fragment.c
 * @brief Memory fragment implementation
 */

#include "memory_fragment.h"
#include "../../../utils/logger.h"
#include <stdlib.h>
#include <string.h>

MemoryFragment* memory_fragment_create(const char* id, const char* title, const char* content) {
    if (!id || !title || !content) {
        LOG_ERROR("memory_fragment_create: NULL parameters");
        return NULL;
    }

    MemoryFragment* fragment = malloc(sizeof(MemoryFragment));
    if (!fragment) {
        LOG_ERROR("memory_fragment_create: malloc failed");
        return NULL;
    }

    /* Initialize structure */
    memset(fragment, 0, sizeof(MemoryFragment));

    /* Copy basic data */
    strncpy(fragment->id, id, sizeof(fragment->id) - 1);
    strncpy(fragment->title, title, sizeof(fragment->title) - 1);
    strncpy(fragment->content, content, sizeof(fragment->content) - 1);

    /* Initialize defaults */
    fragment->discovered = false;
    fragment->discovery_time = 0;
    fragment->chronological_order = 0;
    fragment->related_count = 0;
    fragment->npc_count = 0;
    fragment->location_count = 0;
    fragment->key_memory = false;
    fragment->hidden = false;

    return fragment;
}

void memory_fragment_destroy(MemoryFragment* fragment) {
    if (!fragment) return;
    free(fragment);
}

void memory_fragment_discover(MemoryFragment* fragment, const char* location, const char* method) {
    if (!fragment) return;

    fragment->discovered = true;
    fragment->discovery_time = time(NULL);

    if (location) {
        strncpy(fragment->discovery_location, location, sizeof(fragment->discovery_location) - 1);
    }

    if (method) {
        strncpy(fragment->discovery_method, method, sizeof(fragment->discovery_method) - 1);
    }

    LOG_INFO("Memory discovered: %s (%s at %s)",
             fragment->title, method ? method : "unknown",
             location ? location : "unknown");
}

void memory_fragment_add_related(MemoryFragment* fragment, const char* fragment_id) {
    if (!fragment || !fragment_id) return;

    if (fragment->related_count >= MAX_FRAGMENT_CROSS_REFS) {
        LOG_WARN("Memory fragment %s: max related fragments reached", fragment->id);
        return;
    }

    strncpy(fragment->related_fragments[fragment->related_count], fragment_id, 63);
    fragment->related_fragments[fragment->related_count][63] = '\0';
    fragment->related_count++;
}

void memory_fragment_add_npc(MemoryFragment* fragment, const char* npc_id) {
    if (!fragment || !npc_id) return;

    if (fragment->npc_count >= MAX_FRAGMENT_CROSS_REFS) {
        LOG_WARN("Memory fragment %s: max related NPCs reached", fragment->id);
        return;
    }

    strncpy(fragment->related_npcs[fragment->npc_count], npc_id, 63);
    fragment->related_npcs[fragment->npc_count][63] = '\0';
    fragment->npc_count++;
}

void memory_fragment_add_location(MemoryFragment* fragment, const char* location_id) {
    if (!fragment || !location_id) return;

    if (fragment->location_count >= MAX_FRAGMENT_CROSS_REFS) {
        LOG_WARN("Memory fragment %s: max related locations reached", fragment->id);
        return;
    }

    strncpy(fragment->related_locations[fragment->location_count], location_id, 63);
    fragment->related_locations[fragment->location_count][63] = '\0';
    fragment->location_count++;
}

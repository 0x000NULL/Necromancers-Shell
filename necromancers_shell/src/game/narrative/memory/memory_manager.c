/**
 * @file memory_manager.c
 * @brief Memory manager implementation
 */

#include "memory_manager.h"
#include "../../../data/data_loader.h"
#include "../../../utils/logger.h"
#include <stdlib.h>
#include <string.h>

/* Comparison function for qsort (chronological order) */
static int compare_chronological(const void* a, const void* b) {
    const MemoryFragment* frag_a = *(const MemoryFragment**)a;
    const MemoryFragment* frag_b = *(const MemoryFragment**)b;
    return frag_a->chronological_order - frag_b->chronological_order;
}

MemoryManager* memory_manager_create(void) {
    MemoryManager* manager = malloc(sizeof(MemoryManager));
    if (!manager) {
        LOG_ERROR("memory_manager_create: malloc failed");
        return NULL;
    }

    manager->fragment_capacity = 32;
    manager->fragment_count = 0;
    manager->fragments = calloc(manager->fragment_capacity, sizeof(MemoryFragment*));

    if (!manager->fragments) {
        LOG_ERROR("memory_manager_create: calloc failed");
        free(manager);
        return NULL;
    }

    LOG_DEBUG("Memory manager created");
    return manager;
}

void memory_manager_destroy(MemoryManager* manager) {
    if (!manager) return;

    /* Destroy all fragments */
    for (size_t i = 0; i < manager->fragment_count; i++) {
        memory_fragment_destroy(manager->fragments[i]);
    }

    free(manager->fragments);
    free(manager);

    LOG_DEBUG("Memory manager destroyed");
}

void memory_manager_add_fragment(MemoryManager* manager, MemoryFragment* fragment) {
    if (!manager || !fragment) return;

    /* Resize if needed */
    if (manager->fragment_count >= manager->fragment_capacity) {
        size_t new_capacity = manager->fragment_capacity * 2;
        MemoryFragment** new_fragments = realloc(manager->fragments,
                                                  new_capacity * sizeof(MemoryFragment*));
        if (!new_fragments) {
            LOG_ERROR("memory_manager_add_fragment: realloc failed");
            return;
        }

        manager->fragments = new_fragments;
        manager->fragment_capacity = new_capacity;

        /* Zero new memory */
        for (size_t i = manager->fragment_count; i < new_capacity; i++) {
            manager->fragments[i] = NULL;
        }
    }

    manager->fragments[manager->fragment_count++] = fragment;
    LOG_DEBUG("Added memory fragment: %s", fragment->id);
}

MemoryFragment* memory_manager_get_fragment(const MemoryManager* manager, const char* fragment_id) {
    if (!manager || !fragment_id) return NULL;

    for (size_t i = 0; i < manager->fragment_count; i++) {
        if (strcmp(manager->fragments[i]->id, fragment_id) == 0) {
            return manager->fragments[i];
        }
    }

    return NULL;
}

MemoryFragment** memory_manager_get_discovered(const MemoryManager* manager, size_t* count_out) {
    if (!manager || !count_out) {
        if (count_out) *count_out = 0;
        return NULL;
    }

    /* Count discovered fragments */
    size_t count = 0;
    for (size_t i = 0; i < manager->fragment_count; i++) {
        if (manager->fragments[i]->discovered) {
            count++;
        }
    }

    if (count == 0) {
        *count_out = 0;
        return NULL;
    }

    /* Allocate result array */
    MemoryFragment** result = malloc(count * sizeof(MemoryFragment*));
    if (!result) {
        LOG_ERROR("memory_manager_get_discovered: malloc failed");
        *count_out = 0;
        return NULL;
    }

    /* Fill array */
    size_t idx = 0;
    for (size_t i = 0; i < manager->fragment_count; i++) {
        if (manager->fragments[i]->discovered) {
            result[idx++] = manager->fragments[i];
        }
    }

    *count_out = count;
    return result;
}

MemoryFragment** memory_manager_get_by_category(const MemoryManager* manager,
                                                 const char* category,
                                                 size_t* count_out) {
    if (!manager || !category || !count_out) {
        if (count_out) *count_out = 0;
        return NULL;
    }

    /* Count matching fragments */
    size_t count = 0;
    for (size_t i = 0; i < manager->fragment_count; i++) {
        if (manager->fragments[i]->discovered &&
            strcmp(manager->fragments[i]->category, category) == 0) {
            count++;
        }
    }

    if (count == 0) {
        *count_out = 0;
        return NULL;
    }

    /* Allocate result array */
    MemoryFragment** result = malloc(count * sizeof(MemoryFragment*));
    if (!result) {
        LOG_ERROR("memory_manager_get_by_category: malloc failed");
        *count_out = 0;
        return NULL;
    }

    /* Fill array */
    size_t idx = 0;
    for (size_t i = 0; i < manager->fragment_count; i++) {
        if (manager->fragments[i]->discovered &&
            strcmp(manager->fragments[i]->category, category) == 0) {
            result[idx++] = manager->fragments[i];
        }
    }

    *count_out = count;
    return result;
}

MemoryFragment** memory_manager_get_chronological(const MemoryManager* manager, size_t* count_out) {
    if (!manager || !count_out) {
        if (count_out) *count_out = 0;
        return NULL;
    }

    /* Get discovered fragments */
    MemoryFragment** result = memory_manager_get_discovered(manager, count_out);
    if (!result) {
        return NULL;
    }

    /* Sort by chronological order */
    qsort(result, *count_out, sizeof(MemoryFragment*), compare_chronological);

    return result;
}

MemoryFragment** memory_manager_get_related(const MemoryManager* manager,
                                             const char* fragment_id,
                                             size_t* count_out) {
    if (!manager || !fragment_id || !count_out) {
        if (count_out) *count_out = 0;
        return NULL;
    }

    /* Find source fragment */
    MemoryFragment* source = memory_manager_get_fragment(manager, fragment_id);
    if (!source || source->related_count == 0) {
        *count_out = 0;
        return NULL;
    }

    /* Allocate result array */
    MemoryFragment** result = malloc(source->related_count * sizeof(MemoryFragment*));
    if (!result) {
        LOG_ERROR("memory_manager_get_related: malloc failed");
        *count_out = 0;
        return NULL;
    }

    /* Fill with discovered related fragments */
    size_t idx = 0;
    for (size_t i = 0; i < source->related_count; i++) {
        MemoryFragment* related = memory_manager_get_fragment(manager, source->related_fragments[i]);
        if (related && related->discovered) {
            result[idx++] = related;
        }
    }

    if (idx == 0) {
        free(result);
        *count_out = 0;
        return NULL;
    }

    *count_out = idx;
    return result;
}

void memory_manager_discover_fragment(MemoryManager* manager,
                                      const char* fragment_id,
                                      const char* location,
                                      const char* method) {
    if (!manager || !fragment_id) return;

    MemoryFragment* fragment = memory_manager_get_fragment(manager, fragment_id);
    if (!fragment) {
        LOG_WARN("Cannot discover unknown fragment: %s", fragment_id);
        return;
    }

    memory_fragment_discover(fragment, location, method);
}

bool memory_manager_load_from_file(MemoryManager* manager, const char* filepath) {
    if (!manager || !filepath) {
        LOG_ERROR("memory_manager_load_from_file: NULL parameters");
        return false;
    }

    DataFile* file = data_file_load(filepath);
    if (!file) {
        LOG_ERROR("Failed to load memory fragments: %s", filepath);
        return false;
    }

    /* Get all FRAGMENT sections */
    size_t section_count;
    const DataSection** sections = data_file_get_sections(file, "FRAGMENT", &section_count);

    if (!sections) {
        LOG_WARN("No FRAGMENT sections found in %s", filepath);
        data_file_destroy(file);
        return true; /* Not an error, just empty */
    }

    /* Parse each fragment */
    for (size_t i = 0; i < section_count; i++) {
        const DataSection* section = sections[i];

        /* Extract fragment data */
        const char* id = section->section_id;
        const char* title = data_value_get_string(data_section_get(section, "title"), "Untitled");
        const char* content = data_value_get_string(data_section_get(section, "content"), "");

        /* Create fragment */
        MemoryFragment* fragment = memory_fragment_create(id, title, content);
        if (!fragment) {
            LOG_WARN("Failed to create fragment: %s", id);
            continue;
        }

        /* Set additional properties */
        const char* category = data_value_get_string(data_section_get(section, "category"), "unknown");
        strncpy(fragment->category, category, sizeof(fragment->category) - 1);

        fragment->chronological_order = data_value_get_int(data_section_get(section, "chronological_order"), 0);
        fragment->key_memory = data_value_get_bool(data_section_get(section, "key_memory"), false);
        fragment->hidden = data_value_get_bool(data_section_get(section, "hidden"), false);

        /* Discovery settings */
        const char* disc_location = data_value_get_string(data_section_get(section, "discovery_location"), "");
        if (disc_location[0]) {
            strncpy(fragment->discovery_location, disc_location, sizeof(fragment->discovery_location) - 1);
        }

        const char* disc_method = data_value_get_string(data_section_get(section, "discovery_method"), "");
        if (disc_method[0]) {
            strncpy(fragment->discovery_method, disc_method, sizeof(fragment->discovery_method) - 1);
        }

        /* Auto-discover if method is "automatic" */
        if (strcmp(disc_method, "automatic") == 0) {
            fragment->discovered = true;
            fragment->discovery_time = time(NULL);
        }

        /* Parse cross-references (arrays) */
        size_t related_count = 0;
        const char** related = data_value_get_array(data_section_get(section, "related_fragment"), &related_count);
        for (size_t j = 0; j < related_count && j < MAX_FRAGMENT_CROSS_REFS; j++) {
            memory_fragment_add_related(fragment, related[j]);
        }

        size_t npc_count = 0;
        const char** npcs = data_value_get_array(data_section_get(section, "related_npc"), &npc_count);
        for (size_t j = 0; j < npc_count && j < MAX_FRAGMENT_CROSS_REFS; j++) {
            memory_fragment_add_npc(fragment, npcs[j]);
        }

        size_t loc_count = 0;
        const char** locations = data_value_get_array(data_section_get(section, "related_location"), &loc_count);
        for (size_t j = 0; j < loc_count && j < MAX_FRAGMENT_CROSS_REFS; j++) {
            memory_fragment_add_location(fragment, locations[j]);
        }

        /* Add to manager */
        memory_manager_add_fragment(manager, fragment);
    }

    free(sections);
    data_file_destroy(file);

    LOG_INFO("Loaded %zu memory fragments from %s", section_count, filepath);
    return true;
}

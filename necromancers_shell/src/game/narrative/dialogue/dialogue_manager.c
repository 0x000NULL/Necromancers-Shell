/**
 * @file dialogue_manager.c
 * @brief Dialogue manager implementation
 */

#include "dialogue_manager.h"
#include "../../../data/data_loader.h"
#include "../../../utils/logger.h"
#include <stdlib.h>
#include <string.h>

DialogueManager* dialogue_manager_create(void) {
    DialogueManager* manager = malloc(sizeof(DialogueManager));
    if (!manager) {
        LOG_ERROR("dialogue_manager_create: malloc failed");
        return NULL;
    }

    manager->tree_capacity = 16;
    manager->tree_count = 0;
    manager->trees = calloc(manager->tree_capacity, sizeof(DialogueTree*));

    if (!manager->trees) {
        LOG_ERROR("dialogue_manager_create: calloc failed");
        free(manager);
        return NULL;
    }

    manager->active_tree = NULL;

    LOG_DEBUG("Dialogue manager created");
    return manager;
}

void dialogue_manager_destroy(DialogueManager* manager) {
    if (!manager) return;

    /* Destroy all trees */
    for (size_t i = 0; i < manager->tree_count; i++) {
        dialogue_tree_destroy(manager->trees[i]);
    }

    free(manager->trees);
    free(manager);

    LOG_DEBUG("Dialogue manager destroyed");
}

void dialogue_manager_add_tree(DialogueManager* manager, DialogueTree* tree) {
    if (!manager || !tree) return;

    /* Resize if needed */
    if (manager->tree_count >= manager->tree_capacity) {
        size_t new_capacity = manager->tree_capacity * 2;
        DialogueTree** new_trees = realloc(manager->trees, new_capacity * sizeof(DialogueTree*));
        if (!new_trees) {
            LOG_ERROR("dialogue_manager_add_tree: realloc failed");
            return;
        }

        manager->trees = new_trees;
        manager->tree_capacity = new_capacity;

        /* Zero new memory */
        for (size_t i = manager->tree_count; i < new_capacity; i++) {
            manager->trees[i] = NULL;
        }
    }

    manager->trees[manager->tree_count++] = tree;
    LOG_DEBUG("Added dialogue tree: %s", tree->id);
}

DialogueTree* dialogue_manager_get_tree(const DialogueManager* manager, const char* tree_id) {
    if (!manager || !tree_id) return NULL;

    for (size_t i = 0; i < manager->tree_count; i++) {
        if (strcmp(manager->trees[i]->id, tree_id) == 0) {
            return manager->trees[i];
        }
    }

    return NULL;
}

DialogueTree** dialogue_manager_get_by_npc(const DialogueManager* manager,
                                            const char* npc_id,
                                            size_t* count_out) {
    if (!manager || !npc_id || !count_out) {
        if (count_out) *count_out = 0;
        return NULL;
    }

    /* Count matching trees */
    size_t count = 0;
    for (size_t i = 0; i < manager->tree_count; i++) {
        if (strcmp(manager->trees[i]->npc_id, npc_id) == 0) {
            count++;
        }
    }

    if (count == 0) {
        *count_out = 0;
        return NULL;
    }

    /* Allocate result array */
    DialogueTree** result = malloc(count * sizeof(DialogueTree*));
    if (!result) {
        LOG_ERROR("dialogue_manager_get_by_npc: malloc failed");
        *count_out = 0;
        return NULL;
    }

    /* Fill array */
    size_t idx = 0;
    for (size_t i = 0; i < manager->tree_count; i++) {
        if (strcmp(manager->trees[i]->npc_id, npc_id) == 0) {
            result[idx++] = manager->trees[i];
        }
    }

    *count_out = count;
    return result;
}

bool dialogue_manager_start_dialogue(DialogueManager* manager, const char* tree_id) {
    if (!manager || !tree_id) return false;

    /* End any active dialogue */
    if (manager->active_tree) {
        dialogue_manager_end_dialogue(manager);
    }

    /* Find and start tree */
    DialogueTree* tree = dialogue_manager_get_tree(manager, tree_id);
    if (!tree) {
        LOG_WARN("Cannot start unknown dialogue tree: %s", tree_id);
        return false;
    }

    dialogue_tree_start(tree);
    manager->active_tree = tree;

    return true;
}

void dialogue_manager_end_dialogue(DialogueManager* manager) {
    if (!manager || !manager->active_tree) return;

    dialogue_tree_end(manager->active_tree);
    manager->active_tree = NULL;
}

DialogueTree* dialogue_manager_get_active(const DialogueManager* manager) {
    if (!manager) return NULL;
    return manager->active_tree;
}

DialogueNode* dialogue_manager_get_current_node(const DialogueManager* manager) {
    if (!manager || !manager->active_tree) return NULL;

    return dialogue_tree_get_current_node(manager->active_tree);
}

bool dialogue_manager_choose(DialogueManager* manager, int choice_index) {
    if (!manager || !manager->active_tree) return false;

    bool success = dialogue_tree_choose(manager->active_tree, choice_index);

    /* If dialogue ended, clear active tree */
    if (!dialogue_tree_is_active(manager->active_tree)) {
        manager->active_tree = NULL;
    }

    return success;
}

bool dialogue_manager_is_active(const DialogueManager* manager) {
    if (!manager) return false;
    return manager->active_tree != NULL && dialogue_tree_is_active(manager->active_tree);
}

bool dialogue_manager_load_from_file(DialogueManager* manager, const char* filepath) {
    if (!manager || !filepath) {
        LOG_ERROR("dialogue_manager_load_from_file: NULL parameters");
        return false;
    }

    DataFile* file = data_file_load(filepath);
    if (!file) {
        LOG_ERROR("Failed to load dialogue trees: %s", filepath);
        return false;
    }

    /* Get all DIALOGUE sections */
    size_t section_count;
    const DataSection** sections = data_file_get_sections(file, "DIALOGUE", &section_count);

    if (!sections) {
        LOG_WARN("No DIALOGUE sections found in %s", filepath);
        data_file_destroy(file);
        return true; /* Not an error, just empty */
    }

    /* Parse each dialogue tree */
    for (size_t i = 0; i < section_count; i++) {
        const DataSection* section = sections[i];

        /* Extract dialogue data */
        const char* id = section->section_id;
        const char* npc_id = data_value_get_string(data_section_get(section, "npc_id"), "");
        const char* root_node = data_value_get_string(data_section_get(section, "root_node"), "start");
        const char* title = data_value_get_string(data_section_get(section, "title"), "");

        /* Create tree */
        DialogueTree* tree = dialogue_tree_create(id, npc_id, root_node);
        if (!tree) {
            LOG_WARN("Failed to create dialogue tree: %s", id);
            continue;
        }

        /* Set title */
        if (title[0]) {
            strncpy(tree->title, title, sizeof(tree->title) - 1);
        }

        /* Add to manager */
        dialogue_manager_add_tree(manager, tree);
    }

    free(sections);
    data_file_destroy(file);

    LOG_INFO("Loaded %zu dialogue trees from %s", section_count, filepath);
    return true;
}

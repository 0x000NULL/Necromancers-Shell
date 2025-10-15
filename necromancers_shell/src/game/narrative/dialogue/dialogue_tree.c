/**
 * @file dialogue_tree.c
 * @brief Dialogue tree implementation
 */

#include "dialogue_tree.h"
#include "../../../utils/logger.h"
#include <stdlib.h>
#include <string.h>

DialogueTree* dialogue_tree_create(const char* id, const char* npc_id, const char* root_node_id) {
    if (!id || !npc_id || !root_node_id) {
        LOG_ERROR("dialogue_tree_create: NULL parameters");
        return NULL;
    }

    DialogueTree* tree = malloc(sizeof(DialogueTree));
    if (!tree) {
        LOG_ERROR("dialogue_tree_create: malloc failed");
        return NULL;
    }

    /* Initialize structure */
    memset(tree, 0, sizeof(DialogueTree));

    /* Copy data */
    strncpy(tree->id, id, sizeof(tree->id) - 1);
    strncpy(tree->npc_id, npc_id, sizeof(tree->npc_id) - 1);
    strncpy(tree->root_node_id, root_node_id, sizeof(tree->root_node_id) - 1);

    /* Initialize state */
    tree->node_count = 0;
    tree->is_active = false;

    return tree;
}

void dialogue_tree_destroy(DialogueTree* tree) {
    if (!tree) return;

    /* Destroy all nodes */
    for (size_t i = 0; i < tree->node_count; i++) {
        dialogue_node_destroy(tree->nodes[i]);
    }

    free(tree);
}

void dialogue_tree_add_node(DialogueTree* tree, DialogueNode* node) {
    if (!tree || !node) return;

    if (tree->node_count >= MAX_DIALOGUE_NODES) {
        LOG_WARN("Dialogue tree %s: max nodes reached", tree->id);
        return;
    }

    tree->nodes[tree->node_count++] = node;
    LOG_DEBUG("Dialogue tree %s: added node %s", tree->id, node->id);
}

DialogueNode* dialogue_tree_get_node(const DialogueTree* tree, const char* node_id) {
    if (!tree || !node_id) return NULL;

    for (size_t i = 0; i < tree->node_count; i++) {
        if (strcmp(tree->nodes[i]->id, node_id) == 0) {
            return tree->nodes[i];
        }
    }

    return NULL;
}

void dialogue_tree_start(DialogueTree* tree) {
    if (!tree) return;

    tree->is_active = true;
    strncpy(tree->current_node_id, tree->root_node_id, sizeof(tree->current_node_id) - 1);
    tree->current_node_id[sizeof(tree->current_node_id) - 1] = '\0';

    LOG_INFO("Dialogue started: %s with %s", tree->id, tree->npc_id);
}

void dialogue_tree_end(DialogueTree* tree) {
    if (!tree) return;

    tree->is_active = false;
    tree->current_node_id[0] = '\0';

    LOG_INFO("Dialogue ended: %s", tree->id);
}

DialogueNode* dialogue_tree_get_current_node(const DialogueTree* tree) {
    if (!tree || !tree->is_active) return NULL;

    return dialogue_tree_get_node(tree, tree->current_node_id);
}

bool dialogue_tree_choose(DialogueTree* tree, int choice_index) {
    if (!tree || !tree->is_active) return false;

    /* Get current node */
    DialogueNode* current = dialogue_tree_get_current_node(tree);
    if (!current) {
        LOG_ERROR("Dialogue tree %s: current node not found", tree->id);
        return false;
    }

    /* Validate choice index */
    if (choice_index < 0 || (size_t)choice_index >= current->choice_count) {
        LOG_ERROR("Dialogue tree %s: invalid choice index %d", tree->id, choice_index);
        return false;
    }

    DialogueChoice* choice = &current->choices[choice_index];

    /* Check if choice is available */
    if (!dialogue_choice_is_available(choice)) {
        LOG_WARN("Dialogue tree %s: choice %d not available", tree->id, choice_index);
        return false;
    }

    /* Mark choice as chosen */
    dialogue_choice_mark_chosen(current, choice_index);

    /* Move to next node */
    const char* next_node_id = choice->next_node;

    /* Check for END */
    if (strcmp(next_node_id, "END") == 0) {
        dialogue_tree_end(tree);
        return true;
    }

    /* Find next node */
    DialogueNode* next_node = dialogue_tree_get_node(tree, next_node_id);
    if (!next_node) {
        LOG_ERROR("Dialogue tree %s: next node %s not found", tree->id, next_node_id);
        dialogue_tree_end(tree);
        return false;
    }

    /* Update current node */
    strncpy(tree->current_node_id, next_node_id, sizeof(tree->current_node_id) - 1);
    tree->current_node_id[sizeof(tree->current_node_id) - 1] = '\0';

    /* Check if next node is end node */
    if (next_node->is_end_node) {
        dialogue_tree_end(tree);
    }

    LOG_DEBUG("Dialogue tree %s: moved to node %s", tree->id, next_node_id);
    return true;
}

void dialogue_tree_reset(DialogueTree* tree) {
    if (!tree) return;

    tree->is_active = false;
    tree->current_node_id[0] = '\0';

    /* Reset all choice states */
    for (size_t i = 0; i < tree->node_count; i++) {
        DialogueNode* node = tree->nodes[i];
        for (size_t j = 0; j < node->choice_count; j++) {
            node->choices[j].chosen = false;
        }
    }

    LOG_DEBUG("Dialogue tree %s reset", tree->id);
}

bool dialogue_tree_is_active(const DialogueTree* tree) {
    if (!tree) return false;
    return tree->is_active;
}

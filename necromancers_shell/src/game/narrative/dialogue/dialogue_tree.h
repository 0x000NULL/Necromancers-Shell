/**
 * @file dialogue_tree.h
 * @brief Dialogue tree structure and operations
 */

#ifndef NECROMANCERS_DIALOGUE_TREE_H
#define NECROMANCERS_DIALOGUE_TREE_H

#include "dialogue.h"
#include <stdbool.h>
#include <stddef.h>

/* Maximum nodes per dialogue tree */
#define MAX_DIALOGUE_NODES 32

/**
 * @struct DialogueTree
 * @brief A complete dialogue tree for an NPC
 */
typedef struct {
    char id[64];
    char npc_id[64];
    char title[128];

    /* Nodes */
    DialogueNode* nodes[MAX_DIALOGUE_NODES];
    size_t node_count;

    /* Entry point */
    char root_node_id[64];

    /* Current state (for active dialogue) */
    char current_node_id[64];
    bool is_active;
} DialogueTree;

/**
 * @brief Create a dialogue tree
 * @param id Tree identifier
 * @param npc_id NPC this dialogue is with
 * @param root_node_id ID of the starting node
 * @return Newly allocated DialogueTree or NULL on failure
 */
DialogueTree* dialogue_tree_create(const char* id, const char* npc_id, const char* root_node_id);

/**
 * @brief Destroy a dialogue tree and all nodes
 * @param tree Tree to destroy (can be NULL)
 */
void dialogue_tree_destroy(DialogueTree* tree);

/**
 * @brief Add a node to the tree
 * @param tree Tree to update
 * @param node Node to add (tree takes ownership)
 */
void dialogue_tree_add_node(DialogueTree* tree, DialogueNode* node);

/**
 * @brief Get a node by ID
 * @param tree Tree to search
 * @param node_id Node identifier
 * @return Pointer to node or NULL if not found
 */
DialogueNode* dialogue_tree_get_node(const DialogueTree* tree, const char* node_id);

/**
 * @brief Start the dialogue tree
 * @param tree Tree to start
 */
void dialogue_tree_start(DialogueTree* tree);

/**
 * @brief End the dialogue tree
 * @param tree Tree to end
 */
void dialogue_tree_end(DialogueTree* tree);

/**
 * @brief Get the current node
 * @param tree Tree to query
 * @return Pointer to current node or NULL
 */
DialogueNode* dialogue_tree_get_current_node(const DialogueTree* tree);

/**
 * @brief Choose a dialogue option and advance to next node
 * @param tree Tree to update
 * @param choice_index Index of chosen option
 * @return true if advanced successfully, false otherwise
 */
bool dialogue_tree_choose(DialogueTree* tree, int choice_index);

/**
 * @brief Reset dialogue tree to beginning
 * @param tree Tree to reset
 */
void dialogue_tree_reset(DialogueTree* tree);

/**
 * @brief Check if dialogue is active
 * @param tree Tree to check
 * @return true if active, false otherwise
 */
bool dialogue_tree_is_active(const DialogueTree* tree);

#endif /* NECROMANCERS_DIALOGUE_TREE_H */

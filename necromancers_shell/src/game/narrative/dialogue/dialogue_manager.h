/**
 * @file dialogue_manager.h
 * @brief Dialogue tree collection manager
 */

#ifndef NECROMANCERS_DIALOGUE_MANAGER_H
#define NECROMANCERS_DIALOGUE_MANAGER_H

#include "dialogue_tree.h"
#include <stdbool.h>
#include <stddef.h>

/**
 * @struct DialogueManager
 * @brief Manages collection of dialogue trees
 */
typedef struct {
    DialogueTree** trees;
    size_t tree_count;
    size_t tree_capacity;

    /* Active dialogue */
    DialogueTree* active_tree;
} DialogueManager;

/**
 * @brief Create a dialogue manager
 * @return Newly allocated DialogueManager or NULL on failure
 */
DialogueManager* dialogue_manager_create(void);

/**
 * @brief Destroy dialogue manager and all trees
 * @param manager DialogueManager to destroy (can be NULL)
 */
void dialogue_manager_destroy(DialogueManager* manager);

/**
 * @brief Add a dialogue tree to the manager
 * @param manager DialogueManager to update
 * @param tree Tree to add (manager takes ownership)
 */
void dialogue_manager_add_tree(DialogueManager* manager, DialogueTree* tree);

/**
 * @brief Get a dialogue tree by ID
 * @param manager DialogueManager to search
 * @param tree_id Tree identifier
 * @return Pointer to tree or NULL if not found
 */
DialogueTree* dialogue_manager_get_tree(const DialogueManager* manager, const char* tree_id);

/**
 * @brief Get dialogue trees for an NPC
 * @param manager DialogueManager to search
 * @param npc_id NPC identifier
 * @param count_out Output parameter for number of trees
 * @return Array of tree pointers (caller must free) or NULL if none
 */
DialogueTree** dialogue_manager_get_by_npc(const DialogueManager* manager,
                                            const char* npc_id,
                                            size_t* count_out);

/**
 * @brief Start a dialogue
 * @param manager DialogueManager to update
 * @param tree_id Tree identifier
 * @return true if started successfully, false otherwise
 */
bool dialogue_manager_start_dialogue(DialogueManager* manager, const char* tree_id);

/**
 * @brief End current dialogue
 * @param manager DialogueManager to update
 */
void dialogue_manager_end_dialogue(DialogueManager* manager);

/**
 * @brief Get currently active dialogue tree
 * @param manager DialogueManager to query
 * @return Pointer to active tree or NULL if none
 */
DialogueTree* dialogue_manager_get_active(const DialogueManager* manager);

/**
 * @brief Get current dialogue node
 * @param manager DialogueManager to query
 * @return Pointer to current node or NULL if no active dialogue
 */
DialogueNode* dialogue_manager_get_current_node(const DialogueManager* manager);

/**
 * @brief Choose a dialogue option
 * @param manager DialogueManager to update
 * @param choice_index Index of chosen option
 * @return true if choice successful, false otherwise
 */
bool dialogue_manager_choose(DialogueManager* manager, int choice_index);

/**
 * @brief Check if a dialogue is active
 * @param manager DialogueManager to check
 * @return true if dialogue active, false otherwise
 */
bool dialogue_manager_is_active(const DialogueManager* manager);

/**
 * @brief Load dialogue trees from a data file
 * @param manager DialogueManager to update
 * @param filepath Path to data file
 * @return true on success, false on failure
 */
bool dialogue_manager_load_from_file(DialogueManager* manager, const char* filepath);

#endif /* NECROMANCERS_DIALOGUE_MANAGER_H */

/**
 * @file quest_manager.h
 * @brief Quest collection manager
 */

#ifndef NECROMANCERS_QUEST_MANAGER_H
#define NECROMANCERS_QUEST_MANAGER_H

#include "quest.h"
#include <stdbool.h>
#include <stddef.h>

/**
 * @struct QuestManager
 * @brief Manages collection of quests
 */
typedef struct {
    Quest** quests;
    size_t quest_count;
    size_t quest_capacity;
} QuestManager;

/**
 * @brief Create a quest manager
 * @return Newly allocated QuestManager or NULL on failure
 */
QuestManager* quest_manager_create(void);

/**
 * @brief Destroy quest manager and all quests
 * @param manager QuestManager to destroy (can be NULL)
 */
void quest_manager_destroy(QuestManager* manager);

/**
 * @brief Add a quest to the manager
 * @param manager QuestManager to update
 * @param quest Quest to add (manager takes ownership)
 */
void quest_manager_add_quest(QuestManager* manager, Quest* quest);

/**
 * @brief Get a quest by ID
 * @param manager QuestManager to search
 * @param quest_id Quest identifier
 * @return Pointer to Quest or NULL if not found
 */
Quest* quest_manager_get_quest(const QuestManager* manager, const char* quest_id);

/**
 * @brief Get all active quests
 * @param manager QuestManager to search
 * @param count_out Output parameter for number of quests
 * @return Array of Quest pointers (caller must free) or NULL if none
 */
Quest** quest_manager_get_active_quests(const QuestManager* manager, size_t* count_out);

/**
 * @brief Get all completed quests
 * @param manager QuestManager to search
 * @param count_out Output parameter for number of quests
 * @return Array of Quest pointers (caller must free) or NULL if none
 */
Quest** quest_manager_get_completed_quests(const QuestManager* manager, size_t* count_out);

/**
 * @brief Get all failed quests
 * @param manager QuestManager to search
 * @param count_out Output parameter for number of quests
 * @return Array of Quest pointers (caller must free) or NULL if none
 */
Quest** quest_manager_get_failed_quests(const QuestManager* manager, size_t* count_out);

/**
 * @brief Get quests by quest giver
 * @param manager QuestManager to search
 * @param npc_id NPC identifier
 * @param count_out Output parameter for number of quests
 * @return Array of Quest pointers (caller must free) or NULL if none
 */
Quest** quest_manager_get_by_quest_giver(const QuestManager* manager, const char* npc_id, size_t* count_out);

/**
 * @brief Start a quest
 * @param manager QuestManager to update
 * @param quest_id Quest identifier
 * @return true if quest started, false otherwise
 */
bool quest_manager_start_quest(QuestManager* manager, const char* quest_id);

/**
 * @brief Complete a quest
 * @param manager QuestManager to update
 * @param quest_id Quest identifier
 * @return true if quest completed, false otherwise
 */
bool quest_manager_complete_quest(QuestManager* manager, const char* quest_id);

/**
 * @brief Fail a quest
 * @param manager QuestManager to update
 * @param quest_id Quest identifier
 * @return true if quest failed, false otherwise
 */
bool quest_manager_fail_quest(QuestManager* manager, const char* quest_id);

/**
 * @brief Update an objective in a quest
 * @param manager QuestManager to update
 * @param quest_id Quest identifier
 * @param objective_id Objective identifier
 * @param amount Amount to increment
 * @return true if objective or quest completed, false otherwise
 */
bool quest_manager_update_objective(QuestManager* manager,
                                    const char* quest_id,
                                    const char* objective_id,
                                    int amount);

/**
 * @brief Update all active quests (check deadlines, auto-complete, etc.)
 * @param manager QuestManager to update
 */
void quest_manager_update_all(QuestManager* manager);

/**
 * @brief Load quests from a data file
 * @param manager QuestManager to update
 * @param filepath Path to data file
 * @return true on success, false on failure
 */
bool quest_manager_load_from_file(QuestManager* manager, const char* filepath);

#endif /* NECROMANCERS_QUEST_MANAGER_H */

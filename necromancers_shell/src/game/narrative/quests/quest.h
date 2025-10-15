/**
 * @file quest.h
 * @brief Quest structure and operations
 */

#ifndef NECROMANCERS_QUEST_H
#define NECROMANCERS_QUEST_H

#include "quest_objective.h"
#include <stdbool.h>
#include <stddef.h>
#include <time.h>

/* Maximum objectives per quest */
#define MAX_QUEST_OBJECTIVES 8

/* Quest states */
typedef enum {
    QUEST_STATE_NOT_STARTED,
    QUEST_STATE_ACTIVE,
    QUEST_STATE_COMPLETED,
    QUEST_STATE_FAILED
} QuestState;

/**
 * @struct Quest
 * @brief Represents a quest with objectives and rewards
 */
typedef struct {
    char id[64];
    char title[128];
    char description[512];

    /* Quest giver */
    char quest_giver[64];  /* NPC ID */

    /* State */
    QuestState state;
    time_t started_time;
    time_t completed_time;

    /* Objectives */
    QuestObjective* objectives[MAX_QUEST_OBJECTIVES];
    size_t objective_count;

    /* Rewards */
    int soul_energy_reward;
    int mana_reward;
    int trust_reward;      /* Trust with quest giver */
    int respect_reward;    /* Respect with quest giver */

    /* Unlocks */
    char unlocks_memory[64];    /* Memory fragment ID */
    char unlocks_quest[64];     /* Next quest ID */
    char unlocks_location[64];  /* Location ID */

    /* Failure conditions */
    bool can_fail;
    bool time_limited;
    time_t deadline;
} Quest;

/**
 * @brief Create a quest
 * @param id Quest identifier
 * @param title Quest title
 * @param quest_giver NPC who gives the quest
 * @return Newly allocated Quest or NULL on failure
 */
Quest* quest_create(const char* id, const char* title, const char* quest_giver);

/**
 * @brief Destroy a quest and all objectives
 * @param quest Quest to destroy (can be NULL)
 */
void quest_destroy(Quest* quest);

/**
 * @brief Add an objective to the quest
 * @param quest Quest to update
 * @param objective Objective to add (quest takes ownership)
 */
void quest_add_objective(Quest* quest, QuestObjective* objective);

/**
 * @brief Get an objective by ID
 * @param quest Quest to search
 * @param objective_id Objective identifier
 * @return Pointer to objective or NULL if not found
 */
QuestObjective* quest_get_objective(const Quest* quest, const char* objective_id);

/**
 * @brief Start the quest
 * @param quest Quest to start
 */
void quest_start(Quest* quest);

/**
 * @brief Complete the quest
 * @param quest Quest to complete
 */
void quest_complete(Quest* quest);

/**
 * @brief Fail the quest
 * @param quest Quest to fail
 */
void quest_fail(Quest* quest);

/**
 * @brief Check if all objectives are completed
 * @param quest Quest to check
 * @return true if all required objectives completed, false otherwise
 */
bool quest_all_objectives_completed(const Quest* quest);

/**
 * @brief Update quest state based on objective completion
 * @param quest Quest to update
 * @return true if quest was completed, false otherwise
 */
bool quest_update_state(Quest* quest);

/**
 * @brief Get quest progress as percentage
 * @param quest Quest to check
 * @return Progress percentage (0-100)
 */
int quest_get_progress_percent(const Quest* quest);

/**
 * @brief Check if quest is active
 * @param quest Quest to check
 * @return true if active, false otherwise
 */
bool quest_is_active(const Quest* quest);

/**
 * @brief Check if quest is completed
 * @param quest Quest to check
 * @return true if completed, false otherwise
 */
bool quest_is_completed(const Quest* quest);

/**
 * @brief Check if quest has failed
 * @param quest Quest to check
 * @return true if failed, false otherwise
 */
bool quest_is_failed(const Quest* quest);

/**
 * @brief Check if quest deadline has passed
 * @param quest Quest to check
 * @return true if deadline passed, false otherwise
 */
bool quest_is_past_deadline(const Quest* quest);

/**
 * @brief Get quest state as string
 * @param state Quest state
 * @return Human-readable state name
 */
const char* quest_state_to_string(QuestState state);

#endif /* NECROMANCERS_QUEST_H */

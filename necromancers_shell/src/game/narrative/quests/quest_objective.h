/**
 * @file quest_objective.h
 * @brief Quest objective structure and operations
 */

#ifndef NECROMANCERS_QUEST_OBJECTIVE_H
#define NECROMANCERS_QUEST_OBJECTIVE_H

#include <stdbool.h>
#include <stddef.h>

/* Objective types */
typedef enum {
    OBJECTIVE_TYPE_TALK,          /* Talk to an NPC */
    OBJECTIVE_TYPE_KILL,          /* Kill enemies */
    OBJECTIVE_TYPE_COLLECT,       /* Collect items/souls */
    OBJECTIVE_TYPE_VISIT,         /* Visit a location */
    OBJECTIVE_TYPE_RAISE,         /* Raise minions */
    OBJECTIVE_TYPE_DISCOVER,      /* Discover memory fragments */
    OBJECTIVE_TYPE_RESEARCH,      /* Complete research */
    OBJECTIVE_TYPE_RELATIONSHIP,  /* Reach relationship status */
    OBJECTIVE_TYPE_CUSTOM         /* Custom condition */
} ObjectiveType;

/**
 * @struct QuestObjective
 * @brief Represents a single objective within a quest
 */
typedef struct {
    char id[64];
    char description[256];
    ObjectiveType type;

    /* Target information */
    char target_id[64];  /* NPC ID, enemy type, item type, location ID, etc. */
    int target_count;    /* Required count (e.g., kill 5 enemies) */
    int current_count;   /* Current progress */

    /* State */
    bool completed;
    bool optional;
    bool hidden;  /* Not shown to player until revealed */

    /* Prerequisite */
    char prerequisite_objective[64];  /* Must complete this objective first */
} QuestObjective;

/**
 * @brief Create a quest objective
 * @param id Objective identifier
 * @param description Human-readable description
 * @param type Objective type
 * @return Newly allocated QuestObjective or NULL on failure
 */
QuestObjective* quest_objective_create(const char* id, const char* description, ObjectiveType type);

/**
 * @brief Destroy a quest objective
 * @param objective Objective to destroy (can be NULL)
 */
void quest_objective_destroy(QuestObjective* objective);

/**
 * @brief Set objective target
 * @param objective Objective to update
 * @param target_id Target identifier
 * @param target_count Required count
 */
void quest_objective_set_target(QuestObjective* objective, const char* target_id, int target_count);

/**
 * @brief Increment objective progress
 * @param objective Objective to update
 * @param amount Amount to increment (default 1)
 * @return true if objective is now completed, false otherwise
 */
bool quest_objective_increment(QuestObjective* objective, int amount);

/**
 * @brief Set objective progress directly
 * @param objective Objective to update
 * @param count New progress count
 * @return true if objective is now completed, false otherwise
 */
bool quest_objective_set_progress(QuestObjective* objective, int count);

/**
 * @brief Complete an objective
 * @param objective Objective to complete
 */
void quest_objective_complete(QuestObjective* objective);

/**
 * @brief Check if objective is completed
 * @param objective Objective to check
 * @return true if completed, false otherwise
 */
bool quest_objective_is_completed(const QuestObjective* objective);

/**
 * @brief Get objective progress as percentage
 * @param objective Objective to check
 * @return Progress percentage (0-100)
 */
int quest_objective_get_progress_percent(const QuestObjective* objective);

/**
 * @brief Get objective type as string
 * @param type Objective type
 * @return Human-readable type name
 */
const char* quest_objective_type_to_string(ObjectiveType type);

#endif /* NECROMANCERS_QUEST_OBJECTIVE_H */

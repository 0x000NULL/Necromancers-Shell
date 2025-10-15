/**
 * @file quest_objective.c
 * @brief Quest objective implementation
 */

#include "quest_objective.h"
#include "../../../utils/logger.h"
#include <stdlib.h>
#include <string.h>

QuestObjective* quest_objective_create(const char* id, const char* description, ObjectiveType type) {
    if (!id || !description) {
        LOG_ERROR("quest_objective_create: NULL parameters");
        return NULL;
    }

    QuestObjective* objective = malloc(sizeof(QuestObjective));
    if (!objective) {
        LOG_ERROR("quest_objective_create: malloc failed");
        return NULL;
    }

    /* Initialize structure */
    memset(objective, 0, sizeof(QuestObjective));

    /* Copy data */
    strncpy(objective->id, id, sizeof(objective->id) - 1);
    strncpy(objective->description, description, sizeof(objective->description) - 1);
    objective->type = type;

    /* Initialize defaults */
    objective->target_count = 1;
    objective->current_count = 0;
    objective->completed = false;
    objective->optional = false;
    objective->hidden = false;

    return objective;
}

void quest_objective_destroy(QuestObjective* objective) {
    if (!objective) return;
    free(objective);
}

void quest_objective_set_target(QuestObjective* objective, const char* target_id, int target_count) {
    if (!objective) return;

    if (target_id) {
        strncpy(objective->target_id, target_id, sizeof(objective->target_id) - 1);
    }

    if (target_count > 0) {
        objective->target_count = target_count;
    }

    LOG_DEBUG("Objective %s: target set to %s (count: %d)", objective->id, target_id, target_count);
}

bool quest_objective_increment(QuestObjective* objective, int amount) {
    if (!objective || objective->completed) return false;

    objective->current_count += amount;

    /* Check for completion */
    if (objective->current_count >= objective->target_count) {
        objective->current_count = objective->target_count;
        objective->completed = true;
        LOG_INFO("Objective completed: %s", objective->description);
        return true;
    }

    LOG_DEBUG("Objective %s: progress %d/%d", objective->id, objective->current_count, objective->target_count);
    return false;
}

bool quest_objective_set_progress(QuestObjective* objective, int count) {
    if (!objective || objective->completed) return false;

    objective->current_count = count;

    /* Clamp to target */
    if (objective->current_count > objective->target_count) {
        objective->current_count = objective->target_count;
    }

    /* Check for completion */
    if (objective->current_count >= objective->target_count) {
        objective->completed = true;
        LOG_INFO("Objective completed: %s", objective->description);
        return true;
    }

    return false;
}

void quest_objective_complete(QuestObjective* objective) {
    if (!objective) return;

    objective->completed = true;
    objective->current_count = objective->target_count;

    LOG_INFO("Objective completed: %s", objective->description);
}

bool quest_objective_is_completed(const QuestObjective* objective) {
    if (!objective) return false;
    return objective->completed;
}

int quest_objective_get_progress_percent(const QuestObjective* objective) {
    if (!objective || objective->target_count == 0) return 0;

    return (objective->current_count * 100) / objective->target_count;
}

const char* quest_objective_type_to_string(ObjectiveType type) {
    switch (type) {
        case OBJECTIVE_TYPE_TALK:
            return "Talk";
        case OBJECTIVE_TYPE_KILL:
            return "Kill";
        case OBJECTIVE_TYPE_COLLECT:
            return "Collect";
        case OBJECTIVE_TYPE_VISIT:
            return "Visit";
        case OBJECTIVE_TYPE_RAISE:
            return "Raise";
        case OBJECTIVE_TYPE_DISCOVER:
            return "Discover";
        case OBJECTIVE_TYPE_RESEARCH:
            return "Research";
        case OBJECTIVE_TYPE_RELATIONSHIP:
            return "Relationship";
        case OBJECTIVE_TYPE_CUSTOM:
            return "Custom";
        default:
            return "Unknown";
    }
}

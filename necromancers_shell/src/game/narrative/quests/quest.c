/**
 * @file quest.c
 * @brief Quest implementation
 */

#include "quest.h"
#include "../../../utils/logger.h"
#include <stdlib.h>
#include <string.h>

Quest* quest_create(const char* id, const char* title, const char* quest_giver) {
    if (!id || !title) {
        LOG_ERROR("quest_create: NULL parameters");
        return NULL;
    }

    Quest* quest = malloc(sizeof(Quest));
    if (!quest) {
        LOG_ERROR("quest_create: malloc failed");
        return NULL;
    }

    /* Initialize structure */
    memset(quest, 0, sizeof(Quest));

    /* Copy data */
    strncpy(quest->id, id, sizeof(quest->id) - 1);
    strncpy(quest->title, title, sizeof(quest->title) - 1);
    if (quest_giver) {
        strncpy(quest->quest_giver, quest_giver, sizeof(quest->quest_giver) - 1);
    }

    /* Initialize state */
    quest->state = QUEST_STATE_NOT_STARTED;
    quest->started_time = 0;
    quest->completed_time = 0;
    quest->objective_count = 0;

    /* Initialize rewards */
    quest->soul_energy_reward = 0;
    quest->mana_reward = 0;
    quest->trust_reward = 0;
    quest->respect_reward = 0;

    /* Initialize flags */
    quest->can_fail = false;
    quest->time_limited = false;
    quest->deadline = 0;

    return quest;
}

void quest_destroy(Quest* quest) {
    if (!quest) return;

    /* Destroy all objectives */
    for (size_t i = 0; i < quest->objective_count; i++) {
        quest_objective_destroy(quest->objectives[i]);
    }

    free(quest);
}

void quest_add_objective(Quest* quest, QuestObjective* objective) {
    if (!quest || !objective) return;

    if (quest->objective_count >= MAX_QUEST_OBJECTIVES) {
        LOG_WARN("Quest %s: max objectives reached", quest->id);
        return;
    }

    quest->objectives[quest->objective_count++] = objective;
    LOG_DEBUG("Quest %s: added objective %s", quest->id, objective->id);
}

QuestObjective* quest_get_objective(const Quest* quest, const char* objective_id) {
    if (!quest || !objective_id) return NULL;

    for (size_t i = 0; i < quest->objective_count; i++) {
        if (strcmp(quest->objectives[i]->id, objective_id) == 0) {
            return quest->objectives[i];
        }
    }

    return NULL;
}

void quest_start(Quest* quest) {
    if (!quest) return;

    if (quest->state != QUEST_STATE_NOT_STARTED) {
        LOG_WARN("Quest %s: already started", quest->id);
        return;
    }

    quest->state = QUEST_STATE_ACTIVE;
    quest->started_time = time(NULL);

    LOG_INFO("Quest started: %s", quest->title);
}

void quest_complete(Quest* quest) {
    if (!quest) return;

    if (quest->state != QUEST_STATE_ACTIVE) {
        LOG_WARN("Quest %s: cannot complete (not active)", quest->id);
        return;
    }

    quest->state = QUEST_STATE_COMPLETED;
    quest->completed_time = time(NULL);

    LOG_INFO("Quest completed: %s", quest->title);
}

void quest_fail(Quest* quest) {
    if (!quest) return;

    if (!quest->can_fail) {
        LOG_WARN("Quest %s: cannot fail (fail not allowed)", quest->id);
        return;
    }

    if (quest->state != QUEST_STATE_ACTIVE) {
        LOG_WARN("Quest %s: cannot fail (not active)", quest->id);
        return;
    }

    quest->state = QUEST_STATE_FAILED;
    quest->completed_time = time(NULL);

    LOG_INFO("Quest failed: %s", quest->title);
}

bool quest_all_objectives_completed(const Quest* quest) {
    if (!quest) return false;

    /* Check all required objectives */
    for (size_t i = 0; i < quest->objective_count; i++) {
        QuestObjective* obj = quest->objectives[i];

        /* Skip optional objectives */
        if (obj->optional) continue;

        /* If any required objective not completed, return false */
        if (!obj->completed) {
            return false;
        }
    }

    return true;
}

bool quest_update_state(Quest* quest) {
    if (!quest || quest->state != QUEST_STATE_ACTIVE) {
        return false;
    }

    /* Check for failure (deadline) */
    if (quest->can_fail && quest->time_limited && quest_is_past_deadline(quest)) {
        quest_fail(quest);
        return false;
    }

    /* Check for completion */
    if (quest_all_objectives_completed(quest)) {
        quest_complete(quest);
        return true;
    }

    return false;
}

int quest_get_progress_percent(const Quest* quest) {
    if (!quest || quest->objective_count == 0) return 0;

    int total_progress = 0;
    int required_objectives = 0;

    for (size_t i = 0; i < quest->objective_count; i++) {
        QuestObjective* obj = quest->objectives[i];

        /* Skip optional objectives */
        if (obj->optional) continue;

        required_objectives++;
        total_progress += quest_objective_get_progress_percent(obj);
    }

    if (required_objectives == 0) return 0;

    return total_progress / required_objectives;
}

bool quest_is_active(const Quest* quest) {
    if (!quest) return false;
    return quest->state == QUEST_STATE_ACTIVE;
}

bool quest_is_completed(const Quest* quest) {
    if (!quest) return false;
    return quest->state == QUEST_STATE_COMPLETED;
}

bool quest_is_failed(const Quest* quest) {
    if (!quest) return false;
    return quest->state == QUEST_STATE_FAILED;
}

bool quest_is_past_deadline(const Quest* quest) {
    if (!quest || !quest->time_limited) return false;

    time_t now = time(NULL);
    return now > quest->deadline;
}

const char* quest_state_to_string(QuestState state) {
    switch (state) {
        case QUEST_STATE_NOT_STARTED:
            return "Not Started";
        case QUEST_STATE_ACTIVE:
            return "Active";
        case QUEST_STATE_COMPLETED:
            return "Completed";
        case QUEST_STATE_FAILED:
            return "Failed";
        default:
            return "Unknown";
    }
}

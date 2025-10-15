/**
 * @file quest_manager.c
 * @brief Quest manager implementation
 */

#include "quest_manager.h"
#include "../../../data/data_loader.h"
#include "../../../utils/logger.h"
#include <stdlib.h>
#include <string.h>

QuestManager* quest_manager_create(void) {
    QuestManager* manager = malloc(sizeof(QuestManager));
    if (!manager) {
        LOG_ERROR("quest_manager_create: malloc failed");
        return NULL;
    }

    manager->quest_capacity = 16;
    manager->quest_count = 0;
    manager->quests = calloc(manager->quest_capacity, sizeof(Quest*));

    if (!manager->quests) {
        LOG_ERROR("quest_manager_create: calloc failed");
        free(manager);
        return NULL;
    }

    LOG_DEBUG("Quest manager created");
    return manager;
}

void quest_manager_destroy(QuestManager* manager) {
    if (!manager) return;

    /* Destroy all quests */
    for (size_t i = 0; i < manager->quest_count; i++) {
        quest_destroy(manager->quests[i]);
    }

    free(manager->quests);
    free(manager);

    LOG_DEBUG("Quest manager destroyed");
}

void quest_manager_add_quest(QuestManager* manager, Quest* quest) {
    if (!manager || !quest) return;

    /* Resize if needed */
    if (manager->quest_count >= manager->quest_capacity) {
        size_t new_capacity = manager->quest_capacity * 2;
        Quest** new_quests = realloc(manager->quests, new_capacity * sizeof(Quest*));
        if (!new_quests) {
            LOG_ERROR("quest_manager_add_quest: realloc failed");
            return;
        }

        manager->quests = new_quests;
        manager->quest_capacity = new_capacity;

        /* Zero new memory */
        for (size_t i = manager->quest_count; i < new_capacity; i++) {
            manager->quests[i] = NULL;
        }
    }

    manager->quests[manager->quest_count++] = quest;
    LOG_DEBUG("Added quest: %s", quest->id);
}

Quest* quest_manager_get_quest(const QuestManager* manager, const char* quest_id) {
    if (!manager || !quest_id) return NULL;

    for (size_t i = 0; i < manager->quest_count; i++) {
        if (strcmp(manager->quests[i]->id, quest_id) == 0) {
            return manager->quests[i];
        }
    }

    return NULL;
}

Quest** quest_manager_get_active_quests(const QuestManager* manager, size_t* count_out) {
    if (!manager || !count_out) {
        if (count_out) *count_out = 0;
        return NULL;
    }

    /* Count active quests */
    size_t count = 0;
    for (size_t i = 0; i < manager->quest_count; i++) {
        if (quest_is_active(manager->quests[i])) {
            count++;
        }
    }

    if (count == 0) {
        *count_out = 0;
        return NULL;
    }

    /* Allocate result array */
    Quest** result = malloc(count * sizeof(Quest*));
    if (!result) {
        LOG_ERROR("quest_manager_get_active_quests: malloc failed");
        *count_out = 0;
        return NULL;
    }

    /* Fill array */
    size_t idx = 0;
    for (size_t i = 0; i < manager->quest_count; i++) {
        if (quest_is_active(manager->quests[i])) {
            result[idx++] = manager->quests[i];
        }
    }

    *count_out = count;
    return result;
}

Quest** quest_manager_get_completed_quests(const QuestManager* manager, size_t* count_out) {
    if (!manager || !count_out) {
        if (count_out) *count_out = 0;
        return NULL;
    }

    /* Count completed quests */
    size_t count = 0;
    for (size_t i = 0; i < manager->quest_count; i++) {
        if (quest_is_completed(manager->quests[i])) {
            count++;
        }
    }

    if (count == 0) {
        *count_out = 0;
        return NULL;
    }

    /* Allocate result array */
    Quest** result = malloc(count * sizeof(Quest*));
    if (!result) {
        LOG_ERROR("quest_manager_get_completed_quests: malloc failed");
        *count_out = 0;
        return NULL;
    }

    /* Fill array */
    size_t idx = 0;
    for (size_t i = 0; i < manager->quest_count; i++) {
        if (quest_is_completed(manager->quests[i])) {
            result[idx++] = manager->quests[i];
        }
    }

    *count_out = count;
    return result;
}

Quest** quest_manager_get_failed_quests(const QuestManager* manager, size_t* count_out) {
    if (!manager || !count_out) {
        if (count_out) *count_out = 0;
        return NULL;
    }

    /* Count failed quests */
    size_t count = 0;
    for (size_t i = 0; i < manager->quest_count; i++) {
        if (quest_is_failed(manager->quests[i])) {
            count++;
        }
    }

    if (count == 0) {
        *count_out = 0;
        return NULL;
    }

    /* Allocate result array */
    Quest** result = malloc(count * sizeof(Quest*));
    if (!result) {
        LOG_ERROR("quest_manager_get_failed_quests: malloc failed");
        *count_out = 0;
        return NULL;
    }

    /* Fill array */
    size_t idx = 0;
    for (size_t i = 0; i < manager->quest_count; i++) {
        if (quest_is_failed(manager->quests[i])) {
            result[idx++] = manager->quests[i];
        }
    }

    *count_out = count;
    return result;
}

Quest** quest_manager_get_by_quest_giver(const QuestManager* manager, const char* npc_id, size_t* count_out) {
    if (!manager || !npc_id || !count_out) {
        if (count_out) *count_out = 0;
        return NULL;
    }

    /* Count matching quests */
    size_t count = 0;
    for (size_t i = 0; i < manager->quest_count; i++) {
        if (strcmp(manager->quests[i]->quest_giver, npc_id) == 0) {
            count++;
        }
    }

    if (count == 0) {
        *count_out = 0;
        return NULL;
    }

    /* Allocate result array */
    Quest** result = malloc(count * sizeof(Quest*));
    if (!result) {
        LOG_ERROR("quest_manager_get_by_quest_giver: malloc failed");
        *count_out = 0;
        return NULL;
    }

    /* Fill array */
    size_t idx = 0;
    for (size_t i = 0; i < manager->quest_count; i++) {
        if (strcmp(manager->quests[i]->quest_giver, npc_id) == 0) {
            result[idx++] = manager->quests[i];
        }
    }

    *count_out = count;
    return result;
}

bool quest_manager_start_quest(QuestManager* manager, const char* quest_id) {
    if (!manager || !quest_id) return false;

    Quest* quest = quest_manager_get_quest(manager, quest_id);
    if (!quest) {
        LOG_WARN("Cannot start unknown quest: %s", quest_id);
        return false;
    }

    quest_start(quest);
    return true;
}

bool quest_manager_complete_quest(QuestManager* manager, const char* quest_id) {
    if (!manager || !quest_id) return false;

    Quest* quest = quest_manager_get_quest(manager, quest_id);
    if (!quest) {
        LOG_WARN("Cannot complete unknown quest: %s", quest_id);
        return false;
    }

    quest_complete(quest);
    return true;
}

bool quest_manager_fail_quest(QuestManager* manager, const char* quest_id) {
    if (!manager || !quest_id) return false;

    Quest* quest = quest_manager_get_quest(manager, quest_id);
    if (!quest) {
        LOG_WARN("Cannot fail unknown quest: %s", quest_id);
        return false;
    }

    quest_fail(quest);
    return true;
}

bool quest_manager_update_objective(QuestManager* manager,
                                    const char* quest_id,
                                    const char* objective_id,
                                    int amount) {
    if (!manager || !quest_id || !objective_id) return false;

    Quest* quest = quest_manager_get_quest(manager, quest_id);
    if (!quest) {
        LOG_WARN("Cannot update objective for unknown quest: %s", quest_id);
        return false;
    }

    QuestObjective* objective = quest_get_objective(quest, objective_id);
    if (!objective) {
        LOG_WARN("Cannot update unknown objective: %s in quest %s", objective_id, quest_id);
        return false;
    }

    /* Update objective */
    bool obj_completed = quest_objective_increment(objective, amount);

    /* Update quest state */
    bool quest_completed = quest_update_state(quest);

    return obj_completed || quest_completed;
}

void quest_manager_update_all(QuestManager* manager) {
    if (!manager) return;

    for (size_t i = 0; i < manager->quest_count; i++) {
        Quest* quest = manager->quests[i];

        if (quest_is_active(quest)) {
            quest_update_state(quest);
        }
    }
}

bool quest_manager_load_from_file(QuestManager* manager, const char* filepath) {
    if (!manager || !filepath) {
        LOG_ERROR("quest_manager_load_from_file: NULL parameters");
        return false;
    }

    DataFile* file = data_file_load(filepath);
    if (!file) {
        LOG_ERROR("Failed to load quests: %s", filepath);
        return false;
    }

    /* Get all QUEST sections */
    size_t section_count;
    const DataSection** sections = data_file_get_sections(file, "QUEST", &section_count);

    if (!sections) {
        LOG_WARN("No QUEST sections found in %s", filepath);
        data_file_destroy(file);
        return true; /* Not an error, just empty */
    }

    /* Parse each quest */
    for (size_t i = 0; i < section_count; i++) {
        const DataSection* section = sections[i];

        /* Extract quest data */
        const char* id = section->section_id;
        const char* title = data_value_get_string(data_section_get(section, "title"), "Untitled Quest");
        const char* description = data_value_get_string(data_section_get(section, "description"), "");
        const char* quest_giver = data_value_get_string(data_section_get(section, "quest_giver"), "");

        /* Create quest */
        Quest* quest = quest_create(id, title, quest_giver);
        if (!quest) {
            LOG_WARN("Failed to create quest: %s", id);
            continue;
        }

        /* Set description */
        if (description[0]) {
            strncpy(quest->description, description, sizeof(quest->description) - 1);
        }

        /* Set rewards */
        quest->soul_energy_reward = data_value_get_int(data_section_get(section, "soul_energy_reward"), 0);
        quest->mana_reward = data_value_get_int(data_section_get(section, "mana_reward"), 0);
        quest->trust_reward = data_value_get_int(data_section_get(section, "trust_reward"), 0);
        quest->respect_reward = data_value_get_int(data_section_get(section, "respect_reward"), 0);

        /* Set unlocks */
        const char* unlocks_memory = data_value_get_string(data_section_get(section, "unlocks_memory"), "");
        if (unlocks_memory[0]) {
            strncpy(quest->unlocks_memory, unlocks_memory, sizeof(quest->unlocks_memory) - 1);
        }

        const char* unlocks_quest = data_value_get_string(data_section_get(section, "unlocks_quest"), "");
        if (unlocks_quest[0]) {
            strncpy(quest->unlocks_quest, unlocks_quest, sizeof(quest->unlocks_quest) - 1);
        }

        const char* unlocks_location = data_value_get_string(data_section_get(section, "unlocks_location"), "");
        if (unlocks_location[0]) {
            strncpy(quest->unlocks_location, unlocks_location, sizeof(quest->unlocks_location) - 1);
        }

        /* Set failure conditions */
        quest->can_fail = data_value_get_bool(data_section_get(section, "can_fail"), false);
        quest->time_limited = data_value_get_bool(data_section_get(section, "time_limited"), false);

        /* Add to manager */
        quest_manager_add_quest(manager, quest);
    }

    free(sections);
    data_file_destroy(file);

    LOG_INFO("Loaded %zu quests from %s", section_count, filepath);
    return true;
}

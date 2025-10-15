/**
 * @file dialogue.c
 * @brief Dialogue implementation
 */

#include "dialogue.h"
#include "../../../utils/logger.h"
#include <stdlib.h>
#include <string.h>

DialogueNode* dialogue_node_create(const char* id, const char* speaker, const char* text) {
    if (!id || !speaker || !text) {
        LOG_ERROR("dialogue_node_create: NULL parameters");
        return NULL;
    }

    DialogueNode* node = malloc(sizeof(DialogueNode));
    if (!node) {
        LOG_ERROR("dialogue_node_create: malloc failed");
        return NULL;
    }

    /* Initialize structure */
    memset(node, 0, sizeof(DialogueNode));

    /* Copy data */
    strncpy(node->id, id, sizeof(node->id) - 1);
    strncpy(node->speaker, speaker, sizeof(node->speaker) - 1);
    strncpy(node->text, text, sizeof(node->text) - 1);

    /* Initialize defaults */
    node->choice_count = 0;
    node->is_end_node = false;

    return node;
}

void dialogue_node_destroy(DialogueNode* node) {
    if (!node) return;
    free(node);
}

int dialogue_node_add_choice(DialogueNode* node, const char* choice_text, const char* next_node) {
    if (!node || !choice_text || !next_node) return -1;

    if (node->choice_count >= MAX_DIALOGUE_CHOICES) {
        LOG_WARN("Dialogue node %s: max choices reached", node->id);
        return -1;
    }

    DialogueChoice* choice = &node->choices[node->choice_count];

    /* Initialize choice */
    memset(choice, 0, sizeof(DialogueChoice));

    /* Copy data */
    strncpy(choice->text, choice_text, sizeof(choice->text) - 1);
    strncpy(choice->next_node, next_node, sizeof(choice->next_node) - 1);

    /* Initialize defaults */
    choice->condition_count = 0;
    choice->trust_delta = 0;
    choice->respect_delta = 0;
    choice->fear_delta = 0;
    choice->corruption_delta = 0;
    choice->hidden = false;
    choice->once_only = false;
    choice->chosen = false;

    int index = (int)node->choice_count;
    node->choice_count++;

    LOG_DEBUG("Dialogue node %s: added choice -> %s", node->id, next_node);
    return index;
}

void dialogue_choice_add_condition(DialogueNode* node,
                                   int choice_index,
                                   ConditionType type,
                                   const char* parameter,
                                   int value) {
    if (!node || choice_index < 0 || (size_t)choice_index >= node->choice_count) return;

    DialogueChoice* choice = &node->choices[choice_index];

    if (choice->condition_count >= MAX_CHOICE_CONDITIONS) {
        LOG_WARN("Choice %d in node %s: max conditions reached", choice_index, node->id);
        return;
    }

    DialogueCondition* condition = &choice->conditions[choice->condition_count];
    condition->type = type;
    condition->value = value;

    if (parameter) {
        strncpy(condition->parameter, parameter, sizeof(condition->parameter) - 1);
    }

    choice->condition_count++;

    LOG_DEBUG("Choice %d in node %s: added condition %s",
              choice_index, node->id, dialogue_condition_type_to_string(type));
}

void dialogue_choice_set_effects(DialogueNode* node,
                                int choice_index,
                                int trust_delta,
                                int respect_delta,
                                int fear_delta,
                                int corruption_delta) {
    if (!node || choice_index < 0 || (size_t)choice_index >= node->choice_count) return;

    DialogueChoice* choice = &node->choices[choice_index];
    choice->trust_delta = trust_delta;
    choice->respect_delta = respect_delta;
    choice->fear_delta = fear_delta;
    choice->corruption_delta = corruption_delta;
}

void dialogue_choice_set_unlocks(DialogueNode* node,
                                int choice_index,
                                const char* quest_id,
                                const char* memory_id,
                                const char* flag) {
    if (!node || choice_index < 0 || (size_t)choice_index >= node->choice_count) return;

    DialogueChoice* choice = &node->choices[choice_index];

    if (quest_id) {
        strncpy(choice->unlocks_quest, quest_id, sizeof(choice->unlocks_quest) - 1);
    }

    if (memory_id) {
        strncpy(choice->unlocks_memory, memory_id, sizeof(choice->unlocks_memory) - 1);
    }

    if (flag) {
        strncpy(choice->sets_flag, flag, sizeof(choice->sets_flag) - 1);
    }
}

void dialogue_choice_mark_chosen(DialogueNode* node, int choice_index) {
    if (!node || choice_index < 0 || (size_t)choice_index >= node->choice_count) return;

    DialogueChoice* choice = &node->choices[choice_index];
    choice->chosen = true;

    LOG_DEBUG("Choice %d in node %s marked as chosen", choice_index, node->id);
}

bool dialogue_choice_is_available(const DialogueChoice* choice) {
    if (!choice) return false;

    /* Check if once-only and already chosen */
    if (choice->once_only && choice->chosen) {
        return false;
    }

    /* If no conditions, always available */
    if (choice->condition_count == 0) {
        return true;
    }

    /* Note: Actual condition checking would require game state access */
    /* For now, we just return true if not already chosen */
    /* This will be implemented when integrated with GameState */
    return true;
}

const char* dialogue_condition_type_to_string(ConditionType type) {
    switch (type) {
        case CONDITION_TYPE_NONE:
            return "None";
        case CONDITION_TYPE_QUEST_ACTIVE:
            return "Quest Active";
        case CONDITION_TYPE_QUEST_COMPLETED:
            return "Quest Completed";
        case CONDITION_TYPE_RELATIONSHIP_MIN:
            return "Relationship Minimum";
        case CONDITION_TYPE_SOUL_ENERGY_MIN:
            return "Soul Energy Minimum";
        case CONDITION_TYPE_CORRUPTION_MIN:
            return "Corruption Minimum";
        case CONDITION_TYPE_CORRUPTION_MAX:
            return "Corruption Maximum";
        case CONDITION_TYPE_MEMORY_DISCOVERED:
            return "Memory Discovered";
        case CONDITION_TYPE_ITEM_OWNED:
            return "Item Owned";
        case CONDITION_TYPE_FLAG_SET:
            return "Flag Set";
        default:
            return "Unknown";
    }
}

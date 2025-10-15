/**
 * @file dialogue.h
 * @brief Dialogue node structure and operations
 */

#ifndef NECROMANCERS_DIALOGUE_H
#define NECROMANCERS_DIALOGUE_H

#include <stdbool.h>
#include <stddef.h>

/* Maximum choices per dialogue node */
#define MAX_DIALOGUE_CHOICES 6

/* Maximum conditions per choice */
#define MAX_CHOICE_CONDITIONS 4

/* Condition types for dialogue choices */
typedef enum {
    CONDITION_TYPE_NONE,
    CONDITION_TYPE_QUEST_ACTIVE,
    CONDITION_TYPE_QUEST_COMPLETED,
    CONDITION_TYPE_RELATIONSHIP_MIN,
    CONDITION_TYPE_SOUL_ENERGY_MIN,
    CONDITION_TYPE_CORRUPTION_MIN,
    CONDITION_TYPE_CORRUPTION_MAX,
    CONDITION_TYPE_MEMORY_DISCOVERED,
    CONDITION_TYPE_ITEM_OWNED,
    CONDITION_TYPE_FLAG_SET
} ConditionType;

/**
 * @struct DialogueCondition
 * @brief Condition that must be met for a choice to be available
 */
typedef struct {
    ConditionType type;
    char parameter[64];  /* Quest ID, memory ID, flag name, etc. */
    int value;           /* Minimum relationship, soul energy, etc. */
} DialogueCondition;

/**
 * @struct DialogueChoice
 * @brief A choice available to the player in dialogue
 */
typedef struct {
    char text[256];
    char next_node[64];  /* ID of next dialogue node */

    /* Conditions for this choice */
    DialogueCondition conditions[MAX_CHOICE_CONDITIONS];
    size_t condition_count;

    /* Effects of choosing this option */
    int trust_delta;
    int respect_delta;
    int fear_delta;
    int corruption_delta;

    /* Unlocks */
    char unlocks_quest[64];
    char unlocks_memory[64];
    char sets_flag[64];

    /* Display */
    bool hidden;  /* Don't show unless conditions met */
    bool once_only;  /* Can only be chosen once */
    bool chosen;  /* Has been chosen */
} DialogueChoice;

/**
 * @struct DialogueNode
 * @brief A single node in a dialogue tree
 */
typedef struct {
    char id[64];
    char speaker[64];  /* NPC ID or "PLAYER" */
    char text[512];

    /* Choices available from this node */
    DialogueChoice choices[MAX_DIALOGUE_CHOICES];
    size_t choice_count;

    /* Navigation */
    bool is_end_node;  /* Ends dialogue */
} DialogueNode;

/**
 * @brief Create a dialogue node
 * @param id Node identifier
 * @param speaker Speaker ID (NPC or "PLAYER")
 * @param text Dialogue text
 * @return Newly allocated DialogueNode or NULL on failure
 */
DialogueNode* dialogue_node_create(const char* id, const char* speaker, const char* text);

/**
 * @brief Destroy a dialogue node
 * @param node Node to destroy (can be NULL)
 */
void dialogue_node_destroy(DialogueNode* node);

/**
 * @brief Add a choice to a dialogue node
 * @param node Node to update
 * @param choice_text Text for the choice
 * @param next_node ID of next node
 * @return Index of added choice, or -1 on failure
 */
int dialogue_node_add_choice(DialogueNode* node, const char* choice_text, const char* next_node);

/**
 * @brief Add a condition to a choice
 * @param node Node containing the choice
 * @param choice_index Index of the choice
 * @param type Condition type
 * @param parameter Condition parameter (quest ID, etc.)
 * @param value Condition value (for numeric checks)
 */
void dialogue_choice_add_condition(DialogueNode* node,
                                   int choice_index,
                                   ConditionType type,
                                   const char* parameter,
                                   int value);

/**
 * @brief Set choice effects
 * @param node Node containing the choice
 * @param choice_index Index of the choice
 * @param trust_delta Change in trust
 * @param respect_delta Change in respect
 * @param fear_delta Change in fear
 * @param corruption_delta Change in corruption
 */
void dialogue_choice_set_effects(DialogueNode* node,
                                int choice_index,
                                int trust_delta,
                                int respect_delta,
                                int fear_delta,
                                int corruption_delta);

/**
 * @brief Set what a choice unlocks
 * @param node Node containing the choice
 * @param choice_index Index of the choice
 * @param quest_id Quest to unlock (can be NULL)
 * @param memory_id Memory to unlock (can be NULL)
 * @param flag Flag to set (can be NULL)
 */
void dialogue_choice_set_unlocks(DialogueNode* node,
                                int choice_index,
                                const char* quest_id,
                                const char* memory_id,
                                const char* flag);

/**
 * @brief Mark a choice as chosen
 * @param node Node containing the choice
 * @param choice_index Index of the choice
 */
void dialogue_choice_mark_chosen(DialogueNode* node, int choice_index);

/**
 * @brief Check if a choice is available
 * @param choice Choice to check
 * @return true if available, false otherwise
 */
bool dialogue_choice_is_available(const DialogueChoice* choice);

/**
 * @brief Get condition type as string
 * @param type Condition type
 * @return Human-readable type name
 */
const char* dialogue_condition_type_to_string(ConditionType type);

#endif /* NECROMANCERS_DIALOGUE_H */

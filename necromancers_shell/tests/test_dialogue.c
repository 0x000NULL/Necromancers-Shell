/**
 * @file test_dialogue.c
 * @brief Unit tests for dialogue system
 */

#include "../src/game/narrative/dialogue/dialogue.h"
#include "../src/game/narrative/dialogue/dialogue_tree.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Test: Create dialogue node */
void test_dialogue_node_create(void) {
    DialogueNode* node = dialogue_node_create("greeting", "seraphine", "Welcome, necromancer.");

    assert(node != NULL);
    assert(strcmp(node->id, "greeting") == 0);
    assert(strcmp(node->speaker, "seraphine") == 0);
    assert(strcmp(node->text, "Welcome, necromancer.") == 0);
    assert(node->choice_count == 0);
    assert(node->is_end_node == false);

    dialogue_node_destroy(node);
    printf("✓ test_dialogue_node_create passed\n");
}

/* Test: Create with NULL */
void test_dialogue_node_create_null(void) {
    DialogueNode* n1 = dialogue_node_create(NULL, "speaker", "text");
    assert(n1 == NULL);

    DialogueNode* n2 = dialogue_node_create("id", NULL, "text");
    assert(n2 == NULL);

    DialogueNode* n3 = dialogue_node_create("id", "speaker", NULL);
    assert(n3 == NULL);

    printf("✓ test_dialogue_node_create_null passed\n");
}

/* Test: Add choice */
void test_dialogue_node_add_choice(void) {
    DialogueNode* node = dialogue_node_create("test", "npc", "Hello");
    assert(node != NULL);

    int idx1 = dialogue_node_add_choice(node, "Who are you?", "ask_identity");
    assert(idx1 == 0);
    assert(node->choice_count == 1);
    assert(strcmp(node->choices[0].text, "Who are you?") == 0);
    assert(strcmp(node->choices[0].next_node, "ask_identity") == 0);

    int idx2 = dialogue_node_add_choice(node, "Goodbye", "END");
    assert(idx2 == 1);
    assert(node->choice_count == 2);

    dialogue_node_destroy(node);
    printf("✓ test_dialogue_node_add_choice passed\n");
}

/* Test: Add condition to choice */
void test_dialogue_choice_add_condition(void) {
    DialogueNode* node = dialogue_node_create("test", "npc", "Text");
    assert(node != NULL);

    int idx = dialogue_node_add_choice(node, "Special option", "special");
    assert(idx == 0);

    dialogue_choice_add_condition(node, idx, CONDITION_TYPE_QUEST_COMPLETED, "first_quest", 0);

    assert(node->choices[0].condition_count == 1);
    assert(node->choices[0].conditions[0].type == CONDITION_TYPE_QUEST_COMPLETED);
    assert(strcmp(node->choices[0].conditions[0].parameter, "first_quest") == 0);

    dialogue_node_destroy(node);
    printf("✓ test_dialogue_choice_add_condition passed\n");
}

/* Test: Set choice effects */
void test_dialogue_choice_set_effects(void) {
    DialogueNode* node = dialogue_node_create("test", "npc", "Text");
    assert(node != NULL);

    int idx = dialogue_node_add_choice(node, "Be kind", "kind_response");
    dialogue_choice_set_effects(node, idx, 10, 5, 0, -5);

    assert(node->choices[0].trust_delta == 10);
    assert(node->choices[0].respect_delta == 5);
    assert(node->choices[0].fear_delta == 0);
    assert(node->choices[0].corruption_delta == -5);

    dialogue_node_destroy(node);
    printf("✓ test_dialogue_choice_set_effects passed\n");
}

/* Test: Set choice unlocks */
void test_dialogue_choice_set_unlocks(void) {
    DialogueNode* node = dialogue_node_create("test", "npc", "Text");
    assert(node != NULL);

    int idx = dialogue_node_add_choice(node, "Accept quest", "quest_accepted");
    dialogue_choice_set_unlocks(node, idx, "new_quest", "memory_fragment", "quest_flag");

    assert(strcmp(node->choices[0].unlocks_quest, "new_quest") == 0);
    assert(strcmp(node->choices[0].unlocks_memory, "memory_fragment") == 0);
    assert(strcmp(node->choices[0].sets_flag, "quest_flag") == 0);

    dialogue_node_destroy(node);
    printf("✓ test_dialogue_choice_set_unlocks passed\n");
}

/* Test: Mark choice as chosen */
void test_dialogue_choice_mark_chosen(void) {
    DialogueNode* node = dialogue_node_create("test", "npc", "Text");
    assert(node != NULL);

    int idx = dialogue_node_add_choice(node, "Option", "next");
    assert(node->choices[0].chosen == false);

    dialogue_choice_mark_chosen(node, idx);
    assert(node->choices[0].chosen == true);

    dialogue_node_destroy(node);
    printf("✓ test_dialogue_choice_mark_chosen passed\n");
}

/* Test: Choice availability */
void test_dialogue_choice_is_available(void) {
    DialogueNode* node = dialogue_node_create("test", "npc", "Text");
    assert(node != NULL);

    int idx = dialogue_node_add_choice(node, "Once only option", "next");
    node->choices[0].once_only = true;

    assert(dialogue_choice_is_available(&node->choices[0]) == true);

    dialogue_choice_mark_chosen(node, idx);
    assert(dialogue_choice_is_available(&node->choices[0]) == false);

    dialogue_node_destroy(node);
    printf("✓ test_dialogue_choice_is_available passed\n");
}

/* Test: Create dialogue tree */
void test_dialogue_tree_create(void) {
    DialogueTree* tree = dialogue_tree_create("first_meeting", "seraphine", "greeting");

    assert(tree != NULL);
    assert(strcmp(tree->id, "first_meeting") == 0);
    assert(strcmp(tree->npc_id, "seraphine") == 0);
    assert(strcmp(tree->root_node_id, "greeting") == 0);
    assert(tree->node_count == 0);
    assert(tree->is_active == false);

    dialogue_tree_destroy(tree);
    printf("✓ test_dialogue_tree_create passed\n");
}

/* Test: Add node to tree */
void test_dialogue_tree_add_node(void) {
    DialogueTree* tree = dialogue_tree_create("test", "npc", "start");
    assert(tree != NULL);

    DialogueNode* node1 = dialogue_node_create("start", "npc", "Hello");
    DialogueNode* node2 = dialogue_node_create("next", "npc", "Goodbye");

    dialogue_tree_add_node(tree, node1);
    dialogue_tree_add_node(tree, node2);

    assert(tree->node_count == 2);

    dialogue_tree_destroy(tree);
    printf("✓ test_dialogue_tree_add_node passed\n");
}

/* Test: Get node from tree */
void test_dialogue_tree_get_node(void) {
    DialogueTree* tree = dialogue_tree_create("test", "npc", "start");
    assert(tree != NULL);

    DialogueNode* node = dialogue_node_create("greeting", "npc", "Hello");
    dialogue_tree_add_node(tree, node);

    DialogueNode* found = dialogue_tree_get_node(tree, "greeting");
    assert(found != NULL);
    assert(found == node);

    found = dialogue_tree_get_node(tree, "nonexistent");
    assert(found == NULL);

    dialogue_tree_destroy(tree);
    printf("✓ test_dialogue_tree_get_node passed\n");
}

/* Test: Start dialogue tree */
void test_dialogue_tree_start(void) {
    DialogueTree* tree = dialogue_tree_create("test", "npc", "start");
    assert(tree != NULL);

    assert(tree->is_active == false);

    dialogue_tree_start(tree);

    assert(tree->is_active == true);
    assert(strcmp(tree->current_node_id, "start") == 0);

    dialogue_tree_destroy(tree);
    printf("✓ test_dialogue_tree_start passed\n");
}

/* Test: Navigate dialogue tree */
void test_dialogue_tree_choose(void) {
    DialogueTree* tree = dialogue_tree_create("test", "npc", "start");
    assert(tree != NULL);

    DialogueNode* start = dialogue_node_create("start", "npc", "Hello");
    DialogueNode* next = dialogue_node_create("next", "npc", "Next");

    dialogue_node_add_choice(start, "Continue", "next");
    dialogue_node_add_choice(start, "End", "END");

    dialogue_tree_add_node(tree, start);
    dialogue_tree_add_node(tree, next);

    dialogue_tree_start(tree);

    bool success = dialogue_tree_choose(tree, 0);
    assert(success == true);
    assert(strcmp(tree->current_node_id, "next") == 0);
    assert(tree->is_active == true);

    dialogue_tree_destroy(tree);
    printf("✓ test_dialogue_tree_choose passed\n");
}

/* Test: End dialogue via choice */
void test_dialogue_tree_end_choice(void) {
    DialogueTree* tree = dialogue_tree_create("test", "npc", "start");
    assert(tree != NULL);

    DialogueNode* start = dialogue_node_create("start", "npc", "Hello");
    dialogue_node_add_choice(start, "Goodbye", "END");

    dialogue_tree_add_node(tree, start);
    dialogue_tree_start(tree);

    assert(tree->is_active == true);

    dialogue_tree_choose(tree, 0);

    assert(tree->is_active == false);

    dialogue_tree_destroy(tree);
    printf("✓ test_dialogue_tree_end_choice passed\n");
}

/* Test: Reset dialogue tree */
void test_dialogue_tree_reset(void) {
    DialogueTree* tree = dialogue_tree_create("test", "npc", "start");
    assert(tree != NULL);

    DialogueNode* node = dialogue_node_create("start", "npc", "Hello");
    int idx = dialogue_node_add_choice(node, "Option", "next");
    dialogue_tree_add_node(tree, node);

    dialogue_tree_start(tree);
    dialogue_choice_mark_chosen(node, idx);

    assert(tree->is_active == true);
    assert(node->choices[0].chosen == true);

    dialogue_tree_reset(tree);

    assert(tree->is_active == false);
    assert(node->choices[0].chosen == false);

    dialogue_tree_destroy(tree);
    printf("✓ test_dialogue_tree_reset passed\n");
}

/* Test: Condition type to string */
void test_dialogue_condition_type_to_string(void) {
    assert(strcmp(dialogue_condition_type_to_string(CONDITION_TYPE_NONE), "None") == 0);
    assert(strcmp(dialogue_condition_type_to_string(CONDITION_TYPE_QUEST_ACTIVE), "Quest Active") == 0);
    assert(strcmp(dialogue_condition_type_to_string(CONDITION_TYPE_QUEST_COMPLETED), "Quest Completed") == 0);
    assert(strcmp(dialogue_condition_type_to_string(CONDITION_TYPE_RELATIONSHIP_MIN), "Relationship Minimum") == 0);

    printf("✓ test_dialogue_condition_type_to_string passed\n");
}

/* Run all tests */
int main(void) {
    printf("Running dialogue tests...\n\n");

    test_dialogue_node_create();
    test_dialogue_node_create_null();
    test_dialogue_node_add_choice();
    test_dialogue_choice_add_condition();
    test_dialogue_choice_set_effects();
    test_dialogue_choice_set_unlocks();
    test_dialogue_choice_mark_chosen();
    test_dialogue_choice_is_available();
    test_dialogue_tree_create();
    test_dialogue_tree_add_node();
    test_dialogue_tree_get_node();
    test_dialogue_tree_start();
    test_dialogue_tree_choose();
    test_dialogue_tree_end_choice();
    test_dialogue_tree_reset();
    test_dialogue_condition_type_to_string();

    printf("\n✅ All dialogue tests passed! (16/16)\n");
    return 0;
}

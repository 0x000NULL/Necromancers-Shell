/**
 * @file test_quest_objective.c
 * @brief Unit tests for quest objective system
 */

#include "../src/game/narrative/quests/quest_objective.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Test: Create objective */
void test_quest_objective_create(void) {
    QuestObjective* obj = quest_objective_create("talk_seraphine", "Talk to Seraphine", OBJECTIVE_TYPE_TALK);

    assert(obj != NULL);
    assert(strcmp(obj->id, "talk_seraphine") == 0);
    assert(strcmp(obj->description, "Talk to Seraphine") == 0);
    assert(obj->type == OBJECTIVE_TYPE_TALK);
    assert(obj->target_count == 1);
    assert(obj->current_count == 0);
    assert(obj->completed == false);
    assert(obj->optional == false);
    assert(obj->hidden == false);

    quest_objective_destroy(obj);
    printf("✓ test_quest_objective_create passed\n");
}

/* Test: Create with NULL parameters */
void test_quest_objective_create_null(void) {
    QuestObjective* obj1 = quest_objective_create(NULL, "Description", OBJECTIVE_TYPE_KILL);
    assert(obj1 == NULL);

    QuestObjective* obj2 = quest_objective_create("id", NULL, OBJECTIVE_TYPE_COLLECT);
    assert(obj2 == NULL);

    printf("✓ test_quest_objective_create_null passed\n");
}

/* Test: Destroy NULL */
void test_quest_objective_destroy_null(void) {
    quest_objective_destroy(NULL);
    printf("✓ test_quest_objective_destroy_null passed\n");
}

/* Test: Set target */
void test_quest_objective_set_target(void) {
    QuestObjective* obj = quest_objective_create("kill_enemies", "Kill enemies", OBJECTIVE_TYPE_KILL);
    assert(obj != NULL);

    quest_objective_set_target(obj, "skeleton", 5);
    assert(strcmp(obj->target_id, "skeleton") == 0);
    assert(obj->target_count == 5);

    quest_objective_destroy(obj);
    printf("✓ test_quest_objective_set_target passed\n");
}

/* Test: Increment progress */
void test_quest_objective_increment(void) {
    QuestObjective* obj = quest_objective_create("collect", "Collect souls", OBJECTIVE_TYPE_COLLECT);
    assert(obj != NULL);

    quest_objective_set_target(obj, "soul", 3);

    bool completed = quest_objective_increment(obj, 1);
    assert(completed == false);
    assert(obj->current_count == 1);

    completed = quest_objective_increment(obj, 1);
    assert(completed == false);
    assert(obj->current_count == 2);

    completed = quest_objective_increment(obj, 1);
    assert(completed == true);
    assert(obj->current_count == 3);
    assert(obj->completed == true);

    quest_objective_destroy(obj);
    printf("✓ test_quest_objective_increment passed\n");
}

/* Test: Set progress directly */
void test_quest_objective_set_progress(void) {
    QuestObjective* obj = quest_objective_create("test", "Test", OBJECTIVE_TYPE_VISIT);
    assert(obj != NULL);

    quest_objective_set_target(obj, "location", 5);

    bool completed = quest_objective_set_progress(obj, 3);
    assert(completed == false);
    assert(obj->current_count == 3);

    completed = quest_objective_set_progress(obj, 5);
    assert(completed == true);
    assert(obj->completed == true);

    quest_objective_destroy(obj);
    printf("✓ test_quest_objective_set_progress passed\n");
}

/* Test: Complete objective */
void test_quest_objective_complete(void) {
    QuestObjective* obj = quest_objective_create("test", "Test", OBJECTIVE_TYPE_RAISE);
    assert(obj != NULL);

    quest_objective_set_target(obj, "zombie", 10);

    assert(obj->completed == false);
    assert(obj->current_count == 0);

    quest_objective_complete(obj);

    assert(obj->completed == true);
    assert(obj->current_count == 10);

    quest_objective_destroy(obj);
    printf("✓ test_quest_objective_complete passed\n");
}

/* Test: Is completed */
void test_quest_objective_is_completed(void) {
    QuestObjective* obj = quest_objective_create("test", "Test", OBJECTIVE_TYPE_DISCOVER);
    assert(obj != NULL);

    assert(quest_objective_is_completed(obj) == false);

    quest_objective_complete(obj);

    assert(quest_objective_is_completed(obj) == true);

    quest_objective_destroy(obj);
    printf("✓ test_quest_objective_is_completed passed\n");
}

/* Test: Get progress percent */
void test_quest_objective_get_progress_percent(void) {
    QuestObjective* obj = quest_objective_create("test", "Test", OBJECTIVE_TYPE_KILL);
    assert(obj != NULL);

    quest_objective_set_target(obj, "enemy", 10);

    assert(quest_objective_get_progress_percent(obj) == 0);

    quest_objective_set_progress(obj, 5);
    assert(quest_objective_get_progress_percent(obj) == 50);

    quest_objective_set_progress(obj, 10);
    assert(quest_objective_get_progress_percent(obj) == 100);

    quest_objective_destroy(obj);
    printf("✓ test_quest_objective_get_progress_percent passed\n");
}

/* Test: Type to string */
void test_quest_objective_type_to_string(void) {
    assert(strcmp(quest_objective_type_to_string(OBJECTIVE_TYPE_TALK), "Talk") == 0);
    assert(strcmp(quest_objective_type_to_string(OBJECTIVE_TYPE_KILL), "Kill") == 0);
    assert(strcmp(quest_objective_type_to_string(OBJECTIVE_TYPE_COLLECT), "Collect") == 0);
    assert(strcmp(quest_objective_type_to_string(OBJECTIVE_TYPE_VISIT), "Visit") == 0);
    assert(strcmp(quest_objective_type_to_string(OBJECTIVE_TYPE_RAISE), "Raise") == 0);
    assert(strcmp(quest_objective_type_to_string(OBJECTIVE_TYPE_DISCOVER), "Discover") == 0);
    assert(strcmp(quest_objective_type_to_string(OBJECTIVE_TYPE_RESEARCH), "Research") == 0);
    assert(strcmp(quest_objective_type_to_string(OBJECTIVE_TYPE_RELATIONSHIP), "Relationship") == 0);
    assert(strcmp(quest_objective_type_to_string(OBJECTIVE_TYPE_CUSTOM), "Custom") == 0);

    printf("✓ test_quest_objective_type_to_string passed\n");
}

/* Test: Cannot increment after completed */
void test_quest_objective_no_increment_after_complete(void) {
    QuestObjective* obj = quest_objective_create("test", "Test", OBJECTIVE_TYPE_COLLECT);
    assert(obj != NULL);

    quest_objective_set_target(obj, "item", 5);
    quest_objective_set_progress(obj, 5);

    assert(obj->completed == true);

    /* Try to increment - should fail */
    bool result = quest_objective_increment(obj, 1);
    assert(result == false);
    assert(obj->current_count == 5);

    quest_objective_destroy(obj);
    printf("✓ test_quest_objective_no_increment_after_complete passed\n");
}

/* Test: NULL parameter handling */
void test_quest_objective_null_parameters(void) {
    QuestObjective* obj = quest_objective_create("test", "Test", OBJECTIVE_TYPE_TALK);
    assert(obj != NULL);

    /* Should handle NULL gracefully */
    quest_objective_set_target(NULL, "target", 5);
    quest_objective_increment(NULL, 1);
    quest_objective_set_progress(NULL, 5);
    quest_objective_complete(NULL);

    assert(quest_objective_is_completed(NULL) == false);
    assert(quest_objective_get_progress_percent(NULL) == 0);

    quest_objective_destroy(obj);
    printf("✓ test_quest_objective_null_parameters passed\n");
}

/* Run all tests */
int main(void) {
    printf("Running quest_objective tests...\n\n");

    test_quest_objective_create();
    test_quest_objective_create_null();
    test_quest_objective_destroy_null();
    test_quest_objective_set_target();
    test_quest_objective_increment();
    test_quest_objective_set_progress();
    test_quest_objective_complete();
    test_quest_objective_is_completed();
    test_quest_objective_get_progress_percent();
    test_quest_objective_type_to_string();
    test_quest_objective_no_increment_after_complete();
    test_quest_objective_null_parameters();

    printf("\n✅ All quest_objective tests passed! (12/12)\n");
    return 0;
}

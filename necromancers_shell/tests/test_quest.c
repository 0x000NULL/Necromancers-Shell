/**
 * @file test_quest.c
 * @brief Unit tests for quest system
 */

#include "../src/game/narrative/quests/quest.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Test: Create quest */
void test_quest_create(void) {
    Quest* quest = quest_create("first_contact", "First Contact", "seraphine");

    assert(quest != NULL);
    assert(strcmp(quest->id, "first_contact") == 0);
    assert(strcmp(quest->title, "First Contact") == 0);
    assert(strcmp(quest->quest_giver, "seraphine") == 0);
    assert(quest->state == QUEST_STATE_NOT_STARTED);
    assert(quest->objective_count == 0);
    assert(quest->can_fail == false);

    quest_destroy(quest);
    printf("✓ test_quest_create passed\n");
}

/* Test: Create with NULL */
void test_quest_create_null(void) {
    Quest* q1 = quest_create(NULL, "Title", "npc");
    assert(q1 == NULL);

    Quest* q2 = quest_create("id", NULL, "npc");
    assert(q2 == NULL);

    printf("✓ test_quest_create_null passed\n");
}

/* Test: Add objectives */
void test_quest_add_objective(void) {
    Quest* quest = quest_create("test", "Test Quest", "npc");
    assert(quest != NULL);

    QuestObjective* obj1 = quest_objective_create("talk", "Talk to NPC", OBJECTIVE_TYPE_TALK);
    QuestObjective* obj2 = quest_objective_create("kill", "Kill 5 enemies", OBJECTIVE_TYPE_KILL);

    quest_add_objective(quest, obj1);
    quest_add_objective(quest, obj2);

    assert(quest->objective_count == 2);

    quest_destroy(quest);
    printf("✓ test_quest_add_objective passed\n");
}

/* Test: Get objective */
void test_quest_get_objective(void) {
    Quest* quest = quest_create("test", "Test", "npc");
    assert(quest != NULL);

    QuestObjective* obj = quest_objective_create("talk", "Talk", OBJECTIVE_TYPE_TALK);
    quest_add_objective(quest, obj);

    QuestObjective* found = quest_get_objective(quest, "talk");
    assert(found != NULL);
    assert(found == obj);

    found = quest_get_objective(quest, "nonexistent");
    assert(found == NULL);

    quest_destroy(quest);
    printf("✓ test_quest_get_objective passed\n");
}

/* Test: Start quest */
void test_quest_start(void) {
    Quest* quest = quest_create("test", "Test", "npc");
    assert(quest != NULL);

    assert(quest->state == QUEST_STATE_NOT_STARTED);
    assert(quest->started_time == 0);

    quest_start(quest);

    assert(quest->state == QUEST_STATE_ACTIVE);
    assert(quest->started_time > 0);

    quest_destroy(quest);
    printf("✓ test_quest_start passed\n");
}

/* Test: Complete quest */
void test_quest_complete(void) {
    Quest* quest = quest_create("test", "Test", "npc");
    assert(quest != NULL);

    quest_start(quest);
    assert(quest->state == QUEST_STATE_ACTIVE);

    quest_complete(quest);

    assert(quest->state == QUEST_STATE_COMPLETED);
    assert(quest->completed_time > 0);

    quest_destroy(quest);
    printf("✓ test_quest_complete passed\n");
}

/* Test: Fail quest */
void test_quest_fail(void) {
    Quest* quest = quest_create("test", "Test", "npc");
    assert(quest != NULL);

    quest->can_fail = true;
    quest_start(quest);

    quest_fail(quest);

    assert(quest->state == QUEST_STATE_FAILED);
    assert(quest->completed_time > 0);

    quest_destroy(quest);
    printf("✓ test_quest_fail passed\n");
}

/* Test: All objectives completed */
void test_quest_all_objectives_completed(void) {
    Quest* quest = quest_create("test", "Test", "npc");
    assert(quest != NULL);

    QuestObjective* obj1 = quest_objective_create("obj1", "Objective 1", OBJECTIVE_TYPE_TALK);
    QuestObjective* obj2 = quest_objective_create("obj2", "Objective 2", OBJECTIVE_TYPE_KILL);

    quest_add_objective(quest, obj1);
    quest_add_objective(quest, obj2);

    assert(quest_all_objectives_completed(quest) == false);

    quest_objective_complete(obj1);
    assert(quest_all_objectives_completed(quest) == false);

    quest_objective_complete(obj2);
    assert(quest_all_objectives_completed(quest) == true);

    quest_destroy(quest);
    printf("✓ test_quest_all_objectives_completed passed\n");
}

/* Test: Optional objectives ignored */
void test_quest_optional_objectives(void) {
    Quest* quest = quest_create("test", "Test", "npc");
    assert(quest != NULL);

    QuestObjective* obj1 = quest_objective_create("required", "Required", OBJECTIVE_TYPE_TALK);
    QuestObjective* obj2 = quest_objective_create("optional", "Optional", OBJECTIVE_TYPE_COLLECT);
    obj2->optional = true;

    quest_add_objective(quest, obj1);
    quest_add_objective(quest, obj2);

    quest_objective_complete(obj1);

    /* Should be complete even though optional not done */
    assert(quest_all_objectives_completed(quest) == true);

    quest_destroy(quest);
    printf("✓ test_quest_optional_objectives passed\n");
}

/* Test: Update state */
void test_quest_update_state(void) {
    Quest* quest = quest_create("test", "Test", "npc");
    assert(quest != NULL);

    QuestObjective* obj = quest_objective_create("obj", "Objective", OBJECTIVE_TYPE_TALK);
    quest_add_objective(quest, obj);

    quest_start(quest);

    bool completed = quest_update_state(quest);
    assert(completed == false);
    assert(quest->state == QUEST_STATE_ACTIVE);

    quest_objective_complete(obj);

    completed = quest_update_state(quest);
    assert(completed == true);
    assert(quest->state == QUEST_STATE_COMPLETED);

    quest_destroy(quest);
    printf("✓ test_quest_update_state passed\n");
}

/* Test: Get progress percent */
void test_quest_get_progress_percent(void) {
    Quest* quest = quest_create("test", "Test", "npc");
    assert(quest != NULL);

    QuestObjective* obj1 = quest_objective_create("obj1", "Obj1", OBJECTIVE_TYPE_KILL);
    quest_objective_set_target(obj1, "enemy", 10);

    QuestObjective* obj2 = quest_objective_create("obj2", "Obj2", OBJECTIVE_TYPE_COLLECT);
    quest_objective_set_target(obj2, "item", 10);

    quest_add_objective(quest, obj1);
    quest_add_objective(quest, obj2);

    assert(quest_get_progress_percent(quest) == 0);

    quest_objective_set_progress(obj1, 5);  /* 50% */
    quest_objective_set_progress(obj2, 10); /* 100% */

    /* Average: (50 + 100) / 2 = 75 */
    assert(quest_get_progress_percent(quest) == 75);

    quest_destroy(quest);
    printf("✓ test_quest_get_progress_percent passed\n");
}

/* Test: State to string */
void test_quest_state_to_string(void) {
    assert(strcmp(quest_state_to_string(QUEST_STATE_NOT_STARTED), "Not Started") == 0);
    assert(strcmp(quest_state_to_string(QUEST_STATE_ACTIVE), "Active") == 0);
    assert(strcmp(quest_state_to_string(QUEST_STATE_COMPLETED), "Completed") == 0);
    assert(strcmp(quest_state_to_string(QUEST_STATE_FAILED), "Failed") == 0);

    printf("✓ test_quest_state_to_string passed\n");
}

/* Run all tests */
int main(void) {
    printf("Running quest tests...\n\n");

    test_quest_create();
    test_quest_create_null();
    test_quest_add_objective();
    test_quest_get_objective();
    test_quest_start();
    test_quest_complete();
    test_quest_fail();
    test_quest_all_objectives_completed();
    test_quest_optional_objectives();
    test_quest_update_state();
    test_quest_get_progress_percent();
    test_quest_state_to_string();

    printf("\n✅ All quest tests passed! (12/12)\n");
    return 0;
}

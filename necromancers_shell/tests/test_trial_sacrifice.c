/**
 * @file test_trial_sacrifice.c
 * @brief Unit tests for Trial 6: Test of Sacrifice
 */

#include "../src/game/narrative/trials/trial_sacrifice.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Test counter */
static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) \
    do { \
        tests_run++; \
        printf("Testing %s...\n", name); \
    } while(0)

#define ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf("  FAILED: %s\n", message); \
            return; \
        } \
    } while(0)

#define PASS() \
    do { \
        tests_passed++; \
        printf("  PASSED\n"); \
    } while(0)

/**
 * Test 1: Create and destroy trial state
 */
static void test_create_destroy(void) {
    TEST("create_destroy");

    SacrificeTrialState* state = sacrifice_trial_create();
    ASSERT(state != NULL, "Failed to create sacrifice trial state");
    ASSERT(!state->active, "New trial should not be active");
    ASSERT(state->choice == SACRIFICE_CHOICE_NONE, "Choice should be NONE initially");
    ASSERT(!state->thessara_severed, "Thessara should not be severed initially");
    ASSERT(!state->child_saved, "Child should not be saved initially");

    sacrifice_trial_destroy(state);
    /* If we reach here without crash, destroy works */

    /* Test NULL safety */
    sacrifice_trial_destroy(NULL);

    PASS();
}

/**
 * Test 2: Start trial
 */
static void test_start_trial(void) {
    TEST("start_trial");

    SacrificeTrialState* state = sacrifice_trial_create();
    ASSERT(state != NULL, "Failed to create state");

    bool result = sacrifice_trial_start(state);
    ASSERT(result, "Failed to start trial");
    ASSERT(state->active, "Trial should be active after start");
    ASSERT(state->choice == SACRIFICE_CHOICE_NONE, "Choice should be reset to NONE");

    sacrifice_trial_destroy(state);
    PASS();
}

/**
 * Test 3: Load from data file
 */
static void test_load_from_file(void) {
    TEST("load_from_file");

    SacrificeTrialState* state = sacrifice_trial_create();
    ASSERT(state != NULL, "Failed to create state");

    bool result = sacrifice_trial_load_from_file(state,
        "data/trials/trial_sacrifice.dat");
    ASSERT(result, "Failed to load from file");

    /* Check child info */
    const ChildInfo* child = sacrifice_trial_get_child_info(state);
    ASSERT(child != NULL, "Failed to get child info");
    ASSERT(strcmp(child->name, "Maya") == 0, "Child name should be Maya");
    ASSERT(child->age == 7, "Child age should be 7");
    ASSERT(strcmp(child->location, "Millbrook") == 0, "Location should be Millbrook");
    ASSERT(child->time_limit_hours == 24, "Time limit should be 24 hours");

    /* Check Thessara messages */
    const char* encouragement = sacrifice_trial_get_thessara_encouragement(state);
    ASSERT(encouragement != NULL, "Encouragement message should exist");
    ASSERT(strlen(encouragement) > 0, "Encouragement should not be empty");

    const char* final_msg = sacrifice_trial_get_thessara_final_message(state);
    ASSERT(final_msg != NULL, "Final message should exist");
    ASSERT(strlen(final_msg) > 0, "Final message should not be empty");

    sacrifice_trial_destroy(state);
    PASS();
}

/**
 * Test 4: Choose to save child (PASS)
 */
static void test_save_child_pass(void) {
    TEST("save_child_pass");

    SacrificeTrialState* state = sacrifice_trial_create();
    ASSERT(state != NULL, "Failed to create state");

    sacrifice_trial_start(state);
    ASSERT(state->active, "Trial should be active");

    bool result = sacrifice_trial_choose_save_child(state);
    ASSERT(result, "Save child choice should succeed");
    ASSERT(!state->active, "Trial should be inactive after choice");
    ASSERT(state->choice == SACRIFICE_CHOICE_SAVE_CHILD, "Choice should be SAVE_CHILD");
    ASSERT(state->thessara_severed, "Thessara should be severed");
    ASSERT(state->child_saved, "Child should be saved");

    /* Check completion and pass status */
    ASSERT(sacrifice_trial_is_complete(state), "Trial should be complete");
    ASSERT(sacrifice_trial_is_passed(state), "Trial should be passed");

    /* Check score (perfect selflessness) */
    float score = sacrifice_trial_calculate_score(state);
    ASSERT(score == 100.0f, "Score should be 100");

    sacrifice_trial_destroy(state);
    PASS();
}

/**
 * Test 5: Choose to keep Thessara (FAIL)
 */
static void test_keep_thessara_fail(void) {
    TEST("keep_thessara_fail");

    SacrificeTrialState* state = sacrifice_trial_create();
    ASSERT(state != NULL, "Failed to create state");

    sacrifice_trial_start(state);
    ASSERT(state->active, "Trial should be active");

    bool result = sacrifice_trial_choose_keep_thessara(state);
    ASSERT(!result, "Keep Thessara choice should return false (failed)");
    ASSERT(!state->active, "Trial should be inactive after choice");
    ASSERT(state->choice == SACRIFICE_CHOICE_KEEP_THESSARA, "Choice should be KEEP_THESSARA");
    ASSERT(!state->thessara_severed, "Thessara should not be severed");
    ASSERT(!state->child_saved, "Child should not be saved");

    /* Check completion and pass status */
    ASSERT(sacrifice_trial_is_complete(state), "Trial should be complete");
    ASSERT(!sacrifice_trial_is_passed(state), "Trial should be failed");

    /* Check score (failed) */
    float score = sacrifice_trial_calculate_score(state);
    ASSERT(score == 0.0f, "Score should be 0");

    sacrifice_trial_destroy(state);
    PASS();
}

/**
 * Test 6: Choice permanence
 */
static void test_choice_permanence(void) {
    TEST("choice_permanence");

    SacrificeTrialState* state = sacrifice_trial_create();
    ASSERT(state != NULL, "Failed to create state");

    sacrifice_trial_start(state);
    sacrifice_trial_choose_save_child(state);

    /* Try to change choice (should fail) */
    bool result = sacrifice_trial_choose_keep_thessara(state);
    ASSERT(!result, "Should not be able to change choice");
    ASSERT(state->choice == SACRIFICE_CHOICE_SAVE_CHILD, "Choice should remain SAVE_CHILD");

    sacrifice_trial_destroy(state);
    PASS();
}

/**
 * Test 7: Get child info
 */
static void test_get_child_info(void) {
    TEST("get_child_info");

    SacrificeTrialState* state = sacrifice_trial_create();
    ASSERT(state != NULL, "Failed to create state");

    const ChildInfo* child = sacrifice_trial_get_child_info(state);
    ASSERT(child != NULL, "Should return child info");
    ASSERT(strcmp(child->name, "Maya") == 0, "Default name should be Maya");
    ASSERT(child->age == 7, "Default age should be 7");

    sacrifice_trial_destroy(state);
    PASS();
}

/**
 * Test 8: Get Thessara messages
 */
static void test_get_thessara_messages(void) {
    TEST("get_thessara_messages");

    SacrificeTrialState* state = sacrifice_trial_create();
    ASSERT(state != NULL, "Failed to create state");

    sacrifice_trial_load_from_file(state,
        "data/trials/trial_sacrifice.dat");

    const char* encouragement = sacrifice_trial_get_thessara_encouragement(state);
    ASSERT(encouragement != NULL, "Encouragement should exist");

    const char* final_msg = sacrifice_trial_get_thessara_final_message(state);
    ASSERT(final_msg != NULL, "Final message should exist");

    const char* sadness_msg = sacrifice_trial_get_thessara_sadness_message(state);
    ASSERT(sadness_msg != NULL, "Sadness message should exist");

    sacrifice_trial_destroy(state);
    PASS();
}

/**
 * Test 9: Get child fates
 */
static void test_get_child_fates(void) {
    TEST("get_child_fates");

    SacrificeTrialState* state = sacrifice_trial_create();
    ASSERT(state != NULL, "Failed to create state");

    sacrifice_trial_load_from_file(state,
        "data/trials/trial_sacrifice.dat");

    const char* fate_saved = sacrifice_trial_get_child_fate_saved(state);
    ASSERT(fate_saved != NULL, "Saved fate should exist");
    ASSERT(strlen(fate_saved) > 0, "Saved fate should not be empty");

    const char* fate_died = sacrifice_trial_get_child_fate_died(state);
    ASSERT(fate_died != NULL, "Died fate should exist");
    ASSERT(strlen(fate_died) > 0, "Died fate should not be empty");

    sacrifice_trial_destroy(state);
    PASS();
}

/**
 * Test 10: Check completion states
 */
static void test_completion_checks(void) {
    TEST("completion_checks");

    SacrificeTrialState* state = sacrifice_trial_create();
    ASSERT(state != NULL, "Failed to create state");

    /* Before choice */
    ASSERT(!sacrifice_trial_choice_made(state), "No choice made initially");
    ASSERT(!sacrifice_trial_is_complete(state), "Not complete initially");
    ASSERT(!sacrifice_trial_is_passed(state), "Not passed initially");

    /* After save child choice */
    sacrifice_trial_start(state);
    sacrifice_trial_choose_save_child(state);

    ASSERT(sacrifice_trial_choice_made(state), "Choice should be made");
    ASSERT(sacrifice_trial_is_complete(state), "Should be complete");
    ASSERT(sacrifice_trial_is_passed(state), "Should be passed");
    ASSERT(sacrifice_trial_is_thessara_severed(state), "Thessara severed");
    ASSERT(sacrifice_trial_is_child_saved(state), "Child saved");

    sacrifice_trial_destroy(state);
    PASS();
}

/**
 * Test 11: Choice name strings
 */
static void test_choice_names(void) {
    TEST("choice_names");

    const char* name_none = sacrifice_trial_choice_name(SACRIFICE_CHOICE_NONE);
    ASSERT(name_none != NULL, "NONE name should exist");

    const char* name_save = sacrifice_trial_choice_name(SACRIFICE_CHOICE_SAVE_CHILD);
    ASSERT(name_save != NULL, "SAVE_CHILD name should exist");

    const char* name_keep = sacrifice_trial_choice_name(SACRIFICE_CHOICE_KEEP_THESSARA);
    ASSERT(name_keep != NULL, "KEEP_THESSARA name should exist");

    PASS();
}

/**
 * Test 12: NULL safety
 */
static void test_null_safety(void) {
    TEST("null_safety");

    /* All functions should handle NULL gracefully */
    ASSERT(!sacrifice_trial_start(NULL), "Start with NULL should fail");
    ASSERT(!sacrifice_trial_load_from_file(NULL, "test"), "Load with NULL should fail");
    ASSERT(!sacrifice_trial_choose_save_child(NULL), "Save child with NULL should fail");
    ASSERT(!sacrifice_trial_choose_keep_thessara(NULL), "Keep Thessara with NULL should fail");
    ASSERT(sacrifice_trial_get_child_info(NULL) == NULL, "Get child info with NULL");
    ASSERT(sacrifice_trial_get_thessara_encouragement(NULL) == NULL, "Get encouragement with NULL");
    ASSERT(!sacrifice_trial_choice_made(NULL), "Choice made with NULL");
    ASSERT(!sacrifice_trial_is_complete(NULL), "Is complete with NULL");
    ASSERT(!sacrifice_trial_is_passed(NULL), "Is passed with NULL");
    ASSERT(sacrifice_trial_calculate_score(NULL) == 0.0f, "Score with NULL");

    PASS();
}

/**
 * Main test runner
 */
int main(void) {
    printf("=== Trial Sacrifice Tests ===\n\n");

    test_create_destroy();
    test_start_trial();
    test_load_from_file();
    test_save_child_pass();
    test_keep_thessara_fail();
    test_choice_permanence();
    test_get_child_info();
    test_get_thessara_messages();
    test_get_child_fates();
    test_completion_checks();
    test_choice_names();
    test_null_safety();

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    if (tests_passed == tests_run) {
        printf("\nAll tests PASSED!\n");
        return 0;
    } else {
        printf("\nSome tests FAILED!\n");
        return 1;
    }
}

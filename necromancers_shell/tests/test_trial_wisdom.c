#include "../src/game/narrative/trials/trial_wisdom.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) \
    static void name(void); \
    static void run_##name(void) { \
        tests_run++; \
        printf("Running %s...\n", #name); \
        name(); \
        tests_passed++; \
        printf("  PASSED\n"); \
    } \
    static void name(void)

/* Test: Create and destroy */
TEST(test_wisdom_trial_create_destroy) {
    WisdomTrialState* state = wisdom_trial_create();
    assert(state != NULL);
    assert(state->active == false);
    assert(state->solution_type == WISDOM_SOLUTION_NONE);
    assert(state->attempts_made == 0);
    assert(state->hints_used == 0);
    assert(state->puzzle_solved == false);

    wisdom_trial_destroy(state);
}

/* Test: Start trial */
TEST(test_wisdom_trial_start) {
    WisdomTrialState* state = wisdom_trial_create();
    assert(state != NULL);

    /* Start without data file (uses defaults) */
    bool result = wisdom_trial_start(state, "nonexistent_file.dat");
    assert(result == true);
    assert(state->active == true);
    assert(strcmp(state->soul_id, "soldier_penance_001") == 0);
    assert(strcmp(state->soul_name, "Marcus Valerius") == 0);
    assert(state->stuck_years == 200);

    wisdom_trial_destroy(state);
}

/* Test: Orthodox Heaven solution (should fail) */
TEST(test_wisdom_trial_orthodox_heaven_fails) {
    WisdomTrialState* state = wisdom_trial_create();
    wisdom_trial_start(state, "nonexistent.dat");

    bool result = wisdom_trial_submit_orthodox_heaven(state);
    assert(result == false);
    assert(state->solution_type == WISDOM_SOLUTION_ORTHODOX_HEAVEN);
    assert(state->heaven_percent == 100);
    assert(state->hell_percent == 0);
    assert(state->puzzle_solved == false);
    assert(state->attempts_made == 1);

    wisdom_trial_destroy(state);
}

/* Test: Orthodox Hell solution (should fail) */
TEST(test_wisdom_trial_orthodox_hell_fails) {
    WisdomTrialState* state = wisdom_trial_create();
    wisdom_trial_start(state, "nonexistent.dat");

    bool result = wisdom_trial_submit_orthodox_hell(state);
    assert(result == false);
    assert(state->solution_type == WISDOM_SOLUTION_ORTHODOX_HELL);
    assert(state->heaven_percent == 0);
    assert(state->hell_percent == 100);
    assert(state->puzzle_solved == false);
    assert(state->attempts_made == 1);

    wisdom_trial_destroy(state);
}

/* Test: Correct split route solution (60/40/1000) */
TEST(test_wisdom_trial_correct_solution) {
    WisdomTrialState* state = wisdom_trial_create();
    wisdom_trial_start(state, "nonexistent.dat");

    bool result = wisdom_trial_submit_split_route(state, 60, 40, 1000);
    assert(result == true);
    assert(state->solution_type == WISDOM_SOLUTION_SPLIT_ROUTE);
    assert(state->heaven_percent == 60);
    assert(state->hell_percent == 40);
    assert(state->reunification_years == 1000);
    assert(state->puzzle_solved == true);
    assert(state->score == 100.0f); /* Perfect score on first try */

    wisdom_trial_destroy(state);
}

/* Test: Correct split but wrong time */
TEST(test_wisdom_trial_correct_split_wrong_time) {
    WisdomTrialState* state = wisdom_trial_create();
    wisdom_trial_start(state, "nonexistent.dat");

    bool result = wisdom_trial_submit_split_route(state, 60, 40, 500);
    assert(result == true);
    assert(state->puzzle_solved == true);
    assert(state->score == 80.0f); /* 80% of base score */

    wisdom_trial_destroy(state);
}

/* Test: Close but not perfect split */
TEST(test_wisdom_trial_close_solution) {
    WisdomTrialState* state = wisdom_trial_create();
    wisdom_trial_start(state, "nonexistent.dat");

    /* 55/45 is within 20% total difference */
    bool result = wisdom_trial_submit_split_route(state, 55, 45, 1000);
    assert(result == true);
    assert(state->puzzle_solved == true);
    assert(state->score > 0.0f && state->score < 100.0f);

    wisdom_trial_destroy(state);
}

/* Test: Invalid split (wrong percentages) */
TEST(test_wisdom_trial_invalid_split) {
    WisdomTrialState* state = wisdom_trial_create();
    wisdom_trial_start(state, "nonexistent.dat");

    /* 71/29 is too far from 60/40 (total diff = 22 > 20) */
    bool result = wisdom_trial_submit_split_route(state, 71, 29, 1000);
    assert(result == false);
    assert(state->puzzle_solved == false);

    wisdom_trial_destroy(state);
}

/* Test: Invalid percentages sum */
TEST(test_wisdom_trial_invalid_sum) {
    WisdomTrialState* state = wisdom_trial_create();
    wisdom_trial_start(state, "nonexistent.dat");

    /* Doesn't sum to 100 */
    bool result = wisdom_trial_submit_split_route(state, 50, 40, 1000);
    assert(result == false);

    wisdom_trial_destroy(state);
}

/* Test: Hints */
TEST(test_wisdom_trial_hints) {
    WisdomTrialState* state = wisdom_trial_create();
    wisdom_trial_start(state, "nonexistent.dat");

    const char* hint1 = wisdom_trial_get_hint(state, 1);
    assert(hint1 != NULL);
    assert(state->hints_used == 1);

    const char* hint2 = wisdom_trial_get_hint(state, 2);
    assert(hint2 != NULL);
    assert(state->hints_used == 2);

    const char* hint3 = wisdom_trial_get_hint(state, 3);
    assert(hint3 != NULL);
    assert(state->hints_used == 3);

    const char* invalid = wisdom_trial_get_hint(state, 99);
    assert(invalid == NULL);

    wisdom_trial_destroy(state);
}

/* Test: Score calculation with hints penalty */
TEST(test_wisdom_trial_score_with_hints) {
    WisdomTrialState* state = wisdom_trial_create();
    wisdom_trial_start(state, "nonexistent.dat");

    /* Use 2 hints */
    wisdom_trial_get_hint(state, 1);
    wisdom_trial_get_hint(state, 2);

    /* Submit correct solution */
    wisdom_trial_submit_split_route(state, 60, 40, 1000);

    /* Score should be 100 - (2 * 10) = 80 */
    assert(state->score == 80.0f);

    wisdom_trial_destroy(state);
}

/* Test: Score calculation with attempts penalty */
TEST(test_wisdom_trial_score_with_attempts) {
    WisdomTrialState* state = wisdom_trial_create();
    wisdom_trial_start(state, "nonexistent.dat");

    /* Make 2 wrong attempts */
    wisdom_trial_submit_orthodox_heaven(state);
    wisdom_trial_submit_orthodox_hell(state);

    /* Submit correct solution */
    wisdom_trial_submit_split_route(state, 60, 40, 1000);

    /* Score should be 100 - (2 * 5) = 90 */
    assert(state->score == 90.0f);

    wisdom_trial_destroy(state);
}

/* Test: Is complete */
TEST(test_wisdom_trial_is_complete) {
    WisdomTrialState* state = wisdom_trial_create();
    wisdom_trial_start(state, "nonexistent.dat");

    assert(wisdom_trial_is_complete(state) == false);

    wisdom_trial_submit_split_route(state, 60, 40, 1000);

    assert(wisdom_trial_is_complete(state) == true);

    wisdom_trial_destroy(state);
}

/* Test: Is passed */
TEST(test_wisdom_trial_is_passed) {
    WisdomTrialState* state = wisdom_trial_create();
    wisdom_trial_start(state, "nonexistent.dat");

    assert(wisdom_trial_is_passed(state) == false);

    wisdom_trial_submit_split_route(state, 60, 40, 1000);

    assert(wisdom_trial_is_passed(state) == true);

    wisdom_trial_destroy(state);
}

/* Test: Get soul info */
TEST(test_wisdom_trial_get_soul_info) {
    WisdomTrialState* state = wisdom_trial_create();
    wisdom_trial_start(state, "nonexistent.dat");

    const char* info = wisdom_trial_get_soul_info(state);
    assert(info != NULL);
    assert(strstr(info, "Marcus Valerius") != NULL);
    assert(strstr(info, "200 years") != NULL);

    wisdom_trial_destroy(state);
}

int main(void) {
    printf("=== Testing Trial 2: Wisdom ===\n\n");

    run_test_wisdom_trial_create_destroy();
    run_test_wisdom_trial_start();
    run_test_wisdom_trial_orthodox_heaven_fails();
    run_test_wisdom_trial_orthodox_hell_fails();
    run_test_wisdom_trial_correct_solution();
    run_test_wisdom_trial_correct_split_wrong_time();
    run_test_wisdom_trial_close_solution();
    run_test_wisdom_trial_invalid_split();
    run_test_wisdom_trial_invalid_sum();
    run_test_wisdom_trial_hints();
    run_test_wisdom_trial_score_with_hints();
    run_test_wisdom_trial_score_with_attempts();
    run_test_wisdom_trial_is_complete();
    run_test_wisdom_trial_is_passed();
    run_test_wisdom_trial_get_soul_info();

    printf("\n=== Test Summary ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    return (tests_run == tests_passed) ? 0 : 1;
}

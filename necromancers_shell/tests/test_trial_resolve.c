#include "../src/game/narrative/trials/trial_resolve.h"
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
TEST(test_resolve_trial_create_destroy) {
    ResolveTrialState* state = resolve_trial_create();
    assert(state != NULL);
    assert(state->active == false);
    assert(state->days_remaining == 30);
    assert(state->current_day == 0);
    assert(state->corruption_max_allowed == 60);
    assert(state->trial_failed == false);

    resolve_trial_destroy(state);
}

/* Test: Start trial */
TEST(test_resolve_trial_start) {
    ResolveTrialState* state = resolve_trial_create();
    assert(state != NULL);

    uint8_t current_corruption = 41;
    bool result = resolve_trial_start(state, "nonexistent_file.dat", current_corruption);
    assert(result == true);
    assert(state->active == true);
    assert(state->current_day == 1);
    assert(state->corruption_start == 41);
    assert(state->corruption_current == 41);
    assert(state->todays_temptation != TEMPTATION_NONE);

    resolve_trial_destroy(state);
}

/* Test: Advance day */
TEST(test_resolve_trial_advance_day) {
    ResolveTrialState* state = resolve_trial_create();
    resolve_trial_start(state, "nonexistent.dat", 41);

    uint8_t corruption = state->corruption_current;

    bool result = resolve_trial_advance_day(state, &corruption);
    assert(result == true);
    assert(state->current_day == 2);
    assert(state->days_remaining == 29);

    /* After multiple days, corruption should increase */
    for (int i = 0; i < 10; i++) {
        resolve_trial_advance_day(state, &corruption);
    }
    assert(corruption > 41); /* Should be noticeably higher after 10+ days */

    resolve_trial_destroy(state);
}

/* Test: Resist temptation */
TEST(test_resolve_trial_resist_temptation) {
    ResolveTrialState* state = resolve_trial_create();
    resolve_trial_start(state, "nonexistent.dat", 41);

    assert(state->todays_temptation != TEMPTATION_NONE);

    bool result = resolve_trial_resist_temptation(state);
    assert(result == true);
    assert(state->temptations_resisted == 1);
    assert(state->todays_temptation == TEMPTATION_NONE);

    resolve_trial_destroy(state);
}

/* Test: Accept temptation */
TEST(test_resolve_trial_accept_temptation) {
    ResolveTrialState* state = resolve_trial_create();
    resolve_trial_start(state, "nonexistent.dat", 41);

    int64_t energy = 10000;
    uint8_t corruption = state->corruption_current;
    uint8_t old_corruption = corruption;

    bool result = resolve_trial_accept_temptation(state, &energy, &corruption);
    assert(result == true);
    assert(energy == 15000); /* Gained 5000 */
    assert(corruption == old_corruption + 10); /* +10% corruption */
    assert(state->temptations_accepted == 1);
    assert(state->todays_temptation == TEMPTATION_NONE);

    resolve_trial_destroy(state);
}

/* Test: Accept temptation fails if exceeds limit */
TEST(test_resolve_trial_accept_temptation_exceeds_limit) {
    ResolveTrialState* state = resolve_trial_create();
    resolve_trial_start(state, "nonexistent.dat", 41);

    /* Manually set corruption near limit */
    state->corruption_current = 55;

    int64_t energy = 10000;
    uint8_t corruption = state->corruption_current;

    /* Accepting would push to 65%, exceeding 60% limit */
    bool result = resolve_trial_accept_temptation(state, &energy, &corruption);
    assert(result == false);
    assert(state->trial_failed == true);

    resolve_trial_destroy(state);
}

/* Test: Corruption exceeds limit during advance */
TEST(test_resolve_trial_corruption_exceeds_limit) {
    ResolveTrialState* state = resolve_trial_create();
    resolve_trial_start(state, "nonexistent.dat", 41);

    /* Manually set corruption at the limit */
    state->corruption_current = 60;

    uint8_t corruption = state->corruption_current;

    /* Already at 60%, advancing will exceed limit */
    bool result = resolve_trial_advance_day(state, &corruption);
    assert(result == false);
    assert(state->trial_failed == true);
    assert(strlen(state->failure_reason) > 0);

    resolve_trial_destroy(state);
}

/* Test: Thessara help */
TEST(test_resolve_trial_thessara_help) {
    ResolveTrialState* state = resolve_trial_create();
    resolve_trial_start(state, "nonexistent.dat", 41);

    /* Advance to day 19 */
    uint8_t corruption = state->corruption_current;
    for (int i = 1; i < 19; i++) {
        resolve_trial_advance_day(state, &corruption);
    }

    /* Set corruption above threshold before advancing to day 20 */
    assert(state->current_day == 19);
    state->corruption_current = 56;
    corruption = 56;

    /* Advance to day 20 - this will check and enable Thessara help */
    resolve_trial_advance_day(state, &corruption);

    assert(state->current_day == 20);
    assert(resolve_trial_can_use_thessara_help(state) == true);

    bool result = resolve_trial_request_thessara_help(state, &corruption);
    assert(result == true);
    assert(corruption == 51); /* -5 corruption */
    assert(state->thessara_help_used == true);

    resolve_trial_destroy(state);
}

/* Test: Thessara help not available if corruption low */
TEST(test_resolve_trial_thessara_help_not_available_low_corruption) {
    ResolveTrialState* state = resolve_trial_create();
    resolve_trial_start(state, "nonexistent.dat", 41);

    /* Advance to day 19 */
    uint8_t corruption = state->corruption_current;
    for (int i = 1; i < 19; i++) {
        resolve_trial_advance_day(state, &corruption);
    }

    /* Keep corruption below threshold before advancing to day 20 */
    state->corruption_current = 50;

    /* Advance to day 20 - corruption is below threshold so help not available */
    resolve_trial_advance_day(state, &corruption);

    assert(state->current_day == 20);
    assert(resolve_trial_can_use_thessara_help(state) == false);

    resolve_trial_destroy(state);
}

/* Test: Complete 30 days successfully */
TEST(test_resolve_trial_complete_30_days) {
    ResolveTrialState* state = resolve_trial_create();
    resolve_trial_start(state, "nonexistent.dat", 41);

    uint8_t corruption = state->corruption_current;

    /* Advance through all 30 days, resisting all temptations */
    for (int i = 0; i < 30; i++) {
        if (state->todays_temptation != TEMPTATION_NONE) {
            resolve_trial_resist_temptation(state);
        }

        if (state->days_remaining > 0) {
            resolve_trial_advance_day(state, &corruption);
        }
    }

    assert(state->days_remaining == 0);
    assert(resolve_trial_is_complete(state) == true);
    assert(resolve_trial_is_passed(state) == true);
    assert(state->trial_failed == false);

    resolve_trial_destroy(state);
}

/* Test: Calculate score (perfect) */
TEST(test_resolve_trial_score_perfect) {
    ResolveTrialState* state = resolve_trial_create();

    state->trial_failed = false;
    state->days_remaining = 0;
    state->corruption_current = 48;
    state->temptations_resisted = 30;
    state->temptations_accepted = 0;
    state->thessara_help_used = false;

    float score = resolve_trial_calculate_score(state);
    assert(score == 100.0f);

    resolve_trial_destroy(state);
}

/* Test: Calculate score with penalties */
TEST(test_resolve_trial_score_with_penalties) {
    ResolveTrialState* state = resolve_trial_create();

    state->trial_failed = false;
    state->days_remaining = 0;
    state->corruption_current = 56;
    state->temptations_resisted = 20;
    state->temptations_accepted = 5;
    state->thessara_help_used = true;

    float score = resolve_trial_calculate_score(state);
    assert(score > 0.0f && score < 100.0f);

    resolve_trial_destroy(state);
}

/* Test: Get status */
TEST(test_resolve_trial_get_status) {
    ResolveTrialState* state = resolve_trial_create();
    resolve_trial_start(state, "nonexistent.dat", 41);

    const char* status = resolve_trial_get_status(state);
    assert(status != NULL);
    assert(strstr(status, "Chamber") != NULL);
    assert(strstr(status, "Day: 1") != NULL);

    resolve_trial_destroy(state);
}

/* Test: Temptation names */
TEST(test_resolve_trial_temptation_names) {
    assert(strcmp(resolve_trial_temptation_name(TEMPTATION_NONE), "None") == 0);
    assert(strcmp(resolve_trial_temptation_name(TEMPTATION_POWER), "Power") == 0);
    assert(strcmp(resolve_trial_temptation_name(TEMPTATION_KNOWLEDGE), "Knowledge") == 0);
    assert(strcmp(resolve_trial_temptation_name(TEMPTATION_DOMINION), "Dominion") == 0);
    assert(strcmp(resolve_trial_temptation_name(TEMPTATION_IMMORTALITY), "Immortality") == 0);
}

int main(void) {
    printf("=== Testing Trial 5: Resolve ===\n\n");

    run_test_resolve_trial_create_destroy();
    run_test_resolve_trial_start();
    run_test_resolve_trial_advance_day();
    run_test_resolve_trial_resist_temptation();
    run_test_resolve_trial_accept_temptation();
    run_test_resolve_trial_accept_temptation_exceeds_limit();
    run_test_resolve_trial_corruption_exceeds_limit();
    run_test_resolve_trial_thessara_help();
    run_test_resolve_trial_thessara_help_not_available_low_corruption();
    run_test_resolve_trial_complete_30_days();
    run_test_resolve_trial_score_perfect();
    run_test_resolve_trial_score_with_penalties();
    run_test_resolve_trial_get_status();
    run_test_resolve_trial_temptation_names();

    printf("\n=== Test Summary ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    return (tests_run == tests_passed) ? 0 : 1;
}

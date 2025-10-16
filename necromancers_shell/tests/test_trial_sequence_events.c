/**
 * @file test_trial_sequence_events.c
 * @brief Unit tests for trial sequence event system
 */

#include "../src/game/events/trial_sequence_events.h"
#include "../src/game/game_state.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) \
    do { \
        printf("  Testing %s... ", name); \
        tests_run++; \
    } while(0)

#define PASS() \
    do { \
        printf("PASS\n"); \
        tests_passed++; \
    } while(0)

void test_initial_state(void) {
    TEST("initial state");

    GameState* state = game_state_create();
    assert(state != NULL);

    trial_sequence_reset_for_testing();

    /* Initial state checks */
    assert(trial_sequence_get_state(state) == TRIAL_SEQ_INACTIVE);
    assert(trial_sequence_count_completed(state) == 0);
    assert(trial_sequence_count_failed(state) == 0);
    assert(trial_sequence_all_completed(state) == false);

    game_state_destroy(state);

    PASS();
}

void test_trial_unlocking(void) {
    TEST("trial unlocking");

    GameState* state = game_state_create();
    assert(state != NULL);

    trial_sequence_reset_for_testing();

    /* Initially no trials unlocked */
    assert(trial_sequence_is_unlocked(state, 1) == false);
    assert(trial_sequence_is_unlocked(state, 2) == false);

    /* Simulate Trial 1 completion */
    trial_sequence_unlock_next(state, 1);

    /* Trial 2 should now be unlocked */
    assert(trial_sequence_is_unlocked(state, 2) == true);
    assert(trial_sequence_is_unlocked(state, 3) == false);

    game_state_destroy(state);

    PASS();
}

void test_trial_completion_tracking(void) {
    TEST("trial completion tracking");

    GameState* state = game_state_create();
    assert(state != NULL);

    trial_sequence_reset_for_testing();

    /* Complete trials sequentially */
    for (uint32_t i = 1; i <= 7; i++) {
        assert(trial_sequence_count_completed(state) == i - 1);
        trial_sequence_on_completion(state, i);
        assert(trial_sequence_is_completed(state, i) == true);
        assert(trial_sequence_count_completed(state) == i);
    }

    /* All trials should be complete */
    assert(trial_sequence_all_completed(state) == true);
    assert(trial_sequence_get_state(state) == TRIAL_SEQ_COMPLETED);

    game_state_destroy(state);

    PASS();
}

void test_progress_structure(void) {
    TEST("progress structure");

    GameState* state = game_state_create();
    assert(state != NULL);

    trial_sequence_reset_for_testing();

    /* Get initial progress */
    TrialSequenceProgress progress = trial_sequence_get_progress(state);
    assert(progress.state == TRIAL_SEQ_INACTIVE);
    assert(progress.trials_completed == 0);
    assert(progress.trials_unlocked == 0);

    /* Complete a trial */
    trial_sequence_on_completion(state, 1);
    progress = trial_sequence_get_progress(state);
    assert(progress.trials_completed != 0);

    game_state_destroy(state);

    PASS();
}

int main(void) {
    printf("\n=== Trial Sequence Event Tests ===\n\n");

    test_initial_state();
    test_trial_unlocking();
    test_trial_completion_tracking();
    test_progress_structure();

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    return (tests_run == tests_passed) ? 0 : 1;
}

#include "../src/game/narrative/trials/archon_trial.h"
#include "../src/game/narrative/trials/trial_power.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Test counter */
static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) \
    do { \
        tests_run++; \
        printf("Running test: %s\n", name); \
    } while(0)

#define ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            printf("  FAILED: %s\n", msg); \
            return false; \
        } \
    } while(0)

#define PASS() \
    do { \
        tests_passed++; \
        printf("  PASSED\n"); \
        return true; \
    } while(0)

/* Test 1: Manager creation and destruction */
static bool test_manager_create_destroy(void) {
    TEST("Manager creation and destruction");

    ArchonTrialManager* manager = archon_trial_manager_create();
    ASSERT(manager != NULL, "Manager creation failed");
    ASSERT(manager->trial_count == 0, "Initial trial count should be 0");
    ASSERT(manager->current_trial_id == 0, "No active trial initially");
    ASSERT(manager->path_active == false, "Path not active initially");
    ASSERT(manager->path_completed == false, "Path not completed initially");
    ASSERT(manager->path_failed == false, "Path not failed initially");

    archon_trial_manager_destroy(manager);
    PASS();
}

/* Test 2: Load trials from data file */
static bool test_load_trials_from_file(void) {
    TEST("Load trials from data file");

    ArchonTrialManager* manager = archon_trial_manager_create();
    ASSERT(manager != NULL, "Manager creation failed");

    bool loaded = archon_trial_load_from_file(manager,
        "data/trials/archon_trials.dat");
    ASSERT(loaded == true, "Failed to load trial data");
    ASSERT(manager->trial_count == 7, "Should load 7 trials");

    /* Verify first trial (Test of Power) */
    const ArchonTrial* trial1 = archon_trial_get_by_number(manager, 1);
    ASSERT(trial1 != NULL, "Trial 1 not found");
    ASSERT(strcmp(trial1->name, "Test of Power") == 0, "Trial 1 name mismatch");
    ASSERT(trial1->type == TRIAL_TYPE_COMBAT, "Trial 1 should be combat type");
    ASSERT(trial1->status == TRIAL_STATUS_LOCKED, "Trial 1 starts locked");
    ASSERT(trial1->unlock_corruption_min == 30, "Trial 1 min corruption should be 30");
    ASSERT(trial1->unlock_corruption_max == 60, "Trial 1 max corruption should be 60");
    ASSERT(trial1->prerequisite_trial_id == 0, "Trial 1 has no prerequisite");

    /* Verify last trial (Test of Leadership) */
    const ArchonTrial* trial7 = archon_trial_get_by_number(manager, 7);
    ASSERT(trial7 != NULL, "Trial 7 not found");
    ASSERT(strcmp(trial7->name, "Test of Leadership") == 0, "Trial 7 name mismatch");
    ASSERT(trial7->prerequisite_trial_id == 6, "Trial 7 requires trial 6");

    archon_trial_manager_destroy(manager);
    PASS();
}

/* Test 3: Trial unlock conditions */
static bool test_trial_unlock_conditions(void) {
    TEST("Trial unlock conditions");

    ArchonTrialManager* manager = archon_trial_manager_create();
    ASSERT(manager != NULL, "Manager creation failed");

    archon_trial_load_from_file(manager,
        "data/trials/archon_trials.dat");

    /* Trial 1 should unlock with corruption 30-60, consciousness 50+ */
    bool can_unlock = archon_trial_can_unlock(manager, 1, 45, 60.0f);
    ASSERT(can_unlock == true, "Trial 1 should be unlockable at 45% corruption, 60% consciousness");

    /* Too low corruption */
    can_unlock = archon_trial_can_unlock(manager, 1, 20, 60.0f);
    ASSERT(can_unlock == false, "Trial 1 should not unlock at 20% corruption");

    /* Too high corruption */
    can_unlock = archon_trial_can_unlock(manager, 1, 70, 60.0f);
    ASSERT(can_unlock == false, "Trial 1 should not unlock at 70% corruption");

    /* Too low consciousness */
    can_unlock = archon_trial_can_unlock(manager, 1, 45, 40.0f);
    ASSERT(can_unlock == false, "Trial 1 should not unlock at 40% consciousness");

    /* Trial 2 should not unlock without Trial 1 passed */
    can_unlock = archon_trial_can_unlock(manager, 2, 45, 60.0f);
    ASSERT(can_unlock == false, "Trial 2 requires Trial 1 prerequisite");

    archon_trial_manager_destroy(manager);
    PASS();
}

/* Test 4: Trial unlock */
static bool test_trial_unlock(void) {
    TEST("Trial unlock");

    ArchonTrialManager* manager = archon_trial_manager_create();
    ASSERT(manager != NULL, "Manager creation failed");

    archon_trial_load_from_file(manager,
        "data/trials/archon_trials.dat");

    /* Unlock Trial 1 */
    bool unlocked = archon_trial_unlock(manager, 1, 45, 60.0f);
    ASSERT(unlocked == true, "Trial 1 should unlock");

    const ArchonTrial* trial1 = archon_trial_get(manager, 1);
    ASSERT(trial1 != NULL, "Trial 1 should exist");
    ASSERT(trial1->status == TRIAL_STATUS_AVAILABLE, "Trial 1 should be available");

    /* Try to unlock again (should fail) */
    unlocked = archon_trial_unlock(manager, 1, 45, 60.0f);
    ASSERT(unlocked == false, "Trial 1 already unlocked");

    archon_trial_manager_destroy(manager);
    PASS();
}

/* Test 5: Trial start */
static bool test_trial_start(void) {
    TEST("Trial start");

    ArchonTrialManager* manager = archon_trial_manager_create();
    ASSERT(manager != NULL, "Manager creation failed");

    archon_trial_load_from_file(manager,
        "data/trials/archon_trials.dat");

    /* Unlock and start Trial 1 */
    archon_trial_unlock(manager, 1, 45, 60.0f);
    bool started = archon_trial_start(manager, 1);
    ASSERT(started == true, "Trial 1 should start");

    const ArchonTrial* trial1 = archon_trial_get(manager, 1);
    ASSERT(trial1 != NULL, "Trial 1 should exist");
    ASSERT(trial1->status == TRIAL_STATUS_IN_PROGRESS, "Trial 1 should be in progress");
    ASSERT(manager->current_trial_id == 1, "Current trial should be 1");

    archon_trial_manager_destroy(manager);
    PASS();
}

/* Test 6: Trial completion */
static bool test_trial_complete(void) {
    TEST("Trial completion");

    ArchonTrialManager* manager = archon_trial_manager_create();
    ASSERT(manager != NULL, "Manager creation failed");

    archon_trial_load_from_file(manager,
        "data/trials/archon_trials.dat");

    /* Unlock, start, and complete Trial 1 */
    archon_trial_unlock(manager, 1, 45, 60.0f);
    archon_trial_start(manager, 1);
    bool completed = archon_trial_complete(manager, 1, 85.0f);
    ASSERT(completed == true, "Trial 1 should complete");

    const ArchonTrial* trial1 = archon_trial_get(manager, 1);
    ASSERT(trial1 != NULL, "Trial 1 should exist");
    ASSERT(trial1->status == TRIAL_STATUS_PASSED, "Trial 1 should be passed");
    ASSERT(trial1->best_score >= 84.0f && trial1->best_score <= 86.0f,
           "Trial 1 score should be recorded");
    ASSERT(manager->current_trial_id == 0, "No current trial after completion");

    archon_trial_manager_destroy(manager);
    PASS();
}

/* Test 7: Trial failure and retry */
static bool test_trial_fail_retry(void) {
    TEST("Trial failure and retry");

    ArchonTrialManager* manager = archon_trial_manager_create();
    ASSERT(manager != NULL, "Manager creation failed");

    archon_trial_load_from_file(manager,
        "data/trials/archon_trials.dat");

    /* Unlock and start Trial 1 */
    archon_trial_unlock(manager, 1, 45, 60.0f);
    archon_trial_start(manager, 1);

    /* Fail once (should allow retry) */
    bool can_retry = archon_trial_fail(manager, 1);
    ASSERT(can_retry == true, "Should allow retry after first failure");

    const ArchonTrial* trial1 = archon_trial_get(manager, 1);
    ASSERT(trial1 != NULL, "Trial 1 should exist");
    ASSERT(trial1->status == TRIAL_STATUS_AVAILABLE, "Trial 1 should be available for retry");
    ASSERT(trial1->attempts_made == 1, "Should record 1 attempt");

    /* Start and fail again */
    archon_trial_start(manager, 1);
    can_retry = archon_trial_fail(manager, 1);
    ASSERT(can_retry == true, "Should allow retry after second failure");
    ASSERT(trial1->attempts_made == 2, "Should record 2 attempts");

    /* Start and fail third time (max attempts) */
    archon_trial_start(manager, 1);
    can_retry = archon_trial_fail(manager, 1);
    ASSERT(can_retry == false, "Should not allow retry after max attempts");
    ASSERT(trial1->status == TRIAL_STATUS_FAILED, "Trial 1 should be permanently failed");
    ASSERT(manager->path_failed == true, "Path should be marked failed");

    archon_trial_manager_destroy(manager);
    PASS();
}

/* Test 8: Trial sequence (prerequisite chain) */
static bool test_trial_sequence(void) {
    TEST("Trial sequence with prerequisites");

    ArchonTrialManager* manager = archon_trial_manager_create();
    ASSERT(manager != NULL, "Manager creation failed");

    archon_trial_load_from_file(manager,
        "data/trials/archon_trials.dat");

    /* Complete Trial 1 */
    archon_trial_unlock(manager, 1, 45, 60.0f);
    archon_trial_start(manager, 1);
    archon_trial_complete(manager, 1, 90.0f);

    /* Now Trial 2 should be unlockable */
    bool can_unlock = archon_trial_can_unlock(manager, 2, 45, 60.0f);
    ASSERT(can_unlock == true, "Trial 2 should unlock after Trial 1");

    archon_trial_unlock(manager, 2, 45, 60.0f);
    const ArchonTrial* trial2 = archon_trial_get(manager, 2);
    ASSERT(trial2 != NULL, "Trial 2 should exist");
    ASSERT(trial2->status == TRIAL_STATUS_AVAILABLE, "Trial 2 should be available");

    /* Trial 3 should still be locked */
    can_unlock = archon_trial_can_unlock(manager, 3, 45, 60.0f);
    ASSERT(can_unlock == false, "Trial 3 requires Trial 2");

    archon_trial_manager_destroy(manager);
    PASS();
}

/* Test 9: All trials completed */
static bool test_all_trials_completed(void) {
    TEST("All trials completed");

    ArchonTrialManager* manager = archon_trial_manager_create();
    ASSERT(manager != NULL, "Manager creation failed");

    archon_trial_load_from_file(manager,
        "data/trials/archon_trials.dat");

    /* Complete all 7 trials */
    for (uint32_t i = 1; i <= 7; i++) {
        archon_trial_unlock(manager, i, 45, 60.0f);
        archon_trial_start(manager, i);
        archon_trial_complete(manager, i, 80.0f);
    }

    bool all_done = archon_trial_all_completed(manager);
    ASSERT(all_done == true, "All trials should be completed");
    ASSERT(manager->path_completed == true, "Path should be marked completed");

    size_t passed = archon_trial_count_passed(manager);
    ASSERT(passed == 7, "Should have 7 passed trials");

    archon_trial_manager_destroy(manager);
    PASS();
}

/* Test 10: Path activation */
static bool test_path_activation(void) {
    TEST("Archon path activation");

    ArchonTrialManager* manager = archon_trial_manager_create();
    ASSERT(manager != NULL, "Manager creation failed");

    archon_trial_load_from_file(manager,
        "data/trials/archon_trials.dat");

    /* Activate path with valid corruption/consciousness */
    bool activated = archon_trial_activate_path(manager, 45, 60.0f);
    ASSERT(activated == true, "Path should activate");
    ASSERT(manager->path_active == true, "Path should be active");

    /* Trial 1 should be auto-unlocked */
    const ArchonTrial* trial1 = archon_trial_get(manager, 1);
    ASSERT(trial1 != NULL, "Trial 1 should exist");
    ASSERT(trial1->status == TRIAL_STATUS_AVAILABLE, "Trial 1 should be auto-unlocked");

    archon_trial_manager_destroy(manager);
    PASS();
}

/* Test 11: Path availability with corruption changes */
static bool test_path_availability(void) {
    TEST("Path availability with corruption changes");

    ArchonTrialManager* manager = archon_trial_manager_create();
    ASSERT(manager != NULL, "Manager creation failed");

    archon_trial_load_from_file(manager,
        "data/trials/archon_trials.dat");

    /* Activate path */
    archon_trial_activate_path(manager, 45, 60.0f);

    /* Path should be available at 45% corruption */
    bool available = archon_trial_path_available(manager, 45);
    ASSERT(available == true, "Path should be available at 45%");

    /* Path should be unavailable if corruption drops below 30% */
    available = archon_trial_path_available(manager, 25);
    ASSERT(available == false, "Path should be unavailable at 25%");

    /* Path should be unavailable if corruption rises above 60% */
    available = archon_trial_path_available(manager, 65);
    ASSERT(available == false, "Path should be unavailable at 65%");

    archon_trial_manager_destroy(manager);
    PASS();
}

/* Test 12: Power trial combat mechanics */
static bool test_power_trial_combat(void) {
    TEST("Power trial combat mechanics");

    PowerTrialState* state = power_trial_create();
    ASSERT(state != NULL, "Power trial creation failed");
    ASSERT(state->active == false, "Trial not active initially");

    power_trial_start(state);
    ASSERT(state->active == true, "Trial should be active");
    ASSERT(state->seraphim_current_hp == SERAPHIM_MAX_HP, "Seraphim at full HP");

    /* Deal damage to bring Seraphim to ~10% HP */
    uint32_t target_hp = (uint32_t)(SERAPHIM_MAX_HP * 0.10f);
    uint32_t damage = SERAPHIM_MAX_HP - target_hp;
    bool alive = power_trial_damage_seraphim(state, damage);
    ASSERT(alive == true, "Seraphim should be alive");
    ASSERT(state->victory_condition_met == true, "Victory condition should be met");

    /* Player chooses to yield (correct choice) */
    bool success = power_trial_yield(state);
    ASSERT(success == true, "Yield should succeed");
    ASSERT(power_trial_is_passed(state), "Trial should be passed");
    ASSERT(state->outcome == POWER_OUTCOME_YIELD, "Outcome should be yield");

    power_trial_destroy(state);
    PASS();
}

/* Test 13: Power trial kill failure */
static bool test_power_trial_kill_failure(void) {
    TEST("Power trial kill failure");

    PowerTrialState* state = power_trial_create();
    ASSERT(state != NULL, "Power trial creation failed");

    power_trial_start(state);

    /* Deal damage to bring Seraphim to ~10% HP */
    uint32_t target_hp = (uint32_t)(SERAPHIM_MAX_HP * 0.10f);
    uint32_t damage = SERAPHIM_MAX_HP - target_hp;
    power_trial_damage_seraphim(state, damage);

    /* Player chooses to kill (wrong choice) */
    bool success = power_trial_kill(state);
    ASSERT(success == false, "Kill should fail the trial");
    ASSERT(!power_trial_is_passed(state), "Trial should not be passed");
    ASSERT(state->outcome == POWER_OUTCOME_KILL, "Outcome should be kill");

    power_trial_destroy(state);
    PASS();
}

/* Test 14: Power trial score calculation */
static bool test_power_trial_score(void) {
    TEST("Power trial score calculation");

    PowerTrialState* state = power_trial_create();
    ASSERT(state != NULL, "Power trial creation failed");

    power_trial_start(state);

    /* Deal damage to bring Seraphim to ~10% HP (50 HP out of 500) */
    /* Do this in 8 turns for good performance */
    for (int i = 0; i < 7; i++) {
        power_trial_damage_seraphim(state, 55);  /* 7 * 55 = 385 */
    }
    power_trial_damage_seraphim(state, 65); /* 385 + 65 = 450, leaving 50 HP (10%) */
    power_trial_yield(state);

    float score = power_trial_calculate_score(state);
    /* 8 turns is good performance, should be 100 (no penalty until > 10 turns) */
    ASSERT(score >= 99.0f && score <= 100.0f, "Score should be 100 for optimal performance");

    power_trial_destroy(state);
    PASS();
}

/* Main test runner */
int main(void) {
    printf("=== Archon Trial System Tests ===\n\n");

    test_manager_create_destroy();
    test_load_trials_from_file();
    test_trial_unlock_conditions();
    test_trial_unlock();
    test_trial_start();
    test_trial_complete();
    test_trial_fail_retry();
    test_trial_sequence();
    test_all_trials_completed();
    test_path_activation();
    test_path_availability();
    test_power_trial_combat();
    test_power_trial_kill_failure();
    test_power_trial_score();

    printf("\n=== Test Results ===\n");
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

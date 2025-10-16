#include "../src/game/narrative/trials/trial_morality.h"
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
TEST(test_morality_trial_create_destroy) {
    MoralityTrialState* state = morality_trial_create();
    assert(state != NULL);
    assert(state->active == false);
    assert(state->choice_made == MORALITY_CHOICE_NONE);
    assert(state->choice_is_final == false);
    assert(state->population == 100);
    assert(state->children_count == 23);

    morality_trial_destroy(state);
}

/* Test: Start trial */
TEST(test_morality_trial_start) {
    MoralityTrialState* state = morality_trial_create();
    assert(state != NULL);

    bool result = morality_trial_start(state, "nonexistent_file.dat");
    assert(result == true);
    assert(state->active == true);
    assert(strcmp(state->village_name, "Ashford") == 0);

    morality_trial_destroy(state);
}

/* Test: Choose to save innocents (PASS) */
TEST(test_morality_trial_choose_save) {
    MoralityTrialState* state = morality_trial_create();
    morality_trial_start(state, "nonexistent.dat");

    int64_t energy = 10000;
    int64_t mana = 500;
    uint8_t corruption = 45;

    bool result = morality_trial_choose_save(state, &energy, &mana, &corruption);
    assert(result == true);
    assert(energy == 0); /* All energy spent */
    assert(mana == 0); /* All mana spent */
    assert(corruption == 40); /* -5 corruption */
    assert(state->choice_made == MORALITY_CHOICE_SAVE_INNOCENTS);
    assert(state->choice_is_final == true);
    assert(morality_trial_is_passed(state) == true);

    morality_trial_destroy(state);
}

/* Test: Choose to harvest souls (FAIL) */
TEST(test_morality_trial_choose_harvest) {
    MoralityTrialState* state = morality_trial_create();
    morality_trial_start(state, "nonexistent.dat");

    int64_t energy = 10000;
    uint8_t corruption = 45;

    bool result = morality_trial_choose_harvest(state, &energy, &corruption);
    assert(result == false); /* Trial failed */
    assert(energy == 60000); /* Gained 50000 */
    assert(corruption == 70); /* +25 corruption */
    assert(state->choice_made == MORALITY_CHOICE_HARVEST_SOULS);
    assert(state->choice_is_final == true);
    assert(morality_trial_is_passed(state) == false);

    morality_trial_destroy(state);
}

/* Test: Choose to do nothing (FAIL) */
TEST(test_morality_trial_choose_nothing) {
    MoralityTrialState* state = morality_trial_create();
    morality_trial_start(state, "nonexistent.dat");

    bool result = morality_trial_choose_nothing(state);
    assert(result == false); /* Trial failed */
    assert(state->choice_made == MORALITY_CHOICE_DO_NOTHING);
    assert(state->choice_is_final == true);
    assert(morality_trial_is_passed(state) == false);

    morality_trial_destroy(state);
}

/* Test: Cannot change choice after making one */
TEST(test_morality_trial_choice_is_final) {
    MoralityTrialState* state = morality_trial_create();
    morality_trial_start(state, "nonexistent.dat");

    int64_t energy = 10000;
    int64_t mana = 500;
    uint8_t corruption = 45;

    /* Make first choice */
    morality_trial_choose_save(state, &energy, &mana, &corruption);

    /* Try to make another choice */
    int64_t energy2 = 10000;
    uint8_t corruption2 = 45;
    bool result = morality_trial_choose_harvest(state, &energy2, &corruption2);
    assert(result == false);
    assert(energy2 == 10000); /* No change */

    morality_trial_destroy(state);
}

/* Test: Corruption clamping on save */
TEST(test_morality_trial_save_corruption_clamp) {
    MoralityTrialState* state = morality_trial_create();
    morality_trial_start(state, "nonexistent.dat");

    int64_t energy = 10000;
    int64_t mana = 500;
    uint8_t corruption = 3; /* Very low */

    morality_trial_choose_save(state, &energy, &mana, &corruption);
    assert(corruption == 0); /* Clamped to 0, not negative */

    morality_trial_destroy(state);
}

/* Test: Corruption clamping on harvest */
TEST(test_morality_trial_harvest_corruption_clamp) {
    MoralityTrialState* state = morality_trial_create();
    morality_trial_start(state, "nonexistent.dat");

    int64_t energy = 10000;
    uint8_t corruption = 80; /* Very high */

    morality_trial_choose_harvest(state, &energy, &corruption);
    assert(corruption == 100); /* Clamped to 100, not over */

    morality_trial_destroy(state);
}

/* Test: Get village info */
TEST(test_morality_trial_get_village_info) {
    MoralityTrialState* state = morality_trial_create();
    morality_trial_start(state, "nonexistent.dat");

    const char* info = morality_trial_get_village_info(state);
    assert(info != NULL);
    assert(strstr(info, "Ashford") != NULL);
    assert(strstr(info, "100 innocents") != NULL);
    assert(strstr(info, "23 children") != NULL);

    morality_trial_destroy(state);
}

/* Test: Is complete */
TEST(test_morality_trial_is_complete) {
    MoralityTrialState* state = morality_trial_create();
    morality_trial_start(state, "nonexistent.dat");

    assert(morality_trial_is_complete(state) == false);

    morality_trial_choose_nothing(state);

    assert(morality_trial_is_complete(state) == true);

    morality_trial_destroy(state);
}

int main(void) {
    printf("=== Testing Trial 3: Morality ===\n\n");

    run_test_morality_trial_create_destroy();
    run_test_morality_trial_start();
    run_test_morality_trial_choose_save();
    run_test_morality_trial_choose_harvest();
    run_test_morality_trial_choose_nothing();
    run_test_morality_trial_choice_is_final();
    run_test_morality_trial_save_corruption_clamp();
    run_test_morality_trial_harvest_corruption_clamp();
    run_test_morality_trial_get_village_info();
    run_test_morality_trial_is_complete();

    printf("\n=== Test Summary ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    return (tests_run == tests_passed) ? 0 : 1;
}

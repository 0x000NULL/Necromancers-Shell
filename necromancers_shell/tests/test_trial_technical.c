#include "../src/game/narrative/trials/trial_technical.h"
#include <stdio.h>
#include <stdlib.h>
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
TEST(test_technical_trial_create_destroy) {
    TechnicalTrialState* state = technical_trial_create();
    assert(state != NULL);
    assert(state->active == false);
    assert(state->bugs == NULL);
    assert(state->bug_count == 0);
    assert(state->bugs_discovered == 0);
    assert(state->source_code == NULL);

    technical_trial_destroy(state);
}

/* Test: Start trial with data file */
TEST(test_technical_trial_start) {
    TechnicalTrialState* state = technical_trial_create();
    assert(state != NULL);

    const char* test_file = "necromancers_shell/data/trials/trial_technical.dat";
    bool result = technical_trial_start(state, test_file);

    /* May fail if file doesn't exist, that's ok for this test */
    if (result) {
        assert(state->active == true);
        assert(state->source_code != NULL);
        assert(state->code_lines > 0);
        assert(state->bug_count > 0);
    }

    technical_trial_destroy(state);
}

/* Test: Report bug (simulated) */
TEST(test_technical_trial_report_bug) {
    TechnicalTrialState* state = technical_trial_create();

    /* Manually create a bug for testing */
    state->active = true;
    state->bug_count = 1;
    state->bugs = calloc(1, sizeof(DeathNetworkBug));
    assert(state->bugs != NULL);

    state->bugs[0].line_number = 42;
    state->bugs[0].type = BUG_TYPE_LOGIC_ERROR;
    state->bugs[0].discovered = false;
    state->bugs[0].reported_correctly = false;

    /* Report correct bug with correct type */
    bool result = technical_trial_report_bug(state, 42, BUG_TYPE_LOGIC_ERROR);
    assert(result == true);
    assert(state->bugs[0].discovered == true);
    assert(state->bugs[0].reported_correctly == true);
    assert(state->bugs_discovered == 1);
    assert(state->bugs_reported_correctly == 1);

    technical_trial_destroy(state);
}

/* Test: Report bug with wrong type */
TEST(test_technical_trial_report_wrong_type) {
    TechnicalTrialState* state = technical_trial_create();

    state->active = true;
    state->bug_count = 1;
    state->bugs = calloc(1, sizeof(DeathNetworkBug));

    state->bugs[0].line_number = 42;
    state->bugs[0].type = BUG_TYPE_LOGIC_ERROR;
    state->bugs[0].discovered = false;
    state->bugs[0].reported_correctly = false;

    /* Report with wrong type */
    bool result = technical_trial_report_bug(state, 42, BUG_TYPE_INEFFICIENCY);
    assert(result == false);
    assert(state->bugs[0].discovered == true); /* Still marked discovered */
    assert(state->bugs[0].reported_correctly == false);
    assert(state->bugs_discovered == 1);
    assert(state->bugs_reported_correctly == 0);

    technical_trial_destroy(state);
}

/* Test: Report non-existent bug */
TEST(test_technical_trial_report_nonexistent) {
    TechnicalTrialState* state = technical_trial_create();

    state->active = true;
    state->bug_count = 1;
    state->bugs = calloc(1, sizeof(DeathNetworkBug));

    state->bugs[0].line_number = 42;
    state->bugs[0].type = BUG_TYPE_LOGIC_ERROR;

    /* Report bug at wrong line */
    bool result = technical_trial_report_bug(state, 999, BUG_TYPE_LOGIC_ERROR);
    assert(result == false);
    assert(state->bugs_discovered == 0);

    technical_trial_destroy(state);
}

/* Test: Calculate score (perfect) */
TEST(test_technical_trial_score_perfect) {
    TechnicalTrialState* state = technical_trial_create();

    state->active = true;
    state->bug_count = 27;
    state->bugs_discovered = 27;
    state->bugs_reported_correctly = 27;
    state->inspections_made = 30;
    state->hint_shown = false;

    float score = technical_trial_calculate_score(state);
    assert(score == 100.0f);

    technical_trial_destroy(state);
}

/* Test: Calculate score with penalties */
TEST(test_technical_trial_score_with_penalties) {
    TechnicalTrialState* state = technical_trial_create();

    state->active = true;
    state->bug_count = 27;
    state->bugs_discovered = 20;
    state->bugs_reported_correctly = 15;
    state->inspections_made = 60; /* 10 over limit */
    state->hint_shown = true;

    float score = technical_trial_calculate_score(state);
    assert(score > 0.0f && score < 100.0f);

    technical_trial_destroy(state);
}

/* Test: Is passed (20+ bugs) */
TEST(test_technical_trial_is_passed) {
    TechnicalTrialState* state = technical_trial_create();

    state->active = true;
    state->bug_count = 27;
    state->bugs_discovered = 15;

    assert(technical_trial_is_passed(state) == false);

    state->bugs_discovered = 20;
    assert(technical_trial_is_passed(state) == true);

    state->bugs_discovered = 25;
    assert(technical_trial_is_passed(state) == true);

    technical_trial_destroy(state);
}

/* Test: Is complete */
TEST(test_technical_trial_is_complete) {
    TechnicalTrialState* state = technical_trial_create();

    state->active = true;
    state->bug_count = 27;
    state->bugs_discovered = 10;

    assert(technical_trial_is_complete(state) == false);

    state->bugs_discovered = 20;
    assert(technical_trial_is_complete(state) == true);

    state->bugs_discovered = 27;
    assert(technical_trial_is_complete(state) == true);

    technical_trial_destroy(state);
}

/* Test: Bug type names */
TEST(test_technical_trial_bug_type_names) {
    assert(strcmp(technical_trial_bug_type_name(BUG_TYPE_NONE), "None") == 0);
    assert(strcmp(technical_trial_bug_type_name(BUG_TYPE_LOGIC_ERROR), "Logic Error") == 0);
    assert(strcmp(technical_trial_bug_type_name(BUG_TYPE_INEFFICIENCY), "Inefficiency") == 0);
    assert(strcmp(technical_trial_bug_type_name(BUG_TYPE_INJUSTICE), "Injustice") == 0);
}

/* Test: Bug severity names */
TEST(test_technical_trial_bug_severity_names) {
    assert(strcmp(technical_trial_bug_severity_name(BUG_SEVERITY_LOW), "Low") == 0);
    assert(strcmp(technical_trial_bug_severity_name(BUG_SEVERITY_MEDIUM), "Medium") == 0);
    assert(strcmp(technical_trial_bug_severity_name(BUG_SEVERITY_HIGH), "High") == 0);
    assert(strcmp(technical_trial_bug_severity_name(BUG_SEVERITY_CRITICAL), "Critical") == 0);
}

/* Test: Get stats */
TEST(test_technical_trial_get_stats) {
    TechnicalTrialState* state = technical_trial_create();

    state->active = true;
    state->bug_count = 27;
    state->bugs_discovered = 20;
    state->bugs_reported_correctly = 18;
    state->inspections_made = 45;

    const char* stats = technical_trial_get_stats(state);
    assert(stats != NULL);
    assert(strstr(stats, "20 / 27") != NULL);

    technical_trial_destroy(state);
}

int main(void) {
    printf("=== Testing Trial 4: Technical ===\n\n");

    run_test_technical_trial_create_destroy();
    run_test_technical_trial_start();
    run_test_technical_trial_report_bug();
    run_test_technical_trial_report_wrong_type();
    run_test_technical_trial_report_nonexistent();
    run_test_technical_trial_score_perfect();
    run_test_technical_trial_score_with_penalties();
    run_test_technical_trial_is_passed();
    run_test_technical_trial_is_complete();
    run_test_technical_trial_bug_type_names();
    run_test_technical_trial_bug_severity_names();
    run_test_technical_trial_get_stats();

    printf("\n=== Test Summary ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    return (tests_run == tests_passed) ? 0 : 1;
}

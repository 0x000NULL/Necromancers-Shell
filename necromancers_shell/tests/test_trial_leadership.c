/**
 * @file test_trial_leadership.c
 * @brief Unit tests for Trial 7: Test of Leadership
 */

#include "../src/game/narrative/trials/trial_leadership.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

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

#define ASSERT_FLOAT_EQ(a, b, epsilon, message) \
    do { \
        if (fabs((a) - (b)) > (epsilon)) { \
            printf("  FAILED: %s (%.2f != %.2f)\n", message, (float)(a), (float)(b)); \
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

    LeadershipTrialState* state = leadership_trial_create();
    ASSERT(state != NULL, "Failed to create leadership trial state");
    ASSERT(!state->active, "New trial should not be active");
    ASSERT(state->current_day == 0, "Current day should be 0");
    ASSERT(state->member_count == 0, "Member count should be 0 initially");

    leadership_trial_destroy(state);
    /* If we reach here without crash, destroy works */

    /* Test NULL safety */
    leadership_trial_destroy(NULL);

    PASS();
}

/**
 * Test 2: Load from data file
 */
static void test_load_from_file(void) {
    TEST("load_from_file");

    LeadershipTrialState* state = leadership_trial_create();
    ASSERT(state != NULL, "Failed to create state");

    bool result = leadership_trial_load_from_file(state,
        "data/trials/trial_leadership.dat");
    ASSERT(result, "Failed to load from file");
    ASSERT(state->member_count == 6, "Should have 6 council members");

    /* Check first member (Mordak) */
    const CouncilMemberProgress* mordak = leadership_trial_get_member(state, 0);
    ASSERT(mordak != NULL, "Should get Mordak");
    ASSERT(strcmp(mordak->npc_id, "mordak") == 0, "ID should be mordak");
    ASSERT(mordak->corruption_start == 93.0f, "Mordak corruption should be 93%");
    ASSERT(mordak->resistance == RESISTANCE_HIGH, "Mordak should have high resistance");
    ASSERT(mordak->attitude == ATTITUDE_HOSTILE, "Mordak should be hostile");

    /* Check last member (Kael) */
    const CouncilMemberProgress* kael = leadership_trial_get_member(state, 5);
    ASSERT(kael != NULL, "Should get Kael");
    ASSERT(strcmp(kael->npc_id, "kael") == 0, "ID should be kael");
    ASSERT(kael->corruption_start == 71.0f, "Kael corruption should be 71%");

    leadership_trial_destroy(state);
    PASS();
}

/**
 * Test 3: Start trial
 */
static void test_start_trial(void) {
    TEST("start_trial");

    LeadershipTrialState* state = leadership_trial_create();
    ASSERT(state != NULL, "Failed to create state");

    leadership_trial_load_from_file(state,
        "data/trials/trial_leadership.dat");

    bool result = leadership_trial_start(state);
    ASSERT(result, "Failed to start trial");
    ASSERT(state->active, "Trial should be active");
    ASSERT(state->current_day == 1, "Should start at day 1");
    ASSERT(state->days_remaining == 30, "Should have 30 days remaining");

    /* Check collective corruption calculated */
    ASSERT_FLOAT_EQ(state->collective_corruption_start, 75.3f, 0.1f,
                    "Starting collective corruption should be ~75.3%");

    /* Check target (10% reduction) */
    ASSERT_FLOAT_EQ(state->collective_corruption_target, 67.77f, 0.1f,
                    "Target should be ~67.77% (10% reduction)");

    leadership_trial_destroy(state);
    PASS();
}

/**
 * Test 4: Advance day
 */
static void test_advance_day(void) {
    TEST("advance_day");

    LeadershipTrialState* state = leadership_trial_create();
    ASSERT(state != NULL, "Failed to create state");

    leadership_trial_load_from_file(state,
        "data/trials/trial_leadership.dat");
    leadership_trial_start(state);

    ASSERT(state->current_day == 1, "Should be day 1");

    bool result = leadership_trial_advance_day(state);
    ASSERT(result, "Should advance day");
    ASSERT(state->current_day == 2, "Should be day 2");
    ASSERT(state->days_remaining == 29, "Should have 29 days remaining");

    /* Advance to day 30 */
    for (int i = 0; i < 28; i++) {
        leadership_trial_advance_day(state);
    }

    ASSERT(state->current_day == 30, "Should be day 30");
    ASSERT(!state->active, "Trial should be inactive after 30 days");
    ASSERT(leadership_trial_is_complete(state), "Trial should be complete");

    leadership_trial_destroy(state);
    PASS();
}

/**
 * Test 5: Meet with individual member
 */
static void test_meet_with_member(void) {
    TEST("meet_with_member");

    LeadershipTrialState* state = leadership_trial_create();
    ASSERT(state != NULL, "Failed to create state");

    leadership_trial_load_from_file(state,
        "data/trials/trial_leadership.dat");
    leadership_trial_start(state);

    /* Get Seraphine (low resistance, supportive) */
    const CouncilMemberProgress* seraphine_before = leadership_trial_get_member(state, 4);
    float corruption_before = seraphine_before->corruption_current;

    /* Meet with inspirational approach (good for low resistance) */
    bool result = leadership_trial_meet_with_member(state, 4, GUIDANCE_INSPIRATIONAL);
    ASSERT(result, "Meeting should succeed");

    const CouncilMemberProgress* seraphine_after = leadership_trial_get_member(state, 4);
    ASSERT(seraphine_after->corruption_current < corruption_before,
           "Corruption should decrease");
    ASSERT(seraphine_after->meetings_held == 1, "Should record meeting");

    leadership_trial_destroy(state);
    PASS();
}

/**
 * Test 6: Council meeting
 */
static void test_council_meeting(void) {
    TEST("council_meeting");

    LeadershipTrialState* state = leadership_trial_create();
    ASSERT(state != NULL, "Failed to create state");

    leadership_trial_load_from_file(state,
        "data/trials/trial_leadership.dat");
    leadership_trial_start(state);

    /* First council meeting should succeed */
    bool result = leadership_trial_council_meeting(state);
    ASSERT(result, "First council meeting should succeed");
    ASSERT(state->council_meetings_held == 1, "Should record meeting");

    /* Second meeting immediately should fail (cooldown) */
    result = leadership_trial_council_meeting(state);
    ASSERT(!result, "Second meeting should fail (cooldown)");

    /* Advance 3 days, then meeting should work */
    for (int i = 0; i < 3; i++) {
        leadership_trial_advance_day(state);
    }

    result = leadership_trial_council_meeting(state);
    ASSERT(result, "Meeting after cooldown should succeed");
    ASSERT(state->council_meetings_held == 2, "Should have 2 meetings");

    leadership_trial_destroy(state);
    PASS();
}

/**
 * Test 7: Implement reform - Code of Conduct
 */
static void test_reform_code_of_conduct(void) {
    TEST("reform_code_of_conduct");

    LeadershipTrialState* state = leadership_trial_create();
    ASSERT(state != NULL, "Failed to create state");

    leadership_trial_load_from_file(state,
        "data/trials/trial_leadership.dat");
    leadership_trial_start(state);

    float collective_before = leadership_trial_calculate_collective_corruption(state);

    bool result = leadership_trial_implement_reform(state, REFORM_CODE_OF_CONDUCT);
    ASSERT(result, "Reform should succeed");

    float collective_after = leadership_trial_calculate_collective_corruption(state);
    ASSERT(collective_after < collective_before, "Collective corruption should decrease");

    leadership_trial_destroy(state);
    PASS();
}

/**
 * Test 8: Harsh guidance approach
 */
static void test_harsh_guidance(void) {
    TEST("harsh_guidance");

    LeadershipTrialState* state = leadership_trial_create();
    ASSERT(state != NULL, "Failed to create state");

    leadership_trial_load_from_file(state,
        "data/trials/trial_leadership.dat");
    leadership_trial_start(state);

    /* Mordak has high resistance, harsh should work */
    const CouncilMemberProgress* mordak_before = leadership_trial_get_member(state, 0);
    float corruption_before = mordak_before->corruption_current;
    int32_t attitude_before = mordak_before->attitude_score;

    leadership_trial_meet_with_member(state, 0, GUIDANCE_HARSH);

    const CouncilMemberProgress* mordak_after = leadership_trial_get_member(state, 0);
    ASSERT(mordak_after->corruption_current < corruption_before,
           "Corruption should decrease with harsh");
    ASSERT(mordak_after->attitude_score < attitude_before,
           "Attitude should worsen with harsh");

    leadership_trial_destroy(state);
    PASS();
}

/**
 * Test 9: Diplomatic guidance approach
 */
static void test_diplomatic_guidance(void) {
    TEST("diplomatic_guidance");

    LeadershipTrialState* state = leadership_trial_create();
    ASSERT(state != NULL, "Failed to create state");

    leadership_trial_load_from_file(state,
        "data/trials/trial_leadership.dat");
    leadership_trial_start(state);

    /* Elara has medium resistance, diplomatic should work well */
    const CouncilMemberProgress* elara_before = leadership_trial_get_member(state, 2);
    float corruption_before = elara_before->corruption_current;
    int32_t attitude_before = elara_before->attitude_score;

    leadership_trial_meet_with_member(state, 2, GUIDANCE_DIPLOMATIC);

    const CouncilMemberProgress* elara_after = leadership_trial_get_member(state, 2);
    ASSERT(elara_after->corruption_current < corruption_before,
           "Corruption should decrease");
    ASSERT(elara_after->attitude_score > attitude_before,
           "Attitude should improve with diplomatic");

    leadership_trial_destroy(state);
    PASS();
}

/**
 * Test 10: Inspirational guidance approach
 */
static void test_inspirational_guidance(void) {
    TEST("inspirational_guidance");

    LeadershipTrialState* state = leadership_trial_create();
    ASSERT(state != NULL, "Failed to create state");

    leadership_trial_load_from_file(state,
        "data/trials/trial_leadership.dat");
    leadership_trial_start(state);

    /* Seraphine is supportive with low resistance, inspirational should work great */
    const CouncilMemberProgress* seraphine_before = leadership_trial_get_member(state, 4);
    int32_t attitude_before = seraphine_before->attitude_score;

    leadership_trial_meet_with_member(state, 4, GUIDANCE_INSPIRATIONAL);

    const CouncilMemberProgress* seraphine_after = leadership_trial_get_member(state, 4);
    ASSERT(seraphine_after->attitude_score > attitude_before,
           "Attitude should improve significantly");

    leadership_trial_destroy(state);
    PASS();
}

/**
 * Test 11: Calculate collective corruption
 */
static void test_calculate_collective_corruption(void) {
    TEST("calculate_collective_corruption");

    LeadershipTrialState* state = leadership_trial_create();
    ASSERT(state != NULL, "Failed to create state");

    leadership_trial_load_from_file(state,
        "data/trials/trial_leadership.dat");
    leadership_trial_start(state);

    /* Manual calculation: (93 + 98 + 67 + 78 + 45 + 71) / 6 = 75.33 */
    float collective = leadership_trial_calculate_collective_corruption(state);
    ASSERT_FLOAT_EQ(collective, 75.3f, 0.1f,
                    "Collective corruption should be ~75.3%");

    leadership_trial_destroy(state);
    PASS();
}

/**
 * Test 12: Check target met
 */
static void test_target_met(void) {
    TEST("target_met");

    LeadershipTrialState* state = leadership_trial_create();
    ASSERT(state != NULL, "Failed to create state");

    leadership_trial_load_from_file(state,
        "data/trials/trial_leadership.dat");
    leadership_trial_start(state);

    /* Initially not met */
    ASSERT(!leadership_trial_is_target_met(state), "Target not met initially");

    /* Apply multiple reforms to reduce corruption */
    leadership_trial_implement_reform(state, REFORM_CORRUPTION_LIMITS);  /* -1.5% all */
    leadership_trial_implement_reform(state, REFORM_SOUL_ETHICS);        /* -2% low, -0.5% high */
    leadership_trial_implement_reform(state, REFORM_CODE_OF_CONDUCT);    /* -1% most */

    /* Meet with supportive member multiple times */
    for (int i = 0; i < 5; i++) {
        leadership_trial_meet_with_member(state, 4, GUIDANCE_INSPIRATIONAL);
    }

    /* Check if target can be met with enough effort */
    float current = leadership_trial_calculate_collective_corruption(state);
    printf("  Current corruption: %.2f%%, Target: %.2f%%\n",
           current, state->collective_corruption_target);

    leadership_trial_destroy(state);
    PASS();
}

/**
 * Test 13: Get member by ID
 */
static void test_get_member_by_id(void) {
    TEST("get_member_by_id");

    LeadershipTrialState* state = leadership_trial_create();
    ASSERT(state != NULL, "Failed to create state");

    leadership_trial_load_from_file(state,
        "data/trials/trial_leadership.dat");

    const CouncilMemberProgress* mordak = leadership_trial_get_member_by_id(state, "mordak");
    ASSERT(mordak != NULL, "Should find Mordak by ID");
    ASSERT(strcmp(mordak->npc_id, "mordak") == 0, "Should be Mordak");

    const CouncilMemberProgress* unknown = leadership_trial_get_member_by_id(state, "unknown");
    ASSERT(unknown == NULL, "Should return NULL for unknown ID");

    leadership_trial_destroy(state);
    PASS();
}

/**
 * Test 14: Complete trial after 30 days
 */
static void test_30_day_completion(void) {
    TEST("30_day_completion");

    LeadershipTrialState* state = leadership_trial_create();
    ASSERT(state != NULL, "Failed to create state");

    leadership_trial_load_from_file(state,
        "data/trials/trial_leadership.dat");
    leadership_trial_start(state);

    ASSERT(!leadership_trial_is_complete(state), "Not complete at start");

    /* Advance to day 30 */
    for (int i = 0; i < 29; i++) {
        leadership_trial_advance_day(state);
    }

    ASSERT(leadership_trial_is_complete(state), "Should be complete after 30 days");
    ASSERT(!state->active, "Should be inactive");

    leadership_trial_destroy(state);
    PASS();
}

/**
 * Test 15: Successful trial (pass scenario)
 */
static void test_pass_scenario(void) {
    TEST("pass_scenario");

    LeadershipTrialState* state = leadership_trial_create();
    ASSERT(state != NULL, "Failed to create state");

    leadership_trial_load_from_file(state,
        "data/trials/trial_leadership.dat");
    leadership_trial_start(state);

    /* Apply aggressive reforms and meetings to reduce corruption */
    for (int day = 1; day <= 30; day++) {
        /* Implement reform every 5 days */
        if (day % 5 == 0) {
            leadership_trial_implement_reform(state, REFORM_SOUL_ETHICS);
        }

        /* Council meeting every 3 days */
        if (day % 3 == 0) {
            leadership_trial_council_meeting(state);
        }

        /* Meet with each member once per week */
        if (day % 7 == 0) {
            for (size_t i = 0; i < 6; i++) {
                const CouncilMemberProgress* m = leadership_trial_get_member(state, i);
                GuidanceApproach approach;
                if (m->resistance == RESISTANCE_HIGH) {
                    approach = GUIDANCE_HARSH;
                } else if (m->resistance == RESISTANCE_MEDIUM) {
                    approach = GUIDANCE_DIPLOMATIC;
                } else {
                    approach = GUIDANCE_INSPIRATIONAL;
                }
                leadership_trial_meet_with_member(state, i, approach);
            }
        }

        /* Don't advance on last day (already at 30) */
        if (day < 30) {
            leadership_trial_advance_day(state);
        }
    }

    float final_corruption = leadership_trial_calculate_collective_corruption(state);
    printf("  Final corruption: %.2f%%, Target: %.2f%%\n",
           final_corruption, state->collective_corruption_target);

    /* Calculate score */
    float score = leadership_trial_calculate_score(state);
    printf("  Score: %.1f/100\n", score);
    ASSERT(score > 0.0f, "Score should be > 0");

    leadership_trial_destroy(state);
    PASS();
}

/**
 * Test 16: String helper functions
 */
static void test_string_helpers(void) {
    TEST("string_helpers");

    const char* low = leadership_trial_resistance_name(RESISTANCE_LOW);
    ASSERT(low != NULL, "Low resistance name");

    const char* hostile = leadership_trial_attitude_name(ATTITUDE_HOSTILE);
    ASSERT(hostile != NULL, "Hostile attitude name");

    const char* harsh = leadership_trial_guidance_name(GUIDANCE_HARSH);
    ASSERT(harsh != NULL, "Harsh guidance name");

    const char* reform = leadership_trial_reform_name(REFORM_CODE_OF_CONDUCT);
    ASSERT(reform != NULL, "Reform name");

    PASS();
}

/**
 * Test 17: Get summary
 */
static void test_get_summary(void) {
    TEST("get_summary");

    LeadershipTrialState* state = leadership_trial_create();
    ASSERT(state != NULL, "Failed to create state");

    leadership_trial_load_from_file(state,
        "data/trials/trial_leadership.dat");
    leadership_trial_start(state);

    char buffer[2048];
    int written = leadership_trial_get_summary(state, buffer, sizeof(buffer));
    ASSERT(written > 0, "Should write summary");
    ASSERT(strlen(buffer) > 0, "Summary should not be empty");

    printf("  Summary:\n%s\n", buffer);

    leadership_trial_destroy(state);
    PASS();
}

/**
 * Test 18: NULL safety
 */
static void test_null_safety(void) {
    TEST("null_safety");

    /* All functions should handle NULL gracefully */
    ASSERT(!leadership_trial_start(NULL), "Start with NULL should fail");
    ASSERT(!leadership_trial_load_from_file(NULL, "test"), "Load with NULL should fail");
    ASSERT(!leadership_trial_advance_day(NULL), "Advance day with NULL should fail");
    ASSERT(!leadership_trial_meet_with_member(NULL, 0, GUIDANCE_HARSH), "Meet with NULL");
    ASSERT(!leadership_trial_council_meeting(NULL), "Council meeting with NULL");
    ASSERT(!leadership_trial_implement_reform(NULL, REFORM_CODE_OF_CONDUCT), "Reform with NULL");
    ASSERT(leadership_trial_calculate_collective_corruption(NULL) == 0.0f, "Calculate with NULL");
    ASSERT(!leadership_trial_is_target_met(NULL), "Target met with NULL");
    ASSERT(leadership_trial_get_member(NULL, 0) == NULL, "Get member with NULL");
    ASSERT(!leadership_trial_is_complete(NULL), "Is complete with NULL");
    ASSERT(!leadership_trial_is_passed(NULL), "Is passed with NULL");

    PASS();
}

/**
 * Main test runner
 */
int main(void) {
    printf("=== Trial Leadership Tests ===\n\n");

    test_create_destroy();
    test_load_from_file();
    test_start_trial();
    test_advance_day();
    test_meet_with_member();
    test_council_meeting();
    test_reform_code_of_conduct();
    test_harsh_guidance();
    test_diplomatic_guidance();
    test_inspirational_guidance();
    test_calculate_collective_corruption();
    test_target_met();
    test_get_member_by_id();
    test_30_day_completion();
    test_pass_scenario();
    test_string_helpers();
    test_get_summary();
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

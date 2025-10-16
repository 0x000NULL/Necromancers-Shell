/**
 * test_divine_judgment.c - Unit tests for Divine Council Judgment System
 */

#include "../src/game/narrative/divine_judgment.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Test counter */
static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) \
    do { \
        printf("Running test: %s...", #name); \
        tests_run++; \
    } while(0)

#define PASS() \
    do { \
        printf(" PASSED\n"); \
        tests_passed++; \
    } while(0)

/* Test: Create and destroy divine judgment */
void test_create_destroy(void) {
    TEST(test_create_destroy);

    DivineJudgmentState* state = divine_judgment_create();
    assert(state != NULL);
    assert(state->phase == JUDGMENT_NOT_STARTED);
    assert(state->amnesty_granted == false);
    assert(state->restriction_count == 0);

    divine_judgment_destroy(state);
    PASS();
}

/* Test: Summon before Divine Council */
void test_summon(void) {
    TEST(test_summon);

    DivineJudgmentState* state = divine_judgment_create();
    int trial_scores[7] = {80, 85, 90, 85, 80, 75, 85};

    bool result = divine_judgment_summon(state, 45, trial_scores, true, 10, true);
    assert(result == true);
    assert(state->phase == JUDGMENT_SUMMONED);
    assert(state->player_corruption == 45);
    assert(state->all_trials_passed == true);

    divine_judgment_destroy(state);
    PASS();
}

/* Test: Calculate god votes - Thalor (Justice) */
void test_thalor_vote(void) {
    TEST(test_thalor_vote);

    DivineJudgmentState* state = divine_judgment_create();
    int trial_scores[7] = {80, 80, 80, 80, 80, 80, 80};

    /* Balanced corruption (45%), all trials passed - should approve */
    divine_judgment_summon(state, 45, trial_scores, true, 10, true);
    GodVote vote = divine_judgment_calculate_god_vote(state, GOD_THALOR);
    assert(vote == VOTE_APPROVE);

    /* High corruption (75%) - should deny */
    divine_judgment_summon(state, 75, trial_scores, true, 10, true);
    vote = divine_judgment_calculate_god_vote(state, GOD_THALOR);
    assert(vote == VOTE_DENY);

    divine_judgment_destroy(state);
    PASS();
}

/* Test: Calculate god votes - Anara (Empathy) */
void test_anara_vote(void) {
    TEST(test_anara_vote);

    DivineJudgmentState* state = divine_judgment_create();
    int trial_scores[7] = {80, 80, 80, 80, 80, 80, 80};

    /* Maya saved - should approve */
    divine_judgment_summon(state, 45, trial_scores, true, 5, true);
    GodVote vote = divine_judgment_calculate_god_vote(state, GOD_ANARA);
    assert(vote == VOTE_APPROVE);

    /* Maya not saved - should deny */
    divine_judgment_summon(state, 45, trial_scores, false, 5, true);
    vote = divine_judgment_calculate_god_vote(state, GOD_ANARA);
    assert(vote == VOTE_DENY);

    divine_judgment_destroy(state);
    PASS();
}

/* Test: Calculate god votes - Zymira (Knowledge) */
void test_zymira_vote(void) {
    TEST(test_zymira_vote);

    DivineJudgmentState* state = divine_judgment_create();
    int trial_scores[7] = {80, 80, 80, 85, 80, 80, 80};

    /* High Trial 4 score (85) - should approve */
    divine_judgment_summon(state, 45, trial_scores, true, 10, true);
    GodVote vote = divine_judgment_calculate_god_vote(state, GOD_ZYMIRA);
    assert(vote == VOTE_APPROVE);

    /* Low Trial 4 score (40) - should deny */
    trial_scores[3] = 40;
    divine_judgment_summon(state, 45, trial_scores, true, 10, true);
    vote = divine_judgment_calculate_god_vote(state, GOD_ZYMIRA);
    assert(vote == VOTE_DENY);

    divine_judgment_destroy(state);
    PASS();
}

/* Test: Calculate god votes - Kaelthas (Balance) */
void test_kaelthas_vote(void) {
    TEST(test_kaelthas_vote);

    DivineJudgmentState* state = divine_judgment_create();
    int trial_scores[7] = {80, 80, 80, 80, 80, 80, 80};

    /* Perfect balance (45%) - should approve */
    divine_judgment_summon(state, 45, trial_scores, true, 10, true);
    GodVote vote = divine_judgment_calculate_god_vote(state, GOD_KAELTHAS);
    assert(vote == VOTE_APPROVE);

    /* Too high corruption (70%) - should deny */
    divine_judgment_summon(state, 70, trial_scores, true, 10, true);
    vote = divine_judgment_calculate_god_vote(state, GOD_KAELTHAS);
    assert(vote == VOTE_DENY);

    divine_judgment_destroy(state);
    PASS();
}

/* Test: Conduct full judgment */
void test_conduct_judgment(void) {
    TEST(test_conduct_judgment);

    DivineJudgmentState* state = divine_judgment_create();
    int trial_scores[7] = {80, 80, 80, 85, 80, 80, 85};

    /* Set up for likely approval */
    divine_judgment_summon(state, 45, trial_scores, true, 5, true);

    bool result = divine_judgment_conduct(state);
    assert(result == true);
    assert(state->phase == JUDGMENT_VERDICT_DELIVERED);

    /* Check vote tallies */
    int total_votes = state->approve_count + state->deny_count + state->abstain_count;
    assert(total_votes == GOD_COUNT);

    divine_judgment_destroy(state);
    PASS();
}

/* Test: Amnesty granted with 4+ approvals */
void test_amnesty_granted(void) {
    TEST(test_amnesty_granted);

    DivineJudgmentState* state = divine_judgment_create();
    int trial_scores[7] = {85, 85, 85, 90, 85, 85, 90};

    /* Perfect performance should get amnesty */
    divine_judgment_summon(state, 45, trial_scores, true, 0, true);
    divine_judgment_conduct(state);

    assert(state->approve_count >= 4);
    assert(divine_judgment_is_amnesty_granted(state) == true);

    divine_judgment_destroy(state);
    PASS();
}

/* Test: Amnesty denied with < 4 approvals */
void test_amnesty_denied(void) {
    TEST(test_amnesty_denied);

    DivineJudgmentState* state = divine_judgment_create();
    int trial_scores[7] = {50, 50, 50, 40, 50, 50, 50};

    /* Poor performance should deny amnesty */
    divine_judgment_summon(state, 80, trial_scores, false, 100, false);
    divine_judgment_conduct(state);

    assert(state->approve_count < 4);
    assert(divine_judgment_is_amnesty_granted(state) == false);

    divine_judgment_destroy(state);
    PASS();
}

/* Test: Generate restrictions for high corruption */
void test_restrictions_high_corruption(void) {
    TEST(test_restrictions_high_corruption);

    DivineJudgmentState* state = divine_judgment_create();
    state->player_corruption = 55;
    state->votes[GOD_VORATH] = VOTE_APPROVE;

    divine_judgment_generate_restrictions(state);

    assert(state->restriction_count > 0);
    assert(state->restriction_count <= MAX_RESTRICTIONS);

    /* Should have corruption limit */
    bool has_corruption_limit = false;
    for (size_t i = 0; i < state->restriction_count; i++) {
        if (strstr(state->restrictions[i], "65%")) {
            has_corruption_limit = true;
            break;
        }
    }
    assert(has_corruption_limit == true);

    divine_judgment_destroy(state);
    PASS();
}

/* Test: Verdict text generation */
void test_verdict_text(void) {
    TEST(test_verdict_text);

    DivineJudgmentState* state = divine_judgment_create();
    int trial_scores[7] = {80, 80, 80, 80, 80, 80, 80};

    divine_judgment_summon(state, 45, trial_scores, true, 10, true);
    divine_judgment_conduct(state);

    const char* verdict = divine_judgment_get_verdict_text(state);
    assert(verdict != NULL);
    assert(strlen(verdict) > 0);

    /* Should contain god names (updated to match story.md canon) */
    assert(strstr(verdict, "Keldrin") != NULL);
    assert(strstr(verdict, "Anara") != NULL);

    divine_judgment_destroy(state);
    PASS();
}

/* Test: God name and aspect retrieval */
void test_god_info(void) {
    TEST(test_god_info);

    assert(strcmp(divine_judgment_get_god_name(GOD_THALOR), "Keldrin") == 0);
    assert(strcmp(divine_judgment_get_god_aspect(GOD_THALOR), "Law") == 0);

    assert(strcmp(divine_judgment_get_god_name(GOD_ANARA), "Anara") == 0);
    assert(strcmp(divine_judgment_get_god_aspect(GOD_ANARA), "Empathy") == 0);

    PASS();
}

/* Test: Vote to string conversion */
void test_vote_to_string(void) {
    TEST(test_vote_to_string);

    assert(strcmp(divine_judgment_vote_to_string(VOTE_APPROVE), "APPROVE") == 0);
    assert(strcmp(divine_judgment_vote_to_string(VOTE_DENY), "DENY") == 0);
    assert(strcmp(divine_judgment_vote_to_string(VOTE_ABSTAIN), "ABSTAIN") == 0);

    PASS();
}

int main(void) {
    printf("Running Divine Judgment System tests...\n\n");

    test_create_destroy();
    test_summon();
    test_thalor_vote();
    test_anara_vote();
    test_zymira_vote();
    test_kaelthas_vote();
    test_conduct_judgment();
    test_amnesty_granted();
    test_amnesty_denied();
    test_restrictions_high_corruption();
    test_verdict_text();
    test_god_info();
    test_vote_to_string();

    printf("\n========================================\n");
    printf("Tests passed: %d/%d\n", tests_passed, tests_run);
    printf("========================================\n");

    return (tests_passed == tests_run) ? 0 : 1;
}

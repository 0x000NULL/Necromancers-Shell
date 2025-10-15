#include "../src/game/narrative/gods/god.h"
#include "../src/game/narrative/gods/divine_council.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

/* Test color codes */
#define COLOR_GREEN "\033[32m"
#define COLOR_RED "\033[31m"
#define COLOR_RESET "\033[0m"

/* Test counter */
static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) \
    do { \
        tests_run++; \
        printf("  Testing %s... ", name); \
        fflush(stdout);

#define PASS() \
        tests_passed++; \
        printf(COLOR_GREEN "✓ PASSED" COLOR_RESET "\n"); \
    } while(0)

/* Test: Create and destroy god */
static void test_god_create_destroy(void) {
    TEST("god_create_destroy");

    God* god = god_create("keldrin", "Keldrin, the Lawgiver", DOMAIN_ORDER);
    assert(god != NULL);
    assert(strcmp(god->id, "keldrin") == 0);
    assert(strcmp(god->name, "Keldrin, the Lawgiver") == 0);
    assert(god->domain == DOMAIN_ORDER);
    assert(god->power_level == POWER_DIVINE_ARCHITECT);
    assert(god->favor == 0);
    assert(god->favor_min == -100);
    assert(god->favor_max == 100);
    assert(god->summoned == false);
    assert(god->judgment_given == false);
    assert(god->amnesty_granted == false);
    assert(god->condemned == false);

    god_destroy(god);
    PASS();
}

/* Test: Modify favor */
static void test_god_modify_favor(void) {
    TEST("god_modify_favor");

    God* god = god_create("anara", "Anara, the Weaver", DOMAIN_LIFE);
    assert(god != NULL);

    /* Increase favor */
    god_modify_favor(god, 50);
    assert(god->favor == 50);

    /* Decrease favor */
    god_modify_favor(god, -30);
    assert(god->favor == 20);

    /* Clamp at maximum */
    god_modify_favor(god, 100);
    assert(god->favor == 100);

    /* Clamp at minimum */
    god_modify_favor(god, -250);
    assert(god->favor == -100);

    god_destroy(god);
    PASS();
}

/* Test: Set favor */
static void test_god_set_favor(void) {
    TEST("god_set_favor");

    God* god = god_create("myrith", "Myrith, the Dreamer", DOMAIN_SOULS);
    assert(god != NULL);

    god_set_favor(god, 75);
    assert(god->favor == 75);

    god_set_favor(god, -45);
    assert(god->favor == -45);

    /* Clamp at limits */
    god_set_favor(god, 150);
    assert(god->favor == 100);

    god_set_favor(god, -150);
    assert(god->favor == -100);

    god_destroy(god);
    PASS();
}

/* Test: Add dialogue trees */
static void test_god_add_dialogue_tree(void) {
    TEST("god_add_dialogue_tree");

    God* god = god_create("theros", "Theros, the Eternal", DOMAIN_TIME);
    assert(god != NULL);

    assert(god_add_dialogue_tree(god, "theros_prophecy") == true);
    assert(god->dialogue_tree_count == 1);
    assert(strcmp(god->dialogue_trees[0], "theros_prophecy") == 0);

    assert(god_add_dialogue_tree(god, "theros_judgment") == true);
    assert(god->dialogue_tree_count == 2);

    god_destroy(god);
    PASS();
}

/* Test: Add trials */
static void test_god_add_trial(void) {
    TEST("god_add_trial");

    God* god = god_create("keldrin", "Keldrin, the Lawgiver", DOMAIN_ORDER);
    assert(god != NULL);

    assert(god_add_trial(god, "trial_power") == true);
    assert(god->trial_count == 1);
    assert(strcmp(god->trials[0], "trial_power") == 0);

    god_destroy(god);
    PASS();
}

/* Test: Add restrictions */
static void test_god_add_restriction(void) {
    TEST("god_add_restriction");

    God* god = god_create("seraph", "Seraph, the Guardian", DOMAIN_BOUNDARIES);
    assert(god != NULL);

    assert(god_add_restriction(god, "No mass harvesting") == true);
    assert(god->restriction_count == 1);
    assert(strcmp(god->restrictions[0], "No mass harvesting") == 0);

    assert(god_add_restriction(god, "No child targets") == true);
    assert(god->restriction_count == 2);

    god_destroy(god);
    PASS();
}

/* Test: Grant amnesty */
static void test_god_grant_amnesty(void) {
    TEST("god_grant_amnesty");

    God* god = god_create("anara", "Anara, the Weaver", DOMAIN_LIFE);
    assert(god != NULL);

    int16_t initial_favor = god->favor;

    assert(god_grant_amnesty(god) == true);
    assert(god->amnesty_granted == true);
    assert(god->condemned == false);
    assert(god->favor > initial_favor); /* Favor should increase */

    /* Cannot grant again */
    assert(god_grant_amnesty(god) == false);

    god_destroy(god);
    PASS();
}

/* Test: Condemn */
static void test_god_condemn(void) {
    TEST("god_condemn");

    God* god = god_create("vorathos", "Vorathos, the Void", DOMAIN_ENTROPY);
    assert(god != NULL);

    god_set_favor(god, 50);

    assert(god_condemn(god) == true);
    assert(god->condemned == true);
    assert(god->amnesty_granted == false);
    assert(god->favor == 0); /* 50 - 50 = 0 */

    /* Cannot condemn again */
    assert(god_condemn(god) == false);

    god_destroy(god);
    PASS();
}

/* Test: Favor checks */
static void test_god_favor_checks(void) {
    TEST("god_favor_checks");

    God* god = god_create("nexus", "Nexus, the Connector", DOMAIN_NETWORKS);
    assert(god != NULL);

    god_set_favor(god, 30);
    assert(god_has_positive_favor(god) == true);
    assert(god_has_negative_favor(god) == false);
    assert(god_is_neutral(god) == false);

    god_set_favor(god, -20);
    assert(god_has_positive_favor(god) == false);
    assert(god_has_negative_favor(god) == true);
    assert(god_is_neutral(god) == false);

    god_set_favor(god, 0);
    assert(god_has_positive_favor(god) == false);
    assert(god_has_negative_favor(god) == false);
    assert(god_is_neutral(god) == true);

    god_destroy(god);
    PASS();
}

/* Test: Favor descriptions */
static void test_god_favor_descriptions(void) {
    TEST("god_favor_descriptions");

    God* god = god_create("keldrin", "Keldrin, the Lawgiver", DOMAIN_ORDER);
    assert(god != NULL);

    god_set_favor(god, -85);
    assert(strcmp(god_get_favor_description(god), "Utterly Despised") == 0);

    god_set_favor(god, -50);
    assert(strcmp(god_get_favor_description(god), "Strongly Disfavored") == 0);

    god_set_favor(god, 0);
    assert(strcmp(god_get_favor_description(god), "Neutral") == 0);

    god_set_favor(god, 50);
    assert(strcmp(god_get_favor_description(god), "Strongly Favored") == 0);

    god_set_favor(god, 85);
    assert(strcmp(god_get_favor_description(god), "Revered") == 0);

    god_destroy(god);
    PASS();
}

/* Test: Domain and power names */
static void test_god_names(void) {
    TEST("god_domain_and_power_names");

    assert(strcmp(god_domain_name(DOMAIN_LIFE), "Life") == 0);
    assert(strcmp(god_domain_name(DOMAIN_ORDER), "Order") == 0);
    assert(strcmp(god_domain_name(DOMAIN_TIME), "Time") == 0);
    assert(strcmp(god_domain_name(DOMAIN_SOULS), "Souls") == 0);
    assert(strcmp(god_domain_name(DOMAIN_ENTROPY), "Entropy") == 0);
    assert(strcmp(god_domain_name(DOMAIN_BOUNDARIES), "Boundaries") == 0);
    assert(strcmp(god_domain_name(DOMAIN_NETWORKS), "Networks") == 0);

    assert(strcmp(god_power_level_name(POWER_MORTAL), "Mortal") == 0);
    assert(strcmp(god_power_level_name(POWER_DIVINE_ARCHITECT), "Divine Architect") == 0);

    PASS();
}

/* Test: Divine council create/destroy */
static void test_divine_council_create_destroy(void) {
    TEST("divine_council_create_destroy");

    DivineCouncil* council = divine_council_create();
    assert(council != NULL);
    assert(council->god_count == 0);
    assert(council->council_summoned == false);
    assert(council->judgment_complete == false);
    assert(council->verdict == VERDICT_NONE);

    divine_council_destroy(council);
    PASS();
}

/* Test: Add gods to council */
static void test_divine_council_add_god(void) {
    TEST("divine_council_add_god");

    DivineCouncil* council = divine_council_create();
    assert(council != NULL);

    God* anara = god_create("anara", "Anara, the Weaver", DOMAIN_LIFE);
    God* keldrin = god_create("keldrin", "Keldrin, the Lawgiver", DOMAIN_ORDER);

    assert(divine_council_add_god(council, anara) == true);
    assert(council->god_count == 1);

    assert(divine_council_add_god(council, keldrin) == true);
    assert(council->god_count == 2);

    divine_council_destroy(council);
    PASS();
}

/* Test: Find god in council */
static void test_divine_council_find_god(void) {
    TEST("divine_council_find_god");

    DivineCouncil* council = divine_council_create();
    God* anara = god_create("anara", "Anara, the Weaver", DOMAIN_LIFE);
    God* keldrin = god_create("keldrin", "Keldrin, the Lawgiver", DOMAIN_ORDER);

    divine_council_add_god(council, anara);
    divine_council_add_god(council, keldrin);

    God* found = divine_council_find_god(council, "keldrin");
    assert(found != NULL);
    assert(strcmp(found->name, "Keldrin, the Lawgiver") == 0);

    found = divine_council_find_god(council, "unknown");
    assert(found == NULL);

    divine_council_destroy(council);
    PASS();
}

/* Test: Summon council */
static void test_divine_council_summon(void) {
    TEST("divine_council_summon");

    DivineCouncil* council = divine_council_create();
    assert(council != NULL);

    /* Cannot summon before Day 162 */
    assert(divine_council_can_summon(council, 100) == false);

    /* Can summon on Day 162+ */
    assert(divine_council_can_summon(council, 162) == true);

    /* Summon */
    assert(divine_council_summon(council, 162) == true);
    assert(council->council_summoned == true);
    assert(council->summon_day == 162);

    /* Cannot summon again */
    assert(divine_council_summon(council, 200) == false);

    divine_council_destroy(council);
    PASS();
}

/* Test: Council judgment */
static void test_divine_council_judgment(void) {
    TEST("divine_council_pass_judgment");

    DivineCouncil* council = divine_council_create();

    /* Add 7 gods with varying favor */
    God* g1 = god_create("g1", "God 1", DOMAIN_LIFE);
    God* g2 = god_create("g2", "God 2", DOMAIN_ORDER);
    God* g3 = god_create("g3", "God 3", DOMAIN_TIME);
    God* g4 = god_create("g4", "God 4", DOMAIN_SOULS);
    God* g5 = god_create("g5", "God 5", DOMAIN_ENTROPY);
    God* g6 = god_create("g6", "God 6", DOMAIN_BOUNDARIES);
    God* g7 = god_create("g7", "God 7", DOMAIN_NETWORKS);

    /* Set favors for amnesty vote (4+ with favor >= 40) */
    god_set_favor(g1, 50);
    god_set_favor(g2, 45);
    god_set_favor(g3, 42);
    god_set_favor(g4, 60);
    god_set_favor(g5, 10);
    god_set_favor(g6, -10);
    god_set_favor(g7, 20);

    divine_council_add_god(council, g1);
    divine_council_add_god(council, g2);
    divine_council_add_god(council, g3);
    divine_council_add_god(council, g4);
    divine_council_add_god(council, g5);
    divine_council_add_god(council, g6);
    divine_council_add_god(council, g7);

    /* Pass judgment */
    assert(divine_council_pass_judgment(council, 30, 85.0f) == true);
    assert(council->judgment_complete == true);
    assert(council->verdict == VERDICT_AMNESTY); /* 4 gods voted amnesty */

    /* Cannot judge again */
    assert(divine_council_pass_judgment(council, 30, 85.0f) == false);

    divine_council_destroy(council);
    PASS();
}

/* Test: Average favor calculation */
static void test_divine_council_average_favor(void) {
    TEST("divine_council_average_favor");

    DivineCouncil* council = divine_council_create();

    God* g1 = god_create("g1", "God 1", DOMAIN_LIFE);
    God* g2 = god_create("g2", "God 2", DOMAIN_ORDER);
    God* g3 = god_create("g3", "God 3", DOMAIN_TIME);

    god_set_favor(g1, 30);
    god_set_favor(g2, 60);
    god_set_favor(g3, -30);

    divine_council_add_god(council, g1);
    divine_council_add_god(council, g2);
    divine_council_add_god(council, g3);

    int16_t avg = divine_council_calculate_average_favor(council);
    assert(avg == 20); /* (30 + 60 - 30) / 3 = 20 */

    divine_council_destroy(council);
    PASS();
}

/* Test: Verdict names */
static void test_divine_council_verdict_names(void) {
    TEST("divine_council_verdict_names");

    assert(strcmp(divine_council_verdict_name(VERDICT_NONE), "None") == 0);
    assert(strcmp(divine_council_verdict_name(VERDICT_AMNESTY), "Full Amnesty") == 0);
    assert(strcmp(divine_council_verdict_name(VERDICT_CONDITIONAL), "Conditional Amnesty") == 0);
    assert(strcmp(divine_council_verdict_name(VERDICT_PURGE), "Purge Sentence") == 0);
    assert(strcmp(divine_council_verdict_name(VERDICT_IMMEDIATE_DEATH), "Immediate Execution") == 0);

    PASS();
}

int main(void) {
    printf("Running god system tests...\n\n");

    /* God tests */
    test_god_create_destroy();
    test_god_modify_favor();
    test_god_set_favor();
    test_god_add_dialogue_tree();
    test_god_add_trial();
    test_god_add_restriction();
    test_god_grant_amnesty();
    test_god_condemn();
    test_god_favor_checks();
    test_god_favor_descriptions();
    test_god_names();

    /* Divine Council tests */
    test_divine_council_create_destroy();
    test_divine_council_add_god();
    test_divine_council_find_god();
    test_divine_council_summon();
    test_divine_council_judgment();
    test_divine_council_average_favor();
    test_divine_council_verdict_names();

    /* Summary */
    printf("\n");
    if (tests_passed == tests_run) {
        printf(COLOR_GREEN "All %d tests passed!" COLOR_RESET "\n", tests_passed);
        return 0;
    } else {
        printf(COLOR_RED "%d/%d tests passed" COLOR_RESET "\n", tests_passed, tests_run);
        return 1;
    }
}

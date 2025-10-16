/**
 * @file test_god.c
 * @brief Unit tests for god (Divine Architect) system
 */

#include "../src/game/narrative/gods/god.h"
#include "../src/utils/logger.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

void test_god_creation(void) {
    printf("Test: god_creation... ");

    God* god = god_create("test_god", "TestGod", DOMAIN_LIFE);
    assert(god != NULL);

    /* Initial state */
    assert(strcmp(god->id, "test_god") == 0);
    assert(strcmp(god->name, "TestGod") == 0);
    assert(god->domain == DOMAIN_LIFE);
    assert(god->power_level == POWER_DIVINE_ARCHITECT);
    assert(god->favor == 0);
    assert(god->favor_min == -100);
    assert(god->favor_max == 100);
    assert(god->favor_start == 0);
    assert(god->interactions == 0);
    assert(god->summoned == false);
    assert(god->judgment_given == false);
    assert(god->combat_possible == false);
    assert(god->amnesty_granted == false);
    assert(god->condemned == false);
    assert(god->dialogue_tree_count == 0);
    assert(god->trial_count == 0);
    assert(god->restriction_count == 0);

    god_destroy(god);

    printf("PASS\n");
}

void test_god_favor_modification(void) {
    printf("Test: god_favor_modification... ");

    God* god = god_create("test", "Test", DOMAIN_ORDER);
    assert(god != NULL);

    /* Initial favor */
    assert(god->favor == 0);

    /* Add favor */
    god_modify_favor(god, 25);
    assert(god->favor == 25);

    /* Subtract favor */
    god_modify_favor(god, -15);
    assert(god->favor == 10);

    /* Clamp to maximum */
    god_modify_favor(god, 150);
    assert(god->favor == 100); /* Clamped */

    /* Clamp to minimum */
    god_modify_favor(god, -250);
    assert(god->favor == -100); /* Clamped */

    god_destroy(god);

    printf("PASS\n");
}

void test_god_set_favor(void) {
    printf("Test: god_set_favor... ");

    God* god = god_create("test", "Test", DOMAIN_TIME);
    assert(god != NULL);

    /* Set positive */
    god_set_favor(god, 50);
    assert(god->favor == 50);

    /* Set negative */
    god_set_favor(god, -30);
    assert(god->favor == -30);

    /* Set zero */
    god_set_favor(god, 0);
    assert(god->favor == 0);

    /* Clamp maximum */
    god_set_favor(god, 200);
    assert(god->favor == 100);

    /* Clamp minimum */
    god_set_favor(god, -200);
    assert(god->favor == -100);

    god_destroy(god);

    printf("PASS\n");
}

void test_god_favor_descriptions(void) {
    printf("Test: god_favor_descriptions... ");

    God* god = god_create("test", "Test", DOMAIN_SOULS);
    assert(god != NULL);

    /* Test all description ranges */
    god_set_favor(god, -100);
    assert(strcmp(god_get_favor_description(god), "Utterly Despised") == 0);

    god_set_favor(god, -70);
    assert(strcmp(god_get_favor_description(god), "Deeply Hated") == 0);

    god_set_favor(god, -50);
    assert(strcmp(god_get_favor_description(god), "Strongly Disfavored") == 0);

    god_set_favor(god, -30);
    assert(strcmp(god_get_favor_description(god), "Disfavored") == 0);

    god_set_favor(god, -10);
    assert(strcmp(god_get_favor_description(god), "Slightly Disfavored") == 0);

    god_set_favor(god, 0);
    assert(strcmp(god_get_favor_description(god), "Neutral") == 0);

    god_set_favor(god, 10);
    assert(strcmp(god_get_favor_description(god), "Slightly Favored") == 0);

    god_set_favor(god, 30);
    assert(strcmp(god_get_favor_description(god), "Favored") == 0);

    god_set_favor(god, 50);
    assert(strcmp(god_get_favor_description(god), "Strongly Favored") == 0);

    god_set_favor(god, 70);
    assert(strcmp(god_get_favor_description(god), "Highly Regarded") == 0);

    god_set_favor(god, 90);
    assert(strcmp(god_get_favor_description(god), "Revered") == 0);

    god_destroy(god);

    printf("PASS\n");
}

void test_god_favor_checks(void) {
    printf("Test: god_favor_checks... ");

    God* god = god_create("test", "Test", DOMAIN_ENTROPY);
    assert(god != NULL);

    /* Neutral */
    god_set_favor(god, 0);
    assert(god_is_neutral(god) == true);
    assert(god_has_positive_favor(god) == false);
    assert(god_has_negative_favor(god) == false);

    /* Positive */
    god_set_favor(god, 25);
    assert(god_is_neutral(god) == false);
    assert(god_has_positive_favor(god) == true);
    assert(god_has_negative_favor(god) == false);

    /* Negative */
    god_set_favor(god, -25);
    assert(god_is_neutral(god) == false);
    assert(god_has_positive_favor(god) == false);
    assert(god_has_negative_favor(god) == true);

    god_destroy(god);

    printf("PASS\n");
}

void test_god_dialogue_trees(void) {
    printf("Test: god_dialogue_trees... ");

    God* god = god_create("test", "Test", DOMAIN_BOUNDARIES);
    assert(god != NULL);

    assert(god->dialogue_tree_count == 0);

    /* Add dialogue trees */
    bool success = god_add_dialogue_tree(god, "tree1");
    assert(success == true);
    assert(god->dialogue_tree_count == 1);
    assert(strcmp(god->dialogue_trees[0], "tree1") == 0);

    success = god_add_dialogue_tree(god, "tree2");
    assert(success == true);
    assert(god->dialogue_tree_count == 2);
    assert(strcmp(god->dialogue_trees[1], "tree2") == 0);

    /* Fill to maximum */
    for (size_t i = 2; i < MAX_GOD_DIALOGUE_TREES; i++) {
        success = god_add_dialogue_tree(god, "overflow");
        assert(success == true);
    }
    assert(god->dialogue_tree_count == MAX_GOD_DIALOGUE_TREES);

    /* Cannot add more */
    success = god_add_dialogue_tree(god, "too_many");
    assert(success == false);

    god_destroy(god);

    printf("PASS\n");
}

void test_god_trials(void) {
    printf("Test: god_trials... ");

    God* god = god_create("test", "Test", DOMAIN_NETWORKS);
    assert(god != NULL);

    assert(god->trial_count == 0);

    /* Add trials */
    bool success = god_add_trial(god, "trial1");
    assert(success == true);
    assert(god->trial_count == 1);
    assert(strcmp(god->trials[0], "trial1") == 0);

    success = god_add_trial(god, "trial2");
    assert(success == true);
    assert(god->trial_count == 2);
    assert(strcmp(god->trials[1], "trial2") == 0);

    /* Fill to maximum */
    for (size_t i = 2; i < MAX_GOD_TRIALS; i++) {
        success = god_add_trial(god, "overflow");
        assert(success == true);
    }
    assert(god->trial_count == MAX_GOD_TRIALS);

    /* Cannot add more */
    success = god_add_trial(god, "too_many");
    assert(success == false);

    god_destroy(god);

    printf("PASS\n");
}

void test_god_restrictions(void) {
    printf("Test: god_restrictions... ");

    God* god = god_create("test", "Test", DOMAIN_LIFE);
    assert(god != NULL);

    assert(god->restriction_count == 0);

    /* Add restrictions */
    bool success = god_add_restriction(god, "No soul harvesting");
    assert(success == true);
    assert(god->restriction_count == 1);
    assert(strcmp(god->restrictions[0], "No soul harvesting") == 0);

    success = god_add_restriction(god, "Limit minions to 10");
    assert(success == true);
    assert(god->restriction_count == 2);
    assert(strcmp(god->restrictions[1], "Limit minions to 10") == 0);

    /* Fill to maximum */
    for (size_t i = 2; i < MAX_GOD_RESTRICTIONS; i++) {
        success = god_add_restriction(god, "Test restriction");
        assert(success == true);
    }
    assert(god->restriction_count == MAX_GOD_RESTRICTIONS);

    /* Cannot add more */
    success = god_add_restriction(god, "Too many restrictions");
    assert(success == false);

    god_destroy(god);

    printf("PASS\n");
}

void test_god_amnesty(void) {
    printf("Test: god_amnesty... ");

    God* god = god_create("test", "Test", DOMAIN_ORDER);
    assert(god != NULL);

    /* Initial state */
    assert(god->amnesty_granted == false);
    assert(god->condemned == false);
    god_set_favor(god, 0);

    /* Grant amnesty */
    bool success = god_grant_amnesty(god);
    assert(success == true);
    assert(god->amnesty_granted == true);
    assert(god->condemned == false); /* Mutually exclusive */
    assert(god->favor == 20); /* +20 favor from amnesty */

    /* Cannot grant twice */
    success = god_grant_amnesty(god);
    assert(success == false);

    god_destroy(god);

    printf("PASS\n");
}

void test_god_condemnation(void) {
    printf("Test: god_condemnation... ");

    God* god = god_create("test", "Test", DOMAIN_TIME);
    assert(god != NULL);

    /* Initial state */
    assert(god->amnesty_granted == false);
    assert(god->condemned == false);
    god_set_favor(god, 0);

    /* Condemn */
    bool success = god_condemn(god);
    assert(success == true);
    assert(god->condemned == true);
    assert(god->amnesty_granted == false); /* Mutually exclusive */
    assert(god->favor == -50); /* -50 favor from condemnation */

    /* Cannot condemn twice */
    success = god_condemn(god);
    assert(success == false);

    god_destroy(god);

    printf("PASS\n");
}

void test_god_interactions(void) {
    printf("Test: god_interactions... ");

    God* god = god_create("test", "Test", DOMAIN_SOULS);
    assert(god != NULL);

    /* Initial state */
    assert(god->interactions == 0);
    assert(god->summoned == false);
    assert(god->judgment_given == false);

    /* Record interactions */
    god_record_interaction(god);
    assert(god->interactions == 1);

    god_record_interaction(god);
    assert(god->interactions == 2);

    god_record_interaction(god);
    assert(god->interactions == 3);

    /* Mark summoned */
    god_mark_summoned(god);
    assert(god->summoned == true);

    /* Mark judgment */
    god_mark_judgment_given(god);
    assert(god->judgment_given == true);

    god_destroy(god);

    printf("PASS\n");
}

void test_god_domain_names(void) {
    printf("Test: god_domain_names... ");

    assert(strcmp(god_domain_name(DOMAIN_LIFE), "Life") == 0);
    assert(strcmp(god_domain_name(DOMAIN_ORDER), "Order") == 0);
    assert(strcmp(god_domain_name(DOMAIN_TIME), "Time") == 0);
    assert(strcmp(god_domain_name(DOMAIN_SOULS), "Souls") == 0);
    assert(strcmp(god_domain_name(DOMAIN_ENTROPY), "Entropy") == 0);
    assert(strcmp(god_domain_name(DOMAIN_BOUNDARIES), "Boundaries") == 0);
    assert(strcmp(god_domain_name(DOMAIN_NETWORKS), "Networks") == 0);

    /* Out of range */
    assert(strcmp(god_domain_name((GodDomain)100), "Unknown") == 0);

    printf("PASS\n");
}

void test_god_power_level_names(void) {
    printf("Test: god_power_level_names... ");

    assert(strcmp(god_power_level_name(POWER_MORTAL), "Mortal") == 0);
    assert(strcmp(god_power_level_name(POWER_AVATAR), "Avatar") == 0);
    assert(strcmp(god_power_level_name(POWER_LESSER_DIVINE), "Lesser Divine") == 0);
    assert(strcmp(god_power_level_name(POWER_DIVINE_ARCHITECT), "Divine Architect") == 0);

    /* Out of range */
    assert(strcmp(god_power_level_name((PowerLevel)100), "Unknown") == 0);

    printf("PASS\n");
}

void test_god_combat_check(void) {
    printf("Test: god_combat_check... ");

    God* god = god_create("test", "Test", DOMAIN_ENTROPY);
    assert(god != NULL);

    /* Initially no combat */
    assert(god->combat_possible == false);
    assert(god_can_be_challenged(god) == false);

    /* Enable combat */
    god->combat_possible = true;
    god->combat_difficulty = 85;
    assert(god_can_be_challenged(god) == true);

    god_destroy(god);

    printf("PASS\n");
}

void test_god_null_safety(void) {
    printf("Test: god_null_safety... ");

    /* All functions should handle NULL gracefully */
    assert(god_modify_favor(NULL, 10) == false);
    assert(god_set_favor(NULL, 10) == false);
    assert(god_add_dialogue_tree(NULL, "test") == false);
    assert(god_add_trial(NULL, "test") == false);
    assert(god_add_restriction(NULL, "test") == false);
    assert(god_grant_amnesty(NULL) == false);
    assert(god_condemn(NULL) == false);
    assert(god_has_positive_favor(NULL) == false);
    assert(god_has_negative_favor(NULL) == false);
    assert(god_is_neutral(NULL) == false);
    assert(strcmp(god_get_favor_description(NULL), "Unknown") == 0);
    assert(god_can_be_challenged(NULL) == false);

    /* mark functions should not crash */
    god_mark_summoned(NULL);
    god_mark_judgment_given(NULL);
    god_record_interaction(NULL);

    /* Destroy should handle NULL */
    god_destroy(NULL); /* Should not crash */

    printf("PASS\n");
}

int main(void) {
    /* Suppress log output during tests */
    logger_set_level(LOG_LEVEL_FATAL + 1);

    printf("\n=== God (Divine Architect) System Tests ===\n\n");

    test_god_creation();
    test_god_favor_modification();
    test_god_set_favor();
    test_god_favor_descriptions();
    test_god_favor_checks();
    test_god_dialogue_trees();
    test_god_trials();
    test_god_restrictions();
    test_god_amnesty();
    test_god_condemnation();
    test_god_interactions();
    test_god_domain_names();
    test_god_power_level_names();
    test_god_combat_check();
    test_god_null_safety();

    printf("\n=== All God Tests Passed! ===\n\n");

    return 0;
}

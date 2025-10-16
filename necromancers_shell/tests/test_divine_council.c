/**
 * @file test_divine_council.c
 * @brief Unit tests for Divine Council system
 */

#include "../src/game/narrative/gods/divine_council.h"
#include "../src/game/narrative/gods/god.h"
#include "../src/game/narrative/gods/divine_favor_init.h"
#include "../src/utils/logger.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

void test_divine_council_creation(void) {
    printf("Test: divine_council_creation... ");

    DivineCouncil* council = divine_council_create();
    assert(council != NULL);

    /* Initial state */
    assert(council->god_count == 0);
    assert(council->council_summoned == false);
    assert(council->summon_day == 0);
    assert(council->judgment_complete == false);
    assert(council->verdict == VERDICT_NONE);
    assert(council->restriction_count == 0);
    assert(council->votes_amnesty == 0);
    assert(council->votes_conditional == 0);
    assert(council->votes_purge == 0);
    assert(council->votes_death == 0);
    assert(council->average_favor == 0);
    assert(council->total_interactions == 0);

    divine_council_destroy(council);

    printf("PASS\n");
}

void test_divine_council_add_gods(void) {
    printf("Test: divine_council_add_gods... ");

    DivineCouncil* council = divine_council_create();
    assert(council != NULL);

    /* Add first god */
    God* god1 = god_create("god1", "God One", DOMAIN_LIFE);
    bool success = divine_council_add_god(council, god1);
    assert(success == true);
    assert(council->god_count == 1);

    /* Add second god */
    God* god2 = god_create("god2", "God Two", DOMAIN_ORDER);
    success = divine_council_add_god(council, god2);
    assert(success == true);
    assert(council->god_count == 2);

    /* Fill to maximum */
    for (size_t i = 2; i < MAX_COUNCIL_GODS; i++) {
        God* god = god_create("overflow", "Overflow", DOMAIN_TIME);
        success = divine_council_add_god(council, god);
        assert(success == true);
    }
    assert(council->god_count == MAX_COUNCIL_GODS);

    /* Cannot add more */
    God* overflow = god_create("too_many", "Too Many", DOMAIN_SOULS);
    success = divine_council_add_god(council, overflow);
    assert(success == false);
    god_destroy(overflow); /* Not added, must destroy manually */

    divine_council_destroy(council);

    printf("PASS\n");
}

void test_divine_council_find_god(void) {
    printf("Test: divine_council_find_god... ");

    DivineCouncil* council = divine_council_create();
    assert(council != NULL);

    /* Add some gods */
    God* god1 = god_create("anara", "Anara", DOMAIN_LIFE);
    God* god2 = god_create("keldrin", "Keldrin", DOMAIN_ORDER);
    God* god3 = god_create("theros", "Theros", DOMAIN_TIME);

    divine_council_add_god(council, god1);
    divine_council_add_god(council, god2);
    divine_council_add_god(council, god3);

    /* Find by ID */
    God* found = divine_council_find_god(council, "anara");
    assert(found != NULL);
    assert(strcmp(found->name, "Anara") == 0);

    found = divine_council_find_god(council, "keldrin");
    assert(found != NULL);
    assert(strcmp(found->name, "Keldrin") == 0);

    found = divine_council_find_god(council, "theros");
    assert(found != NULL);
    assert(strcmp(found->name, "Theros") == 0);

    /* Not found */
    found = divine_council_find_god(council, "nonexistent");
    assert(found == NULL);

    divine_council_destroy(council);

    printf("PASS\n");
}

void test_divine_council_get_god(void) {
    printf("Test: divine_council_get_god... ");

    DivineCouncil* council = divine_council_create();
    assert(council != NULL);

    /* Add gods */
    God* god1 = god_create("god1", "God One", DOMAIN_LIFE);
    God* god2 = god_create("god2", "God Two", DOMAIN_ORDER);

    divine_council_add_god(council, god1);
    divine_council_add_god(council, god2);

    /* Get by index */
    God* got = divine_council_get_god(council, 0);
    assert(got != NULL);
    assert(strcmp(got->id, "god1") == 0);

    got = divine_council_get_god(council, 1);
    assert(got != NULL);
    assert(strcmp(got->id, "god2") == 0);

    /* Out of range */
    got = divine_council_get_god(council, 2);
    assert(got == NULL);

    divine_council_destroy(council);

    printf("PASS\n");
}

void test_divine_council_summon(void) {
    printf("Test: divine_council_summon... ");

    DivineCouncil* council = divine_council_create();
    assert(council != NULL);

    /* Add some gods */
    for (size_t i = 0; i < 3; i++) {
        God* god = god_create("god", "God", DOMAIN_LIFE);
        divine_council_add_god(council, god);
    }

    /* Initial state */
    assert(council->council_summoned == false);

    /* Cannot summon before day 162 */
    assert(divine_council_can_summon(council, 100) == false);
    assert(divine_council_can_summon(council, 161) == false);

    /* Can summon at day 162+ */
    assert(divine_council_can_summon(council, 162) == true);
    assert(divine_council_can_summon(council, 200) == true);

    /* Summon on day 175 */
    bool success = divine_council_summon(council, 175);
    assert(success == true);
    assert(council->council_summoned == true);
    assert(council->summon_day == 175);

    /* All gods marked as summoned */
    for (size_t i = 0; i < council->god_count; i++) {
        assert(council->gods[i]->summoned == true);
    }

    /* Cannot summon twice */
    success = divine_council_summon(council, 200);
    assert(success == false);
    assert(council->summon_day == 175); /* Unchanged */

    divine_council_destroy(council);

    printf("PASS\n");
}

void test_divine_council_average_favor(void) {
    printf("Test: divine_council_average_favor... ");

    DivineCouncil* council = divine_council_create();
    assert(council != NULL);

    /* Add gods with different favor levels */
    God* god1 = god_create("god1", "God1", DOMAIN_LIFE);
    god_set_favor(god1, 50);
    divine_council_add_god(council, god1);

    God* god2 = god_create("god2", "God2", DOMAIN_ORDER);
    god_set_favor(god2, -30);
    divine_council_add_god(council, god2);

    God* god3 = god_create("god3", "God3", DOMAIN_TIME);
    god_set_favor(god3, 20);
    divine_council_add_god(council, god3);

    /* Calculate average: (50 + -30 + 20) / 3 = 40 / 3 = 13 */
    int16_t avg = divine_council_calculate_average_favor(council);
    assert(avg == 13);

    /* Update council */
    council->average_favor = avg;
    assert(council->average_favor == 13);

    divine_council_destroy(council);

    printf("PASS\n");
}

void test_divine_council_total_interactions(void) {
    printf("Test: divine_council_total_interactions... ");

    DivineCouncil* council = divine_council_create();
    assert(council != NULL);

    /* Add gods with interactions */
    God* god1 = god_create("god1", "God1", DOMAIN_SOULS);
    god1->interactions = 5;
    divine_council_add_god(council, god1);

    God* god2 = god_create("god2", "God2", DOMAIN_ENTROPY);
    god2->interactions = 3;
    divine_council_add_god(council, god2);

    God* god3 = god_create("god3", "God3", DOMAIN_BOUNDARIES);
    god3->interactions = 7;
    divine_council_add_god(council, god3);

    /* Calculate total: 5 + 3 + 7 = 15 */
    uint32_t total = divine_council_calculate_total_interactions(council);
    assert(total == 15);

    divine_council_destroy(council);

    printf("PASS\n");
}

void test_divine_council_pass_judgment_amnesty(void) {
    printf("Test: divine_council_pass_judgment_amnesty... ");

    DivineCouncil* council = divine_council_create();
    assert(council != NULL);

    /* Add gods with high favor (should vote amnesty) */
    for (size_t i = 0; i < 7; i++) {
        God* god = god_create("god", "God", DOMAIN_LIFE);
        god_set_favor(god, 50); /* High favor */
        divine_council_add_god(council, god);
    }

    /* Pass judgment */
    bool success = divine_council_pass_judgment(council, 30, 80.0f);
    assert(success == true);
    assert(council->judgment_complete == true);
    assert(council->verdict == VERDICT_AMNESTY);
    assert(council->votes_amnesty == 7);

    /* All gods marked as having given judgment */
    for (size_t i = 0; i < council->god_count; i++) {
        assert(council->gods[i]->judgment_given == true);
        assert(council->gods[i]->amnesty_granted == true);
    }

    /* Cannot judge twice */
    success = divine_council_pass_judgment(council, 30, 80.0f);
    assert(success == false);

    divine_council_destroy(council);

    printf("PASS\n");
}

void test_divine_council_pass_judgment_conditional(void) {
    printf("Test: divine_council_pass_judgment_conditional... ");

    DivineCouncil* council = divine_council_create();
    assert(council != NULL);

    /* Add gods with mixed favor (should result in conditional) */
    for (size_t i = 0; i < 7; i++) {
        God* god = god_create("god", "God", DOMAIN_ORDER);
        if (i < 3) {
            god_set_favor(god, 10); /* Weak favor -> conditional */
        } else {
            god_set_favor(god, -10); /* Weak disfavor -> conditional */
        }
        divine_council_add_god(council, god);
    }

    /* Pass judgment */
    bool success = divine_council_pass_judgment(council, 45, 65.0f);
    assert(success == true);
    assert(council->judgment_complete == true);
    assert(council->verdict == VERDICT_CONDITIONAL);
    assert(council->votes_conditional == 7);

    divine_council_destroy(council);

    printf("PASS\n");
}

void test_divine_council_pass_judgment_purge(void) {
    printf("Test: divine_council_pass_judgment_purge... ");

    DivineCouncil* council = divine_council_create();
    assert(council != NULL);

    /* Add gods with strong disfavor (should vote purge) */
    for (size_t i = 0; i < 7; i++) {
        God* god = god_create("god", "God", DOMAIN_TIME);
        god_set_favor(god, -50); /* Strong disfavor */
        divine_council_add_god(council, god);
    }

    /* Pass judgment */
    bool success = divine_council_pass_judgment(council, 80, 30.0f);
    assert(success == true);
    assert(council->judgment_complete == true);
    assert(council->verdict == VERDICT_PURGE);
    assert(council->votes_purge == 7);

    divine_council_destroy(council);

    printf("PASS\n");
}

void test_divine_council_pass_judgment_death(void) {
    printf("Test: divine_council_pass_judgment_death... ");

    DivineCouncil* council = divine_council_create();
    assert(council != NULL);

    /* Add gods with extreme disfavor (should vote death) */
    for (size_t i = 0; i < 7; i++) {
        God* god = god_create("god", "God", DOMAIN_SOULS);
        god_set_favor(god, -80); /* Extreme disfavor */
        divine_council_add_god(council, god);
    }

    /* Pass judgment */
    bool success = divine_council_pass_judgment(council, 95, 10.0f);
    assert(success == true);
    assert(council->judgment_complete == true);
    assert(council->verdict == VERDICT_IMMEDIATE_DEATH);
    assert(council->votes_death == 7);

    divine_council_destroy(council);

    printf("PASS\n");
}

void test_divine_council_restrictions(void) {
    printf("Test: divine_council_restrictions... ");

    DivineCouncil* council = divine_council_create();
    assert(council != NULL);

    /* Add restrictions */
    bool success = divine_council_add_restriction(council, "No soul harvesting");
    assert(success == true);
    assert(council->restriction_count == 1);
    assert(strcmp(council->restrictions[0], "No soul harvesting") == 0);

    success = divine_council_add_restriction(council, "Limit corruption to 50%");
    assert(success == true);
    assert(council->restriction_count == 2);

    /* Fill to maximum */
    for (size_t i = 2; i < MAX_COUNCIL_RESTRICTIONS; i++) {
        success = divine_council_add_restriction(council, "Test restriction");
        assert(success == true);
    }
    assert(council->restriction_count == MAX_COUNCIL_RESTRICTIONS);

    /* Cannot add more */
    success = divine_council_add_restriction(council, "Too many restrictions");
    assert(success == false);

    divine_council_destroy(council);

    printf("PASS\n");
}

void test_divine_council_favor_thresholds(void) {
    printf("Test: divine_council_favor_thresholds... ");

    DivineCouncil* council = divine_council_create();
    assert(council != NULL);

    /* Add gods for averaging */
    God* god1 = god_create("god1", "God1", DOMAIN_ENTROPY);
    god_set_favor(god1, 30);
    divine_council_add_god(council, god1);

    God* god2 = god_create("god2", "God2", DOMAIN_BOUNDARIES);
    god_set_favor(god2, 10);
    divine_council_add_god(council, god2);

    /* Average: (30 + 10) / 2 = 20 */
    council->average_favor = divine_council_calculate_average_favor(council);
    assert(council->average_favor == 20);

    /* Check thresholds */
    assert(divine_council_favor_amnesty(council) == true); /* >= 20 */
    assert(divine_council_favor_purge(council) == false);  /* Not <= -40 */

    /* Change to negative average */
    god_set_favor(god1, -50);
    god_set_favor(god2, -30);
    council->average_favor = divine_council_calculate_average_favor(council);
    assert(council->average_favor == -40);

    assert(divine_council_favor_amnesty(council) == false);
    assert(divine_council_favor_purge(council) == true); /* <= -40 */

    divine_council_destroy(council);

    printf("PASS\n");
}

void test_divine_council_count_favor(void) {
    printf("Test: divine_council_count_favor... ");

    DivineCouncil* council = divine_council_create();
    assert(council != NULL);

    /* Add gods with mixed favor */
    God* god1 = god_create("god1", "God1", DOMAIN_LIFE);
    god_set_favor(god1, 30);
    divine_council_add_god(council, god1);

    God* god2 = god_create("god2", "God2", DOMAIN_ORDER);
    god_set_favor(god2, -20);
    divine_council_add_god(council, god2);

    God* god3 = god_create("god3", "God3", DOMAIN_TIME);
    god_set_favor(god3, 0);
    divine_council_add_god(council, god3);

    God* god4 = god_create("god4", "God4", DOMAIN_SOULS);
    god_set_favor(god4, 15);
    divine_council_add_god(council, god4);

    God* god5 = god_create("god5", "God5", DOMAIN_ENTROPY);
    god_set_favor(god5, -40);
    divine_council_add_god(council, god5);

    /* Count: 2 positive, 2 negative, 1 neutral */
    size_t positive = divine_council_count_positive_favor(council);
    assert(positive == 2);

    size_t negative = divine_council_count_negative_favor(council);
    assert(negative == 2);

    divine_council_destroy(council);

    printf("PASS\n");
}

void test_divine_council_verdict_names(void) {
    printf("Test: divine_council_verdict_names... ");

    assert(strcmp(divine_council_verdict_name(VERDICT_NONE), "None") == 0);
    assert(strcmp(divine_council_verdict_name(VERDICT_AMNESTY), "Full Amnesty") == 0);
    assert(strcmp(divine_council_verdict_name(VERDICT_CONDITIONAL), "Conditional Amnesty") == 0);
    assert(strcmp(divine_council_verdict_name(VERDICT_PURGE), "Purge Sentence") == 0);
    assert(strcmp(divine_council_verdict_name(VERDICT_IMMEDIATE_DEATH), "Immediate Execution") == 0);

    /* Out of range */
    assert(strcmp(divine_council_verdict_name((DivineVerdict)100), "Unknown") == 0);

    printf("PASS\n");
}

void test_divine_favor_initialize_council(void) {
    printf("Test: divine_favor_initialize_council... ");

    DivineCouncil* council = divine_favor_initialize_council();
    assert(council != NULL);

    /* Should have exactly 7 gods */
    assert(council->god_count == 7);

    /* Verify all 7 gods exist with correct domains */
    God* anara = divine_council_find_god(council, "anara");
    assert(anara != NULL);
    assert(anara->domain == DOMAIN_LIFE);
    assert(anara->favor == -20); /* Starts with disfavor */

    God* keldrin = divine_council_find_god(council, "keldrin");
    assert(keldrin != NULL);
    assert(keldrin->domain == DOMAIN_ORDER);
    assert(keldrin->favor == -10);

    God* theros = divine_council_find_god(council, "theros");
    assert(theros != NULL);
    assert(theros->domain == DOMAIN_TIME);
    assert(theros->favor == 0); /* Neutral */

    God* myrith = divine_council_find_god(council, "myrith");
    assert(myrith != NULL);
    assert(myrith->domain == DOMAIN_SOULS);
    assert(myrith->favor == 5);

    God* vorathos = divine_council_find_god(council, "vorathos");
    assert(vorathos != NULL);
    assert(vorathos->domain == DOMAIN_ENTROPY);
    assert(vorathos->favor == 15); /* Favors necromancy */

    God* seraph = divine_council_find_god(council, "seraph");
    assert(seraph != NULL);
    assert(seraph->domain == DOMAIN_BOUNDARIES);
    assert(seraph->favor == -15);

    God* nexus = divine_council_find_god(council, "nexus");
    assert(nexus != NULL);
    assert(nexus->domain == DOMAIN_NETWORKS);
    assert(nexus->favor == 10);

    /* Average favor should be slightly negative */
    /* (-20 + -10 + 0 + 5 + 15 + -15 + 10) / 7 = -15 / 7 = -2 */
    assert(council->average_favor == -2);

    divine_council_destroy(council);

    printf("PASS\n");
}

void test_divine_council_null_safety(void) {
    printf("Test: divine_council_null_safety... ");

    /* All functions should handle NULL gracefully */
    assert(divine_council_add_god(NULL, NULL) == false);
    assert(divine_council_find_god(NULL, "test") == NULL);
    assert(divine_council_get_god(NULL, 0) == NULL);
    assert(divine_council_summon(NULL, 162) == false);
    assert(divine_council_pass_judgment(NULL, 50, 50.0f) == false);
    assert(divine_council_add_restriction(NULL, "test") == false);
    assert(divine_council_calculate_average_favor(NULL) == 0);
    assert(divine_council_calculate_total_interactions(NULL) == 0);
    assert(divine_council_can_summon(NULL, 162) == false);
    assert(divine_council_favor_amnesty(NULL) == false);
    assert(divine_council_favor_purge(NULL) == false);
    assert(strcmp(divine_council_get_verdict_description(NULL), "No council") == 0);
    assert(divine_council_count_positive_favor(NULL) == 0);
    assert(divine_council_count_negative_favor(NULL) == 0);
    assert(divine_council_all_voted(NULL) == false);

    /* Destroy should handle NULL */
    divine_council_destroy(NULL); /* Should not crash */

    printf("PASS\n");
}

int main(void) {
    /* Suppress log output during tests */
    logger_set_level(LOG_LEVEL_FATAL + 1);

    printf("\n=== Divine Council System Tests ===\n\n");

    test_divine_council_creation();
    test_divine_council_add_gods();
    test_divine_council_find_god();
    test_divine_council_get_god();
    test_divine_council_summon();
    test_divine_council_average_favor();
    test_divine_council_total_interactions();
    test_divine_council_pass_judgment_amnesty();
    test_divine_council_pass_judgment_conditional();
    test_divine_council_pass_judgment_purge();
    test_divine_council_pass_judgment_death();
    test_divine_council_restrictions();
    test_divine_council_favor_thresholds();
    test_divine_council_count_favor();
    test_divine_council_verdict_names();
    test_divine_favor_initialize_council();
    test_divine_council_null_safety();

    printf("\n=== All Divine Council Tests Passed! ===\n\n");

    return 0;
}

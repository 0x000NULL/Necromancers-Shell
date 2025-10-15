#include "../src/game/narrative/thessara/thessara.h"
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

/* Test: Create and destroy */
static void test_thessara_create_destroy(void) {
    TEST("thessara_create_destroy");

    ThessaraRelationship* thessara = thessara_create();
    assert(thessara != NULL);
    assert(thessara->discovered == false);
    assert(thessara->severed == false);
    assert(thessara->meetings_count == 0);
    assert(thessara->transfer_count == 0);
    assert(thessara->trust_level == 0.0f);
    assert(thessara->wraith_path_revealed == false);
    assert(thessara->morningstar_path_revealed == false);
    assert(thessara->archon_guidance_given == false);

    thessara_destroy(thessara);
    PASS();
}

/* Test: Discover Thessara */
static void test_thessara_discover(void) {
    TEST("thessara_discover");

    ThessaraRelationship* thessara = thessara_create();
    assert(thessara != NULL);

    /* Discover */
    assert(thessara_discover(thessara, 50) == true);
    assert(thessara->discovered == true);
    assert(thessara->discovery_day == 50);
    assert(thessara->trust_level == 25.0f); /* Initial trust granted */

    /* Cannot discover again */
    assert(thessara_discover(thessara, 100) == false);

    thessara_destroy(thessara);
    PASS();
}

/* Test: Sever connection */
static void test_thessara_sever_connection(void) {
    TEST("thessara_sever_connection");

    ThessaraRelationship* thessara = thessara_create();
    thessara_discover(thessara, 50);

    /* Sever */
    assert(thessara_sever_connection(thessara, 200) == true);
    assert(thessara->severed == true);
    assert(thessara->severed_day == 200);
    assert(thessara_is_severed(thessara) == true);
    assert(thessara_can_meet(thessara) == false);

    /* Cannot sever again */
    assert(thessara_sever_connection(thessara, 250) == false);

    thessara_destroy(thessara);
    PASS();
}

/* Test: Record meetings */
static void test_thessara_record_meeting(void) {
    TEST("thessara_record_meeting");

    ThessaraRelationship* thessara = thessara_create();
    thessara_discover(thessara, 50);

    float initial_trust = thessara->trust_level;

    /* Record meeting */
    assert(thessara_record_meeting(thessara, 60) == true);
    assert(thessara->meetings_count == 1);
    assert(thessara->last_meeting_day == 60);
    assert(thessara->trust_level > initial_trust); /* Meeting increases trust */

    /* Another meeting */
    assert(thessara_record_meeting(thessara, 70) == true);
    assert(thessara->meetings_count == 2);
    assert(thessara->last_meeting_day == 70);

    /* Cannot meet after severance */
    thessara_sever_connection(thessara, 200);
    assert(thessara_record_meeting(thessara, 210) == false);
    assert(thessara->meetings_count == 2); /* Count unchanged */

    thessara_destroy(thessara);
    PASS();
}

/* Test: Knowledge transfer */
static void test_thessara_transfer_knowledge(void) {
    TEST("thessara_transfer_knowledge");

    ThessaraRelationship* thessara = thessara_create();
    thessara_discover(thessara, 50);

    float initial_trust = thessara->trust_level;

    /* Transfer research */
    assert(thessara_transfer_knowledge(thessara, KNOWLEDGE_RESEARCH,
                                       "split_routing", "Split-routing protocol", 60) == true);
    assert(thessara->transfer_count == 1);
    assert(thessara->trust_level > initial_trust); /* Transfer increases trust */

    const KnowledgeTransfer* transfer = thessara_get_transfer(thessara, 0);
    assert(transfer != NULL);
    assert(transfer->type == KNOWLEDGE_RESEARCH);
    assert(strcmp(transfer->id, "split_routing") == 0);
    assert(strcmp(transfer->description, "Split-routing protocol") == 0);
    assert(transfer->day_transferred == 60);

    /* Transfer spell */
    assert(thessara_transfer_knowledge(thessara, KNOWLEDGE_SPELL,
                                       "necrotic_bolt", "Advanced necrotic damage", 65) == true);
    assert(thessara->transfer_count == 2);

    /* Cannot transfer after severance */
    thessara_sever_connection(thessara, 200);
    assert(thessara_transfer_knowledge(thessara, KNOWLEDGE_SPELL,
                                       "test", "Test", 210) == false);
    assert(thessara->transfer_count == 2); /* Count unchanged */

    thessara_destroy(thessara);
    PASS();
}

/* Test: Trust modification */
static void test_thessara_trust(void) {
    TEST("thessara_trust_modification");

    ThessaraRelationship* thessara = thessara_create();

    /* Add trust */
    assert(thessara_add_trust(thessara, 50.0f) == true);
    assert(thessara->trust_level == 50.0f);

    /* Add more trust */
    assert(thessara_add_trust(thessara, 30.0f) == true);
    assert(thessara->trust_level == 80.0f);

    /* Clamp at 100% */
    assert(thessara_add_trust(thessara, 50.0f) == true);
    assert(thessara->trust_level == 100.0f);

    /* Reduce trust */
    assert(thessara_reduce_trust(thessara, 20.0f) == true);
    assert(thessara->trust_level == 80.0f);

    /* Reduce to minimum */
    assert(thessara_reduce_trust(thessara, 100.0f) == true);
    assert(thessara->trust_level == 0.0f);

    thessara_destroy(thessara);
    PASS();
}

/* Test: High trust check */
static void test_thessara_high_trust(void) {
    TEST("thessara_has_high_trust");

    ThessaraRelationship* thessara = thessara_create();

    thessara_add_trust(thessara, 50.0f);
    assert(thessara_has_high_trust(thessara) == false);

    thessara_add_trust(thessara, 30.0f);
    assert(thessara_has_high_trust(thessara) == true); /* >= 75% */

    thessara_destroy(thessara);
    PASS();
}

/* Test: Give warnings */
static void test_thessara_warnings(void) {
    TEST("thessara_give_warning");

    ThessaraRelationship* thessara = thessara_create();

    assert(thessara_give_warning(thessara, "Corruption threshold approaching") == true);
    assert(thessara->warning_count == 1);
    assert(strcmp(thessara->warnings[0], "Corruption threshold approaching") == 0);

    assert(thessara_give_warning(thessara, "Consciousness critical") == true);
    assert(thessara->warning_count == 2);

    thessara_destroy(thessara);
    PASS();
}

/* Test: Path revelations */
static void test_thessara_path_revelations(void) {
    TEST("thessara_path_revelations");

    ThessaraRelationship* thessara = thessara_create();

    /* Reveal Wraith path */
    assert(thessara_reveal_wraith_path(thessara) == true);
    assert(thessara->wraith_path_revealed == true);
    assert(thessara_reveal_wraith_path(thessara) == false); /* Cannot reveal again */

    /* Reveal Morningstar path */
    assert(thessara_reveal_morningstar_path(thessara) == true);
    assert(thessara->morningstar_path_revealed == true);
    assert(thessara_reveal_morningstar_path(thessara) == false); /* Cannot reveal again */

    /* Give Archon guidance */
    assert(thessara_give_archon_guidance(thessara) == true);
    assert(thessara->archon_guidance_given == true);
    assert(thessara_give_archon_guidance(thessara) == false); /* Cannot give again */

    thessara_destroy(thessara);
    PASS();
}

/* Test: Trial assistance */
static void test_thessara_trial_assistance(void) {
    TEST("thessara_assist_trial");

    ThessaraRelationship* thessara = thessara_create();

    assert(thessara->trials_assisted == 0);

    thessara_assist_trial(thessara);
    assert(thessara->trials_assisted == 1);

    thessara_assist_trial(thessara);
    thessara_assist_trial(thessara);
    assert(thessara->trials_assisted == 3);

    thessara_destroy(thessara);
    PASS();
}

/* Test: Guidance time tracking */
static void test_thessara_guidance_time(void) {
    TEST("thessara_add_guidance_time");

    ThessaraRelationship* thessara = thessara_create();

    assert(thessara->total_guidance_time == 0);

    thessara_add_guidance_time(thessara, 10);
    assert(thessara->total_guidance_time == 10);

    thessara_add_guidance_time(thessara, 25);
    assert(thessara->total_guidance_time == 35);

    thessara_destroy(thessara);
    PASS();
}

/* Test: Trust descriptions */
static void test_thessara_trust_descriptions(void) {
    TEST("thessara_get_trust_description");

    ThessaraRelationship* thessara = thessara_create();

    thessara_add_trust(thessara, 10.0f);
    assert(strcmp(thessara_get_trust_description(thessara), "Deeply Suspicious") == 0);

    thessara_add_trust(thessara, 20.0f);
    assert(strcmp(thessara_get_trust_description(thessara), "Wary") == 0);

    thessara_add_trust(thessara, 30.0f);
    assert(strcmp(thessara_get_trust_description(thessara), "Cautiously Trusting") == 0);

    thessara_add_trust(thessara, 15.0f);
    assert(strcmp(thessara_get_trust_description(thessara), "Trusting") == 0);

    thessara_add_trust(thessara, 15.0f);
    assert(strcmp(thessara_get_trust_description(thessara), "Deeply Trusting") == 0);

    thessara_destroy(thessara);
    PASS();
}

/* Test: Knowledge type names */
static void test_thessara_knowledge_type_names(void) {
    TEST("thessara_knowledge_type_name");

    assert(strcmp(thessara_knowledge_type_name(KNOWLEDGE_RESEARCH), "Research") == 0);
    assert(strcmp(thessara_knowledge_type_name(KNOWLEDGE_SPELL), "Spell") == 0);
    assert(strcmp(thessara_knowledge_type_name(KNOWLEDGE_TECHNIQUE), "Technique") == 0);
    assert(strcmp(thessara_knowledge_type_name(KNOWLEDGE_HISTORY), "History") == 0);
    assert(strcmp(thessara_knowledge_type_name(KNOWLEDGE_PATH_SECRET), "Path Secret") == 0);

    PASS();
}

/* Test: Get transfer by index */
static void test_thessara_get_transfer(void) {
    TEST("thessara_get_transfer");

    ThessaraRelationship* thessara = thessara_create();
    thessara_discover(thessara, 50);

    /* No transfers yet */
    assert(thessara_get_transfer_count(thessara) == 0);
    assert(thessara_get_transfer(thessara, 0) == NULL);

    /* Add transfers */
    thessara_transfer_knowledge(thessara, KNOWLEDGE_RESEARCH, "r1", "Research 1", 60);
    thessara_transfer_knowledge(thessara, KNOWLEDGE_SPELL, "s1", "Spell 1", 70);

    assert(thessara_get_transfer_count(thessara) == 2);

    const KnowledgeTransfer* t0 = thessara_get_transfer(thessara, 0);
    assert(t0 != NULL);
    assert(strcmp(t0->id, "r1") == 0);

    const KnowledgeTransfer* t1 = thessara_get_transfer(thessara, 1);
    assert(t1 != NULL);
    assert(strcmp(t1->id, "s1") == 0);

    /* Out of range */
    assert(thessara_get_transfer(thessara, 2) == NULL);

    thessara_destroy(thessara);
    PASS();
}

int main(void) {
    printf("Running Thessara system tests...\n\n");

    test_thessara_create_destroy();
    test_thessara_discover();
    test_thessara_sever_connection();
    test_thessara_record_meeting();
    test_thessara_transfer_knowledge();
    test_thessara_trust();
    test_thessara_high_trust();
    test_thessara_warnings();
    test_thessara_path_revelations();
    test_thessara_trial_assistance();
    test_thessara_guidance_time();
    test_thessara_trust_descriptions();
    test_thessara_knowledge_type_names();
    test_thessara_get_transfer();

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

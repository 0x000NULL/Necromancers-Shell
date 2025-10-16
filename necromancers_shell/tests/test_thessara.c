/**
 * @file test_thessara.c
 * @brief Unit tests for Thessara ghost mentor system
 */

#include "../src/game/narrative/thessara/thessara.h"
#include "../src/utils/logger.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

void test_thessara_creation(void) {
    printf("Test: thessara_creation... ");

    ThessaraRelationship* thess = thessara_create();
    assert(thess != NULL);

    /* Initial state */
    assert(thess->discovered == false);
    assert(thess->severed == false);
    assert(thess->meetings_count == 0);
    assert(thess->transfer_count == 0);
    assert(thess->trust_level == 0.0f);
    assert(thess->warning_count == 0);
    assert(thess->wraith_path_revealed == false);
    assert(thess->morningstar_path_revealed == false);
    assert(thess->archon_guidance_given == false);
    assert(thess->total_guidance_time == 0);
    assert(thess->trials_assisted == 0);

    thessara_destroy(thess);

    printf("PASS\n");
}

void test_thessara_discovery(void) {
    printf("Test: thessara_discovery... ");

    ThessaraRelationship* thess = thessara_create();
    assert(thess != NULL);

    /* Discover Thessara on day 100 */
    bool success = thessara_discover(thess, 100);
    assert(success == true);
    assert(thess->discovered == true);
    assert(thess->discovery_day == 100);
    assert(thess->trust_level == 25.0f); /* Initial trust */

    /* Cannot discover twice */
    success = thessara_discover(thess, 200);
    assert(success == false);
    assert(thess->discovery_day == 100); /* Unchanged */

    thessara_destroy(thess);

    printf("PASS\n");
}

void test_thessara_meetings(void) {
    printf("Test: thessara_meetings... ");

    ThessaraRelationship* thess = thessara_create();
    thessara_discover(thess, 10);

    float initial_trust = thess->trust_level;

    /* First meeting */
    bool success = thessara_record_meeting(thess, 20);
    assert(success == true);
    assert(thess->meetings_count == 1);
    assert(thess->last_meeting_day == 20);
    assert(thess->trust_level == initial_trust + 2.0f); /* +2% per meeting */

    /* Second meeting */
    success = thessara_record_meeting(thess, 30);
    assert(success == true);
    assert(thess->meetings_count == 2);
    assert(thess->last_meeting_day == 30);

    thessara_destroy(thess);

    printf("PASS\n");
}

void test_thessara_knowledge_transfer(void) {
    printf("Test: thessara_knowledge_transfer... ");

    ThessaraRelationship* thess = thessara_create();
    thessara_discover(thess, 10);

    float initial_trust = thess->trust_level;

    /* Transfer research knowledge */
    bool success = thessara_transfer_knowledge(
        thess,
        KNOWLEDGE_RESEARCH,
        "soul_compression",
        "Advanced soul compression techniques",
        50
    );
    assert(success == true);
    assert(thess->transfer_count == 1);
    assert(thess->trust_level == initial_trust + 5.0f); /* +5% per transfer */

    /* Check transfer record */
    const KnowledgeTransfer* transfer = thessara_get_transfer(thess, 0);
    assert(transfer != NULL);
    assert(transfer->type == KNOWLEDGE_RESEARCH);
    assert(strcmp(transfer->id, "soul_compression") == 0);
    assert(strcmp(transfer->description, "Advanced soul compression techniques") == 0);
    assert(transfer->day_transferred == 50);

    /* Transfer spell knowledge */
    success = thessara_transfer_knowledge(
        thess,
        KNOWLEDGE_SPELL,
        "soul_siphon",
        "Instant soul energy siphon spell",
        75
    );
    assert(success == true);
    assert(thess->transfer_count == 2);

    /* Get transfer count */
    assert(thessara_get_transfer_count(thess) == 2);

    thessara_destroy(thess);

    printf("PASS\n");
}

void test_thessara_trust_management(void) {
    printf("Test: thessara_trust_management... ");

    ThessaraRelationship* thess = thessara_create();
    thessara_discover(thess, 10);

    /* Initial trust is 25% */
    assert(thess->trust_level == 25.0f);

    /* Add trust */
    thessara_add_trust(thess, 30.0f);
    assert(thess->trust_level == 55.0f);

    /* Reduce trust */
    thessara_reduce_trust(thess, 10.0f);
    assert(thess->trust_level == 45.0f);

    /* Trust cannot exceed 100% */
    thessara_add_trust(thess, 100.0f);
    assert(thess->trust_level == 100.0f);

    /* Trust cannot go below 0% */
    thessara_reduce_trust(thess, 150.0f);
    assert(thess->trust_level == 0.0f);

    thessara_destroy(thess);

    printf("PASS\n");
}

void test_thessara_trust_descriptions(void) {
    printf("Test: thessara_trust_descriptions... ");

    ThessaraRelationship* thess = thessara_create();

    /* Set different trust levels and check descriptions */
    thess->trust_level = 10.0f;
    assert(strcmp(thessara_get_trust_description(thess), "Deeply Suspicious") == 0);

    thess->trust_level = 30.0f;
    assert(strcmp(thessara_get_trust_description(thess), "Wary") == 0);

    thess->trust_level = 50.0f;
    assert(strcmp(thessara_get_trust_description(thess), "Cautiously Trusting") == 0);

    thess->trust_level = 70.0f;
    assert(strcmp(thessara_get_trust_description(thess), "Trusting") == 0);

    thess->trust_level = 90.0f;
    assert(strcmp(thessara_get_trust_description(thess), "Deeply Trusting") == 0);

    /* Check high trust threshold */
    thess->trust_level = 74.0f;
    assert(thessara_has_high_trust(thess) == false);

    thess->trust_level = 75.0f;
    assert(thessara_has_high_trust(thess) == true);

    thessara_destroy(thess);

    printf("PASS\n");
}

void test_thessara_warnings(void) {
    printf("Test: thessara_warnings... ");

    ThessaraRelationship* thess = thessara_create();
    thessara_discover(thess, 10);

    /* Give first warning */
    bool success = thessara_give_warning(thess, "Your corruption is rising too quickly");
    assert(success == true);
    assert(thess->warning_count == 1);

    /* Give second warning */
    success = thessara_give_warning(thess, "The Ashbrook choice will haunt you");
    assert(success == true);
    assert(thess->warning_count == 2);

    /* Warnings can fill up to MAX_THESSARA_WARNINGS (20) */
    for (size_t i = 2; i < MAX_THESSARA_WARNINGS; i++) {
        success = thessara_give_warning(thess, "Test warning");
        assert(success == true);
    }
    assert(thess->warning_count == MAX_THESSARA_WARNINGS);

    /* Cannot add more warnings */
    success = thessara_give_warning(thess, "Too many warnings");
    assert(success == false);

    thessara_destroy(thess);

    printf("PASS\n");
}

void test_thessara_path_revelations(void) {
    printf("Test: thessara_path_revelations... ");

    ThessaraRelationship* thess = thessara_create();
    thessara_discover(thess, 10);

    /* Reveal Wraith path */
    bool success = thessara_reveal_wraith_path(thess);
    assert(success == true);
    assert(thess->wraith_path_revealed == true);

    /* Cannot reveal twice */
    success = thessara_reveal_wraith_path(thess);
    assert(success == false);

    /* Reveal Morningstar path */
    success = thessara_reveal_morningstar_path(thess);
    assert(success == true);
    assert(thess->morningstar_path_revealed == true);

    /* Cannot reveal twice */
    success = thessara_reveal_morningstar_path(thess);
    assert(success == false);

    /* Provide Archon guidance */
    success = thessara_give_archon_guidance(thess);
    assert(success == true);
    assert(thess->archon_guidance_given == true);

    /* Cannot give twice */
    success = thessara_give_archon_guidance(thess);
    assert(success == false);

    thessara_destroy(thess);

    printf("PASS\n");
}

void test_thessara_mentorship_tracking(void) {
    printf("Test: thessara_mentorship_tracking... ");

    ThessaraRelationship* thess = thessara_create();
    thessara_discover(thess, 10);

    /* Initial state */
    assert(thess->trials_assisted == 0);
    assert(thess->total_guidance_time == 0);

    /* Assist with trials */
    thessara_assist_trial(thess);
    assert(thess->trials_assisted == 1);

    thessara_assist_trial(thess);
    assert(thess->trials_assisted == 2);

    /* Add guidance time */
    thessara_add_guidance_time(thess, 5);
    assert(thess->total_guidance_time == 5);

    thessara_add_guidance_time(thess, 10);
    assert(thess->total_guidance_time == 15);

    thessara_destroy(thess);

    printf("PASS\n");
}

void test_thessara_sever_connection(void) {
    printf("Test: thessara_sever_connection... ");

    ThessaraRelationship* thess = thessara_create();
    thessara_discover(thess, 10);

    /* Can meet initially */
    assert(thessara_can_meet(thess) == true);
    assert(thessara_is_severed(thess) == false);

    /* Have a meeting */
    bool success = thessara_record_meeting(thess, 50);
    assert(success == true);

    /* Transfer knowledge */
    success = thessara_transfer_knowledge(
        thess,
        KNOWLEDGE_TECHNIQUE,
        "advanced_binding",
        "Advanced soul binding",
        75
    );
    assert(success == true);

    /* Sever connection (Trial 6 sacrifice) */
    success = thessara_sever_connection(thess, 100);
    assert(success == true);
    assert(thess->severed == true);
    assert(thess->severed_day == 100);
    assert(thessara_is_severed(thess) == true);
    assert(thessara_can_meet(thess) == false);

    /* Cannot sever twice */
    success = thessara_sever_connection(thess, 150);
    assert(success == false);
    assert(thess->severed_day == 100); /* Unchanged */

    /* Cannot meet after severance */
    success = thessara_record_meeting(thess, 150);
    assert(success == false);

    /* Cannot transfer knowledge after severance */
    success = thessara_transfer_knowledge(
        thess,
        KNOWLEDGE_SPELL,
        "test",
        "Test",
        150
    );
    assert(success == false);

    thessara_destroy(thess);

    printf("PASS\n");
}

void test_thessara_knowledge_types(void) {
    printf("Test: thessara_knowledge_types... ");

    assert(strcmp(thessara_knowledge_type_name(KNOWLEDGE_RESEARCH), "Research") == 0);
    assert(strcmp(thessara_knowledge_type_name(KNOWLEDGE_SPELL), "Spell") == 0);
    assert(strcmp(thessara_knowledge_type_name(KNOWLEDGE_TECHNIQUE), "Technique") == 0);
    assert(strcmp(thessara_knowledge_type_name(KNOWLEDGE_HISTORY), "History") == 0);
    assert(strcmp(thessara_knowledge_type_name(KNOWLEDGE_PATH_SECRET), "Path Secret") == 0);

    printf("PASS\n");
}

void test_thessara_knowledge_transfer_limit(void) {
    printf("Test: thessara_knowledge_transfer_limit... ");

    ThessaraRelationship* thess = thessara_create();
    thessara_discover(thess, 10);

    /* Fill up knowledge transfer list */
    for (size_t i = 0; i < MAX_THESSARA_KNOWLEDGE; i++) {
        char id[32];
        snprintf(id, sizeof(id), "knowledge_%zu", i);

        bool success = thessara_transfer_knowledge(
            thess,
            KNOWLEDGE_RESEARCH,
            id,
            "Test knowledge",
            50 + i
        );
        assert(success == true);
    }
    assert(thess->transfer_count == MAX_THESSARA_KNOWLEDGE);

    /* Cannot add more */
    bool success = thessara_transfer_knowledge(
        thess,
        KNOWLEDGE_SPELL,
        "overflow",
        "Overflow knowledge",
        200
    );
    assert(success == false);

    /* Verify all transfers are accessible */
    for (size_t i = 0; i < MAX_THESSARA_KNOWLEDGE; i++) {
        const KnowledgeTransfer* transfer = thessara_get_transfer(thess, i);
        assert(transfer != NULL);
        assert(transfer->type == KNOWLEDGE_RESEARCH);
    }

    /* Out of range access */
    const KnowledgeTransfer* transfer = thessara_get_transfer(thess, MAX_THESSARA_KNOWLEDGE);
    assert(transfer == NULL);

    thessara_destroy(thess);

    printf("PASS\n");
}

void test_thessara_null_safety(void) {
    printf("Test: thessara_null_safety... ");

    /* All functions should handle NULL gracefully */
    assert(thessara_discover(NULL, 10) == false);
    assert(thessara_sever_connection(NULL, 10) == false);
    assert(thessara_record_meeting(NULL, 10) == false);
    assert(thessara_transfer_knowledge(NULL, KNOWLEDGE_RESEARCH, "test", "test", 10) == false);
    assert(thessara_add_trust(NULL, 10.0f) == false);
    assert(thessara_reduce_trust(NULL, 10.0f) == false);
    assert(thessara_give_warning(NULL, "test") == false);
    assert(thessara_reveal_wraith_path(NULL) == false);
    assert(thessara_reveal_morningstar_path(NULL) == false);
    assert(thessara_give_archon_guidance(NULL) == false);
    assert(thessara_is_severed(NULL) == false);
    assert(thessara_can_meet(NULL) == false);
    assert(thessara_get_transfer_count(NULL) == 0);
    assert(thessara_get_transfer(NULL, 0) == NULL);
    assert(thessara_has_high_trust(NULL) == false);
    assert(strcmp(thessara_get_trust_description(NULL), "Unknown") == 0);

    /* Destroy should handle NULL */
    thessara_destroy(NULL); /* Should not crash */

    printf("PASS\n");
}

int main(void) {
    /* Suppress log output during tests */
    logger_set_level(LOG_LEVEL_FATAL + 1);

    printf("\n=== Thessara Ghost System Tests ===\n\n");

    test_thessara_creation();
    test_thessara_discovery();
    test_thessara_meetings();
    test_thessara_knowledge_transfer();
    test_thessara_trust_management();
    test_thessara_trust_descriptions();
    test_thessara_warnings();
    test_thessara_path_revelations();
    test_thessara_mentorship_tracking();
    test_thessara_sever_connection();
    test_thessara_knowledge_types();
    test_thessara_knowledge_transfer_limit();
    test_thessara_null_safety();

    printf("\n=== All Thessara Tests Passed! ===\n\n");

    return 0;
}

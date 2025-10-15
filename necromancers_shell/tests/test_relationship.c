/**
 * @file test_relationship.c
 * @brief Unit tests for relationship system
 */

#include "../src/game/narrative/relationships/relationship.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Test: Create relationship */
void test_relationship_create(void) {
    Relationship* rel = relationship_create("seraphine");

    assert(rel != NULL);
    assert(strcmp(rel->npc_id, "seraphine") == 0);
    assert(rel->trust == 50);
    assert(rel->respect == 50);
    assert(rel->fear == 0);
    assert(rel->status == RELATIONSHIP_STATUS_NEUTRAL);
    assert(rel->total_interactions == 0);
    assert(rel->event_count == 0);
    assert(rel->is_locked == false);

    relationship_destroy(rel);
    printf("✓ test_relationship_create passed\n");
}

/* Test: Create with NULL */
void test_relationship_create_null(void) {
    Relationship* rel = relationship_create(NULL);
    assert(rel == NULL);

    printf("✓ test_relationship_create_null passed\n");
}

/* Test: Destroy NULL */
void test_relationship_destroy_null(void) {
    relationship_destroy(NULL);
    printf("✓ test_relationship_destroy_null passed\n");
}

/* Test: Modify trust */
void test_relationship_modify_trust(void) {
    Relationship* rel = relationship_create("test");
    assert(rel != NULL);

    assert(rel->trust == 50);

    relationship_modify_trust(rel, 20);
    assert(rel->trust == 70);

    relationship_modify_trust(rel, -30);
    assert(rel->trust == 40);

    /* Test clamping */
    relationship_modify_trust(rel, -100);
    assert(rel->trust == 0);

    relationship_modify_trust(rel, 200);
    assert(rel->trust == 100);

    relationship_destroy(rel);
    printf("✓ test_relationship_modify_trust passed\n");
}

/* Test: Modify respect */
void test_relationship_modify_respect(void) {
    Relationship* rel = relationship_create("test");
    assert(rel != NULL);

    assert(rel->respect == 50);

    relationship_modify_respect(rel, 15);
    assert(rel->respect == 65);

    relationship_modify_respect(rel, -25);
    assert(rel->respect == 40);

    relationship_destroy(rel);
    printf("✓ test_relationship_modify_respect passed\n");
}

/* Test: Modify fear */
void test_relationship_modify_fear(void) {
    Relationship* rel = relationship_create("test");
    assert(rel != NULL);

    assert(rel->fear == 0);

    relationship_modify_fear(rel, 30);
    assert(rel->fear == 30);

    relationship_modify_fear(rel, -10);
    assert(rel->fear == 20);

    relationship_destroy(rel);
    printf("✓ test_relationship_modify_fear passed\n");
}

/* Test: Score calculation and status */
void test_relationship_score_and_status(void) {
    Relationship* rel = relationship_create("test");
    assert(rel != NULL);

    /* Neutral start */
    assert(rel->status == RELATIONSHIP_STATUS_NEUTRAL);

    /* Increase trust to make friendly */
    relationship_modify_trust(rel, 30);  /* trust = 80 */
    assert(rel->status == RELATIONSHIP_STATUS_FRIENDLY);

    /* Increase more to make allied */
    relationship_modify_trust(rel, 20);  /* trust = 100 */
    relationship_modify_respect(rel, 30); /* respect = 80 */
    assert(rel->status == RELATIONSHIP_STATUS_ALLIED);

    /* Reset and make hostile */
    rel->trust = 10;
    rel->respect = 10;
    rel->fear = 80;
    relationship_update_score(rel);
    assert(rel->status == RELATIONSHIP_STATUS_HOSTILE);

    relationship_destroy(rel);
    printf("✓ test_relationship_score_and_status passed\n");
}

/* Test: Add event */
void test_relationship_add_event(void) {
    Relationship* rel = relationship_create("test");
    assert(rel != NULL);

    assert(rel->event_count == 0);
    assert(rel->total_interactions == 0);

    relationship_add_event(rel, RELATIONSHIP_EVENT_QUEST_COMPLETED, 10, 5, 0, "Completed quest");

    assert(rel->event_count == 1);
    assert(rel->total_interactions == 1);
    assert(rel->trust == 60);  /* 50 + 10 */
    assert(rel->respect == 55); /* 50 + 5 */
    assert(rel->fear == 0);

    const RelationshipEvent* event = &rel->events[0];
    assert(event->type == RELATIONSHIP_EVENT_QUEST_COMPLETED);
    assert(event->trust_delta == 10);
    assert(event->respect_delta == 5);
    assert(event->fear_delta == 0);
    assert(strcmp(event->description, "Completed quest") == 0);

    relationship_destroy(rel);
    printf("✓ test_relationship_add_event passed\n");
}

/* Test: Multiple events */
void test_relationship_multiple_events(void) {
    Relationship* rel = relationship_create("test");
    assert(rel != NULL);

    relationship_add_event(rel, RELATIONSHIP_EVENT_HELP_PROVIDED, 5, 3, 0, "Event 1");
    relationship_add_event(rel, RELATIONSHIP_EVENT_GIFT_GIVEN, 3, 2, 0, "Event 2");
    relationship_add_event(rel, RELATIONSHIP_EVENT_PROMISE_KEPT, 4, 4, 0, "Event 3");

    assert(rel->event_count == 3);
    assert(rel->total_interactions == 3);
    assert(rel->trust == 62);  /* 50 + 5 + 3 + 4 */
    assert(rel->respect == 59); /* 50 + 3 + 2 + 4 */

    relationship_destroy(rel);
    printf("✓ test_relationship_multiple_events passed\n");
}

/* Test: Event overflow (circular buffer) */
void test_relationship_event_overflow(void) {
    Relationship* rel = relationship_create("test");
    assert(rel != NULL);

    /* Fill beyond capacity */
    for (int i = 0; i < MAX_RELATIONSHIP_EVENTS + 5; i++) {
        char desc[32];
        snprintf(desc, sizeof(desc), "Event %d", i);
        relationship_add_event(rel, RELATIONSHIP_EVENT_DIALOGUE_CHOICE, 1, 0, 0, desc);
    }

    /* Should have exactly MAX_RELATIONSHIP_EVENTS */
    assert(rel->event_count == MAX_RELATIONSHIP_EVENTS);

    /* First event should be shifted out (Event 5) */
    assert(strcmp(rel->events[0].description, "Event 5") == 0);

    /* Last event should be the most recent (Event 20) */
    assert(strcmp(rel->events[MAX_RELATIONSHIP_EVENTS - 1].description, "Event 20") == 0);

    relationship_destroy(rel);
    printf("✓ test_relationship_event_overflow passed\n");
}

/* Test: Get latest event */
void test_relationship_get_latest_event(void) {
    Relationship* rel = relationship_create("test");
    assert(rel != NULL);

    const RelationshipEvent* event = relationship_get_latest_event(rel);
    assert(event == NULL);

    relationship_add_event(rel, RELATIONSHIP_EVENT_QUEST_COMPLETED, 10, 5, 0, "First");
    relationship_add_event(rel, RELATIONSHIP_EVENT_BETRAYAL, -20, -10, 30, "Second");

    event = relationship_get_latest_event(rel);
    assert(event != NULL);
    assert(event->type == RELATIONSHIP_EVENT_BETRAYAL);
    assert(strcmp(event->description, "Second") == 0);

    relationship_destroy(rel);
    printf("✓ test_relationship_get_latest_event passed\n");
}

/* Test: Lock/unlock */
void test_relationship_lock_unlock(void) {
    Relationship* rel = relationship_create("test");
    assert(rel != NULL);

    assert(rel->is_locked == false);
    assert(rel->trust == 50);

    /* Modifications work when unlocked */
    relationship_modify_trust(rel, 10);
    assert(rel->trust == 60);

    /* Lock and try to modify */
    relationship_lock(rel);
    assert(rel->is_locked == true);

    relationship_modify_trust(rel, 10);
    assert(rel->trust == 60); /* Should not change */

    relationship_modify_respect(rel, 10);
    assert(rel->respect == 50); /* Should not change */

    relationship_modify_fear(rel, 10);
    assert(rel->fear == 0); /* Should not change */

    /* Unlock and modify */
    relationship_unlock(rel);
    assert(rel->is_locked == false);

    relationship_modify_trust(rel, 10);
    assert(rel->trust == 70); /* Should change */

    relationship_destroy(rel);
    printf("✓ test_relationship_lock_unlock passed\n");
}

/* Test: Status thresholds */
void test_relationship_status_thresholds(void) {
    Relationship* rel = relationship_create("test");
    assert(rel != NULL);

    /* Test hostile threshold */
    rel->overall_score = -60;
    rel->status = relationship_get_status_from_score(rel->overall_score);
    assert(rel->status == RELATIONSHIP_STATUS_HOSTILE);

    /* Test unfriendly threshold */
    rel->overall_score = -30;
    rel->status = relationship_get_status_from_score(rel->overall_score);
    assert(rel->status == RELATIONSHIP_STATUS_UNFRIENDLY);

    /* Test neutral threshold */
    rel->overall_score = 0;
    rel->status = relationship_get_status_from_score(rel->overall_score);
    assert(rel->status == RELATIONSHIP_STATUS_NEUTRAL);

    /* Test friendly threshold */
    rel->overall_score = 30;
    rel->status = relationship_get_status_from_score(rel->overall_score);
    assert(rel->status == RELATIONSHIP_STATUS_FRIENDLY);

    /* Test allied threshold */
    rel->overall_score = 90;
    rel->status = relationship_get_status_from_score(rel->overall_score);
    assert(rel->status == RELATIONSHIP_STATUS_ALLIED);

    relationship_destroy(rel);
    printf("✓ test_relationship_status_thresholds passed\n");
}

/* Test: Meets status */
void test_relationship_meets_status(void) {
    Relationship* rel = relationship_create("test");
    assert(rel != NULL);

    rel->status = RELATIONSHIP_STATUS_FRIENDLY;

    assert(relationship_meets_status(rel, RELATIONSHIP_STATUS_HOSTILE) == true);
    assert(relationship_meets_status(rel, RELATIONSHIP_STATUS_UNFRIENDLY) == true);
    assert(relationship_meets_status(rel, RELATIONSHIP_STATUS_NEUTRAL) == true);
    assert(relationship_meets_status(rel, RELATIONSHIP_STATUS_FRIENDLY) == true);
    assert(relationship_meets_status(rel, RELATIONSHIP_STATUS_ALLIED) == false);

    relationship_destroy(rel);
    printf("✓ test_relationship_meets_status passed\n");
}

/* Test: Status to string */
void test_relationship_status_to_string(void) {
    assert(strcmp(relationship_status_to_string(RELATIONSHIP_STATUS_HOSTILE), "Hostile") == 0);
    assert(strcmp(relationship_status_to_string(RELATIONSHIP_STATUS_UNFRIENDLY), "Unfriendly") == 0);
    assert(strcmp(relationship_status_to_string(RELATIONSHIP_STATUS_NEUTRAL), "Neutral") == 0);
    assert(strcmp(relationship_status_to_string(RELATIONSHIP_STATUS_FRIENDLY), "Friendly") == 0);
    assert(strcmp(relationship_status_to_string(RELATIONSHIP_STATUS_ALLIED), "Allied") == 0);

    printf("✓ test_relationship_status_to_string passed\n");
}

/* Test: Event type to string */
void test_relationship_event_type_to_string(void) {
    assert(strcmp(relationship_event_type_to_string(RELATIONSHIP_EVENT_QUEST_COMPLETED), "Quest Completed") == 0);
    assert(strcmp(relationship_event_type_to_string(RELATIONSHIP_EVENT_QUEST_FAILED), "Quest Failed") == 0);
    assert(strcmp(relationship_event_type_to_string(RELATIONSHIP_EVENT_BETRAYAL), "Betrayal") == 0);
    assert(strcmp(relationship_event_type_to_string(RELATIONSHIP_EVENT_PROMISE_KEPT), "Promise Kept") == 0);

    printf("✓ test_relationship_event_type_to_string passed\n");
}

/* Test: NULL parameter handling */
void test_relationship_null_parameters(void) {
    Relationship* rel = relationship_create("test");
    assert(rel != NULL);

    /* Should handle NULL gracefully */
    relationship_modify_trust(NULL, 10);
    relationship_modify_respect(NULL, 10);
    relationship_modify_fear(NULL, 10);
    relationship_add_event(NULL, RELATIONSHIP_EVENT_HELP_PROVIDED, 5, 5, 0, "test");
    relationship_update_score(NULL);
    relationship_lock(NULL);
    relationship_unlock(NULL);

    assert(relationship_meets_status(NULL, RELATIONSHIP_STATUS_NEUTRAL) == false);
    assert(relationship_get_latest_event(NULL) == NULL);

    relationship_destroy(rel);
    printf("✓ test_relationship_null_parameters passed\n");
}

/* Run all tests */
int main(void) {
    printf("Running relationship tests...\n\n");

    test_relationship_create();
    test_relationship_create_null();
    test_relationship_destroy_null();
    test_relationship_modify_trust();
    test_relationship_modify_respect();
    test_relationship_modify_fear();
    test_relationship_score_and_status();
    test_relationship_add_event();
    test_relationship_multiple_events();
    test_relationship_event_overflow();
    test_relationship_get_latest_event();
    test_relationship_lock_unlock();
    test_relationship_status_thresholds();
    test_relationship_meets_status();
    test_relationship_status_to_string();
    test_relationship_event_type_to_string();
    test_relationship_null_parameters();

    printf("\n✅ All relationship tests passed! (17/17)\n");
    return 0;
}

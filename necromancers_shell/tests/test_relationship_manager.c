/**
 * @file test_relationship_manager.c
 * @brief Unit tests for relationship manager
 */

#include "../src/game/narrative/relationships/relationship_manager.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Test: Create relationship manager */
void test_relationship_manager_create(void) {
    RelationshipManager* manager = relationship_manager_create();

    assert(manager != NULL);
    assert(manager->relationship_count == 0);
    assert(manager->relationship_capacity > 0);
    assert(manager->relationships != NULL);

    relationship_manager_destroy(manager);
    printf("✓ test_relationship_manager_create passed\n");
}

/* Test: Destroy NULL manager */
void test_relationship_manager_destroy_null(void) {
    relationship_manager_destroy(NULL);
    printf("✓ test_relationship_manager_destroy_null passed\n");
}

/* Test: Add relationship */
void test_relationship_manager_add_relationship(void) {
    RelationshipManager* manager = relationship_manager_create();
    assert(manager != NULL);

    Relationship* rel = relationship_create("seraphine");
    assert(rel != NULL);

    relationship_manager_add_relationship(manager, rel);
    assert(manager->relationship_count == 1);

    relationship_manager_destroy(manager);
    printf("✓ test_relationship_manager_add_relationship passed\n");
}

/* Test: Get or create (existing) */
void test_relationship_manager_get_or_create_existing(void) {
    RelationshipManager* manager = relationship_manager_create();
    assert(manager != NULL);

    Relationship* rel1 = relationship_manager_get_or_create(manager, "seraphine");
    assert(rel1 != NULL);
    assert(manager->relationship_count == 1);

    Relationship* rel2 = relationship_manager_get_or_create(manager, "seraphine");
    assert(rel2 != NULL);
    assert(rel2 == rel1); /* Should return same relationship */
    assert(manager->relationship_count == 1); /* Count should not increase */

    relationship_manager_destroy(manager);
    printf("✓ test_relationship_manager_get_or_create_existing passed\n");
}

/* Test: Get or create (new) */
void test_relationship_manager_get_or_create_new(void) {
    RelationshipManager* manager = relationship_manager_create();
    assert(manager != NULL);

    Relationship* rel = relationship_manager_get_or_create(manager, "aldric");
    assert(rel != NULL);
    assert(strcmp(rel->npc_id, "aldric") == 0);
    assert(manager->relationship_count == 1);

    relationship_manager_destroy(manager);
    printf("✓ test_relationship_manager_get_or_create_new passed\n");
}

/* Test: Get (existing and nonexistent) */
void test_relationship_manager_get(void) {
    RelationshipManager* manager = relationship_manager_create();
    assert(manager != NULL);

    Relationship* rel = relationship_create("seraphine");
    relationship_manager_add_relationship(manager, rel);

    Relationship* found = relationship_manager_get(manager, "seraphine");
    assert(found != NULL);
    assert(found == rel);

    found = relationship_manager_get(manager, "nonexistent");
    assert(found == NULL);

    relationship_manager_destroy(manager);
    printf("✓ test_relationship_manager_get passed\n");
}

/* Test: Get all relationships */
void test_relationship_manager_get_all(void) {
    RelationshipManager* manager = relationship_manager_create();
    assert(manager != NULL);

    relationship_manager_get_or_create(manager, "npc1");
    relationship_manager_get_or_create(manager, "npc2");
    relationship_manager_get_or_create(manager, "npc3");

    size_t count;
    Relationship** all = relationship_manager_get_all(manager, &count);

    assert(all != NULL);
    assert(count == 3);

    free(all);
    relationship_manager_destroy(manager);
    printf("✓ test_relationship_manager_get_all passed\n");
}

/* Test: Get all - empty */
void test_relationship_manager_get_all_empty(void) {
    RelationshipManager* manager = relationship_manager_create();
    assert(manager != NULL);

    size_t count;
    Relationship** all = relationship_manager_get_all(manager, &count);

    assert(all == NULL);
    assert(count == 0);

    relationship_manager_destroy(manager);
    printf("✓ test_relationship_manager_get_all_empty passed\n");
}

/* Test: Get by status */
void test_relationship_manager_get_by_status(void) {
    RelationshipManager* manager = relationship_manager_create();
    assert(manager != NULL);

    Relationship* rel1 = relationship_manager_get_or_create(manager, "npc1");
    rel1->status = RELATIONSHIP_STATUS_FRIENDLY;

    Relationship* rel2 = relationship_manager_get_or_create(manager, "npc2");
    rel2->status = RELATIONSHIP_STATUS_HOSTILE;

    Relationship* rel3 = relationship_manager_get_or_create(manager, "npc3");
    rel3->status = RELATIONSHIP_STATUS_FRIENDLY;

    size_t count;
    Relationship** friendly = relationship_manager_get_by_status(manager, RELATIONSHIP_STATUS_FRIENDLY, &count);

    assert(friendly != NULL);
    assert(count == 2);

    free(friendly);
    relationship_manager_destroy(manager);
    printf("✓ test_relationship_manager_get_by_status passed\n");
}

/* Test: Get meeting status */
void test_relationship_manager_get_meeting_status(void) {
    RelationshipManager* manager = relationship_manager_create();
    assert(manager != NULL);

    Relationship* rel1 = relationship_manager_get_or_create(manager, "npc1");
    rel1->status = RELATIONSHIP_STATUS_ALLIED;

    Relationship* rel2 = relationship_manager_get_or_create(manager, "npc2");
    rel2->status = RELATIONSHIP_STATUS_FRIENDLY;

    Relationship* rel3 = relationship_manager_get_or_create(manager, "npc3");
    rel3->status = RELATIONSHIP_STATUS_NEUTRAL;

    /* Get all that meet FRIENDLY or higher */
    size_t count;
    Relationship** meeting = relationship_manager_get_meeting_status(manager, RELATIONSHIP_STATUS_FRIENDLY, &count);

    assert(meeting != NULL);
    assert(count == 2); /* Allied and Friendly */

    free(meeting);
    relationship_manager_destroy(manager);
    printf("✓ test_relationship_manager_get_meeting_status passed\n");
}

/* Test: Get high trust */
void test_relationship_manager_get_high_trust(void) {
    RelationshipManager* manager = relationship_manager_create();
    assert(manager != NULL);

    Relationship* rel1 = relationship_manager_get_or_create(manager, "npc1");
    rel1->trust = 80;

    Relationship* rel2 = relationship_manager_get_or_create(manager, "npc2");
    rel2->trust = 60;

    Relationship* rel3 = relationship_manager_get_or_create(manager, "npc3");
    rel3->trust = 90;

    size_t count;
    Relationship** high_trust = relationship_manager_get_high_trust(manager, &count);

    assert(high_trust != NULL);
    assert(count == 2); /* 80 and 90 */

    free(high_trust);
    relationship_manager_destroy(manager);
    printf("✓ test_relationship_manager_get_high_trust passed\n");
}

/* Test: Get high fear */
void test_relationship_manager_get_high_fear(void) {
    RelationshipManager* manager = relationship_manager_create();
    assert(manager != NULL);

    Relationship* rel1 = relationship_manager_get_or_create(manager, "npc1");
    rel1->fear = 85;

    Relationship* rel2 = relationship_manager_get_or_create(manager, "npc2");
    rel2->fear = 50;

    Relationship* rel3 = relationship_manager_get_or_create(manager, "npc3");
    rel3->fear = 75;

    size_t count;
    Relationship** high_fear = relationship_manager_get_high_fear(manager, &count);

    assert(high_fear != NULL);
    assert(count == 2); /* 85 and 75 */

    free(high_fear);
    relationship_manager_destroy(manager);
    printf("✓ test_relationship_manager_get_high_fear passed\n");
}

/* Test: Get romances */
void test_relationship_manager_get_romances(void) {
    RelationshipManager* manager = relationship_manager_create();
    assert(manager != NULL);

    Relationship* rel1 = relationship_manager_get_or_create(manager, "npc1");
    rel1->is_romanceable = true;
    rel1->is_romance_active = true;

    Relationship* rel2 = relationship_manager_get_or_create(manager, "npc2");
    rel2->is_romanceable = true;
    rel2->is_romance_active = false;

    Relationship* rel3 = relationship_manager_get_or_create(manager, "npc3");
    rel3->is_romanceable = true;
    rel3->is_romance_active = true;

    size_t count;
    Relationship** romances = relationship_manager_get_romances(manager, &count);

    assert(romances != NULL);
    assert(count == 2);

    free(romances);
    relationship_manager_destroy(manager);
    printf("✓ test_relationship_manager_get_romances passed\n");
}

/* Test: Get rivals */
void test_relationship_manager_get_rivals(void) {
    RelationshipManager* manager = relationship_manager_create();
    assert(manager != NULL);

    Relationship* rel1 = relationship_manager_get_or_create(manager, "npc1");
    rel1->is_rival = true;

    Relationship* rel2 = relationship_manager_get_or_create(manager, "npc2");
    rel2->is_rival = false;

    Relationship* rel3 = relationship_manager_get_or_create(manager, "npc3");
    rel3->is_rival = true;

    size_t count;
    Relationship** rivals = relationship_manager_get_rivals(manager, &count);

    assert(rivals != NULL);
    assert(count == 2);

    free(rivals);
    relationship_manager_destroy(manager);
    printf("✓ test_relationship_manager_get_rivals passed\n");
}

/* Test: Modify trust via manager */
void test_relationship_manager_modify_trust(void) {
    RelationshipManager* manager = relationship_manager_create();
    assert(manager != NULL);

    relationship_manager_modify_trust(manager, "aldric", 20);

    Relationship* rel = relationship_manager_get(manager, "aldric");
    assert(rel != NULL);
    assert(rel->trust == 70); /* 50 + 20 */

    relationship_manager_destroy(manager);
    printf("✓ test_relationship_manager_modify_trust passed\n");
}

/* Test: Modify respect via manager */
void test_relationship_manager_modify_respect(void) {
    RelationshipManager* manager = relationship_manager_create();
    assert(manager != NULL);

    relationship_manager_modify_respect(manager, "vex", 15);

    Relationship* rel = relationship_manager_get(manager, "vex");
    assert(rel != NULL);
    assert(rel->respect == 65); /* 50 + 15 */

    relationship_manager_destroy(manager);
    printf("✓ test_relationship_manager_modify_respect passed\n");
}

/* Test: Modify fear via manager */
void test_relationship_manager_modify_fear(void) {
    RelationshipManager* manager = relationship_manager_create();
    assert(manager != NULL);

    relationship_manager_modify_fear(manager, "witness", 25);

    Relationship* rel = relationship_manager_get(manager, "witness");
    assert(rel != NULL);
    assert(rel->fear == 25); /* 0 + 25 */

    relationship_manager_destroy(manager);
    printf("✓ test_relationship_manager_modify_fear passed\n");
}

/* Test: Add event via manager */
void test_relationship_manager_add_event(void) {
    RelationshipManager* manager = relationship_manager_create();
    assert(manager != NULL);

    relationship_manager_add_event(manager, "seraphine",
                                   RELATIONSHIP_EVENT_QUEST_COMPLETED,
                                   10, 5, 0, "Helped in quest");

    Relationship* rel = relationship_manager_get(manager, "seraphine");
    assert(rel != NULL);
    assert(rel->trust == 60); /* 50 + 10 */
    assert(rel->respect == 55); /* 50 + 5 */
    assert(rel->event_count == 1);
    assert(rel->total_interactions == 1);

    relationship_manager_destroy(manager);
    printf("✓ test_relationship_manager_add_event passed\n");
}

/* Test: NULL parameter handling */
void test_relationship_manager_null_parameters(void) {
    RelationshipManager* manager = relationship_manager_create();
    assert(manager != NULL);

    size_t count;

    /* Should handle NULL gracefully */
    relationship_manager_add_relationship(NULL, relationship_create("test"));
    relationship_manager_add_relationship(manager, NULL);

    assert(relationship_manager_get_or_create(NULL, "test") == NULL);
    assert(relationship_manager_get_or_create(manager, NULL) == NULL);
    assert(relationship_manager_get(NULL, "test") == NULL);
    assert(relationship_manager_get(manager, NULL) == NULL);

    assert(relationship_manager_get_all(NULL, &count) == NULL);
    assert(relationship_manager_get_by_status(NULL, RELATIONSHIP_STATUS_FRIENDLY, &count) == NULL);
    assert(relationship_manager_get_meeting_status(NULL, RELATIONSHIP_STATUS_FRIENDLY, &count) == NULL);
    assert(relationship_manager_get_high_trust(NULL, &count) == NULL);
    assert(relationship_manager_get_high_fear(NULL, &count) == NULL);
    assert(relationship_manager_get_romances(NULL, &count) == NULL);
    assert(relationship_manager_get_rivals(NULL, &count) == NULL);

    relationship_manager_modify_trust(NULL, "test", 10);
    relationship_manager_modify_trust(manager, NULL, 10);
    relationship_manager_modify_respect(NULL, "test", 10);
    relationship_manager_modify_respect(manager, NULL, 10);
    relationship_manager_modify_fear(NULL, "test", 10);
    relationship_manager_modify_fear(manager, NULL, 10);

    relationship_manager_add_event(NULL, "test", RELATIONSHIP_EVENT_HELP_PROVIDED, 5, 5, 0, "test");
    relationship_manager_add_event(manager, NULL, RELATIONSHIP_EVENT_HELP_PROVIDED, 5, 5, 0, "test");

    relationship_manager_destroy(manager);
    printf("✓ test_relationship_manager_null_parameters passed\n");
}

/* Run all tests */
int main(void) {
    printf("Running relationship manager tests...\n\n");

    test_relationship_manager_create();
    test_relationship_manager_destroy_null();
    test_relationship_manager_add_relationship();
    test_relationship_manager_get_or_create_existing();
    test_relationship_manager_get_or_create_new();
    test_relationship_manager_get();
    test_relationship_manager_get_all();
    test_relationship_manager_get_all_empty();
    test_relationship_manager_get_by_status();
    test_relationship_manager_get_meeting_status();
    test_relationship_manager_get_high_trust();
    test_relationship_manager_get_high_fear();
    test_relationship_manager_get_romances();
    test_relationship_manager_get_rivals();
    test_relationship_manager_modify_trust();
    test_relationship_manager_modify_respect();
    test_relationship_manager_modify_fear();
    test_relationship_manager_add_event();
    test_relationship_manager_null_parameters();

    printf("\n✅ All relationship manager tests passed! (19/19)\n");
    return 0;
}

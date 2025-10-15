/**
 * @file test_memory_fragment.c
 * @brief Unit tests for memory fragment system
 */

#include "../src/game/narrative/memory/memory_fragment.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Test: Create memory fragment */
void test_memory_fragment_create(void) {
    MemoryFragment* frag = memory_fragment_create("test_frag", "Test Memory", "This is a test memory.");

    assert(frag != NULL);
    assert(strcmp(frag->id, "test_frag") == 0);
    assert(strcmp(frag->title, "Test Memory") == 0);
    assert(strcmp(frag->content, "This is a test memory.") == 0);
    assert(frag->discovered == false);
    assert(frag->related_count == 0);
    assert(frag->npc_count == 0);
    assert(frag->location_count == 0);

    memory_fragment_destroy(frag);
    printf("✓ test_memory_fragment_create passed\n");
}

/* Test: Create with NULL parameters */
void test_memory_fragment_create_null(void) {
    MemoryFragment* frag1 = memory_fragment_create(NULL, "Title", "Content");
    assert(frag1 == NULL);

    MemoryFragment* frag2 = memory_fragment_create("id", NULL, "Content");
    assert(frag2 == NULL);

    MemoryFragment* frag3 = memory_fragment_create("id", "Title", NULL);
    assert(frag3 == NULL);

    printf("✓ test_memory_fragment_create_null passed\n");
}

/* Test: Destroy NULL fragment */
void test_memory_fragment_destroy_null(void) {
    memory_fragment_destroy(NULL);
    printf("✓ test_memory_fragment_destroy_null passed\n");
}

/* Test: Discover fragment */
void test_memory_fragment_discover(void) {
    MemoryFragment* frag = memory_fragment_create("test", "Title", "Content");
    assert(frag != NULL);
    assert(frag->discovered == false);

    memory_fragment_discover(frag, "forgotten_crypt", "quest");

    assert(frag->discovered == true);
    assert(frag->discovery_time > 0);
    assert(strcmp(frag->discovery_location, "forgotten_crypt") == 0);
    assert(strcmp(frag->discovery_method, "quest") == 0);

    memory_fragment_destroy(frag);
    printf("✓ test_memory_fragment_discover passed\n");
}

/* Test: Discover with NULL location/method */
void test_memory_fragment_discover_null(void) {
    MemoryFragment* frag = memory_fragment_create("test", "Title", "Content");
    assert(frag != NULL);

    memory_fragment_discover(frag, NULL, NULL);
    assert(frag->discovered == true);
    assert(frag->discovery_time > 0);

    memory_fragment_destroy(frag);
    printf("✓ test_memory_fragment_discover_null passed\n");
}

/* Test: Add related fragment */
void test_memory_fragment_add_related(void) {
    MemoryFragment* frag = memory_fragment_create("test", "Title", "Content");
    assert(frag != NULL);

    memory_fragment_add_related(frag, "related_1");
    assert(frag->related_count == 1);
    assert(strcmp(frag->related_fragments[0], "related_1") == 0);

    memory_fragment_add_related(frag, "related_2");
    assert(frag->related_count == 2);
    assert(strcmp(frag->related_fragments[1], "related_2") == 0);

    memory_fragment_destroy(frag);
    printf("✓ test_memory_fragment_add_related passed\n");
}

/* Test: Add related - max capacity */
void test_memory_fragment_add_related_max(void) {
    MemoryFragment* frag = memory_fragment_create("test", "Title", "Content");
    assert(frag != NULL);

    /* Fill to capacity */
    for (size_t i = 0; i < MAX_FRAGMENT_CROSS_REFS; i++) {
        char id[32];
        snprintf(id, sizeof(id), "related_%zu", i);
        memory_fragment_add_related(frag, id);
    }
    assert(frag->related_count == MAX_FRAGMENT_CROSS_REFS);

    /* Try to add beyond capacity (should be ignored) */
    memory_fragment_add_related(frag, "overflow");
    assert(frag->related_count == MAX_FRAGMENT_CROSS_REFS);

    memory_fragment_destroy(frag);
    printf("✓ test_memory_fragment_add_related_max passed\n");
}

/* Test: Add NPC */
void test_memory_fragment_add_npc(void) {
    MemoryFragment* frag = memory_fragment_create("test", "Title", "Content");
    assert(frag != NULL);

    memory_fragment_add_npc(frag, "seraphine");
    assert(frag->npc_count == 1);
    assert(strcmp(frag->related_npcs[0], "seraphine") == 0);

    memory_fragment_add_npc(frag, "aldric");
    assert(frag->npc_count == 2);
    assert(strcmp(frag->related_npcs[1], "aldric") == 0);

    memory_fragment_destroy(frag);
    printf("✓ test_memory_fragment_add_npc passed\n");
}

/* Test: Add location */
void test_memory_fragment_add_location(void) {
    MemoryFragment* frag = memory_fragment_create("test", "Title", "Content");
    assert(frag != NULL);

    memory_fragment_add_location(frag, "forgotten_crypt");
    assert(frag->location_count == 1);
    assert(strcmp(frag->related_locations[0], "forgotten_crypt") == 0);

    memory_fragment_add_location(frag, "obsidian_tower");
    assert(frag->location_count == 2);
    assert(strcmp(frag->related_locations[1], "obsidian_tower") == 0);

    memory_fragment_destroy(frag);
    printf("✓ test_memory_fragment_add_location passed\n");
}

/* Test: Add with NULL parameters */
void test_memory_fragment_add_null(void) {
    MemoryFragment* frag = memory_fragment_create("test", "Title", "Content");
    assert(frag != NULL);

    memory_fragment_add_related(NULL, "related");
    memory_fragment_add_related(frag, NULL);
    assert(frag->related_count == 0);

    memory_fragment_add_npc(NULL, "npc");
    memory_fragment_add_npc(frag, NULL);
    assert(frag->npc_count == 0);

    memory_fragment_add_location(NULL, "loc");
    memory_fragment_add_location(frag, NULL);
    assert(frag->location_count == 0);

    memory_fragment_destroy(frag);
    printf("✓ test_memory_fragment_add_null passed\n");
}

/* Run all tests */
int main(void) {
    printf("Running memory_fragment tests...\n\n");

    test_memory_fragment_create();
    test_memory_fragment_create_null();
    test_memory_fragment_destroy_null();
    test_memory_fragment_discover();
    test_memory_fragment_discover_null();
    test_memory_fragment_add_related();
    test_memory_fragment_add_related_max();
    test_memory_fragment_add_npc();
    test_memory_fragment_add_location();
    test_memory_fragment_add_null();

    printf("\n✅ All memory_fragment tests passed! (10/10)\n");
    return 0;
}

/**
 * @file test_memory_manager.c
 * @brief Unit tests for memory manager
 */

#include "../src/game/narrative/memory/memory_manager.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Test: Create memory manager */
void test_memory_manager_create(void) {
    MemoryManager* manager = memory_manager_create();

    assert(manager != NULL);
    assert(manager->fragment_count == 0);
    assert(manager->fragment_capacity > 0);
    assert(manager->fragments != NULL);

    memory_manager_destroy(manager);
    printf("✓ test_memory_manager_create passed\n");
}

/* Test: Destroy NULL manager */
void test_memory_manager_destroy_null(void) {
    memory_manager_destroy(NULL);
    printf("✓ test_memory_manager_destroy_null passed\n");
}

/* Test: Add fragment */
void test_memory_manager_add_fragment(void) {
    MemoryManager* manager = memory_manager_create();
    assert(manager != NULL);

    MemoryFragment* frag = memory_fragment_create("test_1", "Title 1", "Content 1");
    assert(frag != NULL);

    memory_manager_add_fragment(manager, frag);
    assert(manager->fragment_count == 1);

    memory_manager_destroy(manager);
    printf("✓ test_memory_manager_add_fragment passed\n");
}

/* Test: Add multiple fragments */
void test_memory_manager_add_multiple(void) {
    MemoryManager* manager = memory_manager_create();
    assert(manager != NULL);

    for (int i = 0; i < 5; i++) {
        char id[32], title[64], content[128];
        snprintf(id, sizeof(id), "frag_%d", i);
        snprintf(title, sizeof(title), "Title %d", i);
        snprintf(content, sizeof(content), "Content %d", i);

        MemoryFragment* frag = memory_fragment_create(id, title, content);
        assert(frag != NULL);
        memory_manager_add_fragment(manager, frag);
    }

    assert(manager->fragment_count == 5);

    memory_manager_destroy(manager);
    printf("✓ test_memory_manager_add_multiple passed\n");
}

/* Test: Add fragment with NULL parameters */
void test_memory_manager_add_null(void) {
    MemoryManager* manager = memory_manager_create();
    assert(manager != NULL);

    MemoryFragment* frag = memory_fragment_create("test", "Title", "Content");
    assert(frag != NULL);

    memory_manager_add_fragment(NULL, frag);
    memory_manager_add_fragment(manager, NULL);

    assert(manager->fragment_count == 0);

    memory_fragment_destroy(frag);
    memory_manager_destroy(manager);
    printf("✓ test_memory_manager_add_null passed\n");
}

/* Test: Get fragment by ID */
void test_memory_manager_get_fragment(void) {
    MemoryManager* manager = memory_manager_create();
    assert(manager != NULL);

    MemoryFragment* frag1 = memory_fragment_create("first", "First", "Content 1");
    MemoryFragment* frag2 = memory_fragment_create("second", "Second", "Content 2");
    memory_manager_add_fragment(manager, frag1);
    memory_manager_add_fragment(manager, frag2);

    MemoryFragment* found = memory_manager_get_fragment(manager, "first");
    assert(found != NULL);
    assert(strcmp(found->id, "first") == 0);

    found = memory_manager_get_fragment(manager, "second");
    assert(found != NULL);
    assert(strcmp(found->id, "second") == 0);

    found = memory_manager_get_fragment(manager, "nonexistent");
    assert(found == NULL);

    memory_manager_destroy(manager);
    printf("✓ test_memory_manager_get_fragment passed\n");
}

/* Test: Get discovered fragments */
void test_memory_manager_get_discovered(void) {
    MemoryManager* manager = memory_manager_create();
    assert(manager != NULL);

    /* Add fragments, some discovered */
    MemoryFragment* frag1 = memory_fragment_create("frag1", "Title 1", "Content 1");
    memory_fragment_discover(frag1, "crypt", "quest");
    memory_manager_add_fragment(manager, frag1);

    MemoryFragment* frag2 = memory_fragment_create("frag2", "Title 2", "Content 2");
    memory_manager_add_fragment(manager, frag2);

    MemoryFragment* frag3 = memory_fragment_create("frag3", "Title 3", "Content 3");
    memory_fragment_discover(frag3, "tower", "explore");
    memory_manager_add_fragment(manager, frag3);

    /* Get discovered */
    size_t count;
    MemoryFragment** discovered = memory_manager_get_discovered(manager, &count);

    assert(discovered != NULL);
    assert(count == 2);

    free(discovered);
    memory_manager_destroy(manager);
    printf("✓ test_memory_manager_get_discovered passed\n");
}

/* Test: Get discovered - none found */
void test_memory_manager_get_discovered_empty(void) {
    MemoryManager* manager = memory_manager_create();
    assert(manager != NULL);

    /* Add undiscovered fragments */
    MemoryFragment* frag = memory_fragment_create("frag1", "Title", "Content");
    memory_manager_add_fragment(manager, frag);

    size_t count;
    MemoryFragment** discovered = memory_manager_get_discovered(manager, &count);

    assert(discovered == NULL);
    assert(count == 0);

    memory_manager_destroy(manager);
    printf("✓ test_memory_manager_get_discovered_empty passed\n");
}

/* Test: Get by category */
void test_memory_manager_get_by_category(void) {
    MemoryManager* manager = memory_manager_create();
    assert(manager != NULL);

    /* Add fragments with different categories */
    MemoryFragment* frag1 = memory_fragment_create("frag1", "Title 1", "Content 1");
    strncpy(frag1->category, "past_life", sizeof(frag1->category) - 1);
    memory_fragment_discover(frag1, "loc", "quest");
    memory_manager_add_fragment(manager, frag1);

    MemoryFragment* frag2 = memory_fragment_create("frag2", "Title 2", "Content 2");
    strncpy(frag2->category, "death", sizeof(frag2->category) - 1);
    memory_fragment_discover(frag2, "loc", "quest");
    memory_manager_add_fragment(manager, frag2);

    MemoryFragment* frag3 = memory_fragment_create("frag3", "Title 3", "Content 3");
    strncpy(frag3->category, "past_life", sizeof(frag3->category) - 1);
    memory_fragment_discover(frag3, "loc", "quest");
    memory_manager_add_fragment(manager, frag3);

    /* Get by category */
    size_t count;
    MemoryFragment** past_life = memory_manager_get_by_category(manager, "past_life", &count);

    assert(past_life != NULL);
    assert(count == 2);

    free(past_life);
    memory_manager_destroy(manager);
    printf("✓ test_memory_manager_get_by_category passed\n");
}

/* Test: Get chronological */
void test_memory_manager_get_chronological(void) {
    MemoryManager* manager = memory_manager_create();
    assert(manager != NULL);

    /* Add fragments with different chronological orders */
    MemoryFragment* frag1 = memory_fragment_create("frag1", "Latest", "Content");
    frag1->chronological_order = 100;
    memory_fragment_discover(frag1, "loc", "quest");
    memory_manager_add_fragment(manager, frag1);

    MemoryFragment* frag2 = memory_fragment_create("frag2", "Earliest", "Content");
    frag2->chronological_order = 10;
    memory_fragment_discover(frag2, "loc", "quest");
    memory_manager_add_fragment(manager, frag2);

    MemoryFragment* frag3 = memory_fragment_create("frag3", "Middle", "Content");
    frag3->chronological_order = 50;
    memory_fragment_discover(frag3, "loc", "quest");
    memory_manager_add_fragment(manager, frag3);

    /* Get chronological */
    size_t count;
    MemoryFragment** chronological = memory_manager_get_chronological(manager, &count);

    assert(chronological != NULL);
    assert(count == 3);
    assert(chronological[0]->chronological_order == 10);
    assert(chronological[1]->chronological_order == 50);
    assert(chronological[2]->chronological_order == 100);

    free(chronological);
    memory_manager_destroy(manager);
    printf("✓ test_memory_manager_get_chronological passed\n");
}

/* Test: Get related fragments */
void test_memory_manager_get_related(void) {
    MemoryManager* manager = memory_manager_create();
    assert(manager != NULL);

    /* Create fragments */
    MemoryFragment* frag1 = memory_fragment_create("frag1", "Title 1", "Content 1");
    memory_fragment_discover(frag1, "loc", "quest");
    memory_fragment_add_related(frag1, "frag2");
    memory_fragment_add_related(frag1, "frag3");
    memory_manager_add_fragment(manager, frag1);

    MemoryFragment* frag2 = memory_fragment_create("frag2", "Title 2", "Content 2");
    memory_fragment_discover(frag2, "loc", "quest");
    memory_manager_add_fragment(manager, frag2);

    MemoryFragment* frag3 = memory_fragment_create("frag3", "Title 3", "Content 3");
    memory_fragment_discover(frag3, "loc", "quest");
    memory_manager_add_fragment(manager, frag3);

    /* Get related */
    size_t count;
    MemoryFragment** related = memory_manager_get_related(manager, "frag1", &count);

    assert(related != NULL);
    assert(count == 2);

    free(related);
    memory_manager_destroy(manager);
    printf("✓ test_memory_manager_get_related passed\n");
}

/* Test: Get related - undiscovered not included */
void test_memory_manager_get_related_undiscovered(void) {
    MemoryManager* manager = memory_manager_create();
    assert(manager != NULL);

    MemoryFragment* frag1 = memory_fragment_create("frag1", "Title 1", "Content 1");
    memory_fragment_discover(frag1, "loc", "quest");
    memory_fragment_add_related(frag1, "frag2");
    memory_fragment_add_related(frag1, "frag3");
    memory_manager_add_fragment(manager, frag1);

    MemoryFragment* frag2 = memory_fragment_create("frag2", "Title 2", "Content 2");
    /* frag2 NOT discovered */
    memory_manager_add_fragment(manager, frag2);

    MemoryFragment* frag3 = memory_fragment_create("frag3", "Title 3", "Content 3");
    memory_fragment_discover(frag3, "loc", "quest");
    memory_manager_add_fragment(manager, frag3);

    /* Get related - should only include discovered */
    size_t count;
    MemoryFragment** related = memory_manager_get_related(manager, "frag1", &count);

    assert(related != NULL);
    assert(count == 1); /* Only frag3 */

    free(related);
    memory_manager_destroy(manager);
    printf("✓ test_memory_manager_get_related_undiscovered passed\n");
}

/* Test: Discover fragment */
void test_memory_manager_discover_fragment(void) {
    MemoryManager* manager = memory_manager_create();
    assert(manager != NULL);

    MemoryFragment* frag = memory_fragment_create("test", "Title", "Content");
    memory_manager_add_fragment(manager, frag);

    assert(frag->discovered == false);

    memory_manager_discover_fragment(manager, "test", "crypt", "quest");

    assert(frag->discovered == true);
    assert(strcmp(frag->discovery_location, "crypt") == 0);
    assert(strcmp(frag->discovery_method, "quest") == 0);

    memory_manager_destroy(manager);
    printf("✓ test_memory_manager_discover_fragment passed\n");
}

/* Test: Discover nonexistent fragment */
void test_memory_manager_discover_nonexistent(void) {
    MemoryManager* manager = memory_manager_create();
    assert(manager != NULL);

    /* Should not crash */
    memory_manager_discover_fragment(manager, "nonexistent", "loc", "method");

    memory_manager_destroy(manager);
    printf("✓ test_memory_manager_discover_nonexistent passed\n");
}

/* Run all tests */
int main(void) {
    printf("Running memory_manager tests...\n\n");

    test_memory_manager_create();
    test_memory_manager_destroy_null();
    test_memory_manager_add_fragment();
    test_memory_manager_add_multiple();
    test_memory_manager_add_null();
    test_memory_manager_get_fragment();
    test_memory_manager_get_discovered();
    test_memory_manager_get_discovered_empty();
    test_memory_manager_get_by_category();
    test_memory_manager_get_chronological();
    test_memory_manager_get_related();
    test_memory_manager_get_related_undiscovered();
    test_memory_manager_discover_fragment();
    test_memory_manager_discover_nonexistent();

    printf("\n✅ All memory_manager tests passed! (14/14)\n");
    return 0;
}

#include "../src/game/souls/soul_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/* Test counter */
static int tests_run = 0;
static int tests_passed = 0;

/* Test helper macros */
#define TEST_START(name) \
    do { \
        printf("Running test: %s\n", name); \
        tests_run++; \
    } while (0)

#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf("  FAIL: %s\n", message); \
            return; \
        } \
    } while (0)

#define TEST_PASS() \
    do { \
        printf("  PASS\n"); \
        tests_passed++; \
    } while (0)

/**
 * Test: soul_manager_create and destroy
 */
void test_soul_manager_create_destroy(void) {
    TEST_START("soul_manager_create_destroy");

    SoulManager* manager = soul_manager_create();
    TEST_ASSERT(manager != NULL, "soul_manager_create should not return NULL");
    TEST_ASSERT(soul_manager_count(manager) == 0, "New manager should have 0 souls");

    soul_manager_destroy(manager);
    soul_manager_destroy(NULL); /* Should not crash */

    TEST_PASS();
}

/**
 * Test: soul_manager_add
 */
void test_soul_manager_add(void) {
    TEST_START("soul_manager_add");

    SoulManager* manager = soul_manager_create();
    TEST_ASSERT(manager != NULL, "Manager creation should succeed");

    /* Add some souls */
    Soul* soul1 = soul_create(SOUL_TYPE_COMMON, 50);
    Soul* soul2 = soul_create(SOUL_TYPE_WARRIOR, 75);
    Soul* soul3 = soul_create(SOUL_TYPE_MAGE, 90);

    TEST_ASSERT(soul_manager_add(manager, soul1) == true, "Adding soul1 should succeed");
    TEST_ASSERT(soul_manager_add(manager, soul2) == true, "Adding soul2 should succeed");
    TEST_ASSERT(soul_manager_add(manager, soul3) == true, "Adding soul3 should succeed");

    TEST_ASSERT(soul_manager_count(manager) == 3, "Manager should have 3 souls");

    /* Test NULL inputs */
    Soul* temp_soul = soul_create(SOUL_TYPE_COMMON, 50);
    TEST_ASSERT(soul_manager_add(NULL, temp_soul) == false,
                "Adding to NULL manager should fail");
    soul_destroy(temp_soul); /* Free the soul since NULL manager doesn't take ownership */
    TEST_ASSERT(soul_manager_add(manager, NULL) == false, "Adding NULL soul should fail");

    soul_manager_destroy(manager);
    TEST_PASS();
}

/**
 * Test: soul_manager_get
 */
void test_soul_manager_get(void) {
    TEST_START("soul_manager_get");

    SoulManager* manager = soul_manager_create();
    Soul* soul1 = soul_create(SOUL_TYPE_WARRIOR, 60);
    Soul* soul2 = soul_create(SOUL_TYPE_MAGE, 80);

    uint32_t id1 = soul1->id;
    uint32_t id2 = soul2->id;

    soul_manager_add(manager, soul1);
    soul_manager_add(manager, soul2);

    /* Get souls by ID */
    Soul* retrieved1 = soul_manager_get(manager, id1);
    TEST_ASSERT(retrieved1 != NULL, "Should retrieve soul1");
    TEST_ASSERT(retrieved1->id == id1, "Retrieved soul should have correct ID");

    Soul* retrieved2 = soul_manager_get(manager, id2);
    TEST_ASSERT(retrieved2 != NULL, "Should retrieve soul2");
    TEST_ASSERT(retrieved2->type == SOUL_TYPE_MAGE, "Retrieved soul should have correct type");

    /* Try to get non-existent soul */
    Soul* not_found = soul_manager_get(manager, 99999);
    TEST_ASSERT(not_found == NULL, "Getting non-existent soul should return NULL");

    /* Test NULL manager */
    TEST_ASSERT(soul_manager_get(NULL, id1) == NULL, "Getting from NULL manager should return NULL");

    soul_manager_destroy(manager);
    TEST_PASS();
}

/**
 * Test: soul_manager_remove
 */
void test_soul_manager_remove(void) {
    TEST_START("soul_manager_remove");

    SoulManager* manager = soul_manager_create();
    Soul* soul1 = soul_create(SOUL_TYPE_COMMON, 50);
    Soul* soul2 = soul_create(SOUL_TYPE_WARRIOR, 70);
    Soul* soul3 = soul_create(SOUL_TYPE_MAGE, 90);

    uint32_t id1 = soul1->id;
    uint32_t id2 = soul2->id;
    uint32_t id3 = soul3->id;

    soul_manager_add(manager, soul1);
    soul_manager_add(manager, soul2);
    soul_manager_add(manager, soul3);

    TEST_ASSERT(soul_manager_count(manager) == 3, "Should have 3 souls");

    /* Remove middle soul */
    TEST_ASSERT(soul_manager_remove(manager, id2) == true, "Removing soul2 should succeed");
    TEST_ASSERT(soul_manager_count(manager) == 2, "Should have 2 souls after removal");
    TEST_ASSERT(soul_manager_get(manager, id2) == NULL, "Removed soul should not be found");

    /* Verify remaining souls */
    TEST_ASSERT(soul_manager_get(manager, id1) != NULL, "Soul1 should still exist");
    TEST_ASSERT(soul_manager_get(manager, id3) != NULL, "Soul3 should still exist");

    /* Try to remove non-existent soul */
    TEST_ASSERT(soul_manager_remove(manager, 99999) == false, "Removing non-existent soul should fail");

    /* Test NULL manager */
    TEST_ASSERT(soul_manager_remove(NULL, id1) == false, "Removing from NULL manager should fail");

    soul_manager_destroy(manager);
    TEST_PASS();
}

/**
 * Test: soul_manager_count_by_type
 */
void test_soul_manager_count_by_type(void) {
    TEST_START("soul_manager_count_by_type");

    SoulManager* manager = soul_manager_create();

    soul_manager_add(manager, soul_create(SOUL_TYPE_COMMON, 50));
    soul_manager_add(manager, soul_create(SOUL_TYPE_COMMON, 60));
    soul_manager_add(manager, soul_create(SOUL_TYPE_WARRIOR, 70));
    soul_manager_add(manager, soul_create(SOUL_TYPE_MAGE, 80));
    soul_manager_add(manager, soul_create(SOUL_TYPE_MAGE, 85));
    soul_manager_add(manager, soul_create(SOUL_TYPE_MAGE, 90));

    TEST_ASSERT(soul_manager_count_by_type(manager, SOUL_TYPE_COMMON) == 2,
                "Should have 2 common souls");
    TEST_ASSERT(soul_manager_count_by_type(manager, SOUL_TYPE_WARRIOR) == 1,
                "Should have 1 warrior soul");
    TEST_ASSERT(soul_manager_count_by_type(manager, SOUL_TYPE_MAGE) == 3,
                "Should have 3 mage souls");
    TEST_ASSERT(soul_manager_count_by_type(manager, SOUL_TYPE_ANCIENT) == 0,
                "Should have 0 ancient souls");

    /* Test NULL manager */
    TEST_ASSERT(soul_manager_count_by_type(NULL, SOUL_TYPE_COMMON) == 0,
                "Count by type on NULL manager should return 0");

    soul_manager_destroy(manager);
    TEST_PASS();
}

/**
 * Test: soul_manager_total_energy
 */
void test_soul_manager_total_energy(void) {
    TEST_START("soul_manager_total_energy");

    SoulManager* manager = soul_manager_create();

    /* Add souls with known energy values */
    Soul* soul1 = soul_create(SOUL_TYPE_COMMON, 0);   /* 10 energy */
    Soul* soul2 = soul_create(SOUL_TYPE_COMMON, 100); /* 20 energy */
    Soul* soul3 = soul_create(SOUL_TYPE_WARRIOR, 50); /* 30 energy */

    uint32_t expected_total = soul1->energy + soul2->energy + soul3->energy;

    soul_manager_add(manager, soul1);
    soul_manager_add(manager, soul2);
    soul_manager_add(manager, soul3);

    uint32_t total = soul_manager_total_energy(manager);
    TEST_ASSERT(total == expected_total, "Total energy should match sum of individual energies");

    /* Test NULL manager */
    TEST_ASSERT(soul_manager_total_energy(NULL) == 0, "Total energy of NULL manager should be 0");

    soul_manager_destroy(manager);
    TEST_PASS();
}

/**
 * Test: soul_manager_total_unbound_energy
 */
void test_soul_manager_total_unbound_energy(void) {
    TEST_START("soul_manager_total_unbound_energy");

    SoulManager* manager = soul_manager_create();

    Soul* soul1 = soul_create(SOUL_TYPE_WARRIOR, 50);
    Soul* soul2 = soul_create(SOUL_TYPE_WARRIOR, 50);
    Soul* soul3 = soul_create(SOUL_TYPE_WARRIOR, 50);

    uint32_t energy_each = soul1->energy;

    soul_manager_add(manager, soul1);
    soul_manager_add(manager, soul2);
    soul_manager_add(manager, soul3);

    /* All souls unbound initially */
    TEST_ASSERT(soul_manager_total_unbound_energy(manager) == energy_each * 3,
                "All souls should be unbound initially");

    /* Bind one soul */
    soul_bind(soul1, 100);
    TEST_ASSERT(soul_manager_total_unbound_energy(manager) == energy_each * 2,
                "Bound soul should not count toward unbound energy");

    /* Bind another soul */
    soul_bind(soul2, 200);
    TEST_ASSERT(soul_manager_total_unbound_energy(manager) == energy_each,
                "Only one soul should be unbound");

    /* Unbind one soul */
    soul_unbind(soul1);
    TEST_ASSERT(soul_manager_total_unbound_energy(manager) == energy_each * 2,
                "Two souls should be unbound after unbinding");

    soul_manager_destroy(manager);
    TEST_PASS();
}

/**
 * Test: soul_manager_clear
 */
void test_soul_manager_clear(void) {
    TEST_START("soul_manager_clear");

    SoulManager* manager = soul_manager_create();

    soul_manager_add(manager, soul_create(SOUL_TYPE_COMMON, 50));
    soul_manager_add(manager, soul_create(SOUL_TYPE_WARRIOR, 60));
    soul_manager_add(manager, soul_create(SOUL_TYPE_MAGE, 70));

    TEST_ASSERT(soul_manager_count(manager) == 3, "Should have 3 souls");

    soul_manager_clear(manager);
    TEST_ASSERT(soul_manager_count(manager) == 0, "Manager should be empty after clear");

    /* Test clearing empty manager */
    soul_manager_clear(manager);
    TEST_ASSERT(soul_manager_count(manager) == 0, "Clearing empty manager should work");

    /* Test NULL manager */
    soul_manager_clear(NULL); /* Should not crash */

    soul_manager_destroy(manager);
    TEST_PASS();
}

/**
 * Test: soul_manager_get_filtered
 */
void test_soul_manager_get_filtered(void) {
    TEST_START("soul_manager_get_filtered");

    SoulManager* manager = soul_manager_create();

    soul_manager_add(manager, soul_create(SOUL_TYPE_COMMON, 30));
    soul_manager_add(manager, soul_create(SOUL_TYPE_COMMON, 70));
    soul_manager_add(manager, soul_create(SOUL_TYPE_WARRIOR, 50));
    soul_manager_add(manager, soul_create(SOUL_TYPE_MAGE, 90));

    /* Filter by type */
    SoulFilter filter = soul_filter_by_type(SOUL_TYPE_COMMON);
    size_t count;
    Soul** filtered = soul_manager_get_filtered(manager, &filter, &count);
    TEST_ASSERT(filtered != NULL, "Filtered result should not be NULL");
    TEST_ASSERT(count == 2, "Should have 2 common souls");
    free(filtered);

    /* Filter by minimum quality */
    filter = soul_filter_min_quality(60);
    filtered = soul_manager_get_filtered(manager, &filter, &count);
    TEST_ASSERT(filtered != NULL, "Filtered result should not be NULL");
    TEST_ASSERT(count == 2, "Should have 2 souls with quality >= 60");
    free(filtered);

    /* Filter unbound souls */
    /* Get all souls to bind one */
    Soul** all_souls = soul_manager_get_filtered(manager, NULL, &count);
    soul_bind(all_souls[0], 100);
    free(all_souls);

    filter = soul_filter_unbound();
    filtered = soul_manager_get_filtered(manager, &filter, &count);
    TEST_ASSERT(count == 3, "Should have 3 unbound souls");
    free(filtered);

    /* No filter (get all) */
    filtered = soul_manager_get_filtered(manager, NULL, &count);
    TEST_ASSERT(count == 4, "Should get all 4 souls with NULL filter");
    free(filtered);

    soul_manager_destroy(manager);
    TEST_PASS();
}

/**
 * Test: soul_manager_sort
 */
void test_soul_manager_sort(void) {
    TEST_START("soul_manager_sort");

    SoulManager* manager = soul_manager_create();

    Soul* soul1 = soul_create(SOUL_TYPE_MAGE, 90);
    Soul* soul2 = soul_create(SOUL_TYPE_COMMON, 30);
    Soul* soul3 = soul_create(SOUL_TYPE_WARRIOR, 60);

    soul_manager_add(manager, soul1);
    soul_manager_add(manager, soul2);
    soul_manager_add(manager, soul3);

    /* Sort by quality ascending */
    soul_manager_sort(manager, SOUL_SORT_QUALITY);
    size_t count;
    Soul** sorted = soul_manager_get_filtered(manager, NULL, &count);
    TEST_ASSERT(sorted[0]->quality == 30, "First soul should have quality 30");
    TEST_ASSERT(sorted[1]->quality == 60, "Second soul should have quality 60");
    TEST_ASSERT(sorted[2]->quality == 90, "Third soul should have quality 90");
    free(sorted);

    /* Sort by quality descending */
    soul_manager_sort(manager, SOUL_SORT_QUALITY_DESC);
    sorted = soul_manager_get_filtered(manager, NULL, &count);
    TEST_ASSERT(sorted[0]->quality == 90, "First soul should have quality 90");
    TEST_ASSERT(sorted[1]->quality == 60, "Second soul should have quality 60");
    TEST_ASSERT(sorted[2]->quality == 30, "Third soul should have quality 30");
    free(sorted);

    /* Sort by type */
    soul_manager_sort(manager, SOUL_SORT_TYPE);
    sorted = soul_manager_get_filtered(manager, NULL, &count);
    TEST_ASSERT(sorted[0]->type == SOUL_TYPE_COMMON, "First should be COMMON");
    TEST_ASSERT(sorted[1]->type == SOUL_TYPE_WARRIOR, "Second should be WARRIOR");
    TEST_ASSERT(sorted[2]->type == SOUL_TYPE_MAGE, "Third should be MAGE");
    free(sorted);

    soul_manager_destroy(manager);
    TEST_PASS();
}

/**
 * Test: Large number of souls (stress test)
 */
void test_soul_manager_large_scale(void) {
    TEST_START("soul_manager_large_scale");

    SoulManager* manager = soul_manager_create();

    /* Add 200 souls (more than initial capacity of 100) */
    for (int i = 0; i < 200; i++) {
        SoulType type = (SoulType)(i % SOUL_TYPE_COUNT);
        SoulQuality quality = (i % 100);
        Soul* soul = soul_create(type, quality);
        TEST_ASSERT(soul_manager_add(manager, soul) == true, "Adding soul should succeed");
    }

    TEST_ASSERT(soul_manager_count(manager) == 200, "Should have 200 souls");

    /* Verify we can still operate on the collection */
    uint32_t total_energy = soul_manager_total_energy(manager);
    TEST_ASSERT(total_energy > 0, "Total energy should be positive");

    soul_manager_destroy(manager);
    TEST_PASS();
}

/**
 * Main test runner
 */
int main(void) {
    printf("=== Soul Manager Tests ===\n\n");

    test_soul_manager_create_destroy();
    test_soul_manager_add();
    test_soul_manager_get();
    test_soul_manager_remove();
    test_soul_manager_count_by_type();
    test_soul_manager_total_energy();
    test_soul_manager_total_unbound_energy();
    test_soul_manager_clear();
    test_soul_manager_get_filtered();
    test_soul_manager_sort();
    test_soul_manager_large_scale();

    printf("\n=== Test Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    if (tests_passed == tests_run) {
        printf("\nAll tests PASSED!\n");
        return 0;
    } else {
        printf("\nSome tests FAILED!\n");
        return 1;
    }
}

/**
 * Hash Table Tests
 */

#include "utils/hash_table.h"
#include "utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Test results */
static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) \
    printf("Running test: %s\n", #name); \
    tests_run++; \
    if (test_##name()) { \
        tests_passed++; \
        printf("  ✓ PASSED\n"); \
    } else { \
        printf("  ✗ FAILED\n"); \
    }

/* Test: Create and destroy */
static bool test_create_destroy(void) {
    HashTable* table = hash_table_create(10);
    if (!table) return false;

    hash_table_destroy(table);
    return true;
}

/* Test: Put and get single entry */
static bool test_put_get_single(void) {
    HashTable* table = hash_table_create(10);
    if (!table) return false;

    int value = 42;
    bool result = hash_table_put(table, "test", &value);
    if (!result) {
        hash_table_destroy(table);
        return false;
    }

    int* retrieved = hash_table_get(table, "test");
    if (!retrieved || *retrieved != 42) {
        hash_table_destroy(table);
        return false;
    }

    hash_table_destroy(table);
    return true;
}

/* Test: Multiple entries */
static bool test_multiple_entries(void) {
    HashTable* table = hash_table_create(10);
    if (!table) return false;

    int values[5] = {1, 2, 3, 4, 5};
    const char* keys[5] = {"one", "two", "three", "four", "five"};

    /* Insert all */
    for (int i = 0; i < 5; i++) {
        if (!hash_table_put(table, keys[i], &values[i])) {
            hash_table_destroy(table);
            return false;
        }
    }

    /* Verify size */
    if (hash_table_size(table) != 5) {
        hash_table_destroy(table);
        return false;
    }

    /* Retrieve all */
    for (int i = 0; i < 5; i++) {
        int* value = hash_table_get(table, keys[i]);
        if (!value || *value != values[i]) {
            hash_table_destroy(table);
            return false;
        }
    }

    hash_table_destroy(table);
    return true;
}

/* Test: Update existing key */
static bool test_update_key(void) {
    HashTable* table = hash_table_create(10);
    if (!table) return false;

    int value1 = 10;
    int value2 = 20;

    hash_table_put(table, "key", &value1);
    hash_table_put(table, "key", &value2);  /* Update */

    int* retrieved = hash_table_get(table, "key");
    if (!retrieved || *retrieved != 20) {
        hash_table_destroy(table);
        return false;
    }

    /* Size should still be 1 */
    if (hash_table_size(table) != 1) {
        hash_table_destroy(table);
        return false;
    }

    hash_table_destroy(table);
    return true;
}

/* Test: Contains check */
static bool test_contains(void) {
    HashTable* table = hash_table_create(10);
    if (!table) return false;

    int value = 42;
    hash_table_put(table, "exists", &value);

    if (!hash_table_contains(table, "exists")) {
        hash_table_destroy(table);
        return false;
    }

    if (hash_table_contains(table, "not_exists")) {
        hash_table_destroy(table);
        return false;
    }

    hash_table_destroy(table);
    return true;
}

/* Test: Remove entry */
static bool test_remove(void) {
    HashTable* table = hash_table_create(10);
    if (!table) return false;

    int value = 42;
    hash_table_put(table, "test", &value);

    /* Remove */
    int* removed = hash_table_remove(table, "test");
    if (!removed || *removed != 42) {
        hash_table_destroy(table);
        return false;
    }

    /* Should not exist */
    if (hash_table_contains(table, "test")) {
        hash_table_destroy(table);
        return false;
    }

    /* Size should be 0 */
    if (hash_table_size(table) != 0) {
        hash_table_destroy(table);
        return false;
    }

    hash_table_destroy(table);
    return true;
}

/* Test: Clear */
static bool test_clear(void) {
    HashTable* table = hash_table_create(10);
    if (!table) return false;

    int values[3] = {1, 2, 3};
    hash_table_put(table, "a", &values[0]);
    hash_table_put(table, "b", &values[1]);
    hash_table_put(table, "c", &values[2]);

    hash_table_clear(table);

    if (hash_table_size(table) != 0) {
        hash_table_destroy(table);
        return false;
    }

    if (hash_table_contains(table, "a")) {
        hash_table_destroy(table);
        return false;
    }

    hash_table_destroy(table);
    return true;
}

/* Test: Resize on many insertions */
static bool test_resize(void) {
    HashTable* table = hash_table_create(4);  /* Small initial size */
    if (!table) return false;

    /* Insert many entries to trigger resize */
    int values[20];
    char keys[20][10];

    for (int i = 0; i < 20; i++) {
        values[i] = i;
        snprintf(keys[i], sizeof(keys[i]), "key%d", i);
        if (!hash_table_put(table, keys[i], &values[i])) {
            hash_table_destroy(table);
            return false;
        }
    }

    /* Verify all entries exist */
    for (int i = 0; i < 20; i++) {
        int* value = hash_table_get(table, keys[i]);
        if (!value || *value != i) {
            hash_table_destroy(table);
            return false;
        }
    }

    /* Capacity should have grown */
    if (hash_table_capacity(table) <= 4) {
        hash_table_destroy(table);
        return false;
    }

    hash_table_destroy(table);
    return true;
}

/* Test: Iteration */
static int g_iteration_count;
static void count_iterator(const char* key, void* value, void* userdata) {
    (void)key;
    (void)value;
    (void)userdata;
    g_iteration_count++;
}

static bool test_foreach(void) {
    HashTable* table = hash_table_create(10);
    if (!table) return false;

    int values[5] = {1, 2, 3, 4, 5};
    hash_table_put(table, "a", &values[0]);
    hash_table_put(table, "b", &values[1]);
    hash_table_put(table, "c", &values[2]);
    hash_table_put(table, "d", &values[3]);
    hash_table_put(table, "e", &values[4]);

    g_iteration_count = 0;
    hash_table_foreach(table, count_iterator, NULL);

    if (g_iteration_count != 5) {
        hash_table_destroy(table);
        return false;
    }

    hash_table_destroy(table);
    return true;
}

/* Test: Collision handling */
static bool test_collisions(void) {
    HashTable* table = hash_table_create(10);
    if (!table) return false;

    /* These keys might hash to same slot - test linear probing */
    int values[10];
    char keys[10][20];

    for (int i = 0; i < 10; i++) {
        values[i] = i * 100;
        snprintf(keys[i], sizeof(keys[i]), "collision_%d", i);
        if (!hash_table_put(table, keys[i], &values[i])) {
            hash_table_destroy(table);
            return false;
        }
    }

    /* Verify all can be retrieved */
    for (int i = 0; i < 10; i++) {
        int* value = hash_table_get(table, keys[i]);
        if (!value || *value != i * 100) {
            hash_table_destroy(table);
            return false;
        }
    }

    hash_table_destroy(table);
    return true;
}

int main(void) {
    /* Initialize logger for tests */
    logger_init("test_hash_table.log", LOG_LEVEL_DEBUG);

    printf("=====================================\n");
    printf("Hash Table Tests\n");
    printf("=====================================\n\n");

    TEST(create_destroy);
    TEST(put_get_single);
    TEST(multiple_entries);
    TEST(update_key);
    TEST(contains);
    TEST(remove);
    TEST(clear);
    TEST(resize);
    TEST(foreach);
    TEST(collisions);

    printf("\n=====================================\n");
    printf("Results: %d/%d tests passed\n", tests_passed, tests_run);
    printf("=====================================\n");

    logger_shutdown();

    return (tests_passed == tests_run) ? EXIT_SUCCESS : EXIT_FAILURE;
}

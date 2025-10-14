/**
 * Memory Pool System Tests
 */

#include "core/memory.h"
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

/* Test: Create and destroy pool */
static bool test_create_destroy(void) {
    MemoryPool* pool = pool_create(64, 10);
    if (!pool) return false;

    pool_destroy(pool);
    return true;
}

/* Test: Allocate and free single block */
static bool test_alloc_free_single(void) {
    MemoryPool* pool = pool_create(64, 10);
    if (!pool) return false;

    void* ptr = pool_alloc(pool);
    if (!ptr) {
        pool_destroy(pool);
        return false;
    }

    pool_free(pool, ptr);
    pool_destroy(pool);
    return true;
}

/* Test: Allocate multiple blocks */
static bool test_alloc_multiple(void) {
    MemoryPool* pool = pool_create(64, 5);
    if (!pool) return false;

    void* ptrs[5];
    for (int i = 0; i < 5; i++) {
        ptrs[i] = pool_alloc(pool);
        if (!ptrs[i]) {
            pool_destroy(pool);
            return false;
        }
    }

    /* Pool should be exhausted */
    void* extra = pool_alloc(pool);
    if (extra != NULL) {
        pool_destroy(pool);
        return false;
    }

    /* Free all */
    for (int i = 0; i < 5; i++) {
        pool_free(pool, ptrs[i]);
    }

    pool_destroy(pool);
    return true;
}

/* Test: Pool reset */
static bool test_reset(void) {
    MemoryPool* pool = pool_create(64, 5);
    if (!pool) return false;

    /* Allocate some blocks */
    void* ptrs[3];
    for (int i = 0; i < 3; i++) {
        ptrs[i] = pool_alloc(pool);
        if (!ptrs[i]) {
            pool_destroy(pool);
            return false;
        }
    }

    /* Reset pool */
    pool_reset(pool);

    /* Should be able to allocate again */
    void* ptr = pool_alloc(pool);
    if (!ptr) {
        pool_destroy(pool);
        return false;
    }

    pool_free(pool, ptr);
    pool_destroy(pool);
    return true;
}

/* Test: Statistics */
static bool test_statistics(void) {
    MemoryPool* pool = pool_create(64, 10);
    if (!pool) return false;

    /* Allocate some blocks */
    void* ptrs[5];
    for (int i = 0; i < 5; i++) {
        ptrs[i] = pool_alloc(pool);
    }

    /* Check stats */
    MemoryStats stats;
    pool_get_stats(pool, &stats);

    if (stats.allocated_blocks != 5) {
        pool_destroy(pool);
        return false;
    }

    if (stats.total_allocs != 5) {
        pool_destroy(pool);
        return false;
    }

    if (stats.peak_usage != 5) {
        pool_destroy(pool);
        return false;
    }

    /* Free some */
    for (int i = 0; i < 3; i++) {
        pool_free(pool, ptrs[i]);
    }

    pool_get_stats(pool, &stats);

    if (stats.allocated_blocks != 2) {
        pool_destroy(pool);
        return false;
    }

    if (stats.total_frees != 3) {
        pool_destroy(pool);
        return false;
    }

    /* Peak should still be 5 */
    if (stats.peak_usage != 5) {
        pool_destroy(pool);
        return false;
    }

    /* Free remaining */
    for (int i = 3; i < 5; i++) {
        pool_free(pool, ptrs[i]);
    }

    pool_destroy(pool);
    return true;
}

/* Test: Leak detection */
static bool test_leak_detection(void) {
    MemoryPool* pool = pool_create(64, 10);
    if (!pool) return false;

    /* Allocate some blocks */
    void* ptrs[3];
    for (int i = 0; i < 3; i++) {
        ptrs[i] = pool_alloc(pool);
    }

    /* Check for leaks (should find 3) */
    bool has_leaks = pool_check_leaks(pool);
    if (!has_leaks) {
        pool_destroy(pool);
        return false;
    }

    /* Free all */
    for (int i = 0; i < 3; i++) {
        pool_free(pool, ptrs[i]);
    }

    /* Check again (should find 0) */
    has_leaks = pool_check_leaks(pool);
    if (has_leaks) {
        pool_destroy(pool);
        return false;
    }

    pool_destroy(pool);
    return true;
}

/* Test: Free NULL is safe */
static bool test_free_null(void) {
    MemoryPool* pool = pool_create(64, 10);
    if (!pool) return false;

    /* Should not crash */
    pool_free(pool, NULL);

    pool_destroy(pool);
    return true;
}

/* Test: Data integrity */
static bool test_data_integrity(void) {
    MemoryPool* pool = pool_create(256, 10);
    if (!pool) return false;

    void* ptr = pool_alloc(pool);
    if (!ptr) {
        pool_destroy(pool);
        return false;
    }

    /* Write data */
    const char* test_str = "Hello, memory pool!";
    strcpy(ptr, test_str);

    /* Verify data */
    if (strcmp(ptr, test_str) != 0) {
        pool_free(pool, ptr);
        pool_destroy(pool);
        return false;
    }

    pool_free(pool, ptr);
    pool_destroy(pool);
    return true;
}

int main(void) {
    /* Initialize logger for tests */
    logger_init("test_memory.log", LOG_LEVEL_DEBUG);

    printf("=====================================\n");
    printf("Memory Pool System Tests\n");
    printf("=====================================\n\n");

    TEST(create_destroy);
    TEST(alloc_free_single);
    TEST(alloc_multiple);
    TEST(reset);
    TEST(statistics);
    TEST(leak_detection);
    TEST(free_null);
    TEST(data_integrity);

    printf("\n=====================================\n");
    printf("Results: %d/%d tests passed\n", tests_passed, tests_run);
    printf("=====================================\n");

    logger_shutdown();

    return (tests_passed == tests_run) ? EXIT_SUCCESS : EXIT_FAILURE;
}

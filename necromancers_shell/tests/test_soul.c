#include "../src/game/souls/soul.h"
#include <stdio.h>
#include <string.h>
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
 * Test: soul_create basic functionality
 */
void test_soul_create_basic(void) {
    TEST_START("soul_create_basic");

    Soul* soul = soul_create(SOUL_TYPE_COMMON, 50);
    TEST_ASSERT(soul != NULL, "soul_create should not return NULL");
    TEST_ASSERT(soul->type == SOUL_TYPE_COMMON, "Soul type should be COMMON");
    TEST_ASSERT(soul->quality == 50, "Soul quality should be 50");
    TEST_ASSERT(soul->energy > 0, "Soul energy should be positive");
    TEST_ASSERT(soul->bound == false, "Soul should not be bound initially");
    TEST_ASSERT(soul->bound_minion_id == 0, "Bound minion ID should be 0");
    TEST_ASSERT(strlen(soul->memories) > 0, "Soul should have memories");

    soul_destroy(soul);
    TEST_PASS();
}

/**
 * Test: soul_create with invalid type
 */
void test_soul_create_invalid_type(void) {
    TEST_START("soul_create_invalid_type");

    Soul* soul = soul_create(SOUL_TYPE_COUNT, 50);
    TEST_ASSERT(soul == NULL, "soul_create should return NULL for invalid type");

    TEST_PASS();
}

/**
 * Test: soul_create with quality clamping
 */
void test_soul_create_quality_clamping(void) {
    TEST_START("soul_create_quality_clamping");

    Soul* soul = soul_create(SOUL_TYPE_WARRIOR, 150);
    TEST_ASSERT(soul != NULL, "soul_create should handle quality > 100");
    TEST_ASSERT(soul->quality == 100, "Quality should be clamped to 100");

    soul_destroy(soul);
    TEST_PASS();
}

/**
 * Test: soul_destroy with NULL
 */
void test_soul_destroy_null(void) {
    TEST_START("soul_destroy_null");

    soul_destroy(NULL);
    /* Should not crash */

    TEST_PASS();
}

/**
 * Test: soul_type_name for all types
 */
void test_soul_type_name(void) {
    TEST_START("soul_type_name");

    TEST_ASSERT(strcmp(soul_type_name(SOUL_TYPE_COMMON), "Common") == 0,
                "COMMON type name should be 'Common'");
    TEST_ASSERT(strcmp(soul_type_name(SOUL_TYPE_WARRIOR), "Warrior") == 0,
                "WARRIOR type name should be 'Warrior'");
    TEST_ASSERT(strcmp(soul_type_name(SOUL_TYPE_MAGE), "Mage") == 0,
                "MAGE type name should be 'Mage'");
    TEST_ASSERT(strcmp(soul_type_name(SOUL_TYPE_INNOCENT), "Innocent") == 0,
                "INNOCENT type name should be 'Innocent'");
    TEST_ASSERT(strcmp(soul_type_name(SOUL_TYPE_CORRUPTED), "Corrupted") == 0,
                "CORRUPTED type name should be 'Corrupted'");
    TEST_ASSERT(strcmp(soul_type_name(SOUL_TYPE_ANCIENT), "Ancient") == 0,
                "ANCIENT type name should be 'Ancient'");
    TEST_ASSERT(strcmp(soul_type_name(SOUL_TYPE_COUNT), "Unknown") == 0,
                "Invalid type should return 'Unknown'");

    TEST_PASS();
}

/**
 * Test: soul_calculate_energy for different types and qualities
 */
void test_soul_calculate_energy(void) {
    TEST_START("soul_calculate_energy");

    /* Common soul: 10-20 energy range */
    uint32_t energy_0 = soul_calculate_energy(SOUL_TYPE_COMMON, 0);
    uint32_t energy_50 = soul_calculate_energy(SOUL_TYPE_COMMON, 50);
    uint32_t energy_100 = soul_calculate_energy(SOUL_TYPE_COMMON, 100);

    TEST_ASSERT(energy_0 == 10, "Common soul at quality 0 should be 10 energy");
    TEST_ASSERT(energy_50 == 15, "Common soul at quality 50 should be 15 energy");
    TEST_ASSERT(energy_100 == 20, "Common soul at quality 100 should be 20 energy");
    TEST_ASSERT(energy_0 < energy_50 && energy_50 < energy_100,
                "Energy should increase with quality");

    /* Warrior soul: 20-40 energy range */
    uint32_t warrior_energy = soul_calculate_energy(SOUL_TYPE_WARRIOR, 50);
    TEST_ASSERT(warrior_energy == 30, "Warrior soul at quality 50 should be 30 energy");

    /* Ancient soul: 50-100 energy range */
    uint32_t ancient_energy_0 = soul_calculate_energy(SOUL_TYPE_ANCIENT, 0);
    uint32_t ancient_energy_100 = soul_calculate_energy(SOUL_TYPE_ANCIENT, 100);
    TEST_ASSERT(ancient_energy_0 == 50, "Ancient soul at quality 0 should be 50 energy");
    TEST_ASSERT(ancient_energy_100 == 100, "Ancient soul at quality 100 should be 100 energy");

    TEST_PASS();
}

/**
 * Test: soul_bind functionality
 */
void test_soul_bind(void) {
    TEST_START("soul_bind");

    Soul* soul = soul_create(SOUL_TYPE_WARRIOR, 75);
    TEST_ASSERT(soul != NULL, "Soul creation should succeed");

    /* Test successful bind */
    bool result = soul_bind(soul, 123);
    TEST_ASSERT(result == true, "soul_bind should return true on success");
    TEST_ASSERT(soul->bound == true, "Soul should be marked as bound");
    TEST_ASSERT(soul->bound_minion_id == 123, "Bound minion ID should be 123");

    /* Test binding already bound soul */
    result = soul_bind(soul, 456);
    TEST_ASSERT(result == false, "soul_bind should fail on already bound soul");
    TEST_ASSERT(soul->bound_minion_id == 123, "Minion ID should not change");

    /* Test NULL soul */
    result = soul_bind(NULL, 789);
    TEST_ASSERT(result == false, "soul_bind should return false for NULL soul");

    soul_destroy(soul);
    TEST_PASS();
}

/**
 * Test: soul_unbind functionality
 */
void test_soul_unbind(void) {
    TEST_START("soul_unbind");

    Soul* soul = soul_create(SOUL_TYPE_MAGE, 60);
    TEST_ASSERT(soul != NULL, "Soul creation should succeed");

    /* Test unbinding unbound soul */
    bool result = soul_unbind(soul);
    TEST_ASSERT(result == false, "soul_unbind should fail on unbound soul");

    /* Bind the soul */
    soul_bind(soul, 999);

    /* Test successful unbind */
    result = soul_unbind(soul);
    TEST_ASSERT(result == true, "soul_unbind should return true on success");
    TEST_ASSERT(soul->bound == false, "Soul should be marked as unbound");
    TEST_ASSERT(soul->bound_minion_id == 0, "Bound minion ID should be reset to 0");

    /* Test NULL soul */
    result = soul_unbind(NULL);
    TEST_ASSERT(result == false, "soul_unbind should return false for NULL soul");

    soul_destroy(soul);
    TEST_PASS();
}

/**
 * Test: soul_generate_memories
 */
void test_soul_generate_memories(void) {
    TEST_START("soul_generate_memories");

    Soul* soul = soul_create(SOUL_TYPE_WARRIOR, 80);
    TEST_ASSERT(soul != NULL, "Soul creation should succeed");
    TEST_ASSERT(strlen(soul->memories) > 0, "Soul should have memories");

    /* Test that different types generate different memories */
    Soul* common = soul_create(SOUL_TYPE_COMMON, 50);
    Soul* warrior = soul_create(SOUL_TYPE_WARRIOR, 50);
    TEST_ASSERT(strcmp(common->memories, warrior->memories) != 0,
                "Different soul types should have different memories");

    soul_destroy(soul);
    soul_destroy(common);
    soul_destroy(warrior);
    TEST_PASS();
}

/**
 * Test: soul_get_description
 */
void test_soul_get_description(void) {
    TEST_START("soul_get_description");

    Soul* soul = soul_create(SOUL_TYPE_ANCIENT, 95);
    TEST_ASSERT(soul != NULL, "Soul creation should succeed");

    char buffer[512];
    int written = soul_get_description(soul, buffer, sizeof(buffer));
    TEST_ASSERT(written > 0, "soul_get_description should write characters");
    TEST_ASSERT(strstr(buffer, "Ancient") != NULL, "Description should contain type");
    TEST_ASSERT(strstr(buffer, "95%") != NULL, "Description should contain quality");

    /* Test bound soul description */
    soul_bind(soul, 42);
    written = soul_get_description(soul, buffer, sizeof(buffer));
    TEST_ASSERT(strstr(buffer, "BOUND") != NULL, "Description should show bound status");
    TEST_ASSERT(strstr(buffer, "42") != NULL, "Description should show minion ID");

    /* Test NULL inputs */
    written = soul_get_description(NULL, buffer, sizeof(buffer));
    TEST_ASSERT(written == 0, "soul_get_description should return 0 for NULL soul");

    written = soul_get_description(soul, NULL, sizeof(buffer));
    TEST_ASSERT(written == 0, "soul_get_description should return 0 for NULL buffer");

    soul_destroy(soul);
    TEST_PASS();
}

/**
 * Test: Multiple soul creation with unique IDs
 */
void test_soul_unique_ids(void) {
    TEST_START("soul_unique_ids");

    Soul* soul1 = soul_create(SOUL_TYPE_COMMON, 50);
    Soul* soul2 = soul_create(SOUL_TYPE_WARRIOR, 60);
    Soul* soul3 = soul_create(SOUL_TYPE_MAGE, 70);

    TEST_ASSERT(soul1->id != soul2->id, "Soul IDs should be unique");
    TEST_ASSERT(soul2->id != soul3->id, "Soul IDs should be unique");
    TEST_ASSERT(soul1->id != soul3->id, "Soul IDs should be unique");

    soul_destroy(soul1);
    soul_destroy(soul2);
    soul_destroy(soul3);
    TEST_PASS();
}

/**
 * Main test runner
 */
int main(void) {
    printf("=== Soul System Tests ===\n\n");

    test_soul_create_basic();
    test_soul_create_invalid_type();
    test_soul_create_quality_clamping();
    test_soul_destroy_null();
    test_soul_type_name();
    test_soul_calculate_energy();
    test_soul_bind();
    test_soul_unbind();
    test_soul_generate_memories();
    test_soul_get_description();
    test_soul_unique_ids();

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

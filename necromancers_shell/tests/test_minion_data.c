#define _POSIX_C_SOURCE 200809L

#include "../src/data/minion_data.h"
#include "../src/data/data_loader.h"
#include "../src/game/minions/minion.h"
#include "../src/utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/**
 * @file test_minion_data.c
 * @brief Unit tests for minion data loading system
 */

/* Test counter */
static int tests_run = 0;
static int tests_passed = 0;

/* Test helper macros */
#define TEST(name) \
    do { \
        printf("Running %s... ", name); \
        fflush(stdout); \
        tests_run++; \
    } while (0)

#define PASS() \
    do { \
        printf("PASS\n"); \
        tests_passed++; \
    } while (0)

#define FAIL(msg) \
    do { \
        printf("FAIL: %s\n", msg); \
        return; \
    } while (0)

#define ASSERT(cond, msg) \
    if (!(cond)) FAIL(msg)

/**
 * Test 1: Parse minion type from string
 */
static void test_parse_minion_type(void) {
    TEST("test_parse_minion_type");

    ASSERT(minion_data_parse_type("zombie") == MINION_TYPE_ZOMBIE, "zombie type mismatch");
    ASSERT(minion_data_parse_type("skeleton") == MINION_TYPE_SKELETON, "skeleton type mismatch");
    ASSERT(minion_data_parse_type("ghoul") == MINION_TYPE_GHOUL, "ghoul type mismatch");
    ASSERT(minion_data_parse_type("wraith") == MINION_TYPE_WRAITH, "wraith type mismatch");
    ASSERT(minion_data_parse_type("wight") == MINION_TYPE_WIGHT, "wight type mismatch");
    ASSERT(minion_data_parse_type("revenant") == MINION_TYPE_REVENANT, "revenant type mismatch");

    /* Test invalid type */
    ASSERT(minion_data_parse_type("invalid") == MINION_TYPE_COUNT, "invalid type should return COUNT");

    PASS();
}

/**
 * Test 2: Create minion definition from data section
 */
static void test_create_single_definition(void) {
    TEST("test_create_single_definition");

    /* Create minimal data file */
    const char* test_data =
        "[MINION:zombie]\n"
        "name = Test Zombie\n"
        "description = A test zombie minion\n"
        "base_health = 100\n"
        "base_attack = 15\n"
        "base_defense = 20\n"
        "base_speed = 5\n"
        "base_loyalty = 90\n"
        "raise_cost = 50\n"
        "unlock_level = 0\n"
        "role = tank\n"
        "specialization = melee_defense\n";

    /* Write to temporary file */
    FILE* f = fopen("/tmp/test_minion.dat", "w");
    ASSERT(f != NULL, "Failed to create temp file");
    fprintf(f, "%s", test_data);
    fclose(f);

    /* Load data file */
    DataFile* data_file = data_file_load("/tmp/test_minion.dat");
    ASSERT(data_file != NULL, "Failed to load data file");

    /* Get sections */
    size_t count = 0;
    const DataSection** sections = data_file_get_sections(data_file, "MINION", &count);
    ASSERT(sections != NULL, "Failed to get sections");
    ASSERT(count == 1, "Expected 1 section");

    /* Create definition from section */
    MinionTypeDefinition def;
    int result = minion_data_create_definition(sections[0], &def);
    ASSERT(result == 0, "Failed to create definition");

    /* Verify properties */
    ASSERT(def.type == MINION_TYPE_ZOMBIE, "Type mismatch");
    ASSERT(strcmp(def.name, "Test Zombie") == 0, "Name mismatch");
    ASSERT(strstr(def.description, "test zombie") != NULL, "Description mismatch");
    ASSERT(def.base_health == 100, "Health mismatch");
    ASSERT(def.base_attack == 15, "Attack mismatch");
    ASSERT(def.base_defense == 20, "Defense mismatch");
    ASSERT(def.base_speed == 5, "Speed mismatch");
    ASSERT(def.base_loyalty == 90, "Loyalty mismatch");
    ASSERT(def.raise_cost == 50, "Raise cost mismatch");
    ASSERT(def.unlock_level == 0, "Unlock level mismatch");
    ASSERT(strcmp(def.role, "tank") == 0, "Role mismatch");
    ASSERT(strcmp(def.specialization, "melee_defense") == 0, "Specialization mismatch");

    /* Cleanup */
    free((void*)sections);
    data_file_destroy(data_file);

    PASS();
}

/**
 * Test 3: Load all minion definitions
 */
static void test_load_all_definitions(void) {
    TEST("test_load_all_definitions");

    /* Create data file with multiple minions */
    const char* test_data =
        "[MINION:zombie]\n"
        "name = Zombie\n"
        "description = Shambling corpse\n"
        "base_health = 100\n"
        "base_attack = 15\n"
        "base_defense = 20\n"
        "base_speed = 5\n"
        "base_loyalty = 90\n"
        "raise_cost = 50\n"
        "unlock_level = 0\n"
        "role = tank\n"
        "specialization = melee_defense\n"
        "\n"
        "[MINION:skeleton]\n"
        "name = Skeleton\n"
        "description = Animated bones\n"
        "base_health = 50\n"
        "base_attack = 25\n"
        "base_defense = 10\n"
        "base_speed = 15\n"
        "base_loyalty = 85\n"
        "raise_cost = 75\n"
        "unlock_level = 0\n"
        "role = scout\n"
        "specialization = speed_attack\n";

    FILE* f = fopen("/tmp/test_minions.dat", "w");
    ASSERT(f != NULL, "Failed to create temp file");
    fprintf(f, "%s", test_data);
    fclose(f);

    DataFile* data_file = data_file_load("/tmp/test_minions.dat");
    ASSERT(data_file != NULL, "Failed to load data file");

    /* Load all definitions */
    MinionTypeDefinition definitions[MINION_TYPE_COUNT];
    memset(definitions, 0, sizeof(definitions));

    size_t loaded = minion_data_load_definitions(data_file, definitions);
    ASSERT(loaded == 2, "Expected 2 definitions loaded");

    /* Verify zombie */
    ASSERT(definitions[MINION_TYPE_ZOMBIE].type == MINION_TYPE_ZOMBIE, "Zombie type mismatch");
    ASSERT(strcmp(definitions[MINION_TYPE_ZOMBIE].name, "Zombie") == 0, "Zombie name mismatch");
    ASSERT(definitions[MINION_TYPE_ZOMBIE].base_health == 100, "Zombie health mismatch");

    /* Verify skeleton */
    ASSERT(definitions[MINION_TYPE_SKELETON].type == MINION_TYPE_SKELETON, "Skeleton type mismatch");
    ASSERT(strcmp(definitions[MINION_TYPE_SKELETON].name, "Skeleton") == 0, "Skeleton name mismatch");
    ASSERT(definitions[MINION_TYPE_SKELETON].base_health == 50, "Skeleton health mismatch");

    data_file_destroy(data_file);

    PASS();
}

/**
 * Test 4: Apply base stats from definition
 */
static void test_apply_base_stats(void) {
    TEST("test_apply_base_stats");

    /* Create a definition */
    MinionTypeDefinition def;
    memset(&def, 0, sizeof(def));
    def.type = MINION_TYPE_ZOMBIE;
    def.base_health = 100;
    def.base_attack = 15;
    def.base_defense = 20;
    def.base_speed = 5;
    def.base_loyalty = 90;

    /* Apply to stats */
    MinionStats stats;
    memset(&stats, 0, sizeof(stats));
    minion_data_apply_base_stats(&def, &stats);

    /* Verify stats */
    ASSERT(stats.health == 100, "Health mismatch");
    ASSERT(stats.health_max == 100, "Health max mismatch");
    ASSERT(stats.attack == 15, "Attack mismatch");
    ASSERT(stats.defense == 20, "Defense mismatch");
    ASSERT(stats.speed == 5, "Speed mismatch");
    ASSERT(stats.loyalty == 90, "Loyalty mismatch");

    PASS();
}

/**
 * Test 5: Load actual minions.dat file
 */
static void test_load_real_minions_file(void) {
    TEST("test_load_real_minions_file");

    /* Try to load the actual game data file */
    DataFile* data_file = data_file_load("data/minions.dat");
    if (!data_file) {
        printf("SKIP (data/minions.dat not found)\n");
        return;
    }

    /* Load all definitions */
    MinionTypeDefinition definitions[MINION_TYPE_COUNT];
    memset(definitions, 0, sizeof(definitions));

    size_t loaded = minion_data_load_definitions(data_file, definitions);
    ASSERT(loaded == 6, "Expected 6 minion types loaded");

    printf("Loaded %zu minion types... ", loaded);

    /* Verify each type has valid data */
    for (size_t i = 0; i < MINION_TYPE_COUNT; i++) {
        if (definitions[i].type == i) {
            ASSERT(definitions[i].base_health > 0, "Health should be > 0");
            ASSERT(definitions[i].base_attack > 0, "Attack should be > 0");
            ASSERT(definitions[i].raise_cost > 0, "Raise cost should be > 0");
            ASSERT(strlen(definitions[i].name) > 0, "Name should not be empty");
        }
    }

    /* Verify specific types */
    ASSERT(definitions[MINION_TYPE_ZOMBIE].base_health == 100, "Zombie health should be 100");
    ASSERT(definitions[MINION_TYPE_SKELETON].base_speed == 15, "Skeleton speed should be 15");
    ASSERT(definitions[MINION_TYPE_REVENANT].raise_cost == 500, "Revenant cost should be 500");

    data_file_destroy(data_file);

    PASS();
}

/**
 * Test 6: Default values
 */
static void test_default_values(void) {
    TEST("test_default_values");

    /* Minimal minion data */
    const char* test_data =
        "[MINION:zombie]\n"
        "name = Minimal Zombie\n";

    FILE* f = fopen("/tmp/test_defaults.dat", "w");
    ASSERT(f != NULL, "Failed to create temp file");
    fprintf(f, "%s", test_data);
    fclose(f);

    DataFile* data_file = data_file_load("/tmp/test_defaults.dat");
    ASSERT(data_file != NULL, "Failed to load data file");

    size_t count = 0;
    const DataSection** sections = data_file_get_sections(data_file, "MINION", &count);
    ASSERT(sections != NULL && count == 1, "Failed to get section");

    MinionTypeDefinition def;
    int result = minion_data_create_definition(sections[0], &def);
    ASSERT(result == 0, "Failed to create definition");

    /* Verify defaults */
    ASSERT(def.base_health == 50, "Default health should be 50");
    ASSERT(def.base_attack == 10, "Default attack should be 10");
    ASSERT(def.base_defense == 10, "Default defense should be 10");
    ASSERT(def.base_speed == 10, "Default speed should be 10");
    ASSERT(def.base_loyalty == 50, "Default loyalty should be 50");
    ASSERT(def.raise_cost == 100, "Default cost should be 100");
    ASSERT(def.unlock_level == 0, "Default unlock level should be 0");

    free((void*)sections);
    data_file_destroy(data_file);

    PASS();
}

/**
 * Test 7: Error handling - NULL parameters
 */
static void test_null_parameters(void) {
    TEST("test_null_parameters");

    MinionTypeDefinition def;

    /* Test NULL section */
    int result = minion_data_create_definition(NULL, &def);
    ASSERT(result == -1, "Should return -1 for NULL section");

    /* Test NULL definition */
    DataFile* data_file = data_file_load("/tmp/test_minion.dat");
    if (data_file) {
        size_t count = 0;
        const DataSection** sections = data_file_get_sections(data_file, "MINION", &count);
        if (sections && count > 0) {
            result = minion_data_create_definition(sections[0], NULL);
            ASSERT(result == -1, "Should return -1 for NULL definition");
            free((void*)sections);
        }
        data_file_destroy(data_file);
    }

    /* Test NULL data file */
    MinionTypeDefinition definitions[MINION_TYPE_COUNT];
    size_t loaded = minion_data_load_definitions(NULL, definitions);
    ASSERT(loaded == 0, "Should return 0 for NULL data file");

    PASS();
}

/**
 * Test 8: Memory leak check - multiple load/destroy cycles
 */
static void test_memory_leak_check(void) {
    TEST("test_memory_leak_check");

    /* Load and destroy 10 times */
    for (int i = 0; i < 10; i++) {
        DataFile* data_file = data_file_load("/tmp/test_minions.dat");
        if (data_file) {
            MinionTypeDefinition definitions[MINION_TYPE_COUNT];
            memset(definitions, 0, sizeof(definitions));
            minion_data_load_definitions(data_file, definitions);
            data_file_destroy(data_file);
        }
    }

    PASS();
}

/**
 * Main test runner
 */
int main(void) {
    printf("\n=== Minion Data Loader Tests ===\n\n");

    /* Initialize logger (quiet mode for tests) */
    logger_init("test_minion_data.log", LOG_LEVEL_ERROR);

    /* Run all tests */
    test_parse_minion_type();
    test_create_single_definition();
    test_load_all_definitions();
    test_apply_base_stats();
    test_load_real_minions_file();
    test_default_values();
    test_null_parameters();
    test_memory_leak_check();

    /* Print summary */
    printf("\n=== Test Summary ===\n");
    printf("Total:  %d\n", tests_run);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_run - tests_passed);
    printf("\n");

    logger_shutdown();

    return (tests_run == tests_passed) ? 0 : 1;
}

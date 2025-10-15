#define _POSIX_C_SOURCE 200809L

#include "../src/data/location_data.h"
#include "../src/data/data_loader.h"
#include "../src/game/world/location.h"
#include "../src/game/world/territory.h"
#include "../src/utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/**
 * @file test_location_data.c
 * @brief Unit tests for location data loading system
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

/* Helper: Simple hash function (must match location_data.c) */
static uint32_t test_hash_string_id(const char* str) {
    uint32_t hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + (uint32_t)c;
    }
    return hash;
}

/**
 * Test 1: Load single location from data section
 */
static void test_load_single_location(void) {
    TEST("test_load_single_location");

    /* Create minimal data file */
    const char* test_data =
        "[LOCATION:test_graveyard]\n"
        "name = Test Graveyard\n"
        "type = graveyard\n"
        "description = A spooky test graveyard\n"
        "corpse_count = 100\n"
        "soul_quality_avg = 50\n"
        "control_level = 75\n"
        "defense_strength = 20\n"
        "discovered = true\n";

    /* Write to temporary file */
    FILE* f = fopen("/tmp/test_location.dat", "w");
    ASSERT(f != NULL, "Failed to create temp file");
    fprintf(f, "%s", test_data);
    fclose(f);

    /* Load data file */
    DataFile* data_file = data_file_load("/tmp/test_location.dat");
    ASSERT(data_file != NULL, "Failed to load data file");

    /* Get sections */
    size_t count = 0;
    const DataSection** sections = data_file_get_sections(data_file, "LOCATION", &count);
    ASSERT(sections != NULL, "Failed to get sections");
    ASSERT(count == 1, "Expected 1 section");

    /* Create location from section */
    Location* loc = location_data_create_from_section(sections[0]);
    ASSERT(loc != NULL, "Failed to create location");

    /* Verify properties */
    ASSERT(strcmp(loc->name, "Test Graveyard") == 0, "Name mismatch");
    ASSERT(loc->type == LOCATION_TYPE_GRAVEYARD, "Type mismatch");
    ASSERT(strstr(loc->description, "spooky") != NULL, "Description mismatch");
    ASSERT(loc->corpse_count == 100, "Corpse count mismatch");
    ASSERT(loc->soul_quality_avg == 50, "Soul quality mismatch");
    ASSERT(loc->control_level == 75, "Control level mismatch");
    ASSERT(loc->defense_strength == 20, "Defense strength mismatch");
    ASSERT(loc->discovered == true, "Discovered flag mismatch");

    /* Verify ID is hash of section_id */
    uint32_t expected_id = test_hash_string_id("test_graveyard");
    ASSERT(loc->id == expected_id, "ID hash mismatch");

    /* Cleanup */
    location_destroy(loc);
    free((void*)sections);
    data_file_destroy(data_file);

    PASS();
}

/**
 * Test 2: Load multiple locations
 */
static void test_load_multiple_locations(void) {
    TEST("test_load_multiple_locations");

    /* Create data file with 3 locations */
    const char* test_data =
        "[LOCATION:graveyard_01]\n"
        "name = Graveyard One\n"
        "type = graveyard\n"
        "description = First graveyard\n"
        "\n"
        "[LOCATION:battlefield_01]\n"
        "name = Battlefield One\n"
        "type = battlefield\n"
        "description = First battlefield\n"
        "\n"
        "[LOCATION:village_01]\n"
        "name = Village One\n"
        "type = village\n"
        "description = First village\n";

    /* Write to temporary file */
    FILE* f = fopen("/tmp/test_locations.dat", "w");
    ASSERT(f != NULL, "Failed to create temp file");
    fprintf(f, "%s", test_data);
    fclose(f);

    /* Load data file */
    DataFile* data_file = data_file_load("/tmp/test_locations.dat");
    ASSERT(data_file != NULL, "Failed to load data file");

    /* Get sections */
    size_t count = 0;
    const DataSection** sections = data_file_get_sections(data_file, "LOCATION", &count);
    ASSERT(sections != NULL, "Failed to get sections");
    ASSERT(count == 3, "Expected 3 sections");

    /* Create territory manager */
    TerritoryManager* territory = territory_manager_create();
    ASSERT(territory != NULL, "Failed to create territory");

    /* Load all locations */
    size_t loaded = location_data_load_all(territory, data_file);
    ASSERT(loaded == 3, "Expected 3 locations loaded");

    /* Verify each location can be retrieved */
    uint32_t id1 = test_hash_string_id("graveyard_01");
    uint32_t id2 = test_hash_string_id("battlefield_01");
    uint32_t id3 = test_hash_string_id("village_01");

    Location* loc1 = territory_manager_get_location(territory, id1);
    Location* loc2 = territory_manager_get_location(territory, id2);
    Location* loc3 = territory_manager_get_location(territory, id3);

    ASSERT(loc1 != NULL, "Graveyard not found");
    ASSERT(loc2 != NULL, "Battlefield not found");
    ASSERT(loc3 != NULL, "Village not found");

    ASSERT(strcmp(loc1->name, "Graveyard One") == 0, "Graveyard name mismatch");
    ASSERT(strcmp(loc2->name, "Battlefield One") == 0, "Battlefield name mismatch");
    ASSERT(strcmp(loc3->name, "Village One") == 0, "Village name mismatch");

    /* Cleanup */
    free((void*)sections);
    data_file_destroy(data_file);
    territory_manager_destroy(territory);

    PASS();
}

/**
 * Test 3: Load location types
 */
static void test_location_types(void) {
    TEST("test_location_types");

    const char* test_data =
        "[LOCATION:loc_graveyard]\n"
        "name = Test\n"
        "type = graveyard\n"
        "description = Test\n"
        "\n"
        "[LOCATION:loc_battlefield]\n"
        "name = Test\n"
        "type = battlefield\n"
        "description = Test\n"
        "\n"
        "[LOCATION:loc_village]\n"
        "name = Test\n"
        "type = village\n"
        "description = Test\n"
        "\n"
        "[LOCATION:loc_crypt]\n"
        "name = Test\n"
        "type = crypt\n"
        "description = Test\n"
        "\n"
        "[LOCATION:loc_ritual]\n"
        "name = Test\n"
        "type = ritual_site\n"
        "description = Test\n";

    FILE* f = fopen("/tmp/test_types.dat", "w");
    ASSERT(f != NULL, "Failed to create temp file");
    fprintf(f, "%s", test_data);
    fclose(f);

    DataFile* data_file = data_file_load("/tmp/test_types.dat");
    ASSERT(data_file != NULL, "Failed to load data file");

    TerritoryManager* territory = territory_manager_create();
    ASSERT(territory != NULL, "Failed to create territory");

    size_t loaded = location_data_load_all(territory, data_file);
    ASSERT(loaded == 5, "Expected 5 locations loaded");

    /* Verify types */
    Location* loc1 = territory_manager_get_location(territory, test_hash_string_id("loc_graveyard"));
    Location* loc2 = territory_manager_get_location(territory, test_hash_string_id("loc_battlefield"));
    Location* loc3 = territory_manager_get_location(territory, test_hash_string_id("loc_village"));
    Location* loc4 = territory_manager_get_location(territory, test_hash_string_id("loc_crypt"));
    Location* loc5 = territory_manager_get_location(territory, test_hash_string_id("loc_ritual"));

    ASSERT(loc1->type == LOCATION_TYPE_GRAVEYARD, "Type 1 mismatch");
    ASSERT(loc2->type == LOCATION_TYPE_BATTLEFIELD, "Type 2 mismatch");
    ASSERT(loc3->type == LOCATION_TYPE_VILLAGE, "Type 3 mismatch");
    ASSERT(loc4->type == LOCATION_TYPE_CRYPT, "Type 4 mismatch");
    ASSERT(loc5->type == LOCATION_TYPE_RITUAL_SITE, "Type 5 mismatch");

    data_file_destroy(data_file);
    territory_manager_destroy(territory);

    PASS();
}

/**
 * Test 4: Build location connections
 */
static void test_location_connections(void) {
    TEST("test_location_connections");

    const char* test_data =
        "[LOCATION:loc_a]\n"
        "name = Location A\n"
        "type = graveyard\n"
        "description = A\n"
        "connections = loc_b,loc_c\n"
        "\n"
        "[LOCATION:loc_b]\n"
        "name = Location B\n"
        "type = battlefield\n"
        "description = B\n"
        "connections = loc_a\n"
        "\n"
        "[LOCATION:loc_c]\n"
        "name = Location C\n"
        "type = village\n"
        "description = C\n"
        "connections = loc_a\n";

    FILE* f = fopen("/tmp/test_connections.dat", "w");
    ASSERT(f != NULL, "Failed to create temp file");
    fprintf(f, "%s", test_data);
    fclose(f);

    DataFile* data_file = data_file_load("/tmp/test_connections.dat");
    ASSERT(data_file != NULL, "Failed to load data file");

    TerritoryManager* territory = territory_manager_create();
    ASSERT(territory != NULL, "Failed to create territory");

    /* Load locations first */
    size_t loaded = location_data_load_all(territory, data_file);
    ASSERT(loaded == 3, "Expected 3 locations loaded");

    /* Build connections */
    size_t connections = location_data_build_connections(territory, data_file);
    /* Connections defined: loc_a->[b,c], loc_b->[a], loc_c->[a]
     * If duplicates are skipped, only unique connections are created */
    ASSERT(connections >= 2, "Expected at least 2 unique connections");

    /* Verify connections exist */
    uint32_t id_a = test_hash_string_id("loc_a");
    uint32_t id_b = test_hash_string_id("loc_b");
    uint32_t id_c = test_hash_string_id("loc_c");

    Location* loc_a = territory_manager_get_location(territory, id_a);
    ASSERT(loc_a != NULL, "Location A not found");
    ASSERT(loc_a->connection_count == 2, "Location A should have 2 connections");
    ASSERT(loc_a->connected_ids[0] == id_b || loc_a->connected_ids[1] == id_b, "Connection to B missing");
    ASSERT(loc_a->connected_ids[0] == id_c || loc_a->connected_ids[1] == id_c, "Connection to C missing");

    data_file_destroy(data_file);
    territory_manager_destroy(territory);

    PASS();
}

/**
 * Test 5: Default values for optional properties
 */
static void test_default_values(void) {
    TEST("test_default_values");

    /* Minimal location data (only required fields) */
    const char* test_data =
        "[LOCATION:minimal_location]\n"
        "name = Minimal Location\n"
        "type = graveyard\n"
        "description = Bare minimum\n";

    FILE* f = fopen("/tmp/test_defaults.dat", "w");
    ASSERT(f != NULL, "Failed to create temp file");
    fprintf(f, "%s", test_data);
    fclose(f);

    DataFile* data_file = data_file_load("/tmp/test_defaults.dat");
    ASSERT(data_file != NULL, "Failed to load data file");

    size_t count = 0;
    const DataSection** sections = data_file_get_sections(data_file, "LOCATION", &count);
    ASSERT(sections != NULL && count == 1, "Failed to get section");

    Location* loc = location_data_create_from_section(sections[0]);
    ASSERT(loc != NULL, "Failed to create location");

    /* Verify defaults */
    ASSERT(loc->corpse_count == 0, "Default corpse_count should be 0");
    ASSERT(loc->soul_quality_avg == 50, "Default soul_quality should be 50");
    ASSERT(loc->control_level == 0, "Default control_level should be 0");
    ASSERT(loc->defense_strength == 0, "Default defense_strength should be 0");
    ASSERT(loc->discovered == false, "Default discovered should be false");

    location_destroy(loc);
    free((void*)sections);
    data_file_destroy(data_file);

    PASS();
}

/**
 * Test 6: Load actual locations.dat file
 */
static void test_load_real_locations_file(void) {
    TEST("test_load_real_locations_file");

    /* Try to load the actual game data file */
    DataFile* data_file = data_file_load("data/locations.dat");
    if (!data_file) {
        printf("SKIP (data/locations.dat not found)\n");
        return;
    }

    TerritoryManager* territory = territory_manager_create();
    ASSERT(territory != NULL, "Failed to create territory");

    /* Load locations */
    size_t loaded = location_data_load_all(territory, data_file);
    ASSERT(loaded > 0, "Should load at least one location");

    printf("Loaded %zu locations... ", loaded);

    /* Build connections */
    size_t connections = location_data_build_connections(territory, data_file);
    printf("%zu connections... ", connections);

    /* Verify starting location exists */
    uint32_t starting_id = test_hash_string_id("graveyard_blackwood");
    Location* starting_loc = territory_manager_get_location(territory, starting_id);
    ASSERT(starting_loc != NULL, "Starting location not found");
    ASSERT(starting_loc->discovered == true, "Starting location should be discovered");

    data_file_destroy(data_file);
    territory_manager_destroy(territory);

    PASS();
}

/**
 * Test 7: Error handling - NULL parameters
 */
static void test_null_parameters(void) {
    TEST("test_null_parameters");

    /* Test NULL section */
    Location* loc = location_data_create_from_section(NULL);
    ASSERT(loc == NULL, "Should return NULL for NULL section");

    /* Test NULL territory */
    DataFile* data_file = data_file_load("/tmp/test_location.dat");
    if (data_file) {
        size_t loaded = location_data_load_all(NULL, data_file);
        ASSERT(loaded == 0, "Should return 0 for NULL territory");
        data_file_destroy(data_file);
    }

    PASS();
}

/**
 * Test 8: Memory leak check - multiple load/destroy cycles
 */
static void test_memory_leak_check(void) {
    TEST("test_memory_leak_check");

    const char* test_data =
        "[LOCATION:leak_test]\n"
        "name = Leak Test\n"
        "type = graveyard\n"
        "description = Testing for leaks\n";

    FILE* f = fopen("/tmp/test_leak.dat", "w");
    ASSERT(f != NULL, "Failed to create temp file");
    fprintf(f, "%s", test_data);
    fclose(f);

    /* Load and destroy 10 times */
    for (int i = 0; i < 10; i++) {
        DataFile* data_file = data_file_load("/tmp/test_leak.dat");
        ASSERT(data_file != NULL, "Failed to load data file");

        TerritoryManager* territory = territory_manager_create();
        ASSERT(territory != NULL, "Failed to create territory");

        size_t loaded = location_data_load_all(territory, data_file);
        ASSERT(loaded == 1, "Expected 1 location");

        data_file_destroy(data_file);
        territory_manager_destroy(territory);
    }

    PASS();
}

/**
 * Main test runner
 */
int main(void) {
    printf("\n=== Location Data Loader Tests ===\n\n");

    /* Initialize logger (quiet mode for tests) */
    logger_init("test_location_data.log", LOG_LEVEL_ERROR);

    /* Run all tests */
    test_load_single_location();
    test_load_multiple_locations();
    test_location_types();
    test_location_connections();
    test_default_values();
    test_load_real_locations_file();
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

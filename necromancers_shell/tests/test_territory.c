/**
 * @file test_territory.c
 * @brief Tests for territory manager
 */

#include "../src/game/world/territory.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

static void test_territory_create(void) {
    TerritoryManager* manager = territory_manager_create();
    assert(manager != NULL);
    assert(territory_manager_count(manager) == 0);
    territory_manager_destroy(manager);
    printf("PASS: test_territory_create\n");
}

static void test_territory_add_location(void) {
    TerritoryManager* manager = territory_manager_create();
    assert(manager != NULL);

    Location* loc1 = location_create(1, "Location 1", LOCATION_TYPE_GRAVEYARD);
    Location* loc2 = location_create(2, "Location 2", LOCATION_TYPE_BATTLEFIELD);

    assert(territory_manager_add_location(manager, loc1) == true);
    assert(territory_manager_count(manager) == 1);

    assert(territory_manager_add_location(manager, loc2) == true);
    assert(territory_manager_count(manager) == 2);

    /* Try adding duplicate ID */
    Location* loc3 = location_create(1, "Duplicate", LOCATION_TYPE_VILLAGE);
    assert(territory_manager_add_location(manager, loc3) == false);
    assert(territory_manager_count(manager) == 2);
    location_destroy(loc3); /* Manager didn't take ownership */

    territory_manager_destroy(manager);
    printf("PASS: test_territory_add_location\n");
}

static void test_territory_get_location(void) {
    TerritoryManager* manager = territory_manager_create();
    assert(manager != NULL);

    Location* loc1 = location_create(1, "Location 1", LOCATION_TYPE_GRAVEYARD);
    Location* loc2 = location_create(2, "Location 2", LOCATION_TYPE_BATTLEFIELD);

    territory_manager_add_location(manager, loc1);
    territory_manager_add_location(manager, loc2);

    /* Get by ID */
    Location* found = territory_manager_get_location(manager, 1);
    assert(found != NULL);
    assert(found->id == 1);
    assert(strcmp(found->name, "Location 1") == 0);

    found = territory_manager_get_location(manager, 2);
    assert(found != NULL);
    assert(found->id == 2);

    /* Not found */
    found = territory_manager_get_location(manager, 999);
    assert(found == NULL);

    territory_manager_destroy(manager);
    printf("PASS: test_territory_get_location\n");
}

static void test_territory_get_location_by_name(void) {
    TerritoryManager* manager = territory_manager_create();
    assert(manager != NULL);

    Location* loc1 = location_create(1, "Forgotten Graveyard", LOCATION_TYPE_GRAVEYARD);
    Location* loc2 = location_create(2, "Old Battlefield", LOCATION_TYPE_BATTLEFIELD);

    territory_manager_add_location(manager, loc1);
    territory_manager_add_location(manager, loc2);

    /* Get by name */
    Location* found = territory_manager_get_location_by_name(manager, "Forgotten Graveyard");
    assert(found != NULL);
    assert(found->id == 1);

    found = territory_manager_get_location_by_name(manager, "Old Battlefield");
    assert(found != NULL);
    assert(found->id == 2);

    /* Not found */
    found = territory_manager_get_location_by_name(manager, "Nonexistent");
    assert(found == NULL);

    territory_manager_destroy(manager);
    printf("PASS: test_territory_get_location_by_name\n");
}

static void test_territory_get_discovered(void) {
    TerritoryManager* manager = territory_manager_create();
    assert(manager != NULL);

    Location* loc1 = location_create(1, "Loc1", LOCATION_TYPE_GRAVEYARD);
    Location* loc2 = location_create(2, "Loc2", LOCATION_TYPE_BATTLEFIELD);
    Location* loc3 = location_create(3, "Loc3", LOCATION_TYPE_VILLAGE);

    loc1->discovered = true;
    loc2->discovered = false;
    loc3->discovered = true;

    territory_manager_add_location(manager, loc1);
    territory_manager_add_location(manager, loc2);
    territory_manager_add_location(manager, loc3);

    Location** results = NULL;
    size_t count = 0;
    assert(territory_manager_get_discovered(manager, &results, &count) == true);
    assert(count == 2);
    assert(results != NULL);

    /* Check results contain only discovered locations */
    bool found_loc1 = false;
    bool found_loc3 = false;
    for (size_t i = 0; i < count; i++) {
        if (results[i]->id == 1) found_loc1 = true;
        if (results[i]->id == 3) found_loc3 = true;
        assert(results[i]->id != 2); /* Loc2 should not be in results */
    }
    assert(found_loc1 == true);
    assert(found_loc3 == true);

    territory_manager_free_results(results);
    territory_manager_destroy(manager);
    printf("PASS: test_territory_get_discovered\n");
}

static void test_territory_count_discovered(void) {
    TerritoryManager* manager = territory_manager_create();
    assert(manager != NULL);

    assert(territory_manager_count_discovered(manager) == 0);

    Location* loc1 = location_create(1, "Loc1", LOCATION_TYPE_GRAVEYARD);
    Location* loc2 = location_create(2, "Loc2", LOCATION_TYPE_BATTLEFIELD);
    Location* loc3 = location_create(3, "Loc3", LOCATION_TYPE_VILLAGE);

    loc1->discovered = true;
    loc2->discovered = false;
    loc3->discovered = true;

    territory_manager_add_location(manager, loc1);
    assert(territory_manager_count_discovered(manager) == 1);

    territory_manager_add_location(manager, loc2);
    assert(territory_manager_count_discovered(manager) == 1);

    territory_manager_add_location(manager, loc3);
    assert(territory_manager_count_discovered(manager) == 2);

    territory_manager_destroy(manager);
    printf("PASS: test_territory_count_discovered\n");
}

static void test_territory_load_from_file(void) {
    TerritoryManager* manager = territory_manager_create();
    assert(manager != NULL);

    size_t loaded = territory_manager_load_from_file(manager, "dummy.json");
    assert(loaded == 5); /* Should load 5 hardcoded locations */
    assert(territory_manager_count(manager) == 5);

    /* Verify starting location is discovered */
    Location* start = territory_manager_get_location(manager, 1);
    assert(start != NULL);
    assert(strcmp(start->name, "Forgotten Graveyard") == 0);
    assert(start->discovered == true);
    assert(start->control_level == 100);

    /* Verify connections */
    assert(location_is_connected(start, 2) == true);
    assert(location_is_connected(start, 3) == true);

    /* Verify other locations exist but are undiscovered */
    Location* battlefield = territory_manager_get_location(manager, 2);
    assert(battlefield != NULL);
    assert(strcmp(battlefield->name, "Old Battlefield") == 0);
    assert(battlefield->discovered == false);

    territory_manager_destroy(manager);
    printf("PASS: test_territory_load_from_file\n");
}

static void test_territory_clear(void) {
    TerritoryManager* manager = territory_manager_create();
    assert(manager != NULL);

    Location* loc1 = location_create(1, "Loc1", LOCATION_TYPE_GRAVEYARD);
    Location* loc2 = location_create(2, "Loc2", LOCATION_TYPE_BATTLEFIELD);

    territory_manager_add_location(manager, loc1);
    territory_manager_add_location(manager, loc2);
    assert(territory_manager_count(manager) == 2);

    territory_manager_clear(manager);
    assert(territory_manager_count(manager) == 0);

    territory_manager_destroy(manager);
    printf("PASS: test_territory_clear\n");
}

static void test_territory_null_safety(void) {
    territory_manager_destroy(NULL); /* Should not crash */
    assert(territory_manager_add_location(NULL, NULL) == false);
    assert(territory_manager_get_location(NULL, 1) == NULL);
    assert(territory_manager_get_location_by_name(NULL, "test") == NULL);
    assert(territory_manager_count(NULL) == 0);
    assert(territory_manager_count_discovered(NULL) == 0);
    assert(territory_manager_load_from_file(NULL, "test") == 0);
    territory_manager_clear(NULL); /* Should not crash */
    territory_manager_free_results(NULL); /* Should not crash */

    printf("PASS: test_territory_null_safety\n");
}

int main(void) {
    printf("Running territory tests...\n\n");

    test_territory_create();
    test_territory_add_location();
    test_territory_get_location();
    test_territory_get_location_by_name();
    test_territory_get_discovered();
    test_territory_count_discovered();
    test_territory_load_from_file();
    test_territory_clear();
    test_territory_null_safety();

    printf("\nAll territory tests passed!\n");
    return 0;
}

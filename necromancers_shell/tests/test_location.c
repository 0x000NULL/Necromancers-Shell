/**
 * @file test_location.c
 * @brief Tests for location system
 */

#include "../src/game/world/location.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

static void test_location_create(void) {
    Location* loc = location_create(1, "Test Location", LOCATION_TYPE_GRAVEYARD);
    assert(loc != NULL);
    assert(loc->id == 1);
    assert(strcmp(loc->name, "Test Location") == 0);
    assert(loc->type == LOCATION_TYPE_GRAVEYARD);
    assert(loc->status == LOCATION_STATUS_UNDISCOVERED);
    assert(loc->discovered == false);
    assert(loc->corpse_count == 0);
    assert(loc->connection_count == 0);
    location_destroy(loc);
    printf("PASS: test_location_create\n");
}

static void test_location_type_names(void) {
    assert(strcmp(location_type_name(LOCATION_TYPE_GRAVEYARD), "Graveyard") == 0);
    assert(strcmp(location_type_name(LOCATION_TYPE_BATTLEFIELD), "Battlefield") == 0);
    assert(strcmp(location_type_name(LOCATION_TYPE_VILLAGE), "Village") == 0);
    assert(strcmp(location_type_name(LOCATION_TYPE_CRYPT), "Crypt") == 0);
    assert(strcmp(location_type_name(LOCATION_TYPE_RITUAL_SITE), "Ritual Site") == 0);
    printf("PASS: test_location_type_names\n");
}

static void test_location_status_names(void) {
    assert(strcmp(location_status_name(LOCATION_STATUS_UNDISCOVERED), "Undiscovered") == 0);
    assert(strcmp(location_status_name(LOCATION_STATUS_DISCOVERED), "Discovered") == 0);
    assert(strcmp(location_status_name(LOCATION_STATUS_CONTROLLED), "Controlled") == 0);
    assert(strcmp(location_status_name(LOCATION_STATUS_HOSTILE), "Hostile") == 0);
    printf("PASS: test_location_status_names\n");
}

static void test_location_connections(void) {
    Location* loc = location_create(1, "Hub", LOCATION_TYPE_GRAVEYARD);
    assert(loc != NULL);

    /* Add connections */
    assert(location_add_connection(loc, 2) == true);
    assert(location_add_connection(loc, 3) == true);
    assert(location_add_connection(loc, 4) == true);
    assert(loc->connection_count == 3);

    /* Check connections */
    assert(location_is_connected(loc, 2) == true);
    assert(location_is_connected(loc, 3) == true);
    assert(location_is_connected(loc, 4) == true);
    assert(location_is_connected(loc, 5) == false);

    /* Duplicate connection should not increase count */
    assert(location_add_connection(loc, 2) == true);
    assert(loc->connection_count == 3);

    location_destroy(loc);
    printf("PASS: test_location_connections\n");
}

static void test_location_discover(void) {
    Location* loc = location_create(1, "Hidden Place", LOCATION_TYPE_CRYPT);
    assert(loc != NULL);
    assert(loc->discovered == false);
    assert(loc->status == LOCATION_STATUS_UNDISCOVERED);

    location_discover(loc, 12345);
    assert(loc->discovered == true);
    assert(loc->discovered_timestamp == 12345);
    assert(loc->status == LOCATION_STATUS_DISCOVERED);

    location_destroy(loc);
    printf("PASS: test_location_discover\n");
}

static void test_location_harvest(void) {
    Location* loc = location_create(1, "Graveyard", LOCATION_TYPE_GRAVEYARD);
    assert(loc != NULL);

    loc->corpse_count = 100;

    /* Harvest some corpses */
    uint32_t harvested = location_harvest_corpses(loc, 30);
    assert(harvested == 30);
    assert(loc->corpse_count == 70);

    /* Harvest more than available */
    harvested = location_harvest_corpses(loc, 100);
    assert(harvested == 70);
    assert(loc->corpse_count == 0);

    /* Harvest from empty location */
    harvested = location_harvest_corpses(loc, 10);
    assert(harvested == 0);
    assert(loc->corpse_count == 0);

    location_destroy(loc);
    printf("PASS: test_location_harvest\n");
}

static void test_location_control(void) {
    Location* loc = location_create(1, "Fortress", LOCATION_TYPE_BATTLEFIELD);
    assert(loc != NULL);
    assert(loc->control_level == 0);

    /* Increase control */
    uint8_t level = location_increase_control(loc, 30);
    assert(level == 30);
    assert(loc->control_level == 30);

    /* Increase to controlled status */
    level = location_increase_control(loc, 50);
    assert(level == 80);
    assert(loc->status == LOCATION_STATUS_CONTROLLED);

    /* Clamp at 100 */
    level = location_increase_control(loc, 50);
    assert(level == 100);
    assert(loc->control_level == 100);

    /* Decrease control */
    level = location_decrease_control(loc, 30);
    assert(level == 70);
    assert(loc->control_level == 70);
    assert(loc->status == LOCATION_STATUS_DISCOVERED);

    /* Clamp at 0 */
    level = location_decrease_control(loc, 200);
    assert(level == 0);
    assert(loc->control_level == 0);

    location_destroy(loc);
    printf("PASS: test_location_control\n");
}

static void test_location_description(void) {
    Location* loc = location_create(1, "Test Place", LOCATION_TYPE_VILLAGE);
    assert(loc != NULL);

    strncpy(loc->description, "A peaceful village", sizeof(loc->description) - 1);
    loc->corpse_count = 50;
    loc->soul_quality_avg = 60;
    loc->control_level = 25;
    loc->defense_strength = 40;

    char buffer[1024];
    size_t written = location_get_description_formatted(loc, buffer, sizeof(buffer));
    assert(written > 0);
    assert(strstr(buffer, "Test Place") != NULL);
    assert(strstr(buffer, "Village") != NULL);
    assert(strstr(buffer, "50") != NULL);

    location_destroy(loc);
    printf("PASS: test_location_description\n");
}

static void test_location_null_safety(void) {
    /* NULL checks */
    assert(location_create(1, NULL, LOCATION_TYPE_GRAVEYARD) == NULL);
    location_destroy(NULL); /* Should not crash */
    assert(location_add_connection(NULL, 1) == false);
    assert(location_is_connected(NULL, 1) == false);
    location_discover(NULL, 0); /* Should not crash */
    assert(location_harvest_corpses(NULL, 10) == 0);
    assert(location_increase_control(NULL, 10) == 0);
    assert(location_decrease_control(NULL, 10) == 0);
    assert(location_get_description_formatted(NULL, NULL, 0) == 0);

    printf("PASS: test_location_null_safety\n");
}

int main(void) {
    printf("Running location tests...\n\n");

    test_location_create();
    test_location_type_names();
    test_location_status_names();
    test_location_connections();
    test_location_discover();
    test_location_harvest();
    test_location_control();
    test_location_description();
    test_location_null_safety();

    printf("\nAll location tests passed!\n");
    return 0;
}

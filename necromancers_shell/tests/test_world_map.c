/**
 * @file test_world_map.c
 * @brief Unit tests for world map system
 */

#include "../src/game/world/world_map.h"
#include "../src/game/world/location.h"
#include "../src/game/world/territory.h"
#include "../src/game/world/location_graph.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Test counters */
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) \
    do { \
        printf("Running %s...", name); \
        tests_run++; \
    } while (0)

#define PASS() \
    do { \
        printf(" PASS\n"); \
        tests_passed++; \
    } while (0)

#define FAIL(msg) \
    do { \
        printf(" FAIL: %s\n", msg); \
        tests_failed++; \
    } while (0)

#define ASSERT(condition, msg) \
    do { \
        if (!(condition)) { \
            FAIL(msg); \
            return; \
        } \
    } while (0)

/* Test fixtures */
static TerritoryManager* create_test_territory(void) {
    TerritoryManager* territory = territory_manager_create();
    if (!territory) return NULL;

    /* Add some test locations */
    Location* loc1 = location_create(1, "Graveyard Alpha", LOCATION_TYPE_GRAVEYARD);
    if (loc1) {
        loc1->status = LOCATION_STATUS_DISCOVERED;
        strncpy(loc1->description, "A dark graveyard", sizeof(loc1->description) - 1);
        loc1->corpse_count = 10;
        territory_manager_add_location(territory, loc1);
    }

    Location* loc2 = location_create(2, "Battlefield Beta", LOCATION_TYPE_BATTLEFIELD);
    if (loc2) {
        loc2->status = LOCATION_STATUS_DISCOVERED;
        strncpy(loc2->description, "A war-torn field", sizeof(loc2->description) - 1);
        loc2->corpse_count = 20;
        territory_manager_add_location(territory, loc2);
    }

    Location* loc3 = location_create(3, "Village Gamma", LOCATION_TYPE_VILLAGE);
    if (loc3) {
        loc3->status = LOCATION_STATUS_UNDISCOVERED;
        strncpy(loc3->description, "A peaceful village", sizeof(loc3->description) - 1);
        loc3->corpse_count = 5;
        territory_manager_add_location(territory, loc3);
    }

    return territory;
}

static LocationGraph* create_test_graph(void) {
    LocationGraph* graph = location_graph_create();
    if (!graph) return NULL;

    location_graph_add_bidirectional(graph, 1, 2, 2, 10);
    location_graph_add_bidirectional(graph, 2, 3, 3, 20);

    return graph;
}

/* Test: Map creation and destruction */
static void test_map_create_destroy(void) {
    TEST("test_map_create_destroy");

    TerritoryManager* territory = create_test_territory();
    LocationGraph* graph = create_test_graph();

    ASSERT(territory != NULL, "Territory should be created");
    ASSERT(graph != NULL, "Graph should be created");

    WorldMap* map = world_map_create(territory, graph);
    ASSERT(map != NULL, "Map should be created");

    world_map_destroy(map);
    world_map_destroy(NULL); /* Should not crash */

    location_graph_destroy(graph);
    territory_manager_destroy(territory);

    PASS();
}

/* Test: Setting and getting coordinates */
static void test_coordinates(void) {
    TEST("test_coordinates");

    TerritoryManager* territory = create_test_territory();
    LocationGraph* graph = create_test_graph();
    WorldMap* map = world_map_create(territory, graph);

    ASSERT(map != NULL, "Map should be created");

    /* Set coordinates */
    bool result = world_map_set_coordinates(map, 1, 10, 20);
    ASSERT(result, "Should set coordinates");

    /* Get coordinates */
    MapCoordinates coords;
    result = world_map_get_coordinates(map, 1, &coords);
    ASSERT(result, "Should get coordinates");
    ASSERT(coords.x == 10, "X coordinate should be 10");
    ASSERT(coords.y == 20, "Y coordinate should be 20");

    /* Non-existent location */
    result = world_map_get_coordinates(map, 999, &coords);
    ASSERT(!result, "Should fail for non-existent location");

    world_map_destroy(map);
    location_graph_destroy(graph);
    territory_manager_destroy(territory);

    PASS();
}

/* Test: Region management */
static void test_regions(void) {
    TEST("test_regions");

    TerritoryManager* territory = create_test_territory();
    LocationGraph* graph = create_test_graph();
    WorldMap* map = world_map_create(territory, graph);

    ASSERT(map != NULL, "Map should be created");

    /* Set region */
    bool result = world_map_set_region(map, 1, MAP_REGION_WESTERN_CRYPTS);
    ASSERT(result, "Should set region");

    /* Get region */
    MapRegion region = world_map_get_region(map, 1);
    ASSERT(region == MAP_REGION_WESTERN_CRYPTS, "Region should be Western Crypts");

    /* Get locations in region */
    uint32_t locations[10];
    size_t count = world_map_get_locations_in_region(map, MAP_REGION_WESTERN_CRYPTS,
                                                      locations, 10);
    ASSERT(count == 1, "Should find 1 location in region");
    ASSERT(locations[0] == 1, "Location should be 1");

    world_map_destroy(map);
    location_graph_destroy(graph);
    territory_manager_destroy(territory);

    PASS();
}

/* Test: Symbol management */
static void test_symbols(void) {
    TEST("test_symbols");

    TerritoryManager* territory = create_test_territory();
    LocationGraph* graph = create_test_graph();
    WorldMap* map = world_map_create(territory, graph);

    ASSERT(map != NULL, "Map should be created");

    /* Set custom symbol */
    bool result = world_map_set_symbol(map, 1, 'X');
    ASSERT(result, "Should set symbol");

    world_map_destroy(map);
    location_graph_destroy(graph);
    territory_manager_destroy(territory);

    PASS();
}

/* Test: Bounding box */
static void test_bounds(void) {
    TEST("test_bounds");

    TerritoryManager* territory = create_test_territory();
    LocationGraph* graph = create_test_graph();
    WorldMap* map = world_map_create(territory, graph);

    ASSERT(map != NULL, "Map should be created");

    /* Set coordinates for locations */
    world_map_set_coordinates(map, 1, -10, -20);
    world_map_set_coordinates(map, 2, 30, 40);
    world_map_set_coordinates(map, 3, 0, 10);

    /* Get bounds */
    int16_t min_x, max_x, min_y, max_y;
    bool result = world_map_get_bounds(map, &min_x, &max_x, &min_y, &max_y);

    ASSERT(result, "Should get bounds");
    ASSERT(min_x == -10, "Min X should be -10");
    ASSERT(max_x == 30, "Max X should be 30");
    ASSERT(min_y == -20, "Min Y should be -20");
    ASSERT(max_y == 40, "Max Y should be 40");

    world_map_destroy(map);
    location_graph_destroy(graph);
    territory_manager_destroy(territory);

    PASS();
}

/* Test: Radius query */
static void test_radius(void) {
    TEST("test_radius");

    TerritoryManager* territory = create_test_territory();
    LocationGraph* graph = create_test_graph();
    WorldMap* map = world_map_create(territory, graph);

    ASSERT(map != NULL, "Map should be created");

    /* Set coordinates */
    world_map_set_coordinates(map, 1, 0, 0);    /* Center */
    world_map_set_coordinates(map, 2, 5, 5);    /* Distance = 10 */
    world_map_set_coordinates(map, 3, 20, 20);  /* Distance = 40 */

    /* Query locations within radius 15 */
    uint32_t locations[10];
    size_t count = world_map_get_locations_in_radius(map, 1, 15, locations, 10);

    ASSERT(count == 1, "Should find 1 location within radius");
    ASSERT(locations[0] == 2, "Location 2 should be within radius");

    /* Query with larger radius */
    count = world_map_get_locations_in_radius(map, 1, 50, locations, 10);
    ASSERT(count == 2, "Should find 2 locations within radius");

    world_map_destroy(map);
    location_graph_destroy(graph);
    territory_manager_destroy(territory);

    PASS();
}

/* Test: Auto layout */
static void test_auto_layout(void) {
    TEST("test_auto_layout");

    TerritoryManager* territory = create_test_territory();
    LocationGraph* graph = create_test_graph();
    WorldMap* map = world_map_create(territory, graph);

    ASSERT(map != NULL, "Map should be created");

    /* Auto layout */
    bool result = world_map_auto_layout(map, 100);
    ASSERT(result, "Auto layout should succeed");

    /* Verify all locations have coordinates */
    MapCoordinates coords;
    ASSERT(world_map_get_coordinates(map, 1, &coords), "Location 1 should have coords");
    ASSERT(world_map_get_coordinates(map, 2, &coords), "Location 2 should have coords");
    ASSERT(world_map_get_coordinates(map, 3, &coords), "Location 3 should have coords");

    world_map_destroy(map);
    location_graph_destroy(graph);
    territory_manager_destroy(territory);

    PASS();
}

/* Test: Map rendering */
static void test_rendering(void) {
    TEST("test_rendering");

    TerritoryManager* territory = create_test_territory();
    LocationGraph* graph = create_test_graph();
    WorldMap* map = world_map_create(territory, graph);

    ASSERT(map != NULL, "Map should be created");

    /* Set coordinates */
    world_map_set_coordinates(map, 1, 0, 0);
    world_map_set_coordinates(map, 2, 10, 5);
    world_map_set_coordinates(map, 3, 20, 10);

    /* Render map */
    char buffer[4096];
    MapRenderOptions opts = map_render_options_default();
    opts.width = 40;
    opts.height = 20;

    size_t written = world_map_render(map, 1, &opts, buffer, sizeof(buffer));

    ASSERT(written > 0, "Should render map");
    ASSERT(written < sizeof(buffer), "Should fit in buffer");
    ASSERT(buffer[0] == '+', "Should start with border");
    ASSERT(strstr(buffer, "@") != NULL, "Should contain current location marker");

    world_map_destroy(map);
    location_graph_destroy(graph);
    territory_manager_destroy(territory);

    PASS();
}

/* Test: Legend */
static void test_legend(void) {
    TEST("test_legend");

    char buffer[512];
    size_t written = world_map_get_legend(buffer, sizeof(buffer));

    ASSERT(written > 0, "Should generate legend");
    ASSERT(strstr(buffer, "Legend") != NULL, "Should contain 'Legend'");
    ASSERT(strstr(buffer, "Graveyard") != NULL, "Should contain 'Graveyard'");

    PASS();
}

/* Test: Region names */
static void test_region_names(void) {
    TEST("test_region_names");

    const char* name = world_map_region_name(MAP_REGION_STARTING_GROUNDS);
    ASSERT(strcmp(name, "Starting Grounds") == 0, "Should return correct region name");

    name = world_map_region_name(MAP_REGION_WESTERN_CRYPTS);
    ASSERT(strcmp(name, "Western Crypts") == 0, "Should return correct region name");

    PASS();
}

/* Test: NULL parameter handling */
static void test_null_parameters(void) {
    TEST("test_null_parameters");

    /* NULL territory/graph */
    ASSERT(world_map_create(NULL, NULL) == NULL, "Should handle NULL parameters");

    TerritoryManager* territory = create_test_territory();
    LocationGraph* graph = create_test_graph();

    ASSERT(world_map_create(NULL, graph) == NULL, "Should handle NULL territory");
    ASSERT(world_map_create(territory, NULL) == NULL, "Should handle NULL graph");

    /* NULL map operations */
    ASSERT(!world_map_set_coordinates(NULL, 1, 0, 0), "Should handle NULL map");
    ASSERT(!world_map_get_coordinates(NULL, 1, NULL), "Should handle NULL map");
    ASSERT(!world_map_set_region(NULL, 1, MAP_REGION_STARTING_GROUNDS),
           "Should handle NULL map");

    location_graph_destroy(graph);
    territory_manager_destroy(territory);

    PASS();
}

/* Test: Default render options */
static void test_default_options(void) {
    TEST("test_default_options");

    MapRenderOptions opts = map_render_options_default();

    ASSERT(opts.width == 60, "Default width should be 60");
    ASSERT(opts.height == 24, "Default height should be 24");
    ASSERT(opts.show_undiscovered == true, "Should show undiscovered by default");
    ASSERT(opts.show_connections == true, "Should show connections by default");
    ASSERT(opts.show_legend == true, "Should show legend by default");

    PASS();
}

/* Main test runner */
int main(void) {
    printf("=== World Map Unit Tests ===\n\n");

    /* Run all tests */
    test_map_create_destroy();
    test_coordinates();
    test_regions();
    test_symbols();
    test_bounds();
    test_radius();
    test_auto_layout();
    test_rendering();
    test_legend();
    test_region_names();
    test_null_parameters();
    test_default_options();

    /* Print summary */
    printf("\n=== Test Summary ===\n");
    printf("Total:  %d\n", tests_run);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);

    if (tests_failed == 0) {
        printf("\nAll tests passed!\n");
        return 0;
    } else {
        printf("\nSome tests failed.\n");
        return 1;
    }
}

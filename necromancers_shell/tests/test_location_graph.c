/**
 * @file test_location_graph.c
 * @brief Unit tests for location graph system
 */

#include "../src/game/world/location_graph.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

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
static LocationGraph* create_simple_graph(void) {
    /*
     * Simple graph:
     *   1 -- 2 -- 3
     *   |         |
     *   4 ------- 5
     */
    LocationGraph* graph = location_graph_create();
    if (!graph) return NULL;

    location_graph_add_bidirectional(graph, 1, 2, 1, 10);
    location_graph_add_bidirectional(graph, 2, 3, 2, 20);
    location_graph_add_bidirectional(graph, 1, 4, 1, 15);
    location_graph_add_bidirectional(graph, 3, 5, 3, 25);
    location_graph_add_bidirectional(graph, 4, 5, 2, 30);

    return graph;
}

static LocationGraph* create_complex_graph(void) {
    /*
     * More complex graph with multiple paths:
     *   1 --> 2 --> 4
     *   |     |     |
     *   v     v     v
     *   3 --> 5 --> 6
     */
    LocationGraph* graph = location_graph_create();
    if (!graph) return NULL;

    location_graph_add_connection(graph, 1, 2, 1, 10);
    location_graph_add_connection(graph, 1, 3, 3, 20);
    location_graph_add_connection(graph, 2, 4, 2, 15);
    location_graph_add_connection(graph, 2, 5, 1, 5);
    location_graph_add_connection(graph, 3, 5, 2, 25);
    location_graph_add_connection(graph, 4, 6, 1, 10);
    location_graph_add_connection(graph, 5, 6, 2, 15);

    return graph;
}

/* Test: Graph creation and destruction */
static void test_graph_create_destroy(void) {
    TEST("test_graph_create_destroy");

    LocationGraph* graph = location_graph_create();
    ASSERT(graph != NULL, "Graph should be created");

    ASSERT(location_graph_get_connection_count(graph) == 0,
           "New graph should have 0 connections");

    uint32_t locations[10];
    size_t count = location_graph_get_all_locations(graph, locations, 10);
    ASSERT(count == 0, "New graph should have 0 locations");

    location_graph_destroy(graph);
    location_graph_destroy(NULL); /* Should not crash */

    PASS();
}

/* Test: Adding connections */
static void test_add_connection(void) {
    TEST("test_add_connection");

    LocationGraph* graph = location_graph_create();
    ASSERT(graph != NULL, "Graph should be created");

    /* Add single connection */
    bool result = location_graph_add_connection(graph, 1, 2, 3, 50);
    ASSERT(result, "Should add connection successfully");

    ASSERT(location_graph_get_connection_count(graph) == 1,
           "Should have 1 connection");

    ASSERT(location_graph_has_connection(graph, 1, 2),
           "Connection 1->2 should exist");

    ASSERT(!location_graph_has_connection(graph, 2, 1),
           "Reverse connection 2->1 should not exist (unidirectional)");

    /* Verify connection data */
    LocationConnection conn;
    result = location_graph_get_connection(graph, 1, 2, &conn);
    ASSERT(result, "Should retrieve connection");
    ASSERT(conn.from_location_id == 1, "From ID should be 1");
    ASSERT(conn.to_location_id == 2, "To ID should be 2");
    ASSERT(conn.travel_time_hours == 3, "Travel time should be 3");
    ASSERT(conn.danger_level == 50, "Danger level should be 50");
    ASSERT(!conn.requires_unlock, "Should not require unlock initially");

    location_graph_destroy(graph);
    PASS();
}

/* Test: Bidirectional connections */
static void test_bidirectional_connection(void) {
    TEST("test_bidirectional_connection");

    LocationGraph* graph = location_graph_create();
    ASSERT(graph != NULL, "Graph should be created");

    bool result = location_graph_add_bidirectional(graph, 1, 2, 2, 30);
    ASSERT(result, "Should add bidirectional connection");

    ASSERT(location_graph_get_connection_count(graph) == 2,
           "Should have 2 connections (both directions)");

    ASSERT(location_graph_has_connection(graph, 1, 2),
           "Forward connection should exist");

    ASSERT(location_graph_has_connection(graph, 2, 1),
           "Reverse connection should exist");

    location_graph_destroy(graph);
    PASS();
}

/* Test: Getting neighbors */
static void test_get_neighbors(void) {
    TEST("test_get_neighbors");

    LocationGraph* graph = create_simple_graph();
    ASSERT(graph != NULL, "Graph should be created");

    uint32_t neighbors[10];

    /* Location 1 should have 2 neighbors: 2 and 4 */
    size_t count = location_graph_get_neighbors(graph, 1, neighbors, 10);
    ASSERT(count == 2, "Location 1 should have 2 neighbors");

    /* Check neighbors (order may vary) */
    bool has_2 = false, has_4 = false;
    for (size_t i = 0; i < count; i++) {
        if (neighbors[i] == 2) has_2 = true;
        if (neighbors[i] == 4) has_4 = true;
    }
    ASSERT(has_2 && has_4, "Neighbors should be 2 and 4");

    /* Location 2 should have 2 neighbors: 1 and 3 */
    count = location_graph_get_neighbors(graph, 2, neighbors, 10);
    ASSERT(count == 2, "Location 2 should have 2 neighbors");

    /* Non-existent location should have 0 neighbors */
    count = location_graph_get_neighbors(graph, 999, neighbors, 10);
    ASSERT(count == 0, "Non-existent location should have 0 neighbors");

    location_graph_destroy(graph);
    PASS();
}

/* Test: Pathfinding - simple path */
static void test_pathfinding_simple(void) {
    TEST("test_pathfinding_simple");

    LocationGraph* graph = create_simple_graph();
    ASSERT(graph != NULL, "Graph should be created");

    PathfindingResult result;

    /* Path from 1 to 3: should go 1 -> 2 -> 3 */
    bool success = location_graph_find_path(graph, 1, 3, &result);
    ASSERT(success, "Pathfinding should succeed");
    ASSERT(result.path_found, "Path should be found");
    ASSERT(result.path_length == 3, "Path should have 3 locations");
    ASSERT(result.path[0] == 1, "Path should start at 1");
    ASSERT(result.path[1] == 2, "Path should go through 2");
    ASSERT(result.path[2] == 3, "Path should end at 3");
    ASSERT(result.total_travel_time == 3, "Total travel time should be 3");

    pathfinding_result_free(&result);
    location_graph_destroy(graph);
    PASS();
}

/* Test: Pathfinding - same location */
static void test_pathfinding_same_location(void) {
    TEST("test_pathfinding_same_location");

    LocationGraph* graph = create_simple_graph();
    ASSERT(graph != NULL, "Graph should be created");

    PathfindingResult result;

    /* Path from location to itself */
    bool success = location_graph_find_path(graph, 1, 1, &result);
    ASSERT(success, "Pathfinding should succeed");
    ASSERT(result.path_found, "Path should be found");
    ASSERT(result.path_length == 1, "Path should have 1 location");
    ASSERT(result.path[0] == 1, "Path should contain only location 1");
    ASSERT(result.total_travel_time == 0, "Travel time should be 0");

    pathfinding_result_free(&result);
    location_graph_destroy(graph);
    PASS();
}

/* Test: Pathfinding - optimal path */
static void test_pathfinding_optimal(void) {
    TEST("test_pathfinding_optimal");

    LocationGraph* graph = create_simple_graph();
    ASSERT(graph != NULL, "Graph should be created");

    PathfindingResult result;

    /* Path from 1 to 5: should go 1 -> 4 -> 5 (cost 3) instead of 1 -> 2 -> 3 -> 5 (cost 6) */
    bool success = location_graph_find_path(graph, 1, 5, &result);
    ASSERT(success, "Pathfinding should succeed");
    ASSERT(result.path_found, "Path should be found");
    ASSERT(result.total_travel_time == 3,
           "Should find optimal path with travel time 3");

    /* Verify path: 1 -> 4 -> 5 */
    ASSERT(result.path_length == 3, "Optimal path should have 3 locations");
    ASSERT(result.path[0] == 1, "Path should start at 1");
    ASSERT(result.path[1] == 4, "Path should go through 4");
    ASSERT(result.path[2] == 5, "Path should end at 5");

    pathfinding_result_free(&result);
    location_graph_destroy(graph);
    PASS();
}

/* Test: Pathfinding - no path */
static void test_pathfinding_no_path(void) {
    TEST("test_pathfinding_no_path");

    LocationGraph* graph = location_graph_create();
    ASSERT(graph != NULL, "Graph should be created");

    /* Create two disconnected islands */
    location_graph_add_bidirectional(graph, 1, 2, 1, 10);
    location_graph_add_bidirectional(graph, 3, 4, 1, 10);

    PathfindingResult result;

    /* No path from 1 to 3 (different islands) */
    bool success = location_graph_find_path(graph, 1, 3, &result);
    ASSERT(success, "Pathfinding should succeed (but find no path)");
    ASSERT(!result.path_found, "Path should not be found");
    ASSERT(result.path == NULL, "Path should be NULL when not found");
    ASSERT(result.path_length == 0, "Path length should be 0");

    pathfinding_result_free(&result);
    location_graph_destroy(graph);
    PASS();
}

/* Test: Pathfinding - invalid locations */
static void test_pathfinding_invalid(void) {
    TEST("test_pathfinding_invalid");

    LocationGraph* graph = create_simple_graph();
    ASSERT(graph != NULL, "Graph should be created");

    PathfindingResult result;

    /* Non-existent starting location */
    bool success = location_graph_find_path(graph, 999, 3, &result);
    ASSERT(success, "Pathfinding should succeed (but find no path)");
    ASSERT(!result.path_found, "Path should not be found");

    pathfinding_result_free(&result);

    /* Non-existent destination */
    success = location_graph_find_path(graph, 1, 999, &result);
    ASSERT(success, "Pathfinding should succeed (but find no path)");
    ASSERT(!result.path_found, "Path should not be found");

    pathfinding_result_free(&result);
    location_graph_destroy(graph);
    PASS();
}

/* Test: Reachability checking */
static void test_reachability(void) {
    TEST("test_reachability");

    LocationGraph* graph = location_graph_create();
    ASSERT(graph != NULL, "Graph should be created");

    /* Create two connected components */
    location_graph_add_bidirectional(graph, 1, 2, 1, 10);
    location_graph_add_bidirectional(graph, 2, 3, 1, 10);
    location_graph_add_bidirectional(graph, 4, 5, 1, 10);

    /* Within component 1 */
    ASSERT(location_graph_is_reachable(graph, 1, 3),
           "3 should be reachable from 1");
    ASSERT(location_graph_is_reachable(graph, 3, 1),
           "1 should be reachable from 3");

    /* Within component 2 */
    ASSERT(location_graph_is_reachable(graph, 4, 5),
           "5 should be reachable from 4");

    /* Between components */
    ASSERT(!location_graph_is_reachable(graph, 1, 4),
           "4 should not be reachable from 1 (different component)");
    ASSERT(!location_graph_is_reachable(graph, 3, 5),
           "5 should not be reachable from 3 (different component)");

    location_graph_destroy(graph);
    PASS();
}

/* Test: Unlock requirements */
static void test_unlock_requirements(void) {
    TEST("test_unlock_requirements");

    LocationGraph* graph = location_graph_create();
    ASSERT(graph != NULL, "Graph should be created");

    location_graph_add_connection(graph, 1, 2, 2, 20);

    /* Set unlock requirement */
    bool result = location_graph_set_unlock_requirement(graph, 1, 2,
                                                        "quest_unlock_bridge");
    ASSERT(result, "Should set unlock requirement");

    /* Verify unlock requirement */
    LocationConnection conn;
    result = location_graph_get_connection(graph, 1, 2, &conn);
    ASSERT(result, "Should get connection");
    ASSERT(conn.requires_unlock, "Connection should require unlock");
    ASSERT(strcmp(conn.unlock_requirement, "quest_unlock_bridge") == 0,
           "Unlock requirement should match");

    /* Try to set requirement on non-existent connection */
    result = location_graph_set_unlock_requirement(graph, 2, 3, "test");
    ASSERT(!result, "Should fail for non-existent connection");

    location_graph_destroy(graph);
    PASS();
}

/* Test: Graph validation */
static void test_graph_validation(void) {
    TEST("test_graph_validation");

    /* Fully connected graph */
    LocationGraph* graph = create_simple_graph();
    ASSERT(graph != NULL, "Graph should be created");

    bool valid = location_graph_validate_connectivity(graph, 1);
    ASSERT(valid, "Fully connected graph should be valid");

    location_graph_destroy(graph);

    /* Disconnected graph */
    graph = location_graph_create();
    ASSERT(graph != NULL, "Graph should be created");

    location_graph_add_bidirectional(graph, 1, 2, 1, 10);
    location_graph_add_bidirectional(graph, 3, 4, 1, 10);

    valid = location_graph_validate_connectivity(graph, 1);
    ASSERT(!valid, "Disconnected graph should be invalid");

    location_graph_destroy(graph);
    PASS();
}

/* Test: Get all locations */
static void test_get_all_locations(void) {
    TEST("test_get_all_locations");

    LocationGraph* graph = create_simple_graph();
    ASSERT(graph != NULL, "Graph should be created");

    uint32_t locations[10];
    size_t count = location_graph_get_all_locations(graph, locations, 10);

    ASSERT(count == 5, "Should have 5 unique locations");

    /* Verify all expected locations are present */
    bool has[6] = {false}; /* Index 0 unused, 1-5 for location IDs */
    for (size_t i = 0; i < count; i++) {
        uint32_t id = locations[i];
        ASSERT(id >= 1 && id <= 5, "Location ID should be in range 1-5");
        has[id] = true;
    }

    for (int i = 1; i <= 5; i++) {
        ASSERT(has[i], "Should have all locations 1-5");
    }

    location_graph_destroy(graph);
    PASS();
}

/* Test: NULL parameter handling */
static void test_null_parameters(void) {
    TEST("test_null_parameters");

    LocationGraph* graph = location_graph_create();
    ASSERT(graph != NULL, "Graph should be created");

    location_graph_add_connection(graph, 1, 2, 1, 10);

    /* Test NULL graph parameters */
    ASSERT(!location_graph_add_connection(NULL, 1, 2, 1, 10),
           "Should handle NULL graph");
    ASSERT(!location_graph_has_connection(NULL, 1, 2),
           "Should handle NULL graph");
    ASSERT(location_graph_get_connection_count(NULL) == 0,
           "Should return 0 for NULL graph");

    /* Test NULL result parameter */
    ASSERT(!location_graph_find_path(NULL, 1, 2, NULL),
           "Should handle NULL parameters");

    ASSERT(!location_graph_find_path(graph, 1, 2, NULL),
           "Should handle NULL result");

    /* Test NULL neighbor array */
    size_t count = location_graph_get_neighbors(graph, 1, NULL, 10);
    ASSERT(count == 0, "Should return 0 for NULL neighbors array");

    pathfinding_result_free(NULL); /* Should not crash */

    location_graph_destroy(graph);
    PASS();
}

/* Test: Empty graph */
static void test_empty_graph(void) {
    TEST("test_empty_graph");

    LocationGraph* graph = location_graph_create();
    ASSERT(graph != NULL, "Graph should be created");

    /* Empty graph validation should succeed */
    ASSERT(location_graph_validate_connectivity(graph, 1),
           "Empty graph should be valid");

    /* Operations on empty graph */
    ASSERT(!location_graph_has_connection(graph, 1, 2),
           "Empty graph should have no connections");

    uint32_t neighbors[10];
    size_t count = location_graph_get_neighbors(graph, 1, neighbors, 10);
    ASSERT(count == 0, "Empty graph should have no neighbors");

    PathfindingResult result;
    bool success = location_graph_find_path(graph, 1, 2, &result);
    ASSERT(success && !result.path_found,
           "Empty graph pathfinding should fail gracefully");

    pathfinding_result_free(&result);
    location_graph_destroy(graph);
    PASS();
}

/* Test: Complex graph pathfinding */
static void test_complex_pathfinding(void) {
    TEST("test_complex_pathfinding");

    LocationGraph* graph = create_complex_graph();
    ASSERT(graph != NULL, "Graph should be created");

    PathfindingResult result;

    /* Path from 1 to 6: optimal is 1 -> 2 -> 5 -> 6 (cost 4) or 1 -> 2 -> 4 -> 6 (cost 4) */
    bool success = location_graph_find_path(graph, 1, 6, &result);
    ASSERT(success && result.path_found, "Path should be found");
    ASSERT(result.total_travel_time == 4, "Optimal path should have cost 4");

    pathfinding_result_free(&result);
    location_graph_destroy(graph);
    PASS();
}

/* Test: Large graph stress test */
static void test_large_graph(void) {
    TEST("test_large_graph");

    LocationGraph* graph = location_graph_create();
    ASSERT(graph != NULL, "Graph should be created");

    /* Create a linear chain of 50 locations */
    for (uint32_t i = 1; i < 50; i++) {
        bool result = location_graph_add_bidirectional(graph, i, i + 1, 1, 10);
        ASSERT(result, "Should add connection");
    }

    /* Verify connection count */
    ASSERT(location_graph_get_connection_count(graph) == 98,
           "Should have 98 bidirectional connections");

    /* Pathfinding across full chain */
    PathfindingResult result;
    bool success = location_graph_find_path(graph, 1, 50, &result);
    ASSERT(success && result.path_found, "Path should be found");
    ASSERT(result.path_length == 50, "Path should span all 50 locations");
    ASSERT(result.total_travel_time == 49, "Total time should be 49");

    pathfinding_result_free(&result);
    location_graph_destroy(graph);
    PASS();
}

/* Main test runner */
int main(void) {
    printf("=== Location Graph Unit Tests ===\n\n");

    /* Run all tests */
    test_graph_create_destroy();
    test_add_connection();
    test_bidirectional_connection();
    test_get_neighbors();
    test_pathfinding_simple();
    test_pathfinding_same_location();
    test_pathfinding_optimal();
    test_pathfinding_no_path();
    test_pathfinding_invalid();
    test_reachability();
    test_unlock_requirements();
    test_graph_validation();
    test_get_all_locations();
    test_null_parameters();
    test_empty_graph();
    test_complex_pathfinding();
    test_large_graph();

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

/**
 * test_split_routing.c - Unit tests for Split-Routing System
 */

#include "../src/game/network/split_routing.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) do { printf("Running test: %s...", #name); tests_run++; } while(0)
#define PASS() do { printf(" PASSED\n"); tests_passed++; } while(0)

void test_create_destroy(void) {
    TEST(test_create_destroy);
    SplitRoutingManager* mgr = split_routing_manager_create();
    assert(mgr != NULL);
    assert(mgr->route_count == 0);
    split_routing_manager_destroy(mgr);
    PASS();
}

void test_validate_percentages(void) {
    TEST(test_validate_percentages);
    int valid[] = {50, 50};
    int invalid[] = {50, 60};

    assert(split_routing_validate_percentages(valid, 2) == true);
    assert(split_routing_validate_percentages(invalid, 2) == false);
    PASS();
}

void test_create_route(void) {
    TEST(test_create_route);
    SplitRoutingManager* mgr = split_routing_manager_create();

    AfterlifeDestination dests[] = {AFTERLIFE_HEAVEN, AFTERLIFE_HELL};
    int percentages[] = {60, 40};

    int route_id = split_routing_create_route(mgr, 1, "Test Soul", dests, percentages, 2, 100);
    assert(route_id > 0);
    assert(mgr->route_count == 1);

    split_routing_manager_destroy(mgr);
    PASS();
}

void test_get_route(void) {
    TEST(test_get_route);
    SplitRoutingManager* mgr = split_routing_manager_create();

    AfterlifeDestination dests[] = {AFTERLIFE_HEAVEN, AFTERLIFE_LIMBO};
    int percentages[] = {70, 30};

    int route_id = split_routing_create_route(mgr, 2, "Marcus", dests, percentages, 2, 100);
    const SplitRoutedSoul* route = split_routing_get_route(mgr, route_id);

    assert(route != NULL);
    assert(route->soul_id == 2);
    assert(route->fragment_count == 2);

    split_routing_manager_destroy(mgr);
    PASS();
}

void test_reunify(void) {
    TEST(test_reunify);
    SplitRoutingManager* mgr = split_routing_manager_create();

    AfterlifeDestination dests[] = {AFTERLIFE_HEAVEN, AFTERLIFE_HELL};
    int percentages[] = {50, 50};

    int route_id = split_routing_create_route(mgr, 3, "Test", dests, percentages, 2, 100);

    int experience = 0;
    bool result = split_routing_reunify(mgr, route_id, &experience);
    assert(result == true);
    assert(experience > 0);

    split_routing_manager_destroy(mgr);
    PASS();
}

void test_advance_time(void) {
    TEST(test_advance_time);
    SplitRoutingManager* mgr = split_routing_manager_create();

    AfterlifeDestination dests[] = {AFTERLIFE_HEAVEN, AFTERLIFE_HELL};
    int percentages[] = {50, 50};

    int route_id = split_routing_create_route(mgr, 4, "Test", dests, percentages, 2, 100);
    const SplitRoutedSoul* route = split_routing_get_route(mgr, route_id);
    int initial_years = route->years_until_reunification;

    split_routing_advance_time(mgr, 500);
    route = split_routing_get_route(mgr, route_id);
    assert(route->years_until_reunification < initial_years);

    split_routing_manager_destroy(mgr);
    PASS();
}

void test_afterlife_names(void) {
    TEST(test_afterlife_names);
    assert(strcmp(split_routing_get_afterlife_name(AFTERLIFE_HEAVEN), "Heaven") == 0);
    assert(strcmp(split_routing_get_afterlife_name(AFTERLIFE_HELL), "Hell") == 0);
    assert(strcmp(split_routing_get_afterlife_name(AFTERLIFE_LIMBO), "Limbo") == 0);
    PASS();
}

void test_fragment_compatibility(void) {
    TEST(test_fragment_compatibility);
    assert(split_routing_is_fragment_compatible(AFTERLIFE_HEAVEN) == true);
    assert(split_routing_is_fragment_compatible(AFTERLIFE_VOID) == false);
    PASS();
}

void test_status_to_string(void) {
    TEST(test_status_to_string);
    assert(strcmp(split_routing_status_to_string(ROUTE_SPLIT), "SPLIT") == 0);
    assert(strcmp(split_routing_status_to_string(ROUTE_REUNIFIED), "REUNIFIED") == 0);
    PASS();
}

void test_void_rejection(void) {
    TEST(test_void_rejection);
    SplitRoutingManager* mgr = split_routing_manager_create();

    AfterlifeDestination dests[] = {AFTERLIFE_HEAVEN, AFTERLIFE_VOID};
    int percentages[] = {50, 50};

    int route_id = split_routing_create_route(mgr, 5, "Test", dests, percentages, 2, 100);
    assert(route_id == -1); /* Should fail - Void not compatible */

    split_routing_manager_destroy(mgr);
    PASS();
}

int main(void) {
    printf("Running Split-Routing System tests...\n\n");

    test_create_destroy();
    test_validate_percentages();
    test_create_route();
    test_get_route();
    test_reunify();
    test_advance_time();
    test_afterlife_names();
    test_fragment_compatibility();
    test_status_to_string();
    test_void_rejection();

    printf("\n========================================\n");
    printf("Tests passed: %d/%d\n", tests_passed, tests_run);
    printf("========================================\n");

    return (tests_passed == tests_run) ? 0 : 1;
}

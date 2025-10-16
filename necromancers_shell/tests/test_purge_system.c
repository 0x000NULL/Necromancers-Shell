/**
 * test_purge_system.c - Unit tests for Fourth Purge System
 */

#include "../src/game/narrative/purge_system.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) do { printf("Running test: %s...", #name); tests_run++; } while(0)
#define PASS() do { printf(" PASSED\n"); tests_passed++; } while(0)

void test_create_destroy(void) {
    TEST(test_create_destroy);
    PurgeState* state = purge_system_create();
    assert(state != NULL);
    assert(state->purge_number == 4);
    assert(state->reformation_target == 147);
    purge_system_destroy(state);
    PASS();
}

void test_initialize(void) {
    TEST(test_initialize);
    PurgeState* state = purge_system_create();
    bool result = purge_system_initialize(state);
    assert(result == true);
    assert(state->days_until_purge == 1825); /* 5 years */
    assert(state->enforcer_count == 30);
    purge_system_destroy(state);
    PASS();
}

void test_accelerate(void) {
    TEST(test_accelerate);
    PurgeState* state = purge_system_create();
    purge_system_initialize(state);
    purge_system_accelerate(state);
    assert(state->accelerated == true);
    assert(state->days_until_purge == 547); /* 1.5 years */
    purge_system_destroy(state);
    PASS();
}

void test_advance_day(void) {
    TEST(test_advance_day);
    PurgeState* state = purge_system_create();
    purge_system_initialize(state);
    int initial = state->days_until_purge;
    bool begun = purge_system_advance_day(state);
    assert(state->days_until_purge == initial - 1);
    assert(begun == false);
    purge_system_destroy(state);
    PASS();
}

void test_reform_necromancer(void) {
    TEST(test_reform_necromancer);
    PurgeState* state = purge_system_create();
    purge_system_initialize(state);
    purge_system_enable_archon_intervention(state);

    bool result = purge_system_reform_necromancer(state, 1001);
    assert(result == true);
    assert(state->necromancers_reformed == 1);
    purge_system_destroy(state);
    PASS();
}

void test_reformation_complete(void) {
    TEST(test_reformation_complete);
    PurgeState* state = purge_system_create();
    purge_system_initialize(state);
    purge_system_enable_archon_intervention(state);

    for (int i = 0; i < 147; i++) {
        purge_system_reform_necromancer(state, 1000 + i);
    }

    assert(purge_system_is_reformation_complete(state) == true);
    purge_system_destroy(state);
    PASS();
}

void test_calculate_casualties_base(void) {
    TEST(test_calculate_casualties_base);
    PurgeState* state = purge_system_create();
    purge_system_initialize(state);

    int casualties = purge_system_calculate_casualties(state);
    assert(casualties == 200); /* Without Archon */
    purge_system_destroy(state);
    PASS();
}

void test_calculate_casualties_with_archon(void) {
    TEST(test_calculate_casualties_with_archon);
    PurgeState* state = purge_system_create();
    purge_system_initialize(state);
    purge_system_enable_archon_intervention(state);

    for (int i = 0; i < 147; i++) {
        purge_system_reform_necromancer(state, 1000 + i);
    }

    int casualties = purge_system_calculate_casualties(state);
    assert(casualties == 50); /* With successful reformation */
    purge_system_destroy(state);
    PASS();
}

void test_enforcer_type_names(void) {
    TEST(test_enforcer_type_names);
    assert(strcmp(purge_system_get_enforcer_type_name(ENFORCER_SERAPHIM), "Seraphim") == 0);
    assert(strcmp(purge_system_get_enforcer_type_name(ENFORCER_HELLKNIGHT), "Hellknight") == 0);
    PASS();
}

void test_get_years_remaining(void) {
    TEST(test_get_years_remaining);
    PurgeState* state = purge_system_create();
    purge_system_initialize(state);
    double years = purge_system_get_years_remaining(state);
    assert(years > 4.9 && years < 5.1); /* ~5 years */
    purge_system_destroy(state);
    PASS();
}

int main(void) {
    printf("Running Fourth Purge System tests...\n\n");

    test_create_destroy();
    test_initialize();
    test_accelerate();
    test_advance_day();
    test_reform_necromancer();
    test_reformation_complete();
    test_calculate_casualties_base();
    test_calculate_casualties_with_archon();
    test_enforcer_type_names();
    test_get_years_remaining();

    printf("\n========================================\n");
    printf("Tests passed: %d/%d\n", tests_passed, tests_run);
    printf("========================================\n");

    return (tests_passed == tests_run) ? 0 : 1;
}

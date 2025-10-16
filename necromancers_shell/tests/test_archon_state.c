/**
 * test_archon_state.c - Unit tests for Archon Transformation System
 */

#include "../src/game/narrative/archon_state.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) do { printf("Running test: %s...", #name); tests_run++; } while(0)
#define PASS() do { printf(" PASSED\n"); tests_passed++; } while(0)

void test_create_destroy(void) {
    TEST(test_create_destroy);
    ArchonState* state = archon_state_create();
    assert(state != NULL);
    assert(state->transformed == false);
    assert(state->administrative_level == 0);
    archon_state_destroy(state);
    PASS();
}

void test_transform(void) {
    TEST(test_transform);
    ArchonState* state = archon_state_create();

    const char* restrictions[] = {
        "No civilian massacres",
        "Reform 147 necromancers"
    };

    bool result = archon_transform(state, true, true, 45, restrictions, 2, 100, true);
    assert(result == true);
    assert(state->transformed == true);
    assert(state->administrative_level == 3);
    assert(state->corruption_locked == 45);
    assert(state->rule_count == 2);

    archon_state_destroy(state);
    PASS();
}

void test_transform_requirements(void) {
    TEST(test_transform_requirements);
    ArchonState* state = archon_state_create();

    /* Missing trials */
    bool result = archon_transform(state, false, true, 45, NULL, 0, 100, true);
    assert(result == false);

    /* Missing amnesty */
    result = archon_transform(state, true, false, 45, NULL, 0, 100, true);
    assert(result == false);

    archon_state_destroy(state);
    PASS();
}

void test_grant_privilege(void) {
    TEST(test_grant_privilege);
    ArchonState* state = archon_state_create();
    archon_transform(state, true, true, 45, NULL, 0, 100, true);

    bool result = archon_grant_privilege(state, PRIVILEGE_NETWORK_PATCHING);
    assert(result == true);
    assert(archon_is_privilege_unlocked(state, PRIVILEGE_NETWORK_PATCHING) == true);

    archon_state_destroy(state);
    PASS();
}

void test_reform_necromancer(void) {
    TEST(test_reform_necromancer);
    ArchonState* state = archon_state_create();
    archon_transform(state, true, true, 45, NULL, 0, 100, true);

    bool result = archon_reform_necromancer(state, 1001);
    assert(result == true);
    assert(state->necromancers_reformed == 1);

    archon_state_destroy(state);
    PASS();
}

void test_advance_level(void) {
    TEST(test_advance_level);
    ArchonState* state = archon_state_create();
    archon_transform(state, true, true, 45, NULL, 0, 100, true);

    /* Need 15 reforms to advance from level 3 to 4 */
    for (int i = 0; i < 15; i++) {
        archon_reform_necromancer(state, 1000 + i);
    }

    assert(state->administrative_level == 4);

    archon_state_destroy(state);
    PASS();
}

void test_check_code_violation_corruption(void) {
    TEST(test_check_code_violation_corruption);
    ArchonState* state = archon_state_create();

    const char* restrictions[] = {
        "Your corruption must not exceed 65%."
    };

    archon_transform(state, true, true, 55, restrictions, 1, 100, true);

    /* Violates corruption limit */
    bool violation = archon_check_code_violation(state, "corrupt action", 15);
    assert(violation == true);

    /* Doesn't violate */
    violation = archon_check_code_violation(state, "minor action", 5);
    assert(violation == false);

    archon_state_destroy(state);
    PASS();
}

void test_check_code_violation_massacre(void) {
    TEST(test_check_code_violation_massacre);
    ArchonState* state = archon_state_create();

    const char* restrictions[] = {
        "No civilian massacres without lawful cause."
    };

    archon_transform(state, true, true, 45, restrictions, 1, 100, true);

    bool violation = archon_check_code_violation(state, "massacre civilians", 0);
    assert(violation == true);

    archon_state_destroy(state);
    PASS();
}

void test_get_transformation_cutscene(void) {
    TEST(test_get_transformation_cutscene);
    ArchonState* state = archon_state_create();
    archon_transform(state, true, true, 45, NULL, 0, 100, true);

    const char* cutscene = archon_get_transformation_cutscene(state);
    assert(cutscene != NULL);
    assert(strlen(cutscene) > 0);
    assert(strstr(cutscene, "ARCHON") != NULL);

    archon_state_destroy(state);
    PASS();
}

void test_get_privilege_info(void) {
    TEST(test_get_privilege_info);
    ArchonState* state = archon_state_create();
    archon_transform(state, true, true, 45, NULL, 0, 100, true);

    const PrivilegeInfo* info = archon_get_privilege_info(state, PRIVILEGE_SPLIT_ROUTING);
    assert(info != NULL);
    assert(info->level_required == 2);
    assert(strlen(info->name) > 0);

    archon_state_destroy(state);
    PASS();
}

void test_get_reforms_needed(void) {
    TEST(test_get_reforms_needed);
    ArchonState* state = archon_state_create();
    archon_transform(state, true, true, 45, NULL, 0, 100, true);

    int needed = archon_get_reforms_needed_for_next_level(state);
    assert(needed == 15); /* Need 15 for level 4 */

    /* Do 5 reforms */
    for (int i = 0; i < 5; i++) {
        archon_reform_necromancer(state, 1000 + i);
    }

    needed = archon_get_reforms_needed_for_next_level(state);
    assert(needed == 10); /* 10 more needed */

    archon_state_destroy(state);
    PASS();
}

void test_privilege_name(void) {
    TEST(test_privilege_name);
    assert(strcmp(archon_get_privilege_name(PRIVILEGE_NETWORK_PATCHING), "Network Patching") == 0);
    assert(strcmp(archon_get_privilege_name(PRIVILEGE_DIVINE_SUMMONING), "Divine Summoning") == 0);
    PASS();
}

int main(void) {
    printf("Running Archon Transformation System tests...\n\n");

    test_create_destroy();
    test_transform();
    test_transform_requirements();
    test_grant_privilege();
    test_reform_necromancer();
    test_advance_level();
    test_check_code_violation_corruption();
    test_check_code_violation_massacre();
    test_get_transformation_cutscene();
    test_get_privilege_info();
    test_get_reforms_needed();
    test_privilege_name();

    printf("\n========================================\n");
    printf("Tests passed: %d/%d\n", tests_passed, tests_run);
    printf("========================================\n");

    return (tests_passed == tests_run) ? 0 : 1;
}

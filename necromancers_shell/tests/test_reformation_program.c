/**
 * test_reformation_program.c - Unit tests for Reformation Program System
 */

#include "../src/game/narrative/reformation_program.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) do { printf("Running test: %s...", #name); tests_run++; } while(0)
#define PASS() do { printf(" PASSED\n"); tests_passed++; } while(0)

void test_create_destroy(void) {
    TEST(test_create_destroy);
    ReformationProgram* prog = reformation_program_create();
    assert(prog != NULL);
    reformation_program_destroy(prog);
    PASS();
}

void test_initialize(void) {
    TEST(test_initialize);
    ReformationProgram* prog = reformation_program_create();
    bool result = reformation_program_initialize(prog, 547);
    assert(result == true);
    assert(prog->target_count == 147);
    assert(prog->days_remaining == 547);
    reformation_program_destroy(prog);
    PASS();
}

void test_start_session(void) {
    TEST(test_start_session);
    ReformationProgram* prog = reformation_program_create();
    reformation_program_initialize(prog, 547);

    int npc_id = prog->targets[0].npc_id;
    int reduction = 0, attitude_change = 0;

    bool result = reformation_program_start_session(prog, npc_id, APPROACH_DIPLOMATIC,
                                                   &reduction, &attitude_change);
    assert(result == true);
    assert(reduction > 0);
    assert(prog->total_sessions == 1);

    reformation_program_destroy(prog);
    PASS();
}

void test_session_cooldown(void) {
    TEST(test_session_cooldown);
    ReformationProgram* prog = reformation_program_create();
    reformation_program_initialize(prog, 547);

    int npc_id = prog->targets[0].npc_id;

    /* First session should succeed */
    bool result = reformation_program_start_session(prog, npc_id, APPROACH_DIPLOMATIC, NULL, NULL);
    assert(result == true);

    /* Immediate second session should fail (cooldown) */
    result = reformation_program_start_session(prog, npc_id, APPROACH_DIPLOMATIC, NULL, NULL);
    assert(result == false);

    reformation_program_destroy(prog);
    PASS();
}

void test_check_reformed(void) {
    TEST(test_check_reformed);
    ReformationProgram* prog = reformation_program_create();
    reformation_program_initialize(prog, 547);

    int npc_id = prog->targets[0].npc_id;

    /* Do multiple sessions until reformed */
    for (int i = 0; i < 20; i++) {
        reformation_program_advance_time(prog, 3);
        reformation_program_start_session(prog, npc_id, APPROACH_HARSH, NULL, NULL);

        if (reformation_program_check_reformed(prog, npc_id)) {
            assert(prog->targets_reformed > 0);
            break;
        }
    }

    reformation_program_destroy(prog);
    PASS();
}

void test_advance_time(void) {
    TEST(test_advance_time);
    ReformationProgram* prog = reformation_program_create();
    reformation_program_initialize(prog, 547);

    int initial_days = prog->days_remaining;
    reformation_program_advance_time(prog, 10);
    assert(prog->days_remaining == initial_days - 10);

    reformation_program_destroy(prog);
    PASS();
}

void test_get_progress(void) {
    TEST(test_get_progress);
    ReformationProgram* prog = reformation_program_create();
    reformation_program_initialize(prog, 547);

    int reformed, in_progress, failed, days;
    double percentage;

    reformation_program_get_progress(prog, &reformed, &in_progress, &failed, &days, &percentage);

    assert(reformed == 0);
    assert(failed == 0);
    assert(days == 547);
    assert(percentage == 0.0);

    reformation_program_destroy(prog);
    PASS();
}

void test_generate_report(void) {
    TEST(test_generate_report);
    ReformationProgram* prog = reformation_program_create();
    reformation_program_initialize(prog, 547);

    char buffer[1024];
    int written = reformation_program_generate_report(prog, buffer, sizeof(buffer));

    assert(written > 0);
    assert(strstr(buffer, "REFORMATION PROGRAM") != NULL);
    assert(strstr(buffer, "147") != NULL);

    reformation_program_destroy(prog);
    PASS();
}

void test_resistance_to_string(void) {
    TEST(test_resistance_to_string);
    assert(strcmp(reformation_program_resistance_to_string(RESISTANCE_LOW), "Low") == 0);
    assert(strcmp(reformation_program_resistance_to_string(RESISTANCE_EXTREME), "Extreme") == 0);
    PASS();
}

void test_approach_to_string(void) {
    TEST(test_approach_to_string);
    assert(strcmp(reformation_program_approach_to_string(APPROACH_DIPLOMATIC), "Diplomatic") == 0);
    assert(strcmp(reformation_program_approach_to_string(APPROACH_HARSH), "Harsh") == 0);
    PASS();
}

void test_calculate_attitude(void) {
    TEST(test_calculate_attitude);
    assert(reformation_program_calculate_attitude(-30) == ATTITUDE_HOSTILE);
    assert(reformation_program_calculate_attitude(0) == ATTITUDE_NEUTRAL);
    assert(reformation_program_calculate_attitude(35) == ATTITUDE_TRUSTING);
    PASS();
}

int main(void) {
    printf("Running Reformation Program System tests...\n\n");

    /* Seed random for consistent test results */
    srand(12345);

    test_create_destroy();
    test_initialize();
    test_start_session();
    test_session_cooldown();
    test_check_reformed();
    test_advance_time();
    test_get_progress();
    test_generate_report();
    test_resistance_to_string();
    test_approach_to_string();
    test_calculate_attitude();

    printf("\n========================================\n");
    printf("Tests passed: %d/%d\n", tests_passed, tests_run);
    printf("========================================\n");

    return (tests_passed == tests_run) ? 0 : 1;
}

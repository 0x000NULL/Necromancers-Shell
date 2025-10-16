/**
 * @file test_divine_summons_event.c
 * @brief Unit tests for Divine summons event system
 */

#include "../src/game/events/divine_summons_event.h"
#include "../src/game/events/event_scheduler.h"
#include "../src/game/game_state.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) \
    do { \
        printf("  Testing %s... ", name); \
        tests_run++; \
    } while(0)

#define PASS() \
    do { \
        printf("PASS\n"); \
        tests_passed++; \
    } while(0)

void test_event_registration(void) {
    TEST("event registration");

    EventScheduler* scheduler = event_scheduler_create();
    GameState* state = game_state_create();

    assert(scheduler != NULL);
    assert(state != NULL);

    divine_summons_reset_for_testing();

    /* Register event */
    bool result = divine_summons_register_event(scheduler, state);
    assert(result == true);

    /* Should not register twice */
    result = divine_summons_register_event(scheduler, state);
    assert(result == false);

    event_scheduler_destroy(scheduler);
    game_state_destroy(state);

    PASS();
}

void test_summons_state_transitions(void) {
    TEST("summons state transitions");

    GameState* state = game_state_create();
    assert(state != NULL);

    divine_summons_reset_for_testing();

    /* Initial state */
    assert(divine_summons_was_received(state) == false);
    assert(divine_summons_trials_unlocked(state) == false);

    /* Simulate event callback */
    state->resources.day_count = 155;
    divine_summons_event_callback(state, 155);

    /* Should be received now */
    assert(divine_summons_was_received(state) == true);
    assert(divine_summons_get_state(state) == SUMMONS_RECEIVED);

    game_state_destroy(state);

    PASS();
}

void test_summons_acknowledgment(void) {
    TEST("summons acknowledgment");

    GameState* state = game_state_create();
    assert(state != NULL);

    divine_summons_reset_for_testing();

    /* Setup: trigger summons */
    state->resources.day_count = 155;
    divine_summons_event_callback(state, 155);

    /* Acknowledge within deadline */
    state->resources.day_count = 156;
    bool result = divine_summons_acknowledge(state);
    assert(result == true);
    assert(divine_summons_get_state(state) == SUMMONS_ACKNOWLEDGED);
    assert(divine_summons_trials_unlocked(state) == true);

    game_state_destroy(state);

    PASS();
}

void test_summons_deadline(void) {
    TEST("summons deadline");

    GameState* state = game_state_create();
    assert(state != NULL);

    divine_summons_reset_for_testing();

    /* Setup: trigger summons */
    state->resources.day_count = 155;
    divine_summons_event_callback(state, 155);

    /* Miss deadline */
    state->resources.day_count = 163; /* Past Day 162 deadline */
    bool result = divine_summons_acknowledge(state);
    assert(result == false); /* Should fail */
    assert(divine_summons_is_ignored(state) == true);

    game_state_destroy(state);

    PASS();
}

int main(void) {
    printf("\n=== Divine Summons Event Tests ===\n\n");

    test_event_registration();
    test_summons_state_transitions();
    test_summons_acknowledgment();
    test_summons_deadline();

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    return (tests_run == tests_passed) ? 0 : 1;
}

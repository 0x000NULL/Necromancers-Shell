/**
 * @file test_thessara_contact_event.c
 * @brief Unit tests for Thessara contact event system
 */

#include "../src/game/events/thessara_contact_event.h"
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

    /* Reset for clean test */
    thessara_contact_reset_for_testing();

    /* Register event */
    bool result = thessara_contact_register_event(scheduler, state);
    assert(result == true);

    /* Should not register twice */
    result = thessara_contact_register_event(scheduler, state);
    assert(result == false);

    event_scheduler_destroy(scheduler);
    game_state_destroy(state);

    PASS();
}

void test_contact_state_transitions(void) {
    TEST("contact state transitions");

    GameState* state = game_state_create();
    assert(state != NULL);

    thessara_contact_reset_for_testing();

    /* Initial state */
    assert(thessara_was_contacted(state) == false);
    assert(thessara_paths_revealed(state) == false);

    /* Simulate event callback */
    state->resources.day_count = 50;
    thessara_contact_event_callback(state, 50);

    /* Should be contacted now */
    assert(thessara_was_contacted(state) == true);
    assert(thessara_contact_get_state(state) == THESSARA_CONTACTED);

    /* Initiate conversation */
    thessara_initiate_conversation(state);

    /* Paths should be revealed */
    assert(thessara_paths_revealed(state) == true);
    assert(thessara_contact_get_state(state) == THESSARA_PATHS_REVEALED);

    game_state_destroy(state);

    PASS();
}

void test_guidance_acceptance(void) {
    TEST("guidance acceptance");

    GameState* state = game_state_create();
    assert(state != NULL);

    thessara_contact_reset_for_testing();

    /* Setup: contact and reveal paths */
    state->resources.day_count = 50;
    thessara_contact_event_callback(state, 50);
    thessara_initiate_conversation(state);

    /* Accept guidance */
    bool result = thessara_accept_guidance(state);
    assert(result == true);
    assert(thessara_contact_get_state(state) == THESSARA_TRUST_ESTABLISHED);

    game_state_destroy(state);

    PASS();
}

void test_guidance_rejection(void) {
    TEST("guidance rejection");

    GameState* state = game_state_create();
    assert(state != NULL);

    thessara_contact_reset_for_testing();

    /* Setup: contact and reveal paths */
    state->resources.day_count = 50;
    thessara_contact_event_callback(state, 50);
    thessara_initiate_conversation(state);

    /* Reject guidance */
    bool result = thessara_reject_guidance(state);
    assert(result == true);
    assert(thessara_contact_get_state(state) == THESSARA_CONTACTED);

    game_state_destroy(state);

    PASS();
}

int main(void) {
    printf("\n=== Thessara Contact Event Tests ===\n\n");

    test_event_registration();
    test_contact_state_transitions();
    test_guidance_acceptance();
    test_guidance_rejection();

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    return (tests_run == tests_passed) ? 0 : 1;
}

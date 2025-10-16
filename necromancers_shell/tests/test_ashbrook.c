/**
 * @file test_ashbrook.c
 * @brief Unit tests for Ashbrook event
 */

#include "../src/game/events/ashbrook_event.h"
#include "../src/game/events/event_scheduler.h"
#include "../src/game/game_state.h"
#include "../src/utils/logger.h"
#include <assert.h>
#include <stdio.h>

void test_ashbrook_initial_state(void) {
    printf("Test: ashbrook_initial_state... ");

    ashbrook_reset_for_testing();
    GameState* state = game_state_create();
    assert(state != NULL);

    /* Initially not triggered */
    assert(ashbrook_get_state(state) == ASHBROOK_NOT_TRIGGERED);
    assert(ashbrook_was_harvested(state) == false);
    assert(ashbrook_was_spared(state) == false);

    uint32_t souls = 0, energy = 0;
    assert(ashbrook_get_statistics(state, &souls, &energy) == false);

    game_state_destroy(state);

    printf("PASS\n");
}

void test_ashbrook_register_event(void) {
    printf("Test: ashbrook_register_event... ");

    ashbrook_reset_for_testing();

    /* Create scheduler first, then state */
    EventScheduler* scheduler = event_scheduler_create();
    assert(scheduler != NULL);

    /* Create GameState without auto-registration by using a fresh scheduler */
    /* Note: game_state_create() will register events, so we need to reset first */
    GameState* state = game_state_create();
    assert(state != NULL);

    /* The event should already be registered during game_state_create() */
    /* So we verify it exists in the state's scheduler */
    const ScheduledEvent* event = event_scheduler_get_event(state->event_scheduler, 47);
    assert(event != NULL);
    assert(event->trigger_type == EVENT_TRIGGER_DAY);
    assert(event->trigger_value == 47);
    assert(event->priority == EVENT_PRIORITY_CRITICAL);

    /* Trying to register again should fail */
    bool success = ashbrook_register_event(state->event_scheduler, state);
    assert(success == false);

    event_scheduler_destroy(scheduler);
    game_state_destroy(state);

    printf("PASS\n");
}

void test_ashbrook_event_triggers_on_day_47(void) {
    printf("Test: ashbrook_event_triggers_on_day_47... ");

    ashbrook_reset_for_testing();
    GameState* state = game_state_create();
    assert(state != NULL);

    /* Register event */
    ashbrook_register_event(state->event_scheduler, state);

    /* Advance to day 46 */
    game_state_advance_time(state, 46 * 24);
    assert(state->resources.day_count == 46);

    /* Event should not trigger yet */
    assert(ashbrook_get_state(state) == ASHBROOK_NOT_TRIGGERED);

    /* Advance to day 47 */
    game_state_advance_time(state, 24);
    assert(state->resources.day_count == 47);

    /* Event callback was called, but state is still NOT_TRIGGERED
     * (waiting for player choice) */
    /* Note: The callback sets warned flag but doesn't change state */

    game_state_destroy(state);

    printf("PASS\n");
}

void test_ashbrook_harvest(void) {
    printf("Test: ashbrook_harvest... ");

    ashbrook_reset_for_testing();
    GameState* state = game_state_create();
    assert(state != NULL);

    /* Record initial state */
    uint32_t initial_corruption = state->corruption.corruption;
    uint32_t initial_soul_count = soul_manager_count(state->souls);
    uint64_t initial_energy = state->resources.soul_energy;

    /* Register and trigger event */
    ashbrook_register_event(state->event_scheduler, state);
    game_state_advance_time(state, 47 * 24);

    /* Now harvest the village */
    bool success = ashbrook_harvest_village(state);
    assert(success == true);

    /* Check state */
    assert(ashbrook_get_state(state) == ASHBROOK_HARVESTED);
    assert(ashbrook_was_harvested(state) == true);
    assert(ashbrook_was_spared(state) == false);

    /* Check corruption increased */
    assert(state->corruption.corruption > initial_corruption);
    assert(state->corruption.corruption == initial_corruption + 13);

    /* Check souls were added */
    uint32_t final_soul_count = soul_manager_count(state->souls);
    assert(final_soul_count > initial_soul_count);
    assert(final_soul_count - initial_soul_count == 147);

    /* Check energy increased significantly */
    assert(state->resources.soul_energy > initial_energy);
    uint64_t energy_gain = state->resources.soul_energy - initial_energy;
    assert(energy_gain > 2000); /* Should be ~2700-3000 based on soul quality distribution */

    /* Check statistics */
    uint32_t souls_gained = 0, energy_gained = 0;
    success = ashbrook_get_statistics(state, &souls_gained, &energy_gained);
    assert(success == true);
    assert(souls_gained == 147);
    assert(energy_gained > 2000);

    /* Check flag was set */
    assert(event_scheduler_has_flag(state->event_scheduler, "ashbrook_harvested") == true);

    game_state_destroy(state);

    printf("PASS\n");
}

void test_ashbrook_spare(void) {
    printf("Test: ashbrook_spare... ");

    ashbrook_reset_for_testing();
    GameState* state = game_state_create();
    assert(state != NULL);

    /* Set initial corruption to something we can reduce */
    state->corruption.corruption = 50;
    uint32_t initial_corruption = state->corruption.corruption;
    uint32_t initial_soul_count = soul_manager_count(state->souls);

    /* Register and trigger event */
    ashbrook_register_event(state->event_scheduler, state);
    game_state_advance_time(state, 47 * 24);

    /* Spare the village */
    bool success = ashbrook_spare_village(state);
    assert(success == true);

    /* Check state */
    assert(ashbrook_get_state(state) == ASHBROOK_SPARED);
    assert(ashbrook_was_harvested(state) == false);
    assert(ashbrook_was_spared(state) == true);

    /* Check corruption decreased */
    assert(state->corruption.corruption < initial_corruption);
    assert(state->corruption.corruption == initial_corruption - 2);

    /* Check no souls were added */
    uint32_t final_soul_count = soul_manager_count(state->souls);
    assert(final_soul_count == initial_soul_count);

    /* Check statistics */
    uint32_t souls_gained = 0, energy_gained = 0;
    success = ashbrook_get_statistics(state, &souls_gained, &energy_gained);
    assert(success == true);
    assert(souls_gained == 0);
    assert(energy_gained == 0);

    /* Check flag was set */
    assert(event_scheduler_has_flag(state->event_scheduler, "ashbrook_spared") == true);

    game_state_destroy(state);

    printf("PASS\n");
}

void test_ashbrook_cannot_harvest_twice(void) {
    printf("Test: ashbrook_cannot_harvest_twice... ");

    ashbrook_reset_for_testing();
    GameState* state = game_state_create();
    assert(state != NULL);

    /* Register and trigger event */
    ashbrook_register_event(state->event_scheduler, state);
    game_state_advance_time(state, 47 * 24);

    /* First harvest succeeds */
    bool success = ashbrook_harvest_village(state);
    assert(success == true);

    uint32_t souls_after_first = soul_manager_count(state->souls);

    /* Second harvest fails */
    success = ashbrook_harvest_village(state);
    assert(success == false);

    /* Soul count unchanged */
    assert(soul_manager_count(state->souls) == souls_after_first);

    game_state_destroy(state);

    printf("PASS\n");
}

void test_ashbrook_cannot_spare_after_harvest(void) {
    printf("Test: ashbrook_cannot_spare_after_harvest... ");

    ashbrook_reset_for_testing();
    GameState* state = game_state_create();
    assert(state != NULL);

    /* Register and trigger event */
    ashbrook_register_event(state->event_scheduler, state);
    game_state_advance_time(state, 47 * 24);

    /* Harvest first */
    ashbrook_harvest_village(state);
    assert(ashbrook_get_state(state) == ASHBROOK_HARVESTED);

    /* Cannot spare after harvesting */
    bool success = ashbrook_spare_village(state);
    assert(success == false);
    assert(ashbrook_get_state(state) == ASHBROOK_HARVESTED);

    game_state_destroy(state);

    printf("PASS\n");
}

void test_ashbrook_cannot_harvest_after_spare(void) {
    printf("Test: ashbrook_cannot_harvest_after_spare... ");

    ashbrook_reset_for_testing();
    GameState* state = game_state_create();
    assert(state != NULL);

    /* Register and trigger event */
    ashbrook_register_event(state->event_scheduler, state);
    game_state_advance_time(state, 47 * 24);

    /* Spare first */
    ashbrook_spare_village(state);
    assert(ashbrook_get_state(state) == ASHBROOK_SPARED);

    uint32_t souls_after_spare = soul_manager_count(state->souls);

    /* Cannot harvest after sparing */
    bool success = ashbrook_harvest_village(state);
    assert(success == false);
    assert(ashbrook_get_state(state) == ASHBROOK_SPARED);

    /* No souls gained */
    assert(soul_manager_count(state->souls) == souls_after_spare);

    game_state_destroy(state);

    printf("PASS\n");
}

void test_ashbrook_soul_distribution(void) {
    printf("Test: ashbrook_soul_distribution... ");

    ashbrook_reset_for_testing();
    GameState* state = game_state_create();
    assert(state != NULL);

    /* Register and trigger event */
    ashbrook_register_event(state->event_scheduler, state);
    game_state_advance_time(state, 47 * 24);

    /* Harvest village */
    ashbrook_harvest_village(state);

    /* Check soul type distribution (approximate) */
    uint32_t common = soul_manager_count_by_type(state->souls, SOUL_TYPE_COMMON);
    uint32_t warrior = soul_manager_count_by_type(state->souls, SOUL_TYPE_WARRIOR);
    uint32_t mage = soul_manager_count_by_type(state->souls, SOUL_TYPE_MAGE);
    uint32_t innocent = soul_manager_count_by_type(state->souls, SOUL_TYPE_INNOCENT);

    /* Ashbrook has:
     * - 120 Common
     * - 20 Warrior
     * - 5 Mage
     * - 2 Innocent
     */
    assert(common == 120);
    assert(warrior == 20);
    assert(mage == 5);
    assert(innocent == 2);

    /* Total should be 147 */
    assert(common + warrior + mage + innocent == 147);

    game_state_destroy(state);

    printf("PASS\n");
}

void test_ashbrook_before_trigger(void) {
    printf("Test: ashbrook_before_trigger... ");

    ashbrook_reset_for_testing();
    GameState* state = game_state_create();
    assert(state != NULL);

    /* Register but don't trigger event */
    ashbrook_register_event(state->event_scheduler, state);

    /* Try to harvest before event triggers */
    bool success = ashbrook_harvest_village(state);
    assert(success == false);

    /* Try to spare before event triggers */
    success = ashbrook_spare_village(state);
    assert(success == false);

    /* State should still be not triggered */
    assert(ashbrook_get_state(state) == ASHBROOK_NOT_TRIGGERED);

    game_state_destroy(state);

    printf("PASS\n");
}

int main(void) {
    /* Suppress log output during tests */
    logger_set_level(LOG_LEVEL_FATAL + 1);

    printf("\n=== Ashbrook Event Tests ===\n\n");

    test_ashbrook_initial_state();
    test_ashbrook_register_event();
    test_ashbrook_event_triggers_on_day_47();
    test_ashbrook_harvest();
    test_ashbrook_spare();
    test_ashbrook_cannot_harvest_twice();
    test_ashbrook_cannot_spare_after_harvest();
    test_ashbrook_cannot_harvest_after_spare();
    test_ashbrook_soul_distribution();
    test_ashbrook_before_trigger();

    printf("\n=== All Ashbrook Event Tests Passed! ===\n\n");

    return 0;
}

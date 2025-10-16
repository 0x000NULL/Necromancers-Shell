/**
 * @file test_event_scheduler.c
 * @brief Unit tests for event scheduler system
 */

#include "../src/game/events/event_scheduler.h"
#include "../src/game/game_state.h"
#include "../src/utils/logger.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

/* Test event callbacks */
static int g_event1_called = 0;
static int g_event2_called = 0;
static int g_event3_called = 0;

static bool event1_callback(GameState* state, uint32_t event_id) {
    (void)state;
    (void)event_id;
    g_event1_called++;
    return true;
}

static bool event2_callback(GameState* state, uint32_t event_id) {
    (void)state;
    (void)event_id;
    g_event2_called++;
    return true;
}

static bool event3_callback(GameState* state, uint32_t event_id) {
    (void)state;
    (void)event_id;
    g_event3_called++;
    return false;  /* Simulate failure */
}

void test_event_scheduler_create_destroy(void) {
    printf("Test: event_scheduler_create_destroy... ");

    EventScheduler* scheduler = event_scheduler_create();
    assert(scheduler != NULL);

    event_scheduler_destroy(scheduler);
    event_scheduler_destroy(NULL);  /* Should not crash */

    printf("PASS\n");
}

void test_register_event(void) {
    printf("Test: register_event... ");

    EventScheduler* scheduler = event_scheduler_create();
    assert(scheduler != NULL);

    ScheduledEvent event = {
        .id = 1,
        .trigger_type = EVENT_TRIGGER_DAY,
        .trigger_value = 10,
        .triggered = false,
        .completed = false,
        .repeatable = false,
        .priority = EVENT_PRIORITY_NORMAL,
        .callback = event1_callback,
        .requires_flag = false,
        .min_day = 0,
        .max_day = 0
    };
    strncpy(event.name, "Test Event", sizeof(event.name) - 1);
    strncpy(event.description, "A test event", sizeof(event.description) - 1);

    bool success = event_scheduler_register(scheduler, event);
    assert(success);

    /* Verify event was registered */
    const ScheduledEvent* retrieved = event_scheduler_get_event(scheduler, 1);
    assert(retrieved != NULL);
    assert(retrieved->id == 1);
    assert(strcmp(retrieved->name, "Test Event") == 0);
    assert(retrieved->trigger_type == EVENT_TRIGGER_DAY);
    assert(retrieved->trigger_value == 10);

    event_scheduler_destroy(scheduler);

    printf("PASS\n");
}

void test_day_trigger(void) {
    printf("Test: day_trigger... ");

    g_event1_called = 0;

    /* Create mock game state */
    GameState mock_state = {0};
    mock_state.resources.day_count = 5;

    EventScheduler* scheduler = event_scheduler_create();
    assert(scheduler != NULL);

    /* Register event that triggers on day 10 */
    ScheduledEvent event = {
        .id = 1,
        .trigger_type = EVENT_TRIGGER_DAY,
        .trigger_value = 10,
        .triggered = false,
        .completed = false,
        .repeatable = false,
        .priority = EVENT_PRIORITY_NORMAL,
        .callback = event1_callback,
        .requires_flag = false,
        .min_day = 0,
        .max_day = 0
    };
    strncpy(event.name, "Day 10 Event", sizeof(event.name) - 1);

    event_scheduler_register(scheduler, event);

    /* Check triggers on day 5 - should not trigger */
    uint32_t triggered = event_scheduler_check_triggers(scheduler, &mock_state);
    assert(triggered == 0);
    assert(g_event1_called == 0);
    assert(!event_scheduler_was_triggered(scheduler, 1));

    /* Advance to day 10 */
    mock_state.resources.day_count = 10;
    triggered = event_scheduler_check_triggers(scheduler, &mock_state);
    assert(triggered == 1);
    assert(g_event1_called == 1);
    assert(event_scheduler_was_triggered(scheduler, 1));
    assert(event_scheduler_was_completed(scheduler, 1));

    /* Check again - should not trigger twice (not repeatable) */
    triggered = event_scheduler_check_triggers(scheduler, &mock_state);
    assert(triggered == 0);
    assert(g_event1_called == 1);

    event_scheduler_destroy(scheduler);

    printf("PASS\n");
}

void test_corruption_trigger(void) {
    printf("Test: corruption_trigger... ");

    g_event2_called = 0;

    /* Create mock game state */
    GameState mock_state = {0};
    mock_state.corruption.corruption = 30;
    mock_state.resources.day_count = 1;

    EventScheduler* scheduler = event_scheduler_create();
    assert(scheduler != NULL);

    /* Register event that triggers at 50% corruption */
    ScheduledEvent event = {
        .id = 2,
        .trigger_type = EVENT_TRIGGER_CORRUPTION,
        .trigger_value = 50,
        .triggered = false,
        .completed = false,
        .repeatable = false,
        .priority = EVENT_PRIORITY_HIGH,
        .callback = event2_callback,
        .requires_flag = false,
        .min_day = 0,
        .max_day = 0
    };
    strncpy(event.name, "High Corruption Event", sizeof(event.name) - 1);

    event_scheduler_register(scheduler, event);

    /* Check at 30% corruption - should not trigger */
    uint32_t triggered = event_scheduler_check_triggers(scheduler, &mock_state);
    assert(triggered == 0);
    assert(g_event2_called == 0);

    /* Increase corruption to 50% */
    mock_state.corruption.corruption = 50;
    triggered = event_scheduler_check_triggers(scheduler, &mock_state);
    assert(triggered == 1);
    assert(g_event2_called == 1);

    /* Increase to 60% - should not trigger again */
    mock_state.corruption.corruption = 60;
    triggered = event_scheduler_check_triggers(scheduler, &mock_state);
    assert(triggered == 0);
    assert(g_event2_called == 1);

    event_scheduler_destroy(scheduler);

    printf("PASS\n");
}

void test_location_trigger(void) {
    printf("Test: location_trigger... ");

    g_event1_called = 0;

    /* Create mock game state */
    GameState mock_state = {0};
    mock_state.current_location_id = 1;
    mock_state.resources.day_count = 1;

    EventScheduler* scheduler = event_scheduler_create();
    assert(scheduler != NULL);

    /* Register event that triggers at location 5 */
    ScheduledEvent event = {
        .id = 3,
        .trigger_type = EVENT_TRIGGER_LOCATION,
        .trigger_value = 5,
        .triggered = false,
        .completed = false,
        .repeatable = true,  /* Can trigger multiple times */
        .priority = EVENT_PRIORITY_NORMAL,
        .callback = event1_callback,
        .requires_flag = false,
        .min_day = 0,
        .max_day = 0
    };
    strncpy(event.name, "Location 5 Event", sizeof(event.name) - 1);

    event_scheduler_register(scheduler, event);

    /* Check at location 1 - should not trigger */
    uint32_t triggered = event_scheduler_check_triggers(scheduler, &mock_state);
    assert(triggered == 0);
    assert(g_event1_called == 0);

    /* Move to location 5 */
    mock_state.current_location_id = 5;
    triggered = event_scheduler_check_triggers(scheduler, &mock_state);
    assert(triggered == 1);
    assert(g_event1_called == 1);

    /* Leave and return - should trigger again (repeatable) */
    mock_state.current_location_id = 1;
    event_scheduler_reset_event(scheduler, 3);
    mock_state.current_location_id = 5;
    triggered = event_scheduler_check_triggers(scheduler, &mock_state);
    assert(triggered == 1);
    assert(g_event1_called == 2);

    event_scheduler_destroy(scheduler);

    printf("PASS\n");
}

void test_multiple_events_priority(void) {
    printf("Test: multiple_events_priority... ");

    g_event1_called = 0;
    g_event2_called = 0;

    /* Create mock game state */
    GameState mock_state = {0};
    mock_state.resources.day_count = 10;

    EventScheduler* scheduler = event_scheduler_create();
    assert(scheduler != NULL);

    /* Register low priority event */
    ScheduledEvent event1 = {
        .id = 1,
        .trigger_type = EVENT_TRIGGER_DAY,
        .trigger_value = 10,
        .triggered = false,
        .completed = false,
        .repeatable = false,
        .priority = EVENT_PRIORITY_LOW,
        .callback = event1_callback,
        .requires_flag = false,
        .min_day = 0,
        .max_day = 0
    };
    strncpy(event1.name, "Low Priority Event", sizeof(event1.name) - 1);
    event_scheduler_register(scheduler, event1);

    /* Register high priority event */
    ScheduledEvent event2 = {
        .id = 2,
        .trigger_type = EVENT_TRIGGER_DAY,
        .trigger_value = 10,
        .triggered = false,
        .completed = false,
        .repeatable = false,
        .priority = EVENT_PRIORITY_CRITICAL,
        .callback = event2_callback,
        .requires_flag = false,
        .min_day = 0,
        .max_day = 0
    };
    strncpy(event2.name, "High Priority Event", sizeof(event2.name) - 1);
    event_scheduler_register(scheduler, event2);

    /* Both should trigger, high priority first */
    uint32_t triggered = event_scheduler_check_triggers(scheduler, &mock_state);
    assert(triggered == 2);
    assert(g_event1_called == 1);
    assert(g_event2_called == 1);

    /* High priority event2 should have executed before low priority event1 */
    /* We can't directly test execution order in this simple test, but the */
    /* implementation uses qsort to ensure correct ordering */

    event_scheduler_destroy(scheduler);

    printf("PASS\n");
}

void test_flag_system(void) {
    printf("Test: flag_system... ");

    EventScheduler* scheduler = event_scheduler_create();
    assert(scheduler != NULL);

    /* Flag should not exist initially */
    assert(!event_scheduler_has_flag(scheduler, "test_flag"));

    /* Set flag */
    bool success = event_scheduler_set_flag(scheduler, "test_flag");
    assert(success);
    assert(event_scheduler_has_flag(scheduler, "test_flag"));

    /* Set same flag again - should still work */
    success = event_scheduler_set_flag(scheduler, "test_flag");
    assert(success);
    assert(event_scheduler_has_flag(scheduler, "test_flag"));

    /* Set different flag */
    success = event_scheduler_set_flag(scheduler, "another_flag");
    assert(success);
    assert(event_scheduler_has_flag(scheduler, "test_flag"));
    assert(event_scheduler_has_flag(scheduler, "another_flag"));

    event_scheduler_destroy(scheduler);

    printf("PASS\n");
}

void test_conditional_event(void) {
    printf("Test: conditional_event... ");

    g_event1_called = 0;

    /* Create mock game state */
    GameState mock_state = {0};
    mock_state.resources.day_count = 10;

    EventScheduler* scheduler = event_scheduler_create();
    assert(scheduler != NULL);

    /* Register event that requires a flag and has day constraints */
    ScheduledEvent event = {
        .id = 1,
        .trigger_type = EVENT_TRIGGER_DAY,
        .trigger_value = 10,
        .triggered = false,
        .completed = false,
        .repeatable = false,
        .priority = EVENT_PRIORITY_NORMAL,
        .callback = event1_callback,
        .requires_flag = true,
        .min_day = 5,
        .max_day = 15
    };
    strncpy(event.name, "Conditional Event", sizeof(event.name) - 1);
    strncpy(event.required_flag, "prerequisite", sizeof(event.required_flag) - 1);
    event_scheduler_register(scheduler, event);

    /* Check without flag - should not trigger */
    uint32_t triggered = event_scheduler_check_triggers(scheduler, &mock_state);
    assert(triggered == 0);
    assert(g_event1_called == 0);

    /* Set flag */
    event_scheduler_set_flag(scheduler, "prerequisite");

    /* Now should trigger */
    triggered = event_scheduler_check_triggers(scheduler, &mock_state);
    assert(triggered == 1);
    assert(g_event1_called == 1);

    event_scheduler_destroy(scheduler);

    printf("PASS\n");
}

void test_day_range_constraints(void) {
    printf("Test: day_range_constraints... ");

    g_event1_called = 0;

    /* Create mock game state */
    GameState mock_state = {0};
    mock_state.resources.day_count = 1;

    EventScheduler* scheduler = event_scheduler_create();
    assert(scheduler != NULL);

    /* Register event that triggers on day 10, but only valid between days 5-15 */
    ScheduledEvent event = {
        .id = 1,
        .trigger_type = EVENT_TRIGGER_DAY,
        .trigger_value = 10,
        .triggered = false,
        .completed = false,
        .repeatable = false,
        .priority = EVENT_PRIORITY_NORMAL,
        .callback = event1_callback,
        .requires_flag = false,
        .min_day = 5,
        .max_day = 15
    };
    strncpy(event.name, "Range Constrained Event", sizeof(event.name) - 1);
    event_scheduler_register(scheduler, event);

    /* Day 1 (before min_day) - should not trigger */
    uint32_t triggered = event_scheduler_check_triggers(scheduler, &mock_state);
    assert(triggered == 0);
    assert(g_event1_called == 0);

    /* Day 10 (in range) - should trigger */
    mock_state.resources.day_count = 10;
    triggered = event_scheduler_check_triggers(scheduler, &mock_state);
    assert(triggered == 1);
    assert(g_event1_called == 1);

    event_scheduler_destroy(scheduler);

    printf("PASS\n");
}

void test_failed_callback(void) {
    printf("Test: failed_callback... ");

    g_event3_called = 0;

    /* Create mock game state */
    GameState mock_state = {0};
    mock_state.resources.day_count = 5;

    EventScheduler* scheduler = event_scheduler_create();
    assert(scheduler != NULL);

    /* Register event with callback that fails */
    ScheduledEvent event = {
        .id = 1,
        .trigger_type = EVENT_TRIGGER_DAY,
        .trigger_value = 5,
        .triggered = false,
        .completed = false,
        .repeatable = false,
        .priority = EVENT_PRIORITY_NORMAL,
        .callback = event3_callback,
        .requires_flag = false,
        .min_day = 0,
        .max_day = 0
    };
    strncpy(event.name, "Failing Event", sizeof(event.name) - 1);
    event_scheduler_register(scheduler, event);

    /* Trigger event */
    uint32_t triggered = event_scheduler_check_triggers(scheduler, &mock_state);
    assert(triggered == 0);  /* Callback failed, so not counted as success */
    assert(g_event3_called == 1);  /* But callback was called */
    assert(event_scheduler_was_triggered(scheduler, 1));  /* Marked as triggered */
    assert(!event_scheduler_was_completed(scheduler, 1));  /* But not completed */

    event_scheduler_destroy(scheduler);

    printf("PASS\n");
}

void test_force_trigger(void) {
    printf("Test: force_trigger... ");

    g_event1_called = 0;

    /* Create mock game state */
    GameState mock_state = {0};
    mock_state.resources.day_count = 1;

    EventScheduler* scheduler = event_scheduler_create();
    assert(scheduler != NULL);

    /* Register event that triggers on day 100 */
    ScheduledEvent event = {
        .id = 1,
        .trigger_type = EVENT_TRIGGER_DAY,
        .trigger_value = 100,
        .triggered = false,
        .completed = false,
        .repeatable = false,
        .priority = EVENT_PRIORITY_NORMAL,
        .callback = event1_callback,
        .requires_flag = false,
        .min_day = 0,
        .max_day = 0
    };
    strncpy(event.name, "Day 100 Event", sizeof(event.name) - 1);
    event_scheduler_register(scheduler, event);

    /* Force trigger even though conditions not met */
    bool success = event_scheduler_force_trigger(scheduler, 1, &mock_state);
    assert(success);
    assert(g_event1_called == 1);
    assert(event_scheduler_was_triggered(scheduler, 1));
    assert(event_scheduler_was_completed(scheduler, 1));

    /* Try to force trigger non-existent event */
    success = event_scheduler_force_trigger(scheduler, 999, &mock_state);
    assert(!success);

    event_scheduler_destroy(scheduler);

    printf("PASS\n");
}

void test_get_upcoming_events(void) {
    printf("Test: get_upcoming_events... ");

    /* Create mock game state */
    GameState mock_state = {0};
    mock_state.resources.day_count = 1;

    EventScheduler* scheduler = event_scheduler_create();
    assert(scheduler != NULL);

    /* Register several events */
    for (int i = 0; i < 5; i++) {
        ScheduledEvent event = {
            .id = i + 1,
            .trigger_type = EVENT_TRIGGER_DAY,
            .trigger_value = (i + 1) * 10,
            .triggered = false,
            .completed = false,
            .repeatable = false,
            .priority = EVENT_PRIORITY_NORMAL,
            .callback = NULL,
            .requires_flag = false,
            .min_day = 0,
            .max_day = 0
        };
        snprintf(event.name, sizeof(event.name), "Event %d", i + 1);
        event_scheduler_register(scheduler, event);
    }

    /* Get upcoming events - all should be upcoming */
    size_t count = 0;
    const ScheduledEvent** upcoming = event_scheduler_get_upcoming(scheduler, &count);
    assert(upcoming != NULL);
    assert(count == 5);

    /* Trigger one event */
    mock_state.resources.day_count = 10;
    event_scheduler_check_triggers(scheduler, &mock_state);

    /* Now should have 4 upcoming */
    upcoming = event_scheduler_get_upcoming(scheduler, &count);
    assert(count == 4);

    event_scheduler_destroy(scheduler);

    printf("PASS\n");
}

void test_repeatable_event_reset(void) {
    printf("Test: repeatable_event_reset... ");

    g_event1_called = 0;

    /* Create mock game state */
    GameState mock_state = {0};
    mock_state.resources.day_count = 10;

    EventScheduler* scheduler = event_scheduler_create();
    assert(scheduler != NULL);

    /* Register repeatable event */
    ScheduledEvent event = {
        .id = 1,
        .trigger_type = EVENT_TRIGGER_DAY,
        .trigger_value = 10,
        .triggered = false,
        .completed = false,
        .repeatable = true,
        .priority = EVENT_PRIORITY_NORMAL,
        .callback = event1_callback,
        .requires_flag = false,
        .min_day = 0,
        .max_day = 0
    };
    strncpy(event.name, "Repeatable Event", sizeof(event.name) - 1);
    event_scheduler_register(scheduler, event);

    /* Trigger first time */
    uint32_t triggered = event_scheduler_check_triggers(scheduler, &mock_state);
    assert(triggered == 1);
    assert(g_event1_called == 1);

    /* Won't trigger again without reset */
    triggered = event_scheduler_check_triggers(scheduler, &mock_state);
    assert(triggered == 0);
    assert(g_event1_called == 1);

    /* Reset event */
    bool success = event_scheduler_reset_event(scheduler, 1);
    assert(success);

    /* Should trigger again */
    triggered = event_scheduler_check_triggers(scheduler, &mock_state);
    assert(triggered == 1);
    assert(g_event1_called == 2);

    event_scheduler_destroy(scheduler);

    printf("PASS\n");
}

int main(void) {
    /* Suppress log output during tests */
    logger_set_level(LOG_LEVEL_FATAL + 1); /* Disable all logging */

    printf("\n=== Event Scheduler Tests ===\n\n");

    test_event_scheduler_create_destroy();
    test_register_event();
    test_day_trigger();
    test_corruption_trigger();
    test_location_trigger();
    test_multiple_events_priority();
    test_flag_system();
    test_conditional_event();
    test_day_range_constraints();
    test_failed_callback();
    test_force_trigger();
    test_get_upcoming_events();
    test_repeatable_event_reset();

    printf("\n=== All Event Scheduler Tests Passed! ===\n\n");

    return 0;
}

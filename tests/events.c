/**
 * Event Bus Tests
 */

#include "core/events.h"
#include "utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Test results */
static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) \
    printf("Running test: %s\n", #name); \
    tests_run++; \
    if (test_##name()) { \
        tests_passed++; \
        printf("  ✓ PASSED\n"); \
    } else { \
        printf("  ✗ FAILED\n"); \
    }

/* Test callback tracking */
static int g_callback_count = 0;
static EventType g_last_event_type = EVENT_NONE;
static void* g_last_event_data = NULL;

static void reset_callback_tracking(void) {
    g_callback_count = 0;
    g_last_event_type = EVENT_NONE;
    g_last_event_data = NULL;
}

static void test_callback(const Event* event, void* userdata) {
    (void)userdata;
    g_callback_count++;
    g_last_event_type = event->type;
    g_last_event_data = event->data;
}

/* Test: Create and destroy */
static bool test_create_destroy(void) {
    EventBus* bus = event_bus_create();
    if (!bus) return false;

    event_bus_destroy(bus);
    return true;
}

/* Test: Subscribe and unsubscribe */
static bool test_subscribe_unsubscribe(void) {
    EventBus* bus = event_bus_create();
    if (!bus) return false;

    size_t sub_id = event_bus_subscribe(bus, EVENT_GAME_START, test_callback, NULL);
    if (sub_id == 0) {
        event_bus_destroy(bus);
        return false;
    }

    if (event_bus_subscriber_count(bus, EVENT_GAME_START) != 1) {
        event_bus_destroy(bus);
        return false;
    }

    if (!event_bus_unsubscribe(bus, sub_id)) {
        event_bus_destroy(bus);
        return false;
    }

    if (event_bus_subscriber_count(bus, EVENT_GAME_START) != 0) {
        event_bus_destroy(bus);
        return false;
    }

    event_bus_destroy(bus);
    return true;
}

/* Test: Publish event */
static bool test_publish(void) {
    EventBus* bus = event_bus_create();
    if (!bus) return false;

    reset_callback_tracking();

    event_bus_subscribe(bus, EVENT_GAME_START, test_callback, NULL);
    event_bus_publish(bus, EVENT_GAME_START, NULL);

    if (g_callback_count != 1) {
        event_bus_destroy(bus);
        return false;
    }

    if (g_last_event_type != EVENT_GAME_START) {
        event_bus_destroy(bus);
        return false;
    }

    event_bus_destroy(bus);
    return true;
}

/* Test: Publish with data */
static bool test_publish_with_data(void) {
    EventBus* bus = event_bus_create();
    if (!bus) return false;

    reset_callback_tracking();

    int test_data = 42;
    event_bus_subscribe(bus, EVENT_PLAYER_DAMAGE_TAKEN, test_callback, NULL);
    event_bus_publish(bus, EVENT_PLAYER_DAMAGE_TAKEN, &test_data);

    if (g_callback_count != 1) {
        event_bus_destroy(bus);
        return false;
    }

    if (!g_last_event_data || *(int*)g_last_event_data != 42) {
        event_bus_destroy(bus);
        return false;
    }

    event_bus_destroy(bus);
    return true;
}

/* Test: Multiple subscribers */
static bool test_multiple_subscribers(void) {
    EventBus* bus = event_bus_create();
    if (!bus) return false;

    reset_callback_tracking();

    /* Subscribe multiple times */
    event_bus_subscribe(bus, EVENT_COMBAT_START, test_callback, NULL);
    event_bus_subscribe(bus, EVENT_COMBAT_START, test_callback, NULL);
    event_bus_subscribe(bus, EVENT_COMBAT_START, test_callback, NULL);

    if (event_bus_subscriber_count(bus, EVENT_COMBAT_START) != 3) {
        event_bus_destroy(bus);
        return false;
    }

    /* Publish should call all subscribers */
    event_bus_publish(bus, EVENT_COMBAT_START, NULL);

    if (g_callback_count != 3) {
        event_bus_destroy(bus);
        return false;
    }

    event_bus_destroy(bus);
    return true;
}

/* Test: Unsubscribe all */
static bool test_unsubscribe_all(void) {
    EventBus* bus = event_bus_create();
    if (!bus) return false;

    /* Subscribe multiple times */
    event_bus_subscribe(bus, EVENT_COMBAT_START, test_callback, NULL);
    event_bus_subscribe(bus, EVENT_COMBAT_START, test_callback, NULL);
    event_bus_subscribe(bus, EVENT_COMBAT_START, test_callback, NULL);

    event_bus_unsubscribe_all(bus, EVENT_COMBAT_START);

    if (event_bus_subscriber_count(bus, EVENT_COMBAT_START) != 0) {
        event_bus_destroy(bus);
        return false;
    }

    event_bus_destroy(bus);
    return true;
}

/* Test: Queue events */
static bool test_queue_events(void) {
    EventBus* bus = event_bus_create();
    if (!bus) return false;

    reset_callback_tracking();

    event_bus_subscribe(bus, EVENT_UI_BUTTON_CLICKED, test_callback, NULL);

    /* Queue events */
    int data1 = 1, data2 = 2, data3 = 3;
    event_bus_queue(bus, EVENT_UI_BUTTON_CLICKED, &data1, sizeof(data1));
    event_bus_queue(bus, EVENT_UI_BUTTON_CLICKED, &data2, sizeof(data2));
    event_bus_queue(bus, EVENT_UI_BUTTON_CLICKED, &data3, sizeof(data3));

    if (event_bus_queue_size(bus) != 3) {
        event_bus_destroy(bus);
        return false;
    }

    /* Dispatch should process all */
    event_bus_dispatch(bus);

    if (g_callback_count != 3) {
        event_bus_destroy(bus);
        return false;
    }

    /* Queue should be empty */
    if (event_bus_queue_size(bus) != 0) {
        event_bus_destroy(bus);
        return false;
    }

    event_bus_destroy(bus);
    return true;
}

/* Test: Clear queue */
static bool test_clear_queue(void) {
    EventBus* bus = event_bus_create();
    if (!bus) return false;

    reset_callback_tracking();

    event_bus_subscribe(bus, EVENT_GAME_QUIT, test_callback, NULL);

    /* Queue events */
    event_bus_queue(bus, EVENT_GAME_QUIT, NULL, 0);
    event_bus_queue(bus, EVENT_GAME_QUIT, NULL, 0);

    if (event_bus_queue_size(bus) != 2) {
        event_bus_destroy(bus);
        return false;
    }

    /* Clear without dispatching */
    event_bus_clear_queue(bus);

    if (event_bus_queue_size(bus) != 0) {
        event_bus_destroy(bus);
        return false;
    }

    /* Callbacks should not have been called */
    if (g_callback_count != 0) {
        event_bus_destroy(bus);
        return false;
    }

    event_bus_destroy(bus);
    return true;
}

/* Test: Different event types */
static bool test_different_event_types(void) {
    EventBus* bus = event_bus_create();
    if (!bus) return false;

    reset_callback_tracking();

    /* Subscribe to different types */
    event_bus_subscribe(bus, EVENT_PLAYER_MOVE, test_callback, NULL);
    event_bus_subscribe(bus, EVENT_ENEMY_SPAWN, test_callback, NULL);

    /* Publish one type */
    event_bus_publish(bus, EVENT_PLAYER_MOVE, NULL);

    /* Only one callback should be called */
    if (g_callback_count != 1) {
        event_bus_destroy(bus);
        return false;
    }

    if (g_last_event_type != EVENT_PLAYER_MOVE) {
        event_bus_destroy(bus);
        return false;
    }

    reset_callback_tracking();

    /* Publish other type */
    event_bus_publish(bus, EVENT_ENEMY_SPAWN, NULL);

    if (g_callback_count != 1) {
        event_bus_destroy(bus);
        return false;
    }

    if (g_last_event_type != EVENT_ENEMY_SPAWN) {
        event_bus_destroy(bus);
        return false;
    }

    event_bus_destroy(bus);
    return true;
}

/* Test: Total subscriptions */
static bool test_total_subscriptions(void) {
    EventBus* bus = event_bus_create();
    if (!bus) return false;

    if (event_bus_total_subscriptions(bus) != 0) {
        event_bus_destroy(bus);
        return false;
    }

    /* Add subscriptions */
    event_bus_subscribe(bus, EVENT_GAME_START, test_callback, NULL);
    event_bus_subscribe(bus, EVENT_GAME_START, test_callback, NULL);
    event_bus_subscribe(bus, EVENT_PLAYER_MOVE, test_callback, NULL);

    if (event_bus_total_subscriptions(bus) != 3) {
        event_bus_destroy(bus);
        return false;
    }

    /* Remove one */
    event_bus_unsubscribe_all(bus, EVENT_GAME_START);

    if (event_bus_total_subscriptions(bus) != 1) {
        event_bus_destroy(bus);
        return false;
    }

    event_bus_destroy(bus);
    return true;
}

/* Test: Event names */
static bool test_event_names(void) {
    const char* name = event_type_name(EVENT_GAME_START);
    if (!name || strcmp(name, "GAME_START") != 0) {
        return false;
    }

    name = event_type_name(EVENT_PLAYER_DAMAGE_TAKEN);
    if (!name || strcmp(name, "PLAYER_DAMAGE_TAKEN") != 0) {
        return false;
    }

    name = event_type_name(EVENT_COMBAT_END);
    if (!name || strcmp(name, "COMBAT_END") != 0) {
        return false;
    }

    return true;
}

/* Test: Queue growth */
static bool test_queue_growth(void) {
    EventBus* bus = event_bus_create();
    if (!bus) return false;

    reset_callback_tracking();

    event_bus_subscribe(bus, EVENT_UI_TEXT_INPUT, test_callback, NULL);

    /* Queue many events to trigger growth */
    for (int i = 0; i < 200; i++) {
        if (!event_bus_queue(bus, EVENT_UI_TEXT_INPUT, &i, sizeof(i))) {
            event_bus_destroy(bus);
            return false;
        }
    }

    if (event_bus_queue_size(bus) != 200) {
        event_bus_destroy(bus);
        return false;
    }

    /* Dispatch all */
    event_bus_dispatch(bus);

    if (g_callback_count != 200) {
        event_bus_destroy(bus);
        return false;
    }

    event_bus_destroy(bus);
    return true;
}

int main(void) {
    /* Initialize logger for tests */
    logger_init("test_events.log", LOG_LEVEL_DEBUG);

    printf("=====================================\n");
    printf("Event Bus Tests\n");
    printf("=====================================\n\n");

    TEST(create_destroy);
    TEST(subscribe_unsubscribe);
    TEST(publish);
    TEST(publish_with_data);
    TEST(multiple_subscribers);
    TEST(unsubscribe_all);
    TEST(queue_events);
    TEST(clear_queue);
    TEST(different_event_types);
    TEST(total_subscriptions);
    TEST(event_names);
    TEST(queue_growth);

    printf("\n=====================================\n");
    printf("Results: %d/%d tests passed\n", tests_passed, tests_run);
    printf("=====================================\n");

    logger_shutdown();

    return (tests_passed == tests_run) ? EXIT_SUCCESS : EXIT_FAILURE;
}

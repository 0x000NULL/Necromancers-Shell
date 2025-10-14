#include "../src/game/resources/resources.h"
#include "../src/game/resources/corruption.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>

/* Test counter */
static int tests_run = 0;
static int tests_passed = 0;

/* Test helper macros */
#define TEST_START(name) \
    do { \
        printf("Running test: %s\n", name); \
        tests_run++; \
    } while (0)

#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf("  FAIL: %s\n", message); \
            return; \
        } \
    } while (0)

#define TEST_PASS() \
    do { \
        printf("  PASS\n"); \
        tests_passed++; \
    } while (0)

/* ============ Resources Tests ============ */

void test_resources_init(void) {
    TEST_START("resources_init");

    Resources res;
    resources_init(&res);

    TEST_ASSERT(res.soul_energy == 0, "Initial soul energy should be 0");
    TEST_ASSERT(res.mana == 100, "Initial mana should be 100");
    TEST_ASSERT(res.mana_max == 100, "Initial max mana should be 100");
    TEST_ASSERT(res.day_count == 0, "Initial day count should be 0");
    TEST_ASSERT(res.time_hours == 0, "Initial time should be 0");

    /* Test NULL */
    resources_init(NULL); /* Should not crash */

    TEST_PASS();
}

void test_resources_soul_energy(void) {
    TEST_START("resources_soul_energy");

    Resources res;
    resources_init(&res);

    /* Test adding energy */
    TEST_ASSERT(resources_add_soul_energy(&res, 100) == true, "Adding energy should succeed");
    TEST_ASSERT(res.soul_energy == 100, "Soul energy should be 100");

    TEST_ASSERT(resources_add_soul_energy(&res, 50) == true, "Adding more energy should succeed");
    TEST_ASSERT(res.soul_energy == 150, "Soul energy should be 150");

    /* Test has_soul_energy */
    TEST_ASSERT(resources_has_soul_energy(&res, 150) == true, "Should have 150 energy");
    TEST_ASSERT(resources_has_soul_energy(&res, 200) == false, "Should not have 200 energy");

    /* Test spending energy */
    TEST_ASSERT(resources_spend_soul_energy(&res, 50) == true, "Spending 50 should succeed");
    TEST_ASSERT(res.soul_energy == 100, "Soul energy should be 100");

    TEST_ASSERT(resources_spend_soul_energy(&res, 150) == false, "Spending 150 should fail");
    TEST_ASSERT(res.soul_energy == 100, "Soul energy should still be 100");

    TEST_ASSERT(resources_spend_soul_energy(&res, 100) == true, "Spending exact amount should succeed");
    TEST_ASSERT(res.soul_energy == 0, "Soul energy should be 0");

    /* Test NULL handling */
    TEST_ASSERT(resources_add_soul_energy(NULL, 100) == false, "Adding to NULL should fail");
    TEST_ASSERT(resources_spend_soul_energy(NULL, 50) == false, "Spending from NULL should fail");
    TEST_ASSERT(resources_has_soul_energy(NULL, 50) == false, "Checking NULL should return false");

    TEST_PASS();
}

void test_resources_mana(void) {
    TEST_START("resources_mana");

    Resources res;
    resources_init(&res);

    /* Test initial mana */
    TEST_ASSERT(res.mana == 100, "Initial mana should be 100");
    TEST_ASSERT(res.mana_max == 100, "Initial max mana should be 100");

    /* Test spending mana */
    TEST_ASSERT(resources_spend_mana(&res, 30) == true, "Spending mana should succeed");
    TEST_ASSERT(res.mana == 70, "Mana should be 70");

    TEST_ASSERT(resources_has_mana(&res, 70) == true, "Should have 70 mana");
    TEST_ASSERT(resources_has_mana(&res, 100) == false, "Should not have 100 mana");

    TEST_ASSERT(resources_spend_mana(&res, 100) == false, "Spending more than available should fail");
    TEST_ASSERT(res.mana == 70, "Mana should still be 70");

    /* Test adding mana (with cap) */
    TEST_ASSERT(resources_add_mana(&res, 20) == true, "Adding mana should succeed");
    TEST_ASSERT(res.mana == 90, "Mana should be 90");

    TEST_ASSERT(resources_add_mana(&res, 50) == true, "Adding mana should succeed");
    TEST_ASSERT(res.mana == 100, "Mana should be capped at 100");

    /* Test regenerate mana */
    resources_spend_mana(&res, 50);
    resources_regenerate_mana(&res, 20);
    TEST_ASSERT(res.mana == 70, "Mana should regenerate to 70");

    resources_regenerate_mana(&res, 100);
    TEST_ASSERT(res.mana == 100, "Mana regeneration should be capped");

    /* Test increasing max mana */
    TEST_ASSERT(resources_increase_max_mana(&res, 50) == true, "Increasing max mana should succeed");
    TEST_ASSERT(res.mana_max == 150, "Max mana should be 150");

    resources_add_mana(&res, 100);
    TEST_ASSERT(res.mana == 150, "Mana should reach new cap");

    /* Test NULL handling */
    TEST_ASSERT(resources_add_mana(NULL, 50) == false, "Adding to NULL should fail");
    TEST_ASSERT(resources_spend_mana(NULL, 50) == false, "Spending from NULL should fail");
    TEST_ASSERT(resources_has_mana(NULL, 50) == false, "Checking NULL should return false");

    TEST_PASS();
}

void test_resources_time(void) {
    TEST_START("resources_time");

    Resources res;
    resources_init(&res);

    /* Test initial time */
    TEST_ASSERT(res.day_count == 0, "Initial day should be 0");
    TEST_ASSERT(res.time_hours == 0, "Initial hour should be 0");

    /* Test advancing hours */
    resources_advance_time(&res, 5);
    TEST_ASSERT(res.time_hours == 5, "Time should be 5 hours");
    TEST_ASSERT(res.day_count == 0, "Day should still be 0");

    /* Test day rollover */
    resources_advance_time(&res, 20);
    TEST_ASSERT(res.time_hours == 1, "Time should be 1 hour (5+20-24)");
    TEST_ASSERT(res.day_count == 1, "Day should be 1");

    /* Test multiple day rollover */
    resources_advance_time(&res, 50);
    TEST_ASSERT(res.time_hours == 3, "Time should be 3 hours (1+50-48)");
    TEST_ASSERT(res.day_count == 3, "Day should be 3");

    /* Test exact 24 hours */
    resources_advance_time(&res, 21);
    TEST_ASSERT(res.time_hours == 0, "Time should be 0 (midnight)");
    TEST_ASSERT(res.day_count == 4, "Day should be 4");

    /* Test NULL handling */
    resources_advance_time(NULL, 10); /* Should not crash */

    TEST_PASS();
}

void test_resources_format_time(void) {
    TEST_START("resources_format_time");

    Resources res;
    resources_init(&res);
    char buffer[64];

    /* Test initial time */
    int written = resources_format_time(&res, buffer, sizeof(buffer));
    TEST_ASSERT(written > 0, "Should write characters");
    TEST_ASSERT(strcmp(buffer, "Day 0, 00:00") == 0, "Should format as 'Day 0, 00:00'");

    /* Test different times */
    resources_advance_time(&res, 15);
    resources_format_time(&res, buffer, sizeof(buffer));
    TEST_ASSERT(strcmp(buffer, "Day 0, 15:00") == 0, "Should format as 'Day 0, 15:00'");

    resources_advance_time(&res, 20);
    resources_format_time(&res, buffer, sizeof(buffer));
    TEST_ASSERT(strcmp(buffer, "Day 1, 11:00") == 0, "Should format as 'Day 1, 11:00'");

    /* Test NULL handling */
    written = resources_format_time(NULL, buffer, sizeof(buffer));
    TEST_ASSERT(written == 0, "Formatting NULL should return 0");

    written = resources_format_time(&res, NULL, sizeof(buffer));
    TEST_ASSERT(written == 0, "Formatting to NULL buffer should return 0");

    TEST_PASS();
}

void test_resources_time_of_day(void) {
    TEST_START("resources_time_of_day");

    Resources res;
    resources_init(&res);

    /* Test different times of day */
    res.time_hours = 0;
    TEST_ASSERT(strcmp(resources_get_time_of_day(&res), "midnight") == 0, "Hour 0 should be midnight");

    res.time_hours = 3;
    TEST_ASSERT(strcmp(resources_get_time_of_day(&res), "night") == 0, "Hour 3 should be night");

    res.time_hours = 8;
    TEST_ASSERT(strcmp(resources_get_time_of_day(&res), "morning") == 0, "Hour 8 should be morning");

    res.time_hours = 14;
    TEST_ASSERT(strcmp(resources_get_time_of_day(&res), "afternoon") == 0, "Hour 14 should be afternoon");

    res.time_hours = 19;
    TEST_ASSERT(strcmp(resources_get_time_of_day(&res), "evening") == 0, "Hour 19 should be evening");

    res.time_hours = 23;
    TEST_ASSERT(strcmp(resources_get_time_of_day(&res), "night") == 0, "Hour 23 should be night");

    /* Test NULL handling */
    TEST_ASSERT(strcmp(resources_get_time_of_day(NULL), "unknown") == 0,
                "NULL should return 'unknown'");

    TEST_PASS();
}

/* ============ Corruption Tests ============ */

void test_corruption_init(void) {
    TEST_START("corruption_init");

    CorruptionState state;
    corruption_init(&state);

    TEST_ASSERT(state.corruption == 0, "Initial corruption should be 0");
    TEST_ASSERT(state.event_count == 0, "Initial event count should be 0");

    /* Test NULL */
    corruption_init(NULL); /* Should not crash */

    TEST_PASS();
}

void test_corruption_add_reduce(void) {
    TEST_START("corruption_add_reduce");

    CorruptionState state;
    corruption_init(&state);

    /* Test adding corruption */
    TEST_ASSERT(corruption_add(&state, 10, "Raised undead", 0) == true,
                "Adding corruption should succeed");
    TEST_ASSERT(state.corruption == 10, "Corruption should be 10");
    TEST_ASSERT(state.event_count == 1, "Should have 1 event");

    TEST_ASSERT(corruption_add(&state, 25, "Harvested innocent souls", 1) == true,
                "Adding more corruption should succeed");
    TEST_ASSERT(state.corruption == 35, "Corruption should be 35");

    /* Test capping at 100 */
    TEST_ASSERT(corruption_add(&state, 80, "Performed dark ritual", 2) == true,
                "Adding corruption should succeed");
    TEST_ASSERT(state.corruption == 100, "Corruption should be capped at 100");

    /* Test reducing corruption */
    TEST_ASSERT(corruption_reduce(&state, 20, "Showed mercy", 3) == true,
                "Reducing corruption should succeed");
    TEST_ASSERT(state.corruption == 80, "Corruption should be 80");

    /* Test minimum at 0 */
    TEST_ASSERT(corruption_reduce(&state, 100, "Redeemed yourself", 4) == true,
                "Reducing corruption should succeed");
    TEST_ASSERT(state.corruption == 0, "Corruption should be 0");

    /* Test NULL handling */
    TEST_ASSERT(corruption_add(NULL, 10, "Test", 0) == false, "Adding to NULL should fail");
    TEST_ASSERT(corruption_reduce(NULL, 10, "Test", 0) == false, "Reducing NULL should fail");

    TEST_PASS();
}

void test_corruption_levels(void) {
    TEST_START("corruption_levels");

    CorruptionState state;
    corruption_init(&state);

    /* Test PURE level */
    state.corruption = 0;
    TEST_ASSERT(corruption_get_level(&state) == CORRUPTION_PURE, "Should be PURE at 0");
    TEST_ASSERT(corruption_is_pure(&state) == true, "Should be pure");
    TEST_ASSERT(corruption_is_damned(&state) == false, "Should not be damned");

    state.corruption = 19;
    TEST_ASSERT(corruption_get_level(&state) == CORRUPTION_PURE, "Should be PURE at 19");

    /* Test TAINTED level */
    state.corruption = 20;
    TEST_ASSERT(corruption_get_level(&state) == CORRUPTION_TAINTED, "Should be TAINTED at 20");
    TEST_ASSERT(corruption_is_pure(&state) == false, "Should not be pure");

    /* Test COMPROMISED level */
    state.corruption = 40;
    TEST_ASSERT(corruption_get_level(&state) == CORRUPTION_COMPROMISED, "Should be COMPROMISED at 40");

    /* Test CORRUPTED level */
    state.corruption = 60;
    TEST_ASSERT(corruption_get_level(&state) == CORRUPTION_CORRUPTED, "Should be CORRUPTED at 60");

    /* Test DAMNED level */
    state.corruption = 80;
    TEST_ASSERT(corruption_get_level(&state) == CORRUPTION_DAMNED, "Should be DAMNED at 80");
    TEST_ASSERT(corruption_is_damned(&state) == true, "Should be damned");

    state.corruption = 100;
    TEST_ASSERT(corruption_get_level(&state) == CORRUPTION_DAMNED, "Should be DAMNED at 100");

    TEST_PASS();
}

void test_corruption_level_names(void) {
    TEST_START("corruption_level_names");

    TEST_ASSERT(strcmp(corruption_level_name(CORRUPTION_PURE), "Pure") == 0,
                "PURE name should be 'Pure'");
    TEST_ASSERT(strcmp(corruption_level_name(CORRUPTION_TAINTED), "Tainted") == 0,
                "TAINTED name should be 'Tainted'");
    TEST_ASSERT(strcmp(corruption_level_name(CORRUPTION_COMPROMISED), "Compromised") == 0,
                "COMPROMISED name should be 'Compromised'");
    TEST_ASSERT(strcmp(corruption_level_name(CORRUPTION_CORRUPTED), "Corrupted") == 0,
                "CORRUPTED name should be 'Corrupted'");
    TEST_ASSERT(strcmp(corruption_level_name(CORRUPTION_DAMNED), "Damned") == 0,
                "DAMNED name should be 'Damned'");

    TEST_PASS();
}

void test_corruption_descriptions(void) {
    TEST_START("corruption_descriptions");

    CorruptionState state;
    corruption_init(&state);

    state.corruption = 0;
    const char* desc = corruption_get_description(&state);
    TEST_ASSERT(desc != NULL, "Description should not be NULL");
    TEST_ASSERT(strlen(desc) > 0, "Description should not be empty");

    state.corruption = 50;
    desc = corruption_get_description(&state);
    TEST_ASSERT(desc != NULL, "Description should not be NULL");

    state.corruption = 100;
    desc = corruption_get_description(&state);
    TEST_ASSERT(desc != NULL, "Description should not be NULL");

    /* Test NULL handling */
    desc = corruption_get_description(NULL);
    TEST_ASSERT(desc != NULL, "NULL description should return a string");

    TEST_PASS();
}

void test_corruption_penalty(void) {
    TEST_START("corruption_penalty");

    CorruptionState state;
    corruption_init(&state);

    /* Test penalty at different corruption levels */
    state.corruption = 0;
    float penalty = corruption_calculate_penalty(&state);
    TEST_ASSERT(fabs(penalty - 0.0f) < 0.01f, "Penalty at 0 corruption should be 0.0");

    state.corruption = 50;
    penalty = corruption_calculate_penalty(&state);
    TEST_ASSERT(fabs(penalty - 0.25f) < 0.01f, "Penalty at 50 corruption should be 0.25");

    state.corruption = 100;
    penalty = corruption_calculate_penalty(&state);
    TEST_ASSERT(fabs(penalty - 0.5f) < 0.01f, "Penalty at 100 corruption should be 0.5");

    /* Test NULL handling */
    penalty = corruption_calculate_penalty(NULL);
    TEST_ASSERT(penalty == 0.0f, "NULL penalty should be 0.0");

    TEST_PASS();
}

void test_corruption_events(void) {
    TEST_START("corruption_events");

    CorruptionState state;
    corruption_init(&state);

    /* Add some events */
    corruption_add(&state, 10, "First event", 0);
    corruption_add(&state, 20, "Second event", 1);
    corruption_add(&state, 15, "Third event", 2);

    TEST_ASSERT(state.event_count == 3, "Should have 3 events");

    /* Test getting events */
    const CorruptionEvent* event = corruption_get_event(&state, 0);
    TEST_ASSERT(event != NULL, "First event should exist");
    TEST_ASSERT(strcmp(event->description, "First event") == 0, "First event description should match");
    TEST_ASSERT(event->change == 10, "First event change should be 10");
    TEST_ASSERT(event->day == 0, "First event day should be 0");

    event = corruption_get_event(&state, 1);
    TEST_ASSERT(event != NULL, "Second event should exist");
    TEST_ASSERT(strcmp(event->description, "Second event") == 0, "Second event description should match");

    /* Test getting latest event */
    event = corruption_get_latest_event(&state);
    TEST_ASSERT(event != NULL, "Latest event should exist");
    TEST_ASSERT(strcmp(event->description, "Third event") == 0, "Latest event should be third");

    /* Test out of range */
    event = corruption_get_event(&state, 10);
    TEST_ASSERT(event == NULL, "Out of range event should be NULL");

    /* Test NULL handling */
    event = corruption_get_event(NULL, 0);
    TEST_ASSERT(event == NULL, "Getting event from NULL should return NULL");

    event = corruption_get_latest_event(NULL);
    TEST_ASSERT(event == NULL, "Getting latest from NULL should return NULL");

    TEST_PASS();
}

void test_corruption_event_overflow(void) {
    TEST_START("corruption_event_overflow");

    CorruptionState state;
    corruption_init(&state);

    /* Add more events than MAX_CORRUPTION_EVENTS */
    for (int i = 0; i < MAX_CORRUPTION_EVENTS + 10; i++) {
        corruption_add(&state, 1, "Event", i);
    }

    TEST_ASSERT(state.event_count == MAX_CORRUPTION_EVENTS,
                "Event count should be capped at MAX_CORRUPTION_EVENTS");

    TEST_PASS();
}

/* ============ Main Test Runner ============ */

int main(void) {
    printf("=== Resources & Corruption Tests ===\n\n");

    printf("-- Resources Tests --\n");
    test_resources_init();
    test_resources_soul_energy();
    test_resources_mana();
    test_resources_time();
    test_resources_format_time();
    test_resources_time_of_day();

    printf("\n-- Corruption Tests --\n");
    test_corruption_init();
    test_corruption_add_reduce();
    test_corruption_levels();
    test_corruption_level_names();
    test_corruption_descriptions();
    test_corruption_penalty();
    test_corruption_events();
    test_corruption_event_overflow();

    printf("\n=== Test Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    if (tests_passed == tests_run) {
        printf("\nAll tests PASSED!\n");
        return 0;
    } else {
        printf("\nSome tests FAILED!\n");
        return 1;
    }
}

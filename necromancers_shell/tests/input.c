/**
 * Input System Tests
 */

#include "terminal/input.h"
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

/* Test: Create and destroy */
static bool test_create_destroy(void) {
    InputSystem* input = input_system_create();
    if (!input) return false;

    input_system_destroy(input);
    return true;
}

/* Test: Key state initialization */
static bool test_initial_state(void) {
    InputSystem* input = input_system_create();
    if (!input) return false;

    /* All keys should be up initially */
    for (int i = 0; i < 128; i++) {
        if (input_get_key_state(input, i) != KEY_STATE_UP) {
            input_system_destroy(input);
            return false;
        }
    }

    /* No keys pressed */
    if (input_is_any_key_pressed(input)) {
        input_system_destroy(input);
        return false;
    }

    if (input_get_pressed_count(input) != 0) {
        input_system_destroy(input);
        return false;
    }

    input_system_destroy(input);
    return true;
}

/* Test: Get last key */
static bool test_last_key(void) {
    InputSystem* input = input_system_create();
    if (!input) return false;

    /* Initially no key */
    if (input_get_last_key(input) != -1) {
        input_system_destroy(input);
        return false;
    }

    input_system_destroy(input);
    return true;
}

/* Test: Clear all */
static bool test_clear_all(void) {
    InputSystem* input = input_system_create();
    if (!input) return false;

    /* Clear should reset everything */
    input_clear_all(input);

    if (input_is_any_key_pressed(input)) {
        input_system_destroy(input);
        return false;
    }

    if (input_get_last_key(input) != -1) {
        input_system_destroy(input);
        return false;
    }

    input_system_destroy(input);
    return true;
}

/* Test: Key repeat setting */
static bool test_key_repeat(void) {
    InputSystem* input = input_system_create();
    if (!input) return false;

    /* Default should be enabled */
    if (!input_get_key_repeat(input)) {
        input_system_destroy(input);
        return false;
    }

    /* Disable */
    input_set_key_repeat(input, false);
    if (input_get_key_repeat(input)) {
        input_system_destroy(input);
        return false;
    }

    /* Enable again */
    input_set_key_repeat(input, true);
    if (!input_get_key_repeat(input)) {
        input_system_destroy(input);
        return false;
    }

    input_system_destroy(input);
    return true;
}

/* Test: Text input mode */
static bool test_text_input_mode(void) {
    InputSystem* input = input_system_create();
    if (!input) return false;

    /* Initially not active */
    if (input_is_text_input_active(input)) {
        input_system_destroy(input);
        return false;
    }

    /* Begin text input */
    char buffer[64] = "";
    input_begin_text_input(input, buffer, sizeof(buffer));

    if (!input_is_text_input_active(input)) {
        input_system_destroy(input);
        return false;
    }

    const char* text = input_get_text(input);
    if (!text || strcmp(text, "") != 0) {
        input_system_destroy(input);
        return false;
    }

    /* End text input */
    input_end_text_input(input);

    if (input_is_text_input_active(input)) {
        input_system_destroy(input);
        return false;
    }

    input_system_destroy(input);
    return true;
}

/* Test: Text cursor */
static bool test_text_cursor(void) {
    InputSystem* input = input_system_create();
    if (!input) return false;

    char buffer[64] = "hello";
    input_begin_text_input(input, buffer, sizeof(buffer));

    /* Cursor should be at end of existing text */
    if (input_get_text_cursor(input) != 5) {
        input_end_text_input(input);
        input_system_destroy(input);
        return false;
    }

    input_end_text_input(input);
    input_system_destroy(input);
    return true;
}

/* Test: Key code constants */
static bool test_key_constants(void) {
    /* Verify some key codes */
    if (KEY_SPACE != ' ') return false;
    if (KEY_A != 'A') return false;
    if (KEY_a != 'a') return false;
    if (KEY_0 != '0') return false;
    if (KEY_9 != '9') return false;

    return true;
}

/* Test: Key state enum */
static bool test_key_states(void) {
    /* Verify state values are distinct */
    if (KEY_STATE_UP == KEY_STATE_PRESSED) return false;
    if (KEY_STATE_PRESSED == KEY_STATE_DOWN) return false;
    if (KEY_STATE_DOWN == KEY_STATE_RELEASED) return false;

    return true;
}

/* Test: Boundary checking */
static bool test_boundary_checks(void) {
    InputSystem* input = input_system_create();
    if (!input) return false;

    /* Invalid key codes should return false/UP */
    if (input_is_key_pressed(input, -1)) {
        input_system_destroy(input);
        return false;
    }

    if (input_is_key_pressed(input, MAX_TRACKED_KEYS)) {
        input_system_destroy(input);
        return false;
    }

    if (input_get_key_state(input, -1) != KEY_STATE_UP) {
        input_system_destroy(input);
        return false;
    }

    input_system_destroy(input);
    return true;
}

/* Test: NULL safety */
static bool test_null_safety(void) {
    /* All functions should handle NULL gracefully */
    input_system_update(NULL);
    input_system_destroy(NULL);
    input_clear_all(NULL);
    input_set_key_repeat(NULL, true);
    input_end_text_input(NULL);

    if (input_is_key_pressed(NULL, KEY_A)) return false;
    if (input_is_key_down(NULL, KEY_A)) return false;
    if (input_is_key_released(NULL, KEY_A)) return false;
    if (input_get_key_state(NULL, KEY_A) != KEY_STATE_UP) return false;
    if (input_get_pressed_count(NULL) != 0) return false;
    if (input_is_any_key_pressed(NULL)) return false;
    if (input_get_last_key(NULL) != -1) return false;
    if (input_get_key_repeat(NULL)) return false;
    if (input_is_text_input_active(NULL)) return false;
    if (input_get_text(NULL) != NULL) return false;
    if (input_get_text_cursor(NULL) != 0) return false;

    return true;
}

int main(void) {
    /* Initialize logger for tests */
    logger_init("test_input.log", LOG_LEVEL_DEBUG);

    printf("=====================================\n");
    printf("Input System Tests\n");
    printf("=====================================\n\n");

    TEST(create_destroy);
    TEST(initial_state);
    TEST(last_key);
    TEST(clear_all);
    TEST(key_repeat);
    TEST(text_input_mode);
    TEST(text_cursor);
    TEST(key_constants);
    TEST(key_states);
    TEST(boundary_checks);
    TEST(null_safety);

    printf("\n=====================================\n");
    printf("Results: %d/%d tests passed\n", tests_passed, tests_run);
    printf("=====================================\n");

    logger_shutdown();

    return (tests_passed == tests_run) ? EXIT_SUCCESS : EXIT_FAILURE;
}

/**
 * State Manager Tests
 */

#include "core/state_manager.h"
#include "utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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
static int g_enter_count = 0;
static int g_exit_count = 0;
static int g_update_count = 0;
static int g_render_count = 0;
static int g_pause_count = 0;
static int g_resume_count = 0;

static void reset_callback_counts(void) {
    g_enter_count = 0;
    g_exit_count = 0;
    g_update_count = 0;
    g_render_count = 0;
    g_pause_count = 0;
    g_resume_count = 0;
}

static void on_enter(void* userdata) {
    (void)userdata;
    g_enter_count++;
}

static void on_exit(void* userdata) {
    (void)userdata;
    g_exit_count++;
}

static void on_update(double delta, void* userdata) {
    (void)delta;
    (void)userdata;
    g_update_count++;
}

static void on_render(void* userdata) {
    (void)userdata;
    g_render_count++;
}

static void on_pause(void* userdata) {
    (void)userdata;
    g_pause_count++;
}

static void on_resume(void* userdata) {
    (void)userdata;
    g_resume_count++;
}

/* Test: Create and destroy */
static bool test_create_destroy(void) {
    StateManager* manager = state_manager_create();
    if (!manager) return false;

    state_manager_destroy(manager);
    return true;
}

/* Test: Register state */
static bool test_register_state(void) {
    StateManager* manager = state_manager_create();
    if (!manager) return false;

    StateCallbacks callbacks = {
        .on_enter = on_enter,
        .on_exit = on_exit,
        .on_update = on_update,
        .on_render = on_render
    };

    bool result = state_manager_register(manager, STATE_MAIN_MENU, &callbacks, NULL);
    if (!result) {
        state_manager_destroy(manager);
        return false;
    }

    if (!state_manager_is_registered(manager, STATE_MAIN_MENU)) {
        state_manager_destroy(manager);
        return false;
    }

    state_manager_destroy(manager);
    return true;
}

/* Test: Push state */
static bool test_push_state(void) {
    StateManager* manager = state_manager_create();
    if (!manager) return false;

    reset_callback_counts();

    StateCallbacks callbacks = {
        .on_enter = on_enter,
        .on_exit = on_exit
    };

    state_manager_register(manager, STATE_MAIN_MENU, &callbacks, NULL);

    if (!state_manager_push(manager, STATE_MAIN_MENU)) {
        state_manager_destroy(manager);
        return false;
    }

    /* Should call on_enter */
    if (g_enter_count != 1) {
        state_manager_destroy(manager);
        return false;
    }

    /* Check current state */
    if (state_manager_current(manager) != STATE_MAIN_MENU) {
        state_manager_destroy(manager);
        return false;
    }

    /* Check depth */
    if (state_manager_depth(manager) != 1) {
        state_manager_destroy(manager);
        return false;
    }

    state_manager_destroy(manager);
    return true;
}

/* Test: Pop state */
static bool test_pop_state(void) {
    StateManager* manager = state_manager_create();
    if (!manager) return false;

    reset_callback_counts();

    StateCallbacks callbacks = {
        .on_enter = on_enter,
        .on_exit = on_exit
    };

    state_manager_register(manager, STATE_MAIN_MENU, &callbacks, NULL);
    state_manager_push(manager, STATE_MAIN_MENU);

    reset_callback_counts();

    if (!state_manager_pop(manager)) {
        state_manager_destroy(manager);
        return false;
    }

    /* Should call on_exit */
    if (g_exit_count != 1) {
        state_manager_destroy(manager);
        return false;
    }

    /* Stack should be empty */
    if (state_manager_depth(manager) != 0) {
        state_manager_destroy(manager);
        return false;
    }

    if (state_manager_current(manager) != STATE_NONE) {
        state_manager_destroy(manager);
        return false;
    }

    state_manager_destroy(manager);
    return true;
}

/* Test: Change state */
static bool test_change_state(void) {
    StateManager* manager = state_manager_create();
    if (!manager) return false;

    reset_callback_counts();

    StateCallbacks callbacks = {
        .on_enter = on_enter,
        .on_exit = on_exit
    };

    state_manager_register(manager, STATE_MAIN_MENU, &callbacks, NULL);
    state_manager_register(manager, STATE_GAME_WORLD, &callbacks, NULL);

    state_manager_push(manager, STATE_MAIN_MENU);
    reset_callback_counts();

    /* Change to new state */
    if (!state_manager_change(manager, STATE_GAME_WORLD)) {
        state_manager_destroy(manager);
        return false;
    }

    /* Should exit old and enter new */
    if (g_exit_count != 1 || g_enter_count != 1) {
        state_manager_destroy(manager);
        return false;
    }

    /* Current should be new state */
    if (state_manager_current(manager) != STATE_GAME_WORLD) {
        state_manager_destroy(manager);
        return false;
    }

    /* Depth should stay the same */
    if (state_manager_depth(manager) != 1) {
        state_manager_destroy(manager);
        return false;
    }

    state_manager_destroy(manager);
    return true;
}

/* Test: Push multiple states */
static bool test_push_multiple(void) {
    StateManager* manager = state_manager_create();
    if (!manager) return false;

    StateCallbacks callbacks = {
        .on_enter = on_enter,
        .on_pause = on_pause,
        .on_resume = on_resume
    };

    state_manager_register(manager, STATE_MAIN_MENU, &callbacks, NULL);
    state_manager_register(manager, STATE_GAME_WORLD, &callbacks, NULL);
    state_manager_register(manager, STATE_PAUSE, &callbacks, NULL);

    reset_callback_counts();

    /* Push first state */
    state_manager_push(manager, STATE_MAIN_MENU);
    if (g_enter_count != 1 || g_pause_count != 0) {
        state_manager_destroy(manager);
        return false;
    }

    reset_callback_counts();

    /* Push second state (should pause first) */
    state_manager_push(manager, STATE_GAME_WORLD);
    if (g_enter_count != 1 || g_pause_count != 1) {
        state_manager_destroy(manager);
        return false;
    }

    reset_callback_counts();

    /* Push third state */
    state_manager_push(manager, STATE_PAUSE);
    if (g_enter_count != 1 || g_pause_count != 1) {
        state_manager_destroy(manager);
        return false;
    }

    /* Check depth and current */
    if (state_manager_depth(manager) != 3) {
        state_manager_destroy(manager);
        return false;
    }

    if (state_manager_current(manager) != STATE_PAUSE) {
        state_manager_destroy(manager);
        return false;
    }

    if (state_manager_previous(manager) != STATE_GAME_WORLD) {
        state_manager_destroy(manager);
        return false;
    }

    state_manager_destroy(manager);
    return true;
}

/* Test: Pop multiple states */
static bool test_pop_multiple(void) {
    StateManager* manager = state_manager_create();
    if (!manager) return false;

    StateCallbacks callbacks = {
        .on_enter = on_enter,
        .on_exit = on_exit,
        .on_pause = on_pause,
        .on_resume = on_resume
    };

    state_manager_register(manager, STATE_MAIN_MENU, &callbacks, NULL);
    state_manager_register(manager, STATE_GAME_WORLD, &callbacks, NULL);
    state_manager_register(manager, STATE_PAUSE, &callbacks, NULL);

    /* Push three states */
    state_manager_push(manager, STATE_MAIN_MENU);
    state_manager_push(manager, STATE_GAME_WORLD);
    state_manager_push(manager, STATE_PAUSE);

    reset_callback_counts();

    /* Pop once (should resume previous) */
    state_manager_pop(manager);
    if (g_exit_count != 1 || g_resume_count != 1) {
        state_manager_destroy(manager);
        return false;
    }

    if (state_manager_current(manager) != STATE_GAME_WORLD) {
        state_manager_destroy(manager);
        return false;
    }

    reset_callback_counts();

    /* Pop again */
    state_manager_pop(manager);
    if (g_exit_count != 1 || g_resume_count != 1) {
        state_manager_destroy(manager);
        return false;
    }

    if (state_manager_current(manager) != STATE_MAIN_MENU) {
        state_manager_destroy(manager);
        return false;
    }

    state_manager_destroy(manager);
    return true;
}

/* Test: Update and render */
static bool test_update_render(void) {
    StateManager* manager = state_manager_create();
    if (!manager) return false;

    StateCallbacks callbacks = {
        .on_enter = on_enter,
        .on_update = on_update,
        .on_render = on_render
    };

    state_manager_register(manager, STATE_MAIN_MENU, &callbacks, NULL);
    state_manager_push(manager, STATE_MAIN_MENU);

    reset_callback_counts();

    /* Update and render */
    state_manager_update(manager, 0.016);
    state_manager_render(manager);

    if (g_update_count != 1 || g_render_count != 1) {
        state_manager_destroy(manager);
        return false;
    }

    /* Multiple updates */
    state_manager_update(manager, 0.016);
    state_manager_update(manager, 0.016);

    if (g_update_count != 3) {
        state_manager_destroy(manager);
        return false;
    }

    state_manager_destroy(manager);
    return true;
}

/* Test: Clear stack */
static bool test_clear_stack(void) {
    StateManager* manager = state_manager_create();
    if (!manager) return false;

    StateCallbacks callbacks = {
        .on_enter = on_enter,
        .on_exit = on_exit
    };

    state_manager_register(manager, STATE_MAIN_MENU, &callbacks, NULL);
    state_manager_register(manager, STATE_GAME_WORLD, &callbacks, NULL);
    state_manager_register(manager, STATE_PAUSE, &callbacks, NULL);

    /* Push three states */
    state_manager_push(manager, STATE_MAIN_MENU);
    state_manager_push(manager, STATE_GAME_WORLD);
    state_manager_push(manager, STATE_PAUSE);

    reset_callback_counts();

    /* Clear all */
    state_manager_clear(manager);

    /* Should call exit for all 3 states */
    if (g_exit_count != 3) {
        state_manager_destroy(manager);
        return false;
    }

    /* Stack should be empty */
    if (state_manager_depth(manager) != 0) {
        state_manager_destroy(manager);
        return false;
    }

    state_manager_destroy(manager);
    return true;
}

/* Test: State names */
static bool test_state_names(void) {
    const char* name = state_manager_state_name(STATE_MAIN_MENU);
    if (!name || strcmp(name, "MAIN_MENU") != 0) {
        return false;
    }

    name = state_manager_state_name(STATE_GAME_WORLD);
    if (!name || strcmp(name, "GAME_WORLD") != 0) {
        return false;
    }

    name = state_manager_state_name(STATE_PAUSE);
    if (!name || strcmp(name, "PAUSE") != 0) {
        return false;
    }

    return true;
}

int main(void) {
    /* Initialize logger for tests */
    logger_init("test_state_manager.log", LOG_LEVEL_DEBUG);

    printf("=====================================\n");
    printf("State Manager Tests\n");
    printf("=====================================\n\n");

    TEST(create_destroy);
    TEST(register_state);
    TEST(push_state);
    TEST(pop_state);
    TEST(change_state);
    TEST(push_multiple);
    TEST(pop_multiple);
    TEST(update_render);
    TEST(clear_stack);
    TEST(state_names);

    printf("\n=====================================\n");
    printf("Results: %d/%d tests passed\n", tests_passed, tests_run);
    printf("=====================================\n");

    logger_shutdown();

    return (tests_passed == tests_run) ? EXIT_SUCCESS : EXIT_FAILURE;
}

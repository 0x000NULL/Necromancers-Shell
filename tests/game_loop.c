/**
 * Game Loop Tests
 */

#include "core/game_loop.h"
#include "utils/logger.h"
#include <stdio.h>
#include <stdlib.h>

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
static int g_init_count = 0;
static int g_update_count = 0;
static int g_render_count = 0;
static int g_cleanup_count = 0;
static int g_pause_count = 0;
static int g_resume_count = 0;
static GameLoop* g_test_loop = NULL;

static void reset_callback_counts(void) {
    g_init_count = 0;
    g_update_count = 0;
    g_render_count = 0;
    g_cleanup_count = 0;
    g_pause_count = 0;
    g_resume_count = 0;
}

static bool test_init(void* userdata) {
    (void)userdata;
    g_init_count++;
    return true;
}

static void test_update(double delta_time, void* userdata) {
    (void)delta_time;
    (void)userdata;
    g_update_count++;

    /* Stop after a few frames for testing */
    if (g_update_count >= 5 && g_test_loop) {
        game_loop_stop(g_test_loop);
    }
}

static void test_render(void* userdata) {
    (void)userdata;
    g_render_count++;
}

static void test_cleanup(void* userdata) {
    (void)userdata;
    g_cleanup_count++;
}

static void test_pause(void* userdata) __attribute__((unused));
static void test_pause(void* userdata) {
    (void)userdata;
    g_pause_count++;
}

static void test_resume(void* userdata) __attribute__((unused));
static void test_resume(void* userdata) {
    (void)userdata;
    g_resume_count++;
}

/* Test: Create and destroy */
static bool test_create_destroy(void) {
    GameLoopCallbacks callbacks = {
        .on_init = test_init,
        .on_update = test_update,
        .on_render = test_render,
        .on_cleanup = test_cleanup
    };

    GameLoop* loop = game_loop_create(&callbacks, NULL);
    if (!loop) return false;

    game_loop_destroy(loop);
    return true;
}

/* Test: Initial state */
static bool test_initial_state(void) {
    GameLoopCallbacks callbacks = { 0 };
    GameLoop* loop = game_loop_create(&callbacks, NULL);
    if (!loop) return false;

    if (game_loop_is_running(loop)) {
        game_loop_destroy(loop);
        return false;
    }

    if (game_loop_is_paused(loop)) {
        game_loop_destroy(loop);
        return false;
    }

    if (game_loop_get_state(loop) != LOOP_STATE_STOPPED) {
        game_loop_destroy(loop);
        return false;
    }

    if (game_loop_get_frame_count(loop) != 0) {
        game_loop_destroy(loop);
        return false;
    }

    game_loop_destroy(loop);
    return true;
}

/* Test: Run loop */
static bool test_run_loop(void) {
    reset_callback_counts();

    GameLoopCallbacks callbacks = {
        .on_init = test_init,
        .on_update = test_update,
        .on_render = test_render,
        .on_cleanup = test_cleanup
    };

    GameLoop* loop = game_loop_create(&callbacks, NULL);
    if (!loop) return false;

    /* Set global loop so test_update can stop it */
    g_test_loop = loop;

    /* Run loop (will stop after 5 updates) */
    if (!game_loop_run(loop, 60)) {
        game_loop_destroy(loop);
        return false;
    }

    /* Check callbacks were called */
    if (g_init_count != 1) {
        game_loop_destroy(loop);
        return false;
    }

    if (g_update_count != 5) {
        game_loop_destroy(loop);
        return false;
    }

    if (g_render_count < 5) {  /* At least 5 renders */
        game_loop_destroy(loop);
        return false;
    }

    if (g_cleanup_count != 1) {
        game_loop_destroy(loop);
        return false;
    }

    /* Check state */
    if (game_loop_get_state(loop) != LOOP_STATE_STOPPED) {
        game_loop_destroy(loop);
        return false;
    }

    game_loop_destroy(loop);
    return true;
}

/* Test: Frame count */
static bool test_frame_count(void) {
    reset_callback_counts();

    GameLoopCallbacks callbacks = {
        .on_update = test_update,
        .on_render = test_render
    };

    GameLoop* loop = game_loop_create(&callbacks, NULL);
    if (!loop) return false;

    g_test_loop = loop;
    game_loop_run(loop, 60);

    /* Should have at least 5 frames */
    if (game_loop_get_frame_count(loop) < 5) {
        game_loop_destroy(loop);
        return false;
    }

    game_loop_destroy(loop);
    return true;
}

/* Test: Max delta time */
static bool test_max_delta(void) {
    GameLoopCallbacks callbacks = { 0 };
    GameLoop* loop = game_loop_create(&callbacks, NULL);
    if (!loop) return false;

    /* Default max delta */
    if (game_loop_get_max_delta(loop) != 0.1) {
        game_loop_destroy(loop);
        return false;
    }

    /* Set new max delta */
    game_loop_set_max_delta(loop, 0.05);
    if (game_loop_get_max_delta(loop) != 0.05) {
        game_loop_destroy(loop);
        return false;
    }

    game_loop_destroy(loop);
    return true;
}

/* Test: State enum */
static bool test_state_enum(void) {
    /* Verify states are distinct */
    if (LOOP_STATE_STOPPED == LOOP_STATE_RUNNING) return false;
    if (LOOP_STATE_RUNNING == LOOP_STATE_PAUSED) return false;

    return true;
}

/* Test: NULL callbacks */
static bool test_null_callbacks(void) {
    /* NULL callbacks should fail */
    GameLoop* loop = game_loop_create(NULL, NULL);
    if (loop != NULL) {
        game_loop_destroy(loop);
        return false;
    }

    return true;
}

/* Test: Partial callbacks */
static bool test_partial_callbacks(void) {
    /* Only some callbacks provided should work */
    GameLoopCallbacks callbacks = {
        .on_update = test_update
    };

    GameLoop* loop = game_loop_create(&callbacks, NULL);
    if (!loop) return false;

    reset_callback_counts();
    g_test_loop = loop;
    game_loop_run(loop, 60);

    /* Update should be called */
    if (g_update_count != 5) {
        game_loop_destroy(loop);
        return false;
    }

    /* Init/render/cleanup should not crash even though NULL */

    game_loop_destroy(loop);
    return true;
}

/* Test: Stop from outside loop */
static bool test_stop_external(void) {
    GameLoopCallbacks callbacks = {
        .on_update = NULL  /* No update callback */
    };

    GameLoop* loop = game_loop_create(&callbacks, NULL);
    if (!loop) return false;

    /* Immediately stop */
    game_loop_stop(loop);

    /* Loop should already be stopped */
    if (game_loop_is_running(loop)) {
        game_loop_destroy(loop);
        return false;
    }

    game_loop_destroy(loop);
    return true;
}

/* Test: NULL safety */
static bool test_null_safety(void) {
    /* All functions should handle NULL gracefully */
    game_loop_destroy(NULL);
    game_loop_stop(NULL);
    game_loop_pause(NULL);
    game_loop_resume(NULL);
    game_loop_set_max_delta(NULL, 0.1);

    if (game_loop_run(NULL, 60)) return false;
    if (game_loop_is_running(NULL)) return false;
    if (game_loop_is_paused(NULL)) return false;
    if (game_loop_get_state(NULL) != LOOP_STATE_STOPPED) return false;
    if (game_loop_get_fps(NULL) != 0.0) return false;
    if (game_loop_get_frame_count(NULL) != 0) return false;
    if (game_loop_get_elapsed_time(NULL) != 0.0) return false;
    if (game_loop_get_max_delta(NULL) != 0.0) return false;

    return true;
}

int main(void) {
    /* Initialize logger for tests */
    logger_init("test_game_loop.log", LOG_LEVEL_DEBUG);

    printf("=====================================\n");
    printf("Game Loop Tests\n");
    printf("=====================================\n\n");

    TEST(create_destroy);
    TEST(initial_state);
    TEST(run_loop);
    TEST(frame_count);
    TEST(max_delta);
    TEST(state_enum);
    TEST(null_callbacks);
    TEST(partial_callbacks);
    TEST(stop_external);
    TEST(null_safety);

    printf("\n=====================================\n");
    printf("Results: %d/%d tests passed\n", tests_passed, tests_run);
    printf("=====================================\n");

    logger_shutdown();

    return (tests_passed == tests_run) ? EXIT_SUCCESS : EXIT_FAILURE;
}

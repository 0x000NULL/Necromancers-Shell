/**
 * Necromancer's Shell - Phase 0 Foundation Demo
 *
 * Demonstrates: Terminal interface, colors, timing, memory management, logging
 */

#include "core/memory.h"
#include "core/timing.h"
#include "terminal/ncurses_wrapper.h"
#include "terminal/colors.h"
#include "utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Demo state */
typedef struct {
    bool running;
    int frame_count;
    MemoryPool* pool;
} DemoState;

static void draw_header(int width) {
    term_draw_box(0, 0, width, 3, colors_get_game_pair(COLOR_PAIR_UI_BORDER));
    term_print(2, 1, colors_get_game_pair(COLOR_PAIR_UI_HIGHLIGHT),
               "NECROMANCER'S SHELL - Phase 0 Foundation Demo");
}

static void draw_instructions(int width, int height) {
    int y = height - 5;
    term_draw_hline(0, y++, width, colors_get_game_pair(COLOR_PAIR_UI_BORDER));

    term_print(2, y++, colors_get_game_pair(COLOR_PAIR_UI_TEXT),
               "Instructions:");
    term_print(2, y++, colors_get_game_pair(COLOR_PAIR_UI_TEXT),
               "  [Q] Quit  [T] Test Memory  [C] Change Color");
    term_print(2, y++, colors_get_game_pair(COLOR_PAIR_UI_TEXT),
               "  Press any key to test input system...");
}

static void draw_stats(DemoState* state, int y) {
    term_print(2, y++, colors_get_game_pair(COLOR_PAIR_UI_TEXT), "System Status:");
    term_printf(2, y++, colors_get_game_pair(COLOR_PAIR_SUCCESS),
                "  FPS: %.1f", timing_get_fps());
    term_printf(2, y++, colors_get_game_pair(COLOR_PAIR_SUCCESS),
                "  Frame: %d", state->frame_count);
    term_printf(2, y++, colors_get_game_pair(COLOR_PAIR_SUCCESS),
                "  Time: %.1fs", timing_get_time());

    /* Memory pool stats */
    if (state->pool) {
        MemoryStats stats;
        pool_get_stats(state->pool, &stats);
        term_printf(2, y++, colors_get_game_pair(COLOR_PAIR_UI_TEXT),
                    "  Memory: %zu/%zu blocks", stats.allocated_blocks, stats.block_count);
    }
}

static void draw_demo_area(int width, int height) {
    int box_y = 4;
    int box_height = height - 10;

    term_draw_box(0, box_y, width, box_height,
                  colors_get_game_pair(COLOR_PAIR_UI_BORDER));

    term_print(2, box_y + 1, colors_get_game_pair(COLOR_PAIR_UI_HIGHLIGHT),
               "PHASE 0 SYSTEMS OPERATIONAL:");

    int y = box_y + 3;
    term_print(4, y++, colors_get_game_pair(COLOR_PAIR_SUCCESS),
               "✓ Terminal Interface (ncurses wrapper)");
    term_print(4, y++, colors_get_game_pair(COLOR_PAIR_SUCCESS),
               "✓ Color System (8 color pairs)");
    term_print(4, y++, colors_get_game_pair(COLOR_PAIR_SUCCESS),
               "✓ Timing System (FPS control)");
    term_print(4, y++, colors_get_game_pair(COLOR_PAIR_SUCCESS),
               "✓ Memory Pool Manager");
    term_print(4, y++, colors_get_game_pair(COLOR_PAIR_SUCCESS),
               "✓ Logger System");
    y++;

    term_print(4, y++, colors_get_game_pair(COLOR_PAIR_UI_TEXT),
               "Next Phase: Command System, Game Loop, State Machine");
}

int main(void) {
    /* Initialize logger */
    if (!logger_init("necromancer_shell.log", LOG_LEVEL_DEBUG)) {
        fprintf(stderr, "Failed to initialize logger\n");
        return EXIT_FAILURE;
    }

    LOG_INFO("Necromancer's Shell - Phase 0 starting");

    /* Initialize timing */
    timing_init();

    /* Initialize terminal */
    if (!term_init()) {
        LOG_ERROR("Failed to initialize terminal");
        logger_shutdown();
        return EXIT_FAILURE;
    }

    /* Initialize colors */
    if (!colors_init()) {
        LOG_WARN("Colors not available");
    }

    /* Create demo memory pool */
    MemoryPool* demo_pool = pool_create(256, 50);
    if (!demo_pool) {
        LOG_ERROR("Failed to create memory pool");
        term_shutdown();
        logger_shutdown();
        return EXIT_FAILURE;
    }

    /* Demo state */
    DemoState state = {
        .running = true,
        .frame_count = 0,
        .pool = demo_pool
    };

    LOG_INFO("Entering main loop");

    /* Main loop */
    while (state.running) {
        timing_frame_start();

        /* Handle input */
        int ch = term_get_char();
        if (ch != -1) {
            if (ch == 'q' || ch == 'Q') {
                LOG_INFO("User requested quit");
                state.running = false;
            } else if (ch == 't' || ch == 'T') {
                /* Test memory allocation */
                void* ptr = pool_alloc(demo_pool);
                if (ptr) {
                    LOG_DEBUG("Allocated memory block");
                    strcpy(ptr, "Test allocation");
                    pool_free(demo_pool, ptr);
                    LOG_DEBUG("Freed memory block");
                }
            }
        }

        /* Render */
        term_clear();

        int width, height;
        term_get_size(&width, &height);

        draw_header(width);
        draw_demo_area(width, height);
        draw_stats(&state, height - 10);
        draw_instructions(width, height);

        term_refresh();

        /* Update */
        state.frame_count++;

        /* Limit FPS */
        timing_frame_end(60);
    }

    LOG_INFO("Shutting down");

    /* Cleanup */
    pool_check_leaks(demo_pool);
    pool_destroy(demo_pool);
    term_shutdown();
    logger_shutdown();

    printf("\nNecromancer's Shell - Phase 0 Foundation Complete!\n");
    printf("All systems operational. Ready for Phase 1.\n");

    return EXIT_SUCCESS;
}

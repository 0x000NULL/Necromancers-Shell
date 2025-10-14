#include "core/game_loop.h"
#include "core/timing.h"
#include "utils/logger.h"
#include <stdlib.h>
#include <time.h>

/* Game loop structure */
struct GameLoop {
    GameLoopCallbacks callbacks;
    void* userdata;
    GameLoopState state;
    unsigned long long frame_count;
    double elapsed_time;
    double current_fps;
    double max_delta_time;
};

GameLoop* game_loop_create(const GameLoopCallbacks* callbacks, void* userdata) {
    if (!callbacks) {
        LOG_ERROR("NULL callbacks provided");
        return NULL;
    }

    GameLoop* loop = calloc(1, sizeof(GameLoop));
    if (!loop) {
        LOG_ERROR("Failed to allocate game loop");
        return NULL;
    }

    loop->callbacks = *callbacks;
    loop->userdata = userdata;
    loop->state = LOOP_STATE_STOPPED;
    loop->frame_count = 0;
    loop->elapsed_time = 0.0;
    loop->current_fps = 0.0;
    loop->max_delta_time = 0.1;  /* 100ms maximum delta */

    LOG_DEBUG("Created game loop");
    return loop;
}

void game_loop_destroy(GameLoop* loop) {
    if (!loop) return;

    /* Stop loop if running */
    if (loop->state != LOOP_STATE_STOPPED) {
        game_loop_stop(loop);
    }

    free(loop);
    LOG_DEBUG("Destroyed game loop");
}

bool game_loop_run(GameLoop* loop, unsigned int target_fps) {
    if (!loop) return false;

    if (loop->state != LOOP_STATE_STOPPED) {
        LOG_WARN("Loop already running");
        return false;
    }

    /* Call init callback */
    if (loop->callbacks.on_init) {
        if (!loop->callbacks.on_init(loop->userdata)) {
            LOG_ERROR("Initialization failed");
            return false;
        }
    }

    loop->state = LOOP_STATE_RUNNING;
    loop->frame_count = 0;
    loop->elapsed_time = 0.0;

    LOG_INFO("Starting game loop (target FPS: %u)", target_fps);

    /* Main loop */
    double frame_start_time = timing_get_time();
    double last_fps_update = frame_start_time;
    unsigned int fps_frame_count = 0;

    while (loop->state != LOOP_STATE_STOPPED) {
        timing_frame_start();

        /* Calculate delta time */
        double current_time = timing_get_time();
        double delta_time = timing_get_delta();

        /* Clamp delta time to prevent spiral of death */
        if (delta_time > loop->max_delta_time) {
            delta_time = loop->max_delta_time;
        }

        /* Update */
        if (loop->state == LOOP_STATE_RUNNING) {
            if (loop->callbacks.on_update) {
                loop->callbacks.on_update(delta_time, loop->userdata);
            }
            loop->elapsed_time += delta_time;
        }

        /* Render (always render, even when paused) */
        if (loop->callbacks.on_render) {
            loop->callbacks.on_render(loop->userdata);
        }

        loop->frame_count++;
        fps_frame_count++;

        /* Update FPS counter every second */
        if (current_time - last_fps_update >= 1.0) {
            loop->current_fps = fps_frame_count / (current_time - last_fps_update);
            fps_frame_count = 0;
            last_fps_update = current_time;
        }

        /* Frame limiting */
        if (target_fps > 0) {
            timing_frame_end(target_fps);
        }
    }

    LOG_INFO("Game loop stopped (frames: %llu, time: %.2f)",
             loop->frame_count, loop->elapsed_time);

    /* Call cleanup callback */
    if (loop->callbacks.on_cleanup) {
        loop->callbacks.on_cleanup(loop->userdata);
    }

    return true;
}

void game_loop_stop(GameLoop* loop) {
    if (!loop) return;

    if (loop->state == LOOP_STATE_STOPPED) {
        LOG_WARN("Loop already stopped");
        return;
    }

    loop->state = LOOP_STATE_STOPPED;
    LOG_INFO("Stopping game loop");
}

void game_loop_pause(GameLoop* loop) {
    if (!loop) return;

    if (loop->state != LOOP_STATE_RUNNING) {
        LOG_WARN("Cannot pause: loop not running");
        return;
    }

    loop->state = LOOP_STATE_PAUSED;

    if (loop->callbacks.on_pause) {
        loop->callbacks.on_pause(loop->userdata);
    }

    LOG_DEBUG("Game loop paused");
}

void game_loop_resume(GameLoop* loop) {
    if (!loop) return;

    if (loop->state != LOOP_STATE_PAUSED) {
        LOG_WARN("Cannot resume: loop not paused");
        return;
    }

    loop->state = LOOP_STATE_RUNNING;

    if (loop->callbacks.on_resume) {
        loop->callbacks.on_resume(loop->userdata);
    }

    LOG_DEBUG("Game loop resumed");
}

bool game_loop_is_running(const GameLoop* loop) {
    if (!loop) return false;
    return loop->state == LOOP_STATE_RUNNING || loop->state == LOOP_STATE_PAUSED;
}

bool game_loop_is_paused(const GameLoop* loop) {
    if (!loop) return false;
    return loop->state == LOOP_STATE_PAUSED;
}

GameLoopState game_loop_get_state(const GameLoop* loop) {
    return loop ? loop->state : LOOP_STATE_STOPPED;
}

double game_loop_get_fps(const GameLoop* loop) {
    return loop ? loop->current_fps : 0.0;
}

unsigned long long game_loop_get_frame_count(const GameLoop* loop) {
    return loop ? loop->frame_count : 0;
}

double game_loop_get_elapsed_time(const GameLoop* loop) {
    return loop ? loop->elapsed_time : 0.0;
}

void game_loop_set_max_delta(GameLoop* loop, double max_delta) {
    if (!loop) return;
    loop->max_delta_time = max_delta;
    LOG_DEBUG("Max delta time set to %.3f", max_delta);
}

double game_loop_get_max_delta(const GameLoop* loop) {
    return loop ? loop->max_delta_time : 0.0;
}

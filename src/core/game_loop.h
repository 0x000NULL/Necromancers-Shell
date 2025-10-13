#ifndef GAME_LOOP_H
#define GAME_LOOP_H

#include <stdbool.h>

/**
 * Game Loop - Main game loop abstraction
 *
 * Provides callback-based game loop with initialization, update, render,
 * and cleanup phases. Integrates with timing system for fixed frame rate.
 *
 * Usage:
 *   GameLoopCallbacks callbacks = {
 *       .on_init = init_game,
 *       .on_update = update_game,
 *       .on_render = render_game,
 *       .on_cleanup = cleanup_game
 *   };
 *   GameLoop* loop = game_loop_create(&callbacks, userdata);
 *   game_loop_run(loop, 60);  // 60 FPS
 *   game_loop_destroy(loop);
 */

/* Forward declaration */
typedef struct GameLoop GameLoop;

/* Game loop callbacks */
typedef struct {
    /**
     * Called once at initialization
     * Return false to abort loop start
     */
    bool (*on_init)(void* userdata);

    /**
     * Called every frame for game logic
     * delta_time is in seconds
     */
    void (*on_update)(double delta_time, void* userdata);

    /**
     * Called every frame for rendering
     */
    void (*on_render)(void* userdata);

    /**
     * Called once at cleanup
     */
    void (*on_cleanup)(void* userdata);

    /**
     * Called when loop is paused (optional)
     */
    void (*on_pause)(void* userdata);

    /**
     * Called when loop is resumed (optional)
     */
    void (*on_resume)(void* userdata);

} GameLoopCallbacks;

/* Game loop state */
typedef enum {
    LOOP_STATE_STOPPED = 0,
    LOOP_STATE_RUNNING,
    LOOP_STATE_PAUSED
} GameLoopState;

/**
 * Create game loop
 *
 * @param callbacks Loop callbacks
 * @param userdata User data passed to callbacks
 * @return Game loop pointer or NULL on failure
 */
GameLoop* game_loop_create(const GameLoopCallbacks* callbacks, void* userdata);

/**
 * Destroy game loop
 * Automatically stops loop if running
 *
 * @param loop Game loop
 */
void game_loop_destroy(GameLoop* loop);

/**
 * Run the game loop
 * Blocks until loop is stopped
 *
 * @param loop Game loop
 * @param target_fps Target frames per second (0 for unlimited)
 * @return true if loop completed successfully
 */
bool game_loop_run(GameLoop* loop, unsigned int target_fps);

/**
 * Stop the game loop
 * Loop will exit after current frame
 *
 * @param loop Game loop
 */
void game_loop_stop(GameLoop* loop);

/**
 * Pause the game loop
 * Update callbacks will not be called, but render will continue
 *
 * @param loop Game loop
 */
void game_loop_pause(GameLoop* loop);

/**
 * Resume the game loop
 *
 * @param loop Game loop
 */
void game_loop_resume(GameLoop* loop);

/**
 * Check if loop is running
 *
 * @param loop Game loop
 * @return true if running or paused
 */
bool game_loop_is_running(const GameLoop* loop);

/**
 * Check if loop is paused
 *
 * @param loop Game loop
 * @return true if paused
 */
bool game_loop_is_paused(const GameLoop* loop);

/**
 * Get current loop state
 *
 * @param loop Game loop
 * @return Loop state
 */
GameLoopState game_loop_get_state(const GameLoop* loop);

/**
 * Get current FPS
 *
 * @param loop Game loop
 * @return Current frames per second
 */
double game_loop_get_fps(const GameLoop* loop);

/**
 * Get total frames processed
 *
 * @param loop Game loop
 * @return Frame count
 */
unsigned long long game_loop_get_frame_count(const GameLoop* loop);

/**
 * Get total time elapsed (seconds)
 *
 * @param loop Game loop
 * @return Elapsed time
 */
double game_loop_get_elapsed_time(const GameLoop* loop);

/**
 * Set maximum delta time (prevents spiral of death)
 * Default is 0.1 seconds
 *
 * @param loop Game loop
 * @param max_delta Maximum delta time in seconds
 */
void game_loop_set_max_delta(GameLoop* loop, double max_delta);

/**
 * Get maximum delta time
 *
 * @param loop Game loop
 * @return Maximum delta time
 */
double game_loop_get_max_delta(const GameLoop* loop);

#endif /* GAME_LOOP_H */

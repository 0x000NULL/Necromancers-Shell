#ifndef TIMING_H
#define TIMING_H

#include <stdint.h>
#include <stdbool.h>

/**
 * Timing and FPS Control
 *
 * Cross-platform timing for game loop, FPS limiting, delta time.
 *
 * Usage:
 *   timing_init();
 *   while (running) {
 *       timing_frame_start();
 *       update(timing_get_delta());
 *       render();
 *       timing_frame_end(60);  // 60 FPS target
 *   }
 */

/**
 * Initialize timing system
 */
void timing_init(void);

/**
 * Mark start of frame
 */
void timing_frame_start(void);

/**
 * Mark end of frame and limit FPS
 *
 * @param target_fps Target frames per second (0 = unlimited)
 */
void timing_frame_end(int target_fps);

/**
 * Get delta time (seconds since last frame)
 *
 * @return Delta time in seconds
 */
double timing_get_delta(void);

/**
 * Get current FPS
 *
 * @return Frames per second (averaged)
 */
double timing_get_fps(void);

/**
 * Get current time (seconds since init)
 *
 * @return Time in seconds
 */
double timing_get_time(void);

/**
 * Sleep for specified milliseconds
 *
 * @param ms Milliseconds to sleep
 */
void timing_sleep_ms(int ms);

#endif /* TIMING_H */

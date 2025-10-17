/**
 * @file ending_cinematics.h
 * @brief Narrative cinematics for all six ending paths
 *
 * Displays multi-stage story sequences with text, dialogue, and achievements
 * for each ending the player can achieve.
 *
 * Each ending cinematic consists of:
 * - Introduction scene setting
 * - Main narrative body (multiple paragraphs)
 * - Key dialogue or revelation
 * - Epilogue (what happens next)
 * - Achievement summary
 */

#ifndef ENDING_CINEMATICS_H
#define ENDING_CINEMATICS_H

#include "ending_system.h"
#include <stddef.h>

/* Forward declaration for ncurses WINDOW */
struct _win_st;
typedef struct _win_st WINDOW;

/**
 * @brief Cinematic scene structure
 */
typedef struct {
    const char* title;          /**< Scene title (e.g., "THE RESURRECTION") */
    const char** paragraphs;    /**< Array of paragraph strings */
    size_t paragraph_count;     /**< Number of paragraphs */
} CinematicScene;

/**
 * @brief Play complete ending cinematic sequence
 *
 * Displays the full narrative sequence for the achieved ending,
 * including introduction, main story, epilogue, and achievements.
 *
 * @param win ncurses window
 * @param ending Ending type to display
 * @param achievement Player's achievement data
 *
 * @note Window must be at least 80x24 for proper display
 * @note Player must press key to advance between scenes
 */
void play_ending_cinematic(
    WINDOW* win,
    EndingType ending,
    const EndingAchievement* achievement
);

/**
 * @brief Display achievement summary screen
 *
 * Shows final statistics and accomplishments:
 * - Ending achieved
 * - Final corruption %
 * - Trials passed (X/7)
 * - Average trial score
 * - Divine Council verdict
 * - Key choices made
 *
 * @param win ncurses window
 * @param achievement Achievement data
 */
void display_achievement_screen(
    WINDOW* win,
    const EndingAchievement* achievement
);

/**
 * @brief Display credits screen
 *
 * Shows game credits and thank you message.
 *
 * @param win ncurses window
 */
void display_credits(WINDOW* win);

/**
 * @brief Get ending-specific epilogue text
 *
 * Returns what happens in the future after this ending.
 *
 * @param ending Ending type
 * @return Array of epilogue paragraphs
 */
const char** get_ending_epilogue(EndingType ending, size_t* count_out);

#endif /* ENDING_CINEMATICS_H */

/**
 * @file narrative_display.h
 * @brief Narrative scene rendering and text formatting for story events
 *
 * Provides utilities for displaying story scenes, wrapping text,
 * and formatting narrative content in ncurses windows.
 */

#ifndef NARRATIVE_DISPLAY_H
#define NARRATIVE_DISPLAY_H

#include <stddef.h>
#include <stdbool.h>

/* Forward declaration for ncurses WINDOW */
struct _win_st;
typedef struct _win_st WINDOW;

/* Maximum line width for text wrapping (76 chars leaves 2-char margins) */
#define MAX_LINE_WIDTH 76

/**
 * @brief Wrapped text structure for multi-line content
 */
typedef struct {
    char** lines;            /**< Array of wrapped text lines */
    size_t line_count;       /**< Number of lines in array */
    size_t capacity;         /**< Allocated capacity */
} WrappedText;

/**
 * @brief Color pair for scene elements
 */
typedef enum {
    SCENE_COLOR_TITLE,       /**< Scene title color */
    SCENE_COLOR_TEXT,        /**< Normal narrative text */
    SCENE_COLOR_EMPHASIS,    /**< Emphasized text (quotes, important info) */
    SCENE_COLOR_WARNING,     /**< Warning or danger text */
    SCENE_COLOR_SUCCESS,     /**< Positive outcome text */
    SCENE_COLOR_DIM          /**< Dimmed/secondary text */
} SceneColor;

/**
 * @brief Wrap text to fit within specified line width
 *
 * Breaks text into multiple lines using word boundaries.
 * Preserves paragraph breaks (double newlines).
 *
 * @param text Source text to wrap
 * @param max_width Maximum width per line
 * @return WrappedText structure (must be freed with wrapped_text_destroy)
 *
 * @note Returns NULL on allocation failure
 */
WrappedText* wrap_text(const char* text, int max_width);

/**
 * @brief Free wrapped text structure
 *
 * @param wrapped Wrapped text to free
 */
void wrapped_text_destroy(WrappedText* wrapped);

/**
 * @brief Display a narrative scene with title and paragraphs
 *
 * Renders a formatted story scene with:
 * - Centered title with decorative borders
 * - Multiple paragraphs with automatic wrapping
 * - Proper spacing and color coding
 *
 * @param win ncurses window
 * @param title Scene title (e.g., "ASHBROOK VILLAGE - DAY 47")
 * @param paragraphs Array of paragraph strings
 * @param paragraph_count Number of paragraphs
 * @param title_color Color for title text
 *
 * @note Window is cleared before rendering
 */
void display_narrative_scene(
    WINDOW* win,
    const char* title,
    const char** paragraphs,
    size_t paragraph_count,
    SceneColor title_color
);

/**
 * @brief Display single paragraph with wrapping
 *
 * Renders a paragraph of text with automatic line wrapping.
 *
 * @param win ncurses window
 * @param start_y Starting Y coordinate
 * @param start_x Starting X coordinate
 * @param text Paragraph text
 * @param max_width Maximum line width
 * @param color Text color
 * @return Number of lines rendered
 */
int display_wrapped_paragraph(
    WINDOW* win,
    int start_y,
    int start_x,
    const char* text,
    int max_width,
    SceneColor color
);

/**
 * @brief Display a horizontal separator line
 *
 * Renders a decorative line (e.g., "========")
 *
 * @param win ncurses window
 * @param y Y coordinate
 * @param x Starting X coordinate
 * @param width Width of separator
 * @param ch Character to use for separator
 */
void display_separator(
    WINDOW* win,
    int y,
    int x,
    int width,
    char ch
);

/**
 * @brief Wait for user to press any key
 *
 * Displays "Press any key to continue..." and waits.
 *
 * @param win ncurses window
 * @param y Y coordinate for prompt
 */
void wait_for_keypress(
    WINDOW* win,
    int y
);

/**
 * @brief Display a dialogue line with speaker name
 *
 * Formats dialogue as:
 * Speaker Name: "Dialogue text here..."
 *
 * @param win ncurses window
 * @param y Y coordinate
 * @param speaker Speaker name
 * @param dialogue Dialogue text
 * @param speaker_color Color for speaker name
 * @param dialogue_color Color for dialogue text
 * @return Number of lines rendered
 */
int display_dialogue_line(
    WINDOW* win,
    int y,
    const char* speaker,
    const char* dialogue,
    SceneColor speaker_color,
    SceneColor dialogue_color
);

/**
 * @brief Display centered text
 *
 * Centers text horizontally in window.
 *
 * @param win ncurses window
 * @param y Y coordinate
 * @param text Text to center
 * @param color Text color
 */
void display_centered_text(
    WINDOW* win,
    int y,
    const char* text,
    SceneColor color
);

/**
 * @brief Convert SceneColor to ncurses color pair
 *
 * @param color Scene color enum
 * @return ncurses color pair number
 */
int scene_color_to_pair(SceneColor color);

#endif /* NARRATIVE_DISPLAY_H */

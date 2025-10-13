#ifndef NCURSES_WRAPPER_H
#define NCURSES_WRAPPER_H

#include <stdbool.h>

/**
 * Terminal Interface Wrapper
 *
 * Abstracts ncurses for cross-platform compatibility and cleaner API.
 *
 * Usage:
 *   term_init();
 *   term_clear();
 *   term_print(10, 5, COLOR_PAIR_RED_BLACK, "Hello");
 *   term_refresh();
 *   term_shutdown();
 */

/**
 * Initialize terminal
 *
 * @return true on success
 */
bool term_init(void);

/**
 * Shutdown terminal
 */
void term_shutdown(void);

/**
 * Clear screen
 */
void term_clear(void);

/**
 * Refresh screen (display changes)
 */
void term_refresh(void);

/**
 * Get terminal dimensions
 *
 * @param width Output width
 * @param height Output height
 */
void term_get_size(int* width, int* height);

/**
 * Print string at position with color
 *
 * @param x X coordinate
 * @param y Y coordinate
 * @param color_pair Color pair ID
 * @param str String to print
 */
void term_print(int x, int y, int color_pair, const char* str);

/**
 * Print formatted string at position
 *
 * @param x X coordinate
 * @param y Y coordinate
 * @param color_pair Color pair ID
 * @param fmt Format string
 */
void term_printf(int x, int y, int color_pair, const char* fmt, ...);

/**
 * Draw box
 *
 * @param x X coordinate (top-left)
 * @param y Y coordinate (top-left)
 * @param width Box width
 * @param height Box height
 * @param color_pair Color pair ID
 */
void term_draw_box(int x, int y, int width, int height, int color_pair);

/**
 * Draw horizontal line
 *
 * @param x X start
 * @param y Y position
 * @param length Line length
 * @param color_pair Color pair ID
 */
void term_draw_hline(int x, int y, int length, int color_pair);

/**
 * Draw vertical line
 *
 * @param x X position
 * @param y Y start
 * @param length Line length
 * @param color_pair Color pair ID
 */
void term_draw_vline(int x, int y, int length, int color_pair);

/**
 * Set cursor visibility
 *
 * @param visible true to show cursor
 */
void term_set_cursor(bool visible);

/**
 * Move cursor
 *
 * @param x X coordinate
 * @param y Y coordinate
 */
void term_move_cursor(int x, int y);

/**
 * Enable/disable input echo
 *
 * @param enable true to enable echo
 */
void term_set_echo(bool enable);

/**
 * Set raw/cooked mode
 *
 * @param raw true for raw mode
 */
void term_set_raw(bool raw);

/**
 * Check if color supported
 *
 * @return true if colors available
 */
bool term_has_colors(void);

/**
 * Get maximum color pairs
 *
 * @return Max color pairs
 */
int term_max_color_pairs(void);

/**
 * Get a character (non-blocking)
 *
 * @return Character code or -1 if no input
 */
int term_get_char(void);

#endif /* NCURSES_WRAPPER_H */

#ifndef COLORS_H
#define COLORS_H

#include <stdbool.h>

/**
 * Color Management System
 *
 * Manages color pairs and provides game color palette.
 */

/* Game color palette */
typedef enum {
    COLOR_PAIR_DEFAULT = 0,
    COLOR_PAIR_UI_BORDER = 1,
    COLOR_PAIR_UI_TEXT = 2,
    COLOR_PAIR_UI_HIGHLIGHT = 3,
    COLOR_PAIR_HP_HIGH = 4,
    COLOR_PAIR_HP_LOW = 5,
    COLOR_PAIR_ERROR = 6,
    COLOR_PAIR_SUCCESS = 7,
    COLOR_PAIR_COUNT
} GameColorPair;

/**
 * Initialize color system
 *
 * @return true on success
 */
bool colors_init(void);

/**
 * Get game color pair
 *
 * @param pair Game color pair enum
 * @return ncurses color pair ID
 */
int colors_get_game_pair(GameColorPair pair);

/* Convenience macros for UI components */
#define UI_BORDER     COLOR_PAIR_UI_BORDER
#define UI_TEXT       COLOR_PAIR_UI_TEXT
#define UI_HIGHLIGHT  COLOR_PAIR_UI_HIGHLIGHT
#define TEXT_NORMAL   COLOR_PAIR_UI_TEXT
#define TEXT_INFO     COLOR_PAIR_UI_HIGHLIGHT
#define TEXT_ERROR    COLOR_PAIR_ERROR
#define TEXT_SUCCESS  COLOR_PAIR_SUCCESS
#define TEXT_WARNING  COLOR_PAIR_HP_LOW
#define TEXT_DIM      COLOR_PAIR_UI_BORDER

#endif /* COLORS_H */

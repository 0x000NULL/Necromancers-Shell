#include "terminal/colors.h"
#include "terminal/ncurses_wrapper.h"
#include "terminal/platform_curses.h"
#include "utils/logger.h"

/* Game color pair mappings */
static int g_game_pairs[COLOR_PAIR_COUNT];

bool colors_init(void) {
    if (!term_has_colors()) {
        LOG_WARN("Terminal does not support colors");
        return false;
    }

    LOG_INFO("Initializing color system with %d game color pairs", COLOR_PAIR_COUNT);

    /* Initialize game color pairs */
    g_game_pairs[COLOR_PAIR_DEFAULT] = 0;

    init_pair(COLOR_PAIR_UI_BORDER, COLOR_CYAN, COLOR_BLACK);
    g_game_pairs[COLOR_PAIR_UI_BORDER] = COLOR_PAIR_UI_BORDER;

    init_pair(COLOR_PAIR_UI_TEXT, COLOR_WHITE, COLOR_BLACK);
    g_game_pairs[COLOR_PAIR_UI_TEXT] = COLOR_PAIR_UI_TEXT;

    init_pair(COLOR_PAIR_UI_HIGHLIGHT, COLOR_YELLOW, COLOR_BLACK);
    g_game_pairs[COLOR_PAIR_UI_HIGHLIGHT] = COLOR_PAIR_UI_HIGHLIGHT;

    init_pair(COLOR_PAIR_HP_HIGH, COLOR_GREEN, COLOR_BLACK);
    g_game_pairs[COLOR_PAIR_HP_HIGH] = COLOR_PAIR_HP_HIGH;

    init_pair(COLOR_PAIR_HP_LOW, COLOR_RED, COLOR_BLACK);
    g_game_pairs[COLOR_PAIR_HP_LOW] = COLOR_PAIR_HP_LOW;

    init_pair(COLOR_PAIR_ERROR, COLOR_RED, COLOR_BLACK);
    g_game_pairs[COLOR_PAIR_ERROR] = COLOR_PAIR_ERROR;

    init_pair(COLOR_PAIR_SUCCESS, COLOR_GREEN, COLOR_BLACK);
    g_game_pairs[COLOR_PAIR_SUCCESS] = COLOR_PAIR_SUCCESS;

    LOG_DEBUG("Color system initialized");
    return true;
}

int colors_get_game_pair(GameColorPair pair) {
    if (pair < 0 || pair >= COLOR_PAIR_COUNT) {
        return 0;
    }
    return g_game_pairs[pair];
}

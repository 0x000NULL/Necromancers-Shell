#include "terminal/ncurses_wrapper.h"
#include "terminal/platform_curses.h"
#include "utils/logger.h"
#include <stdarg.h>
#include <string.h>

/* Global terminal state */
static bool g_term_initialized = false;

bool term_init(void) {
    if (g_term_initialized) {
        LOG_WARN("Terminal already initialized");
        return true;
    }

    /* Initialize ncurses */
    if (!initscr()) {
        LOG_ERROR("Failed to initialize ncurses");
        return false;
    }

    /* Configure terminal */
    cbreak();              /* Disable line buffering */
    noecho();              /* Don't echo input */
    keypad(stdscr, TRUE);  /* Enable function keys */
    nodelay(stdscr, TRUE); /* Non-blocking input */
    curs_set(0);           /* Hide cursor */

    /* Initialize colors if available */
    if (has_colors()) {
        start_color();
        use_default_colors();
        LOG_INFO("Terminal supports colors: %d pairs", COLOR_PAIRS);
    } else {
        LOG_WARN("Terminal does not support colors");
    }

    /* Get terminal size */
    int width, height;
    term_get_size(&width, &height);
    LOG_INFO("Terminal initialized: %dx%d", width, height);

    g_term_initialized = true;
    return true;
}

void term_shutdown(void) {
    if (!g_term_initialized) return;

    LOG_DEBUG("Shutting down terminal");
    endwin();
    g_term_initialized = false;
}

void term_clear(void) {
    clear();
}

void term_refresh(void) {
    refresh();
}

void term_get_size(int* width, int* height) {
    if (width) *width = COLS;
    if (height) *height = LINES;
}

void term_print(int x, int y, int color_pair, const char* str) {
    if (!str) return;

    if (color_pair > 0) {
        attron(COLOR_PAIR(color_pair));
    }
    mvprintw(y, x, "%s", str);
    if (color_pair > 0) {
        attroff(COLOR_PAIR(color_pair));
    }
}

void term_printf(int x, int y, int color_pair, const char* fmt, ...) {
    char buffer[1024];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    term_print(x, y, color_pair, buffer);
}

void term_draw_box(int x, int y, int width, int height, int color_pair) {
    if (color_pair > 0) {
        attron(COLOR_PAIR(color_pair));
    }

    /* Corners */
    mvaddch(y, x, ACS_ULCORNER);
    mvaddch(y, x + width - 1, ACS_URCORNER);
    mvaddch(y + height - 1, x, ACS_LLCORNER);
    mvaddch(y + height - 1, x + width - 1, ACS_LRCORNER);

    /* Horizontal lines */
    for (int i = 1; i < width - 1; i++) {
        mvaddch(y, x + i, ACS_HLINE);
        mvaddch(y + height - 1, x + i, ACS_HLINE);
    }

    /* Vertical lines */
    for (int i = 1; i < height - 1; i++) {
        mvaddch(y + i, x, ACS_VLINE);
        mvaddch(y + i, x + width - 1, ACS_VLINE);
    }

    if (color_pair > 0) {
        attroff(COLOR_PAIR(color_pair));
    }
}

void term_draw_hline(int x, int y, int length, int color_pair) {
    if (color_pair > 0) {
        attron(COLOR_PAIR(color_pair));
    }
    mvhline(y, x, ACS_HLINE, length);
    if (color_pair > 0) {
        attroff(COLOR_PAIR(color_pair));
    }
}

void term_draw_vline(int x, int y, int length, int color_pair) {
    if (color_pair > 0) {
        attron(COLOR_PAIR(color_pair));
    }
    mvvline(y, x, ACS_VLINE, length);
    if (color_pair > 0) {
        attroff(COLOR_PAIR(color_pair));
    }
}

void term_set_cursor(bool visible) {
    curs_set(visible ? 1 : 0);
}

void term_move_cursor(int x, int y) {
    move(y, x);
}

void term_set_echo(bool enable) {
    if (enable) {
        echo();
    } else {
        noecho();
    }
}

void term_set_raw(bool enable_raw) {
    if (enable_raw) {
        raw();
    } else {
        noraw();
    }
}

bool term_has_colors(void) {
    return has_colors();
}

int term_max_color_pairs(void) {
    return COLOR_PAIRS;
}

int term_get_char(void) {
    return getch();
}

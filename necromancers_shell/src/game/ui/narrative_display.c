/**
 * @file narrative_display.c
 * @brief Implementation of narrative scene rendering
 */

#include "narrative_display.h"
#include "../../terminal/platform_curses.h"
#include "../../terminal/colors.h"
#include "../../utils/logger.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Initial capacity for wrapped text lines */
#define INITIAL_LINE_CAPACITY 16

/**
 * @brief Grow wrapped text capacity
 */
static bool grow_wrapped_capacity(WrappedText* wrapped) {
    if (!wrapped) return false;

    size_t new_capacity = wrapped->capacity * 2;
    char** new_lines = realloc(wrapped->lines, sizeof(char*) * new_capacity);

    if (!new_lines) {
        LOG_ERROR("Failed to grow wrapped text capacity");
        return false;
    }

    wrapped->lines = new_lines;
    wrapped->capacity = new_capacity;
    return true;
}

WrappedText* wrap_text(const char* text, int max_width) {
    if (!text || max_width <= 0) {
        return NULL;
    }

    WrappedText* wrapped = malloc(sizeof(WrappedText));
    if (!wrapped) {
        LOG_ERROR("Failed to allocate WrappedText");
        return NULL;
    }

    wrapped->line_count = 0;
    wrapped->capacity = INITIAL_LINE_CAPACITY;
    wrapped->lines = malloc(sizeof(char*) * wrapped->capacity);

    if (!wrapped->lines) {
        free(wrapped);
        return NULL;
    }

    /* Process text line by line */
    const char* start = text;
    const char* current = text;

    while (*current) {
        /* Handle newlines (paragraph breaks) */
        if (*current == '\n') {
            /* Add empty line for paragraph break */
            if (wrapped->line_count >= wrapped->capacity) {
                if (!grow_wrapped_capacity(wrapped)) {
                    wrapped_text_destroy(wrapped);
                    return NULL;
                }
            }

            wrapped->lines[wrapped->line_count] = malloc(1);
            if (wrapped->lines[wrapped->line_count]) {
                wrapped->lines[wrapped->line_count][0] = '\0';
                wrapped->line_count++;
            }

            start = current + 1;
            current = start;
            continue;
        }

        /* Find word boundaries within max_width */
        const char* line_end = start + max_width;

        if (*line_end && *line_end != ' ' && *line_end != '\n') {
            /* Back up to last space */
            while (line_end > start && *line_end != ' ') {
                line_end--;
            }

            /* Force break if no space found */
            if (line_end == start) {
                line_end = start + max_width;
            }
        }

        /* Copy line */
        size_t line_len = line_end - start;
        if (line_len > 0) {
            if (wrapped->line_count >= wrapped->capacity) {
                if (!grow_wrapped_capacity(wrapped)) {
                    wrapped_text_destroy(wrapped);
                    return NULL;
                }
            }

            wrapped->lines[wrapped->line_count] = malloc(line_len + 1);
            if (wrapped->lines[wrapped->line_count]) {
                strncpy(wrapped->lines[wrapped->line_count], start, line_len);
                wrapped->lines[wrapped->line_count][line_len] = '\0';

                /* Trim trailing whitespace */
                while (line_len > 0 && isspace((unsigned char)wrapped->lines[wrapped->line_count][line_len - 1])) {
                    wrapped->lines[wrapped->line_count][--line_len] = '\0';
                }

                wrapped->line_count++;
            }
        }

        /* Skip whitespace */
        start = line_end;
        while (*start && isspace((unsigned char)*start) && *start != '\n') {
            start++;
        }
        current = start;

        /* Check if we've reached end */
        if (!*current) break;
    }

    return wrapped;
}

void wrapped_text_destroy(WrappedText* wrapped) {
    if (!wrapped) return;

    for (size_t i = 0; i < wrapped->line_count; i++) {
        free(wrapped->lines[i]);
    }

    free(wrapped->lines);
    free(wrapped);
}

int scene_color_to_pair(SceneColor color) {
    switch (color) {
        case SCENE_COLOR_TITLE:
            return UI_HIGHLIGHT;
        case SCENE_COLOR_TEXT:
            return TEXT_NORMAL;
        case SCENE_COLOR_EMPHASIS:
            return TEXT_INFO;
        case SCENE_COLOR_WARNING:
            return TEXT_WARNING;
        case SCENE_COLOR_SUCCESS:
            return TEXT_SUCCESS;
        case SCENE_COLOR_DIM:
            return TEXT_DIM;
        default:
            return TEXT_NORMAL;
    }
}

void display_narrative_scene(
    WINDOW* win,
    const char* title,
    const char** paragraphs,
    size_t paragraph_count,
    SceneColor title_color
) {
    if (!win || !title || !paragraphs) {
        return;
    }

    wclear(win);

    int current_y = 2;

    /* Display title */
    int title_pair = scene_color_to_pair(title_color);
    wattron(win, COLOR_PAIR(title_pair) | A_BOLD);
    mvwprintw(win, current_y, 2, "=== %s ===", title);
    wattroff(win, COLOR_PAIR(title_pair) | A_BOLD);
    current_y += 2;

    /* Display paragraphs */
    for (size_t i = 0; i < paragraph_count; i++) {
        current_y += display_wrapped_paragraph(
            win, current_y, 2, paragraphs[i],
            MAX_LINE_WIDTH, SCENE_COLOR_TEXT
        );
        current_y++; /* Blank line between paragraphs */
    }

    wrefresh(win);
}

int display_wrapped_paragraph(
    WINDOW* win,
    int start_y,
    int start_x,
    const char* text,
    int max_width,
    SceneColor color
) {
    if (!win || !text) {
        return 0;
    }

    WrappedText* wrapped = wrap_text(text, max_width);
    if (!wrapped) {
        return 0;
    }

    int color_pair = scene_color_to_pair(color);
    wattron(win, COLOR_PAIR(color_pair));

    for (size_t i = 0; i < wrapped->line_count; i++) {
        mvwprintw(win, start_y + (int)i, start_x, "%s", wrapped->lines[i]);
    }

    wattroff(win, COLOR_PAIR(color_pair));

    int lines_rendered = (int)wrapped->line_count;
    wrapped_text_destroy(wrapped);

    return lines_rendered;
}

void display_separator(
    WINDOW* win,
    int y,
    int x,
    int width,
    char ch
) {
    if (!win || width <= 0) {
        return;
    }

    for (int i = 0; i < width; i++) {
        mvwaddch(win, y, x + i, ch);
    }
}

void wait_for_keypress(
    WINDOW* win,
    int y
) {
    if (!win) {
        return;
    }

    wattron(win, COLOR_PAIR(TEXT_DIM));
    mvwprintw(win, y, 2, "Press any key to continue...");
    wattroff(win, COLOR_PAIR(TEXT_DIM));
    wrefresh(win);

    wgetch(win);
}

int display_dialogue_line(
    WINDOW* win,
    int y,
    const char* speaker,
    const char* dialogue,
    SceneColor speaker_color,
    SceneColor dialogue_color
) {
    if (!win || !speaker || !dialogue) {
        return 0;
    }

    /* Display speaker name */
    int speaker_pair = scene_color_to_pair(speaker_color);
    wattron(win, COLOR_PAIR(speaker_pair) | A_BOLD);
    mvwprintw(win, y, 2, "%s: ", speaker);
    wattroff(win, COLOR_PAIR(speaker_pair) | A_BOLD);

    /* Calculate dialogue start position */
    int dialogue_x = 2 + (int)strlen(speaker) + 2;

    /* Wrap and display dialogue */
    int dialogue_width = MAX_LINE_WIDTH - dialogue_x + 2;
    if (dialogue_width < 20) {
        dialogue_width = 20; /* Minimum dialogue width */
    }

    return display_wrapped_paragraph(
        win, y, dialogue_x, dialogue,
        dialogue_width, dialogue_color
    );
}

void display_centered_text(
    WINDOW* win,
    int y,
    const char* text,
    SceneColor color
) {
    if (!win || !text) {
        return;
    }

    int max_x, max_y;
    getmaxyx(win, max_y, max_x);
    (void)max_y; /* Unused */

    int text_len = (int)strlen(text);
    int start_x = (max_x - text_len) / 2;

    if (start_x < 0) {
        start_x = 0;
    }

    int color_pair = scene_color_to_pair(color);
    wattron(win, COLOR_PAIR(color_pair));
    mvwprintw(win, y, start_x, "%s", text);
    wattroff(win, COLOR_PAIR(color_pair));
}

/**
 * @file choice_prompt.c
 * @brief Implementation of generic choice prompt system
 */

#include "choice_prompt.h"
#include "../../terminal/platform_curses.h"
#include "../../terminal/colors.h"
#include "../../utils/logger.h"
#include <string.h>
#include <ctype.h>

/* Maximum number of input attempts before giving up */
#define MAX_INPUT_ATTEMPTS 5

/* Color pairs for choice prompts (defined in colors.h) */
#define COLOR_PROMPT_TITLE UI_HIGHLIGHT
#define COLOR_PROMPT_TEXT TEXT_NORMAL
#define COLOR_PROMPT_CHOICE UI_BORDER
#define COLOR_PROMPT_ERROR TEXT_ERROR

/**
 * @brief Normalize input string (lowercase, trim whitespace)
 */
static void normalize_input(char* str) {
    if (!str) return;

    /* Convert to lowercase */
    for (char* p = str; *p; p++) {
        *p = tolower((unsigned char)*p);
    }

    /* Trim trailing whitespace */
    size_t len = strlen(str);
    while (len > 0 && isspace((unsigned char)str[len - 1])) {
        str[--len] = '\0';
    }

    /* Trim leading whitespace */
    char* start = str;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
}

/**
 * @brief Match input against choice options
 */
static bool match_choice(const char* input, const Choice* choices, size_t count, int* result) {
    if (!input || !choices || !result) return false;

    for (size_t i = 0; i < count; i++) {
        /* Match single-key shortcut */
        char key_str[2] = { choices[i].key, '\0' };
        if (strcmp(input, key_str) == 0) {
            *result = (int)i;
            return true;
        }

        /* Match full label (case-insensitive) */
        char label_lower[64];
        strncpy(label_lower, choices[i].label, sizeof(label_lower) - 1);
        label_lower[sizeof(label_lower) - 1] = '\0';
        normalize_input(label_lower);

        if (strcmp(input, label_lower) == 0) {
            *result = (int)i;
            return true;
        }
    }

    return false;
}

bool display_choice_prompt(
    WINDOW* win,
    const char* title,
    const char* description,
    const Choice* choices,
    size_t choice_count,
    int* selected_index
) {
    /* Validate parameters */
    if (!win || !title || !choices || choice_count == 0 || !selected_index) {
        LOG_ERROR("Invalid parameters to display_choice_prompt");
        return false;
    }

    if (choice_count > 10) {
        LOG_WARN("Too many choices (%zu), limiting to 10", choice_count);
        choice_count = 10;
    }

    /* Display prompt */
    wclear(win);

    int current_line = 2;

    /* Title */
    wattron(win, COLOR_PAIR(COLOR_PROMPT_TITLE) | A_BOLD);
    mvwprintw(win, current_line, 2, "=== %s ===", title);
    wattroff(win, COLOR_PAIR(COLOR_PROMPT_TITLE) | A_BOLD);
    current_line += 2;

    /* Description (if provided) */
    if (description) {
        wattron(win, COLOR_PAIR(COLOR_PROMPT_TEXT));
        mvwprintw(win, current_line, 2, "%s", description);
        wattroff(win, COLOR_PAIR(COLOR_PROMPT_TEXT));
        current_line += 2;
    }

    /* Choices */
    wattron(win, COLOR_PAIR(COLOR_PROMPT_CHOICE));
    for (size_t i = 0; i < choice_count; i++) {
        mvwprintw(win, current_line, 2, "[%c] %s",
                  choices[i].key, choices[i].label);

        if (choices[i].description[0] != '\0') {
            wprintw(win, " - %s", choices[i].description);
        }

        current_line++;
    }
    wattroff(win, COLOR_PAIR(COLOR_PROMPT_CHOICE));
    current_line++;

    /* Prompt line */
    int prompt_line = current_line;
    wattron(win, COLOR_PAIR(COLOR_PROMPT_TEXT));
    mvwprintw(win, prompt_line, 2, "Enter choice: ");
    wattroff(win, COLOR_PAIR(COLOR_PROMPT_TEXT));

    int error_line = prompt_line + 2;

    wrefresh(win);

    /* Get user input with retry logic */
    char input[128];
    int attempts = 0;

    while (attempts < MAX_INPUT_ATTEMPTS) {
        /* Clear input area */
        mvwprintw(win, prompt_line, 16, "                                        ");
        wmove(win, prompt_line, 16);
        wrefresh(win);

        /* Get input */
        echo();
        wgetnstr(win, input, sizeof(input) - 1);
        noecho();

        /* Normalize and match */
        normalize_input(input);

        int choice;
        if (match_choice(input, choices, choice_count, &choice)) {
            /* Valid choice */
            clear_prompt_error(win, error_line);
            *selected_index = choice;
            return true;
        }

        /* Invalid input */
        attempts++;
        display_prompt_error(win, error_line,
                           "Invalid choice. Please try again.");
        wrefresh(win);
    }

    /* Max attempts exceeded */
    display_prompt_error(win, error_line,
                        "Maximum attempts exceeded. Operation cancelled.");
    wrefresh(win);

    LOG_WARN("User exceeded max input attempts for choice prompt");
    return false;
}

bool prompt_yes_no(
    WINDOW* win,
    const char* title,
    const char* question,
    bool* yes_result
) {
    if (!win || !title || !question || !yes_result) {
        return false;
    }

    Choice choices[] = {
        { .label = "Yes", .key = 'y', .description = "" },
        { .label = "No", .key = 'n', .description = "" }
    };

    int selected;
    if (display_choice_prompt(win, title, question, choices, 2, &selected)) {
        *yes_result = (selected == 0);
        return true;
    }

    return false;
}

bool get_validated_input(
    WINDOW* win,
    const char* prompt,
    char* buffer,
    size_t buffer_size,
    int max_attempts
) {
    if (!win || !prompt || !buffer || buffer_size == 0) {
        return false;
    }

    if (max_attempts <= 0) {
        max_attempts = MAX_INPUT_ATTEMPTS;
    }

    int attempts = 0;
    int prompt_line = 2;
    int error_line = 4;

    while (attempts < max_attempts) {
        /* Display prompt */
        wclear(win);
        wattron(win, COLOR_PAIR(COLOR_PROMPT_TEXT));
        mvwprintw(win, prompt_line, 2, "%s: ", prompt);
        wattroff(win, COLOR_PAIR(COLOR_PROMPT_TEXT));
        wrefresh(win);

        /* Get input */
        echo();
        wgetnstr(win, buffer, buffer_size - 1);
        noecho();

        /* Validate (non-empty after trimming) */
        normalize_input(buffer);

        if (strlen(buffer) > 0) {
            clear_prompt_error(win, error_line);
            return true;
        }

        /* Empty input */
        attempts++;
        display_prompt_error(win, error_line,
                           "Input cannot be empty. Please try again.");
        wrefresh(win);
    }

    return false;
}

void display_prompt_error(
    WINDOW* win,
    int line_y,
    const char* message
) {
    if (!win || !message) return;

    wattron(win, COLOR_PAIR(COLOR_PROMPT_ERROR) | A_BOLD);
    mvwprintw(win, line_y, 2, "ERROR: %s", message);
    wattroff(win, COLOR_PAIR(COLOR_PROMPT_ERROR) | A_BOLD);
}

void clear_prompt_error(
    WINDOW* win,
    int line_y
) {
    if (!win) return;

    /* Clear the entire line */
    mvwprintw(win, line_y, 2, "                                                                      ");
}

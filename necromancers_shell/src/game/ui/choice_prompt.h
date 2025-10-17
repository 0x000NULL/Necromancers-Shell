/**
 * @file choice_prompt.h
 * @brief Generic choice prompt system for interactive story events
 *
 * Provides reusable UI components for binary and multi-choice prompts
 * Used by Ashbrook event, Thessara dialogues, trial decisions, etc.
 */

#ifndef CHOICE_PROMPT_H
#define CHOICE_PROMPT_H

#include <stdbool.h>
#include <stddef.h>

/* Forward declaration for ncurses WINDOW */
struct _win_st;
typedef struct _win_st WINDOW;

/**
 * @brief Choice option for prompt display
 */
typedef struct {
    char label[64];          /**< Full label text (e.g., "Harvest souls") */
    char key;                /**< Single-key shortcut (e.g., 'h') */
    char description[128];   /**< Additional context (e.g., "+147 souls, +13% corruption") */
} Choice;

/**
 * @brief Type of choice prompt
 */
typedef enum {
    CHOICE_BINARY,           /**< Two options (yes/no, harvest/spare) */
    CHOICE_MULTI,            /**< Multiple options (3-6 choices) */
    CHOICE_TEXT_INPUT        /**< Free-form text input */
} ChoiceType;

/**
 * @brief Display a choice prompt and get user selection
 *
 * Presents choices to the user with visual formatting and validates input.
 * Allows retry on invalid input up to MAX_ATTEMPTS (default 5).
 *
 * @param win ncurses window to display prompt in
 * @param title Title of the prompt (e.g., "ASHBROOK VILLAGE")
 * @param description Optional description text (can be NULL)
 * @param choices Array of choice options
 * @param choice_count Number of choices in the array
 * @param selected_index Output parameter - index of selected choice (0-based)
 * @return true if valid selection made, false if max attempts exceeded or error
 *
 * @note Window must be created and destroyed by caller
 * @note selected_index is only valid if function returns true
 */
bool display_choice_prompt(
    WINDOW* win,
    const char* title,
    const char* description,
    const Choice* choices,
    size_t choice_count,
    int* selected_index
);

/**
 * @brief Display a simple yes/no prompt
 *
 * Convenience wrapper around display_choice_prompt for binary decisions.
 *
 * @param win ncurses window
 * @param title Prompt title
 * @param question Question text
 * @param yes_result Output parameter - true if user selected yes
 * @return true if valid selection, false on error
 */
bool prompt_yes_no(
    WINDOW* win,
    const char* title,
    const char* question,
    bool* yes_result
);

/**
 * @brief Get validated text input from user
 *
 * Displays a prompt and gets text input with optional validation.
 *
 * @param win ncurses window
 * @param prompt Prompt text to display
 * @param buffer Output buffer for input text
 * @param buffer_size Size of output buffer
 * @param max_attempts Maximum number of retry attempts
 * @return true if valid input received, false if max attempts exceeded
 *
 * @note Input is trimmed and validated for non-empty content
 */
bool get_validated_input(
    WINDOW* win,
    const char* prompt,
    char* buffer,
    size_t buffer_size,
    int max_attempts
);

/**
 * @brief Display an error message in the prompt window
 *
 * Shows error text in red color at specified line.
 *
 * @param win ncurses window
 * @param line_y Y coordinate for error message
 * @param message Error message text
 */
void display_prompt_error(
    WINDOW* win,
    int line_y,
    const char* message
);

/**
 * @brief Clear error message from prompt window
 *
 * Clears the line where error was displayed.
 *
 * @param win ncurses window
 * @param line_y Y coordinate of error message to clear
 */
void clear_prompt_error(
    WINDOW* win,
    int line_y
);

#endif /* CHOICE_PROMPT_H */

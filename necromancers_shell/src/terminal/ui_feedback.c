#include "ui_feedback.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* ANSI color codes */
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_WHITE   "\033[37m"
#define COLOR_GRAY    "\033[90m"

/* Bold colors */
#define COLOR_BOLD_RED     "\033[1;31m"
#define COLOR_BOLD_GREEN   "\033[1;32m"
#define COLOR_BOLD_YELLOW  "\033[1;33m"
#define COLOR_BOLD_BLUE    "\033[1;34m"
#define COLOR_BOLD_MAGENTA "\033[1;35m"

/* UI feedback state */
static struct {
    bool initialized;
    bool color_enabled;
    bool is_tty;
} g_ui_state = {
    .initialized = false,
    .color_enabled = true,
    .is_tty = false
};

bool ui_feedback_init(void) {
    if (g_ui_state.initialized) return true;

    /* Check if stdout is a TTY */
    g_ui_state.is_tty = isatty(STDOUT_FILENO);

    /* Enable colors by default if TTY */
    g_ui_state.color_enabled = g_ui_state.is_tty;

    g_ui_state.initialized = true;
    return true;
}

void ui_feedback_shutdown(void) {
    g_ui_state.initialized = false;
}

void ui_feedback_set_color_enabled(bool enabled) {
    g_ui_state.color_enabled = enabled && g_ui_state.is_tty;
}

bool ui_feedback_is_color_enabled(void) {
    return g_ui_state.color_enabled;
}

void ui_feedback_print_colored(const char* color, const char* prefix,
                              const char* message) {
    if (!message) return;

    if (g_ui_state.color_enabled && color) {
        printf("%s%s%s ", color, prefix ? prefix : "", COLOR_RESET);
        printf("%s\n", message);
    } else {
        printf("%s %s\n", prefix ? prefix : "", message);
    }
    fflush(stdout);
}

void ui_feedback_success(const char* message) {
    ui_feedback_print_colored(COLOR_BOLD_GREEN, "[SUCCESS]", message);
}

void ui_feedback_error(const char* message) {
    ui_feedback_print_colored(COLOR_BOLD_RED, "[ERROR]", message);
}

void ui_feedback_warning(const char* message) {
    ui_feedback_print_colored(COLOR_BOLD_YELLOW, "[WARNING]", message);
}

void ui_feedback_info(const char* message) {
    ui_feedback_print_colored(COLOR_BOLD_BLUE, "[INFO]", message);
}

void ui_feedback_command_result(const CommandResult* result) {
    if (!result) return;

    if (result->success) {
        if (result->output && result->output[0] != '\0') {
            printf("%s", result->output);
            if (result->output[strlen(result->output) - 1] != '\n') {
                printf("\n");
            }
            fflush(stdout);
        }
    } else {
        /* Display error */
        if (result->error_message) {
            ui_feedback_error(result->error_message);
        } else {
            ui_feedback_error(execution_status_string(result->status));
        }
    }
}

void ui_feedback_prompt(const char* prompt) {
    if (!prompt) prompt = "> ";

    if (g_ui_state.color_enabled) {
        printf("%s%s%s", COLOR_BOLD_MAGENTA, prompt, COLOR_RESET);
    } else {
        printf("%s", prompt);
    }
    fflush(stdout);
}

void ui_feedback_autocomplete(char** suggestions, size_t count, int current_index) {
    if (!suggestions || count == 0) return;

    printf("\n");

    if (g_ui_state.color_enabled) {
        printf("%s[Suggestions]%s\n", COLOR_CYAN, COLOR_RESET);
    } else {
        printf("[Suggestions]\n");
    }

    for (size_t i = 0; i < count && i < 20; i++) {  /* Limit to 20 suggestions */
        if ((int)i == current_index) {
            if (g_ui_state.color_enabled) {
                printf("%s> %s%s\n", COLOR_BOLD_GREEN, suggestions[i], COLOR_RESET);
            } else {
                printf("> %s\n", suggestions[i]);
            }
        } else {
            printf("  %s\n", suggestions[i]);
        }
    }

    if (count > 20) {
        printf("  ... and %zu more\n", count - 20);
    }

    fflush(stdout);
}

void ui_feedback_clear_autocomplete(void) {
    /* Move cursor up and clear lines */
    /* This is a simple implementation - a full implementation would track
     * how many lines were printed and clear exactly those */
    if (g_ui_state.is_tty) {
        printf("\033[F");  /* Move cursor up */
        printf("\033[K");  /* Clear line */
        fflush(stdout);
    }
}

void ui_feedback_history_search(const char* query, char** results,
                               size_t count, int current_index) {
    if (!query) return;

    printf("\n");

    if (g_ui_state.color_enabled) {
        printf("%s[History Search: \"%s\"]%s\n", COLOR_YELLOW, query, COLOR_RESET);
    } else {
        printf("[History Search: \"%s\"]\n", query);
    }

    if (count == 0) {
        printf("  No matches found\n");
    } else {
        for (size_t i = 0; i < count && i < 10; i++) {  /* Limit to 10 results */
            if ((int)i == current_index) {
                if (g_ui_state.color_enabled) {
                    printf("%s> %s%s\n", COLOR_BOLD_GREEN, results[i], COLOR_RESET);
                } else {
                    printf("> %s\n", results[i]);
                }
            } else {
                printf("  %s\n", results[i]);
            }
        }

        if (count > 10) {
            printf("  ... and %zu more\n", count - 10);
        }
    }

    fflush(stdout);
}

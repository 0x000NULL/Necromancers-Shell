#include "input_handler.h"
#include "ui_feedback.h"
#include "../commands/parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <termios.h>

/* Input handler internal structure */
struct InputHandler {
    CommandRegistry* registry;
    CommandHistory* history;
    Autocomplete* autocomplete;
    struct termios orig_termios;
    bool raw_mode_enabled;
};

/* Helper: Enable raw mode for character-by-character input */
/* Note: Currently unused but reserved for future Phase 2 line editing */
static bool enable_raw_mode(InputHandler* handler) __attribute__((unused));
static bool enable_raw_mode(InputHandler* handler) {
    if (!isatty(STDIN_FILENO)) return false;

    if (tcgetattr(STDIN_FILENO, &handler->orig_termios) == -1) {
        return false;
    }

    struct termios raw = handler->orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        return false;
    }

    handler->raw_mode_enabled = true;
    return true;
}

/* Helper: Disable raw mode */
static void disable_raw_mode(InputHandler* handler) {
    if (!handler->raw_mode_enabled) return;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &handler->orig_termios);
    handler->raw_mode_enabled = false;
}

InputHandler* input_handler_create(CommandRegistry* registry) {
    if (!registry) return NULL;

    InputHandler* handler = malloc(sizeof(InputHandler));
    if (!handler) return NULL;

    handler->registry = registry;
    handler->history = command_history_create(100);
    handler->autocomplete = autocomplete_create(registry);
    handler->raw_mode_enabled = false;

    if (!handler->history || !handler->autocomplete) {
        input_handler_destroy(handler);
        return NULL;
    }

    /* Try to load history */
    input_handler_load_history(handler);

    return handler;
}

void input_handler_destroy(InputHandler* handler) {
    if (!handler) return;

    /* Save history before destroying */
    input_handler_save_history(handler);

    disable_raw_mode(handler);

    command_history_destroy(handler->history);
    autocomplete_destroy(handler->autocomplete);
    free(handler);
}

bool input_handler_read_line(InputHandler* handler, const char* prompt,
                            char* buffer, size_t buffer_size) {
    if (!handler || !buffer || buffer_size == 0) return false;

    /* For now, use simple line reading with readline-like functionality
     * A full implementation would handle character-by-character input */

    /* Display prompt */
    ui_feedback_prompt(prompt);

    /* Read line */
    if (!fgets(buffer, buffer_size, stdin)) {
        return false;  /* EOF or error */
    }

    /* Remove trailing newline */
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    }

    return true;
}

CommandResult input_handler_read_and_execute(InputHandler* handler, const char* prompt) {
    if (!handler) {
        return command_result_error(EXEC_ERROR_INTERNAL, "Invalid input handler");
    }

    char input[4096];
    if (!input_handler_read_line(handler, prompt, input, sizeof(input))) {
        /* EOF - treat as quit */
        return command_result_exit("EOF received");
    }

    /* Skip empty input */
    if (input[0] == '\0') {
        return command_result_success(NULL);
    }

    /* Add to history */
    command_history_add(handler->history, input);

    /* Execute */
    return input_handler_execute(handler, input);
}

CommandResult input_handler_execute(InputHandler* handler, const char* input) {
    if (!handler || !input) {
        return command_result_error(EXEC_ERROR_INTERNAL, "Invalid parameters");
    }

    /* Parse command */
    ParsedCommand* cmd = NULL;
    ParseResult parse_result = parse_command_string(input, handler->registry, &cmd);

    if (parse_result != PARSE_SUCCESS) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "Parse error: %s",
                parse_error_string(parse_result));
        return command_result_error(EXEC_ERROR_COMMAND_FAILED, error_msg);
    }

    /* Execute command */
    CommandResult result = execute_command(cmd);

    /* Cleanup */
    parsed_command_destroy(cmd);

    return result;
}

CommandHistory* input_handler_get_history(InputHandler* handler) {
    return handler ? handler->history : NULL;
}

Autocomplete* input_handler_get_autocomplete(InputHandler* handler) {
    return handler ? handler->autocomplete : NULL;
}

CommandRegistry* input_handler_get_registry(InputHandler* handler) {
    return handler ? handler->registry : NULL;
}

bool input_handler_save_history(InputHandler* handler) {
    if (!handler || !handler->history) return false;

    char* path = command_history_default_path();
    if (!path) return false;

    bool result = command_history_save(handler->history, path);
    free(path);

    return result;
}

bool input_handler_load_history(InputHandler* handler) {
    if (!handler || !handler->history) return false;

    char* path = command_history_default_path();
    if (!path) return false;

    bool result = command_history_load(handler->history, path);
    free(path);

    return result;
}

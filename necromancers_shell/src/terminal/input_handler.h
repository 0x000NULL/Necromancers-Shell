#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include "../commands/registry.h"
#include "../commands/history.h"
#include "../commands/autocomplete.h"
#include "../commands/executor.h"
#include <stdbool.h>

/**
 * Input Handler
 *
 * High-level input handling that integrates:
 * - Line editing
 * - Command history (up/down arrows)
 * - Autocomplete (Tab)
 * - History search (Ctrl+R)
 * - Command parsing and execution
 *
 * Usage:
 *   InputHandler* handler = input_handler_create(registry);
 *   while (!should_exit) {
 *       CommandResult result = input_handler_read_and_execute(handler);
 *       ui_feedback_command_result(&result);
 *       command_result_destroy(&result);
 *       if (result.should_exit) break;
 *   }
 *   input_handler_destroy(handler);
 */

/* Opaque input handler structure */
typedef struct InputHandler InputHandler;

/**
 * Create input handler
 *
 * @param registry Command registry
 * @return InputHandler pointer or NULL on failure
 */
InputHandler* input_handler_create(CommandRegistry* registry);

/**
 * Destroy input handler
 *
 * @param handler Input handler to destroy
 */
void input_handler_destroy(InputHandler* handler);

/**
 * Read input line with editing support
 * Handles history navigation and autocomplete
 *
 * @param handler Input handler
 * @param prompt Prompt string
 * @param buffer Output buffer
 * @param buffer_size Size of buffer
 * @return true if line was read successfully, false on EOF
 */
bool input_handler_read_line(InputHandler* handler, const char* prompt,
                            char* buffer, size_t buffer_size);

/**
 * Read line and execute command
 * Convenience function that combines read_line, parse, and execute
 *
 * @param handler Input handler
 * @param prompt Prompt string
 * @return CommandResult from execution
 */
CommandResult input_handler_read_and_execute(InputHandler* handler, const char* prompt);

/**
 * Parse and execute command string
 *
 * @param handler Input handler
 * @param input Command string
 * @return CommandResult from execution
 */
CommandResult input_handler_execute(InputHandler* handler, const char* input);

/**
 * Get command history
 *
 * @param handler Input handler
 * @return CommandHistory pointer
 */
CommandHistory* input_handler_get_history(InputHandler* handler);

/**
 * Get autocomplete system
 *
 * @param handler Input handler
 * @return Autocomplete pointer
 */
Autocomplete* input_handler_get_autocomplete(InputHandler* handler);

/**
 * Get command registry
 *
 * @param handler Input handler
 * @return CommandRegistry pointer
 */
CommandRegistry* input_handler_get_registry(InputHandler* handler);

/**
 * Save history to default file
 *
 * @param handler Input handler
 * @return true on success
 */
bool input_handler_save_history(InputHandler* handler);

/**
 * Load history from default file
 *
 * @param handler Input handler
 * @return true on success
 */
bool input_handler_load_history(InputHandler* handler);

#endif /* INPUT_HANDLER_H */

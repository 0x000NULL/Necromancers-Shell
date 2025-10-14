#ifndef UI_FEEDBACK_H
#define UI_FEEDBACK_H

#include "../commands/executor.h"
#include <stdbool.h>

/**
 * UI Feedback System
 *
 * Handles formatted output for command results, errors, and status messages.
 * Provides consistent visual feedback with colors and formatting.
 *
 * Usage:
 *   ui_feedback_init();
 *   ui_feedback_success("Command executed successfully");
 *   ui_feedback_error("Command failed: invalid argument");
 *   ui_feedback_command_result(&result);
 *   ui_feedback_shutdown();
 */

/**
 * Initialize UI feedback system
 *
 * @return true on success
 */
bool ui_feedback_init(void);

/**
 * Shutdown UI feedback system
 */
void ui_feedback_shutdown(void);

/**
 * Display success message
 *
 * @param message Success message
 */
void ui_feedback_success(const char* message);

/**
 * Display error message
 *
 * @param message Error message
 */
void ui_feedback_error(const char* message);

/**
 * Display warning message
 *
 * @param message Warning message
 */
void ui_feedback_warning(const char* message);

/**
 * Display info message
 *
 * @param message Info message
 */
void ui_feedback_info(const char* message);

/**
 * Display command result with appropriate formatting
 *
 * @param result Command result to display
 */
void ui_feedback_command_result(const CommandResult* result);

/**
 * Display command prompt
 *
 * @param prompt Prompt string (e.g., "> " or "necromancer> ")
 */
void ui_feedback_prompt(const char* prompt);

/**
 * Display autocomplete suggestions
 *
 * @param suggestions Array of suggestion strings
 * @param count Number of suggestions
 * @param current_index Currently selected suggestion (-1 for none)
 */
void ui_feedback_autocomplete(char** suggestions, size_t count, int current_index);

/**
 * Clear autocomplete display
 */
void ui_feedback_clear_autocomplete(void);

/**
 * Display history search results
 *
 * @param query Search query
 * @param results Array of matching commands
 * @param count Number of results
 * @param current_index Currently selected result
 */
void ui_feedback_history_search(const char* query, char** results,
                               size_t count, int current_index);

/**
 * Format and print a message with color
 *
 * @param color Color code (ANSI color)
 * @param prefix Prefix (e.g., "[ERROR]", "[INFO]")
 * @param message Message text
 */
void ui_feedback_print_colored(const char* color, const char* prefix,
                              const char* message);

/**
 * Enable/disable color output
 *
 * @param enabled true to enable colors
 */
void ui_feedback_set_color_enabled(bool enabled);

/**
 * Check if color output is enabled
 *
 * @return true if colors are enabled
 */
bool ui_feedback_is_color_enabled(void);

#endif /* UI_FEEDBACK_H */

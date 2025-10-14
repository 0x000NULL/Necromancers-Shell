#ifndef COMMAND_HISTORY_H
#define COMMAND_HISTORY_H

#include <stddef.h>
#include <stdbool.h>

/**
 * Command History
 *
 * Circular buffer for command history with:
 * - Persistent storage to disk
 * - Navigation (up/down arrows)
 * - Search functionality (Ctrl+R)
 * - Duplicate detection
 *
 * Usage:
 *   CommandHistory* history = command_history_create(100);
 *   command_history_add(history, "help");
 *   const char* cmd = command_history_get(history, 0);
 *   command_history_save(history, "~/.necromancers_shell_history");
 *   command_history_destroy(history);
 */

/* Opaque history structure */
typedef struct CommandHistory CommandHistory;

/**
 * Create command history
 *
 * @param capacity Maximum number of commands to store
 * @return CommandHistory pointer or NULL on failure
 */
CommandHistory* command_history_create(size_t capacity);

/**
 * Destroy command history
 *
 * @param history History to destroy
 */
void command_history_destroy(CommandHistory* history);

/**
 * Add command to history
 * Ignores empty commands and consecutive duplicates
 *
 * @param history Command history
 * @param command Command string to add
 * @return true on success
 */
bool command_history_add(CommandHistory* history, const char* command);

/**
 * Get command at index (0 = most recent)
 *
 * @param history Command history
 * @param index Command index
 * @return Command string or NULL if out of bounds
 */
const char* command_history_get(const CommandHistory* history, size_t index);

/**
 * Get number of commands in history
 *
 * @param history Command history
 * @return Number of commands
 */
size_t command_history_size(const CommandHistory* history);

/**
 * Get capacity of history
 *
 * @param history Command history
 * @return Maximum capacity
 */
size_t command_history_capacity(const CommandHistory* history);

/**
 * Clear all history
 *
 * @param history Command history
 */
void command_history_clear(CommandHistory* history);

/**
 * Save history to file
 *
 * @param history Command history
 * @param filepath Path to history file
 * @return true on success
 */
bool command_history_save(const CommandHistory* history, const char* filepath);

/**
 * Load history from file
 *
 * @param history Command history
 * @param filepath Path to history file
 * @return true on success (false if file doesn't exist is okay)
 */
bool command_history_load(CommandHistory* history, const char* filepath);

/**
 * Search history for commands matching pattern
 *
 * @param history Command history
 * @param pattern Search pattern (substring match)
 * @param results Output array of matching commands (allocated by function)
 * @param count Output number of matches
 * @return true on success
 */
bool command_history_search(const CommandHistory* history, const char* pattern,
                           char*** results, size_t* count);

/**
 * Free search results
 *
 * @param results Results array to free
 * @param count Number of results
 */
void command_history_free_search_results(char** results, size_t count);

/**
 * Get default history file path
 * Returns ~/.necromancers_shell_history
 *
 * @return Allocated string with path (caller must free)
 */
char* command_history_default_path(void);

#endif /* COMMAND_HISTORY_H */

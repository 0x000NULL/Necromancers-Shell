#ifndef COMMAND_SYSTEM_H
#define COMMAND_SYSTEM_H

#include "registry.h"
#include "history.h"
#include "autocomplete.h"
#include "executor.h"
#include "../terminal/input_handler.h"
#include <stdbool.h>

/**
 * Command System - Top-level API
 *
 * Provides high-level interface to the entire command system.
 * Manages global registry, history, and input handling.
 *
 * Usage:
 *   command_system_init();
 *   while (running) {
 *       CommandResult result = command_system_process_input("necromancer> ");
 *       if (result.should_exit) break;
 *       command_result_destroy(&result);
 *   }
 *   command_system_shutdown();
 */

/**
 * Initialize command system
 * Creates registry, registers built-in commands, sets up history
 *
 * @return true on success
 */
bool command_system_init(void);

/**
 * Shutdown command system
 * Saves history, cleans up resources
 */
void command_system_shutdown(void);

/**
 * Check if command system is initialized
 *
 * @return true if initialized
 */
bool command_system_is_initialized(void);

/**
 * Process user input (read and execute)
 * Displays prompt, reads input, executes command
 *
 * @param prompt Prompt string to display
 * @return CommandResult from execution
 */
CommandResult command_system_process_input(const char* prompt);

/**
 * Execute command string directly
 *
 * @param input Command string
 * @return CommandResult from execution
 */
CommandResult command_system_execute(const char* input);

/**
 * Get global command registry
 *
 * @return CommandRegistry pointer
 */
CommandRegistry* command_system_get_registry(void);

/**
 * Get global command history
 *
 * @return CommandHistory pointer
 */
CommandHistory* command_system_get_history(void);

/**
 * Get global autocomplete system
 *
 * @return Autocomplete pointer
 */
Autocomplete* command_system_get_autocomplete(void);

/**
 * Get input handler
 *
 * @return InputHandler pointer
 */
InputHandler* command_system_get_input_handler(void);

/**
 * Register a custom command
 *
 * @param info Command info
 * @return true on success
 */
bool command_system_register_command(const CommandInfo* info);

/**
 * Unregister a command
 *
 * @param name Command name
 * @return true if command was found and removed
 */
bool command_system_unregister_command(const char* name);

#endif /* COMMAND_SYSTEM_H */

#ifndef COMMAND_EXECUTOR_H
#define COMMAND_EXECUTOR_H

#include "parser.h"
#include <stdbool.h>

/**
 * Command Executor
 *
 * Executes parsed commands and manages execution results.
 * Handles:
 * - Command execution
 * - Result collection
 * - Error handling
 * - Output formatting
 *
 * Usage:
 *   CommandResult result = execute_command(parsed_cmd);
 *   if (result.success) {
 *       printf("%s\n", result.output);
 *   }
 *   command_result_destroy(&result);
 */

/* Command execution result codes */
typedef enum {
    EXEC_SUCCESS = 0,
    EXEC_ERROR_COMMAND_FAILED,
    EXEC_ERROR_INVALID_COMMAND,
    EXEC_ERROR_PERMISSION_DENIED,
    EXEC_ERROR_NOT_IMPLEMENTED,
    EXEC_ERROR_INTERNAL
} ExecutionStatus;

/* Command execution result */
typedef struct CommandResult {
    ExecutionStatus status;   /* Execution status code */
    bool success;             /* Whether command succeeded */
    char* output;             /* Command output text (may be NULL) */
    char* error_message;      /* Error message (may be NULL) */
    int exit_code;            /* Exit code (0 = success) */
    bool should_exit;         /* Whether game should exit */
} CommandResult;

/**
 * Execute a parsed command
 *
 * @param cmd Parsed command to execute
 * @return CommandResult with execution outcome
 */
CommandResult execute_command(ParsedCommand* cmd);

/**
 * Create success result
 *
 * @param output Output text (copied, may be NULL)
 * @return CommandResult
 */
CommandResult command_result_success(const char* output);

/**
 * Create error result
 *
 * @param status Execution status
 * @param error_message Error message (copied, may be NULL)
 * @return CommandResult
 */
CommandResult command_result_error(ExecutionStatus status, const char* error_message);

/**
 * Create exit result (success + should_exit flag)
 *
 * @param output Output text (copied, may be NULL)
 * @return CommandResult with should_exit = true
 */
CommandResult command_result_exit(const char* output);

/**
 * Destroy command result (free allocated memory)
 *
 * @param result Result to destroy
 */
void command_result_destroy(CommandResult* result);

/**
 * Get human-readable status message
 *
 * @param status ExecutionStatus code
 * @return Status message string
 */
const char* execution_status_string(ExecutionStatus status);

#endif /* COMMAND_EXECUTOR_H */

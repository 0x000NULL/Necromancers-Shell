/* POSIX features (strdup) */
#define _POSIX_C_SOURCE 200809L

#include "executor.h"
#include <stdlib.h>
#include <string.h>

CommandResult execute_command(ParsedCommand* cmd) {
    if (!cmd || !cmd->info || !cmd->info->function) {
        return command_result_error(EXEC_ERROR_INVALID_COMMAND,
                                   "Invalid command or missing function");
    }

    /* Execute the command function */
    CommandResult result = cmd->info->function(cmd);

    return result;
}

CommandResult command_result_success(const char* output) {
    CommandResult result;
    result.status = EXEC_SUCCESS;
    result.success = true;
    result.output = output ? strdup(output) : NULL;
    result.error_message = NULL;
    result.exit_code = 0;
    result.should_exit = false;

    return result;
}

CommandResult command_result_error(ExecutionStatus status, const char* error_message) {
    CommandResult result;
    result.status = status;
    result.success = false;
    result.output = NULL;
    result.error_message = error_message ? strdup(error_message) : NULL;
    result.exit_code = (int)status;
    result.should_exit = false;

    return result;
}

CommandResult command_result_exit(const char* output) {
    CommandResult result;
    result.status = EXEC_SUCCESS;
    result.success = true;
    result.output = output ? strdup(output) : NULL;
    result.error_message = NULL;
    result.exit_code = 0;
    result.should_exit = true;

    return result;
}

void command_result_destroy(CommandResult* result) {
    if (!result) return;

    free(result->output);
    free(result->error_message);

    result->output = NULL;
    result->error_message = NULL;
}

const char* execution_status_string(ExecutionStatus status) {
    switch (status) {
        case EXEC_SUCCESS:
            return "Success";
        case EXEC_ERROR_COMMAND_FAILED:
            return "Command failed";
        case EXEC_ERROR_INVALID_COMMAND:
            return "Invalid command";
        case EXEC_ERROR_PERMISSION_DENIED:
            return "Permission denied";
        case EXEC_ERROR_NOT_IMPLEMENTED:
            return "Not implemented";
        case EXEC_ERROR_INTERNAL:
            return "Internal error";
        default:
            return "Unknown error";
    }
}

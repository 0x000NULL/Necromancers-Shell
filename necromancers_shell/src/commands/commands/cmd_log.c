#include "commands.h"
#include "../../utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CommandResult cmd_log(ParsedCommand* cmd) {
    if (!cmd) {
        return command_result_error(EXEC_ERROR_INVALID_COMMAND, "Invalid command");
    }

    /* Get log level argument */
    const char* level_str = parsed_command_get_arg(cmd, 0);
    if (!level_str) {
        /* Show current log level */
        LogLevel current_level = logger_get_level();
        const char* level_names[] = {
            "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
        };

        char msg[256];
        snprintf(msg, sizeof(msg), "Current log level: %s\n\n"
                "Available levels: trace, debug, info, warn, error, fatal\n"
                "Usage: log <level> [--file <path>]",
                level_names[current_level]);

        return command_result_success(msg);
    }

    /* Parse log level */
    LogLevel new_level;
    if (strcmp(level_str, "trace") == 0) {
        new_level = LOG_LEVEL_TRACE;
    } else if (strcmp(level_str, "debug") == 0) {
        new_level = LOG_LEVEL_DEBUG;
    } else if (strcmp(level_str, "info") == 0) {
        new_level = LOG_LEVEL_INFO;
    } else if (strcmp(level_str, "warn") == 0) {
        new_level = LOG_LEVEL_WARN;
    } else if (strcmp(level_str, "error") == 0) {
        new_level = LOG_LEVEL_ERROR;
    } else if (strcmp(level_str, "fatal") == 0) {
        new_level = LOG_LEVEL_FATAL;
    } else {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg),
                "Invalid log level: %s\n"
                "Valid levels: trace, debug, info, warn, error, fatal",
                level_str);
        return command_result_error(EXEC_ERROR_COMMAND_FAILED, error_msg);
    }

    /* Set log level */
    logger_set_level(new_level);

    /* Check for file flag */
    const ArgumentValue* file_arg = parsed_command_get_flag(cmd, "file");
    if (file_arg && file_arg->type == ARG_TYPE_STRING) {
        /* Reinitialize logger with new file */
        logger_shutdown();
        if (!logger_init(file_arg->value.str_value, new_level)) {
            return command_result_error(EXEC_ERROR_COMMAND_FAILED,
                                       "Failed to change log file");
        }
    }

    char msg[256];
    snprintf(msg, sizeof(msg), "Log level set to: %s", level_str);
    return command_result_success(msg);
}

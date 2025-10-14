/* POSIX features (open_memstream) */
#define _POSIX_C_SOURCE 200809L

#include "commands.h"
#include "../registry.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External reference to global registry (managed by command system) */
extern CommandRegistry* g_command_registry;

CommandResult cmd_help(ParsedCommand* cmd) {
    if (!cmd) {
        return command_result_error(EXEC_ERROR_INVALID_COMMAND, "Invalid command");
    }

    /* Buffer for building help output */
    char* output = NULL;
    size_t output_size = 0;
    FILE* stream = open_memstream(&output, &output_size);
    if (!stream) {
        return command_result_error(EXEC_ERROR_INTERNAL, "Failed to create output buffer");
    }

    /* Check if specific command was requested */
    const char* cmd_name = parsed_command_get_arg(cmd, 0);

    if (cmd_name) {
        /* Show help for specific command */
        const CommandInfo* info = command_registry_get(g_command_registry, cmd_name);
        if (!info) {
            fclose(stream);
            free(output);
            char error_msg[256];
            snprintf(error_msg, sizeof(error_msg), "Unknown command: %s", cmd_name);
            return command_result_error(EXEC_ERROR_COMMAND_FAILED, error_msg);
        }

        fprintf(stream, "\n=== %s ===\n\n", info->name);
        fprintf(stream, "Description: %s\n\n", info->description ? info->description : "No description");
        fprintf(stream, "Usage: %s\n\n", info->usage ? info->usage : info->name);

        if (info->help_text) {
            fprintf(stream, "%s\n\n", info->help_text);
        }

        if (info->flag_count > 0) {
            fprintf(stream, "Options:\n");
            for (size_t i = 0; i < info->flag_count; i++) {
                const FlagDefinition* flag = &info->flags[i];
                fprintf(stream, "  ");
                if (flag->short_name) {
                    fprintf(stream, "-%c, ", flag->short_name);
                }
                fprintf(stream, "--%s", flag->name);

                /* Show type */
                switch (flag->type) {
                    case ARG_TYPE_STRING:
                        fprintf(stream, " <string>");
                        break;
                    case ARG_TYPE_INT:
                        fprintf(stream, " <int>");
                        break;
                    case ARG_TYPE_FLOAT:
                        fprintf(stream, " <float>");
                        break;
                    case ARG_TYPE_BOOL:
                        /* Boolean flags don't need value */
                        break;
                }

                if (flag->required) {
                    fprintf(stream, " (required)");
                }
                fprintf(stream, "\n");

                if (flag->description) {
                    fprintf(stream, "      %s\n", flag->description);
                }
            }
            fprintf(stream, "\n");
        }

        if (info->min_args > 0 || info->max_args > 0) {
            fprintf(stream, "Arguments:\n");
            fprintf(stream, "  Minimum: %zu\n", info->min_args);
            if (info->max_args > 0) {
                fprintf(stream, "  Maximum: %zu\n", info->max_args);
            } else {
                fprintf(stream, "  Maximum: unlimited\n");
            }
        }
    } else {
        /* Show general help with all commands */
        fprintf(stream, "\n=== Necromancer's Shell - Command Help ===\n\n");
        fprintf(stream, "Available commands:\n\n");

        /* Get all command names */
        char** names = NULL;
        size_t count = 0;
        if (command_registry_get_all_names(g_command_registry, &names, &count)) {
            /* Sort names alphabetically for better presentation */
            for (size_t i = 0; i < count - 1; i++) {
                for (size_t j = i + 1; j < count; j++) {
                    if (strcmp(names[i], names[j]) > 0) {
                        char* temp = names[i];
                        names[i] = names[j];
                        names[j] = temp;
                    }
                }
            }

            /* Print each command */
            for (size_t i = 0; i < count; i++) {
                const CommandInfo* info = command_registry_get(g_command_registry, names[i]);
                if (info && !info->hidden) {
                    fprintf(stream, "  %-12s - %s\n", info->name,
                           info->description ? info->description : "No description");
                }
            }

            command_registry_free_names(names, count);
        }

        fprintf(stream, "\nType 'help <command>' for detailed information on a specific command.\n");
    }

    fclose(stream);
    CommandResult result = command_result_success(output);
    free(output);
    return result;
}

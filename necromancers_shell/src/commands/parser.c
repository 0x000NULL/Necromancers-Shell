/* POSIX features (strdup) */
#define _POSIX_C_SOURCE 200809L

#include "parser.h"
#include "../utils/hash_table.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

/* Helper: Check if token is a flag (starts with - or --) */
static bool is_flag(const char* token) {
    return token && token[0] == '-' && token[1] != '\0';
}

/* Helper: Get flag name from token (strips - or --) */
static const char* get_flag_name(const char* token, bool* is_short) {
    if (!token || token[0] != '-') return NULL;

    if (token[1] == '-') {
        /* Long flag: --flag */
        *is_short = false;
        return token + 2;
    } else {
        /* Short flag: -f */
        *is_short = true;
        return token + 1;
    }
}

/* Helper: Find flag definition in CommandInfo */
static const FlagDefinition* find_flag_definition(const CommandInfo* info,
                                                  const char* name,
                                                  bool is_short) {
    if (!info || !name) return NULL;

    for (size_t i = 0; i < info->flag_count; i++) {
        if (is_short) {
            if (name[0] == info->flags[i].short_name && name[1] == '\0') {
                return &info->flags[i];
            }
        } else {
            if (info->flags[i].name && strcmp(name, info->flags[i].name) == 0) {
                return &info->flags[i];
            }
        }
    }

    return NULL;
}

ArgumentValue* argument_value_create(const char* str, ArgumentType type) {
    if (!str) return NULL;

    ArgumentValue* value = malloc(sizeof(ArgumentValue));
    if (!value) return NULL;

    value->type = type;

    switch (type) {
        case ARG_TYPE_STRING:
            value->value.str_value = strdup(str);
            if (!value->value.str_value) {
                free(value);
                return NULL;
            }
            break;

        case ARG_TYPE_INT: {
            char* endptr;
            errno = 0;
            long result = strtol(str, &endptr, 10);
            if (errno != 0 || *endptr != '\0' || endptr == str) {
                free(value);
                return NULL;
            }
            value->value.int_value = (int)result;
            break;
        }

        case ARG_TYPE_FLOAT: {
            char* endptr;
            errno = 0;
            float result = strtof(str, &endptr);
            if (errno != 0 || *endptr != '\0' || endptr == str) {
                free(value);
                return NULL;
            }
            value->value.float_value = result;
            break;
        }

        case ARG_TYPE_BOOL:
            /* Parse boolean: true/false, yes/no, 1/0 */
            if (strcmp(str, "true") == 0 || strcmp(str, "yes") == 0 ||
                strcmp(str, "1") == 0) {
                value->value.bool_value = true;
            } else if (strcmp(str, "false") == 0 || strcmp(str, "no") == 0 ||
                      strcmp(str, "0") == 0) {
                value->value.bool_value = false;
            } else {
                free(value);
                return NULL;
            }
            break;

        default:
            free(value);
            return NULL;
    }

    return value;
}

void argument_value_destroy(ArgumentValue* value) {
    if (!value) return;

    if (value->type == ARG_TYPE_STRING) {
        free(value->value.str_value);
    }

    free(value);
}

/* Callback for destroying ArgumentValue in hash table */
static void destroy_argument_value_callback(const char* key, void* value, void* userdata) {
    (void)key;
    (void)userdata;
    argument_value_destroy((ArgumentValue*)value);
}

ParseResult parse_command(const Token* tokens, size_t token_count,
                         const CommandRegistry* registry,
                         ParsedCommand** output) {
    if (!tokens || token_count == 0 || !registry || !output) {
        return PARSE_ERROR_EMPTY_COMMAND;
    }

    /* First token is command name */
    const char* cmd_name = tokens[0].value;
    const CommandInfo* info = command_registry_get(registry, cmd_name);
    if (!info) {
        return PARSE_ERROR_UNKNOWN_COMMAND;
    }

    /* Allocate ParsedCommand */
    ParsedCommand* cmd = malloc(sizeof(ParsedCommand));
    if (!cmd) return PARSE_ERROR_MEMORY;

    cmd->command_name = strdup(cmd_name);
    cmd->info = info;
    cmd->flags = hash_table_create(16);
    cmd->args = NULL;
    cmd->arg_count = 0;
    cmd->raw_input = NULL;

    if (!cmd->command_name || !cmd->flags) {
        parsed_command_destroy(cmd);
        return PARSE_ERROR_MEMORY;
    }

    /* Allocate temporary args array (max size) */
    size_t args_capacity = token_count;
    cmd->args = malloc(args_capacity * sizeof(char*));
    if (!cmd->args) {
        parsed_command_destroy(cmd);
        return PARSE_ERROR_MEMORY;
    }

    /* Parse tokens */
    for (size_t i = 1; i < token_count; i++) {
        const char* token = tokens[i].value;

        if (is_flag(token)) {
            bool is_short;
            const char* flag_name = get_flag_name(token, &is_short);
            if (!flag_name || *flag_name == '\0') {
                parsed_command_destroy(cmd);
                return PARSE_ERROR_INVALID_FLAG;
            }

            /* Find flag definition */
            const FlagDefinition* flag_def = find_flag_definition(info, flag_name, is_short);
            if (!flag_def) {
                parsed_command_destroy(cmd);
                return PARSE_ERROR_INVALID_FLAG;
            }

            /* Boolean flags don't require a value */
            if (flag_def->type == ARG_TYPE_BOOL) {
                ArgumentValue* value = malloc(sizeof(ArgumentValue));
                if (!value) {
                    parsed_command_destroy(cmd);
                    return PARSE_ERROR_MEMORY;
                }
                value->type = ARG_TYPE_BOOL;
                value->value.bool_value = true;
                hash_table_put(cmd->flags, flag_def->name, value);
                continue;
            }

            /* Get next token as value */
            i++;
            if (i >= token_count) {
                parsed_command_destroy(cmd);
                return PARSE_ERROR_MISSING_FLAG_VALUE;
            }

            const char* flag_value = tokens[i].value;
            ArgumentValue* value = argument_value_create(flag_value, flag_def->type);
            if (!value) {
                parsed_command_destroy(cmd);
                return PARSE_ERROR_INVALID_FLAG_VALUE;
            }

            hash_table_put(cmd->flags, flag_def->name, value);
        } else {
            /* Positional argument */
            cmd->args[cmd->arg_count] = strdup(token);
            if (!cmd->args[cmd->arg_count]) {
                parsed_command_destroy(cmd);
                return PARSE_ERROR_MEMORY;
            }
            cmd->arg_count++;
        }
    }

    /* Validate argument count */
    if (cmd->arg_count < info->min_args) {
        parsed_command_destroy(cmd);
        return PARSE_ERROR_TOO_FEW_ARGS;
    }

    if (info->max_args > 0 && cmd->arg_count > info->max_args) {
        parsed_command_destroy(cmd);
        return PARSE_ERROR_TOO_MANY_ARGS;
    }

    /* Validate required flags */
    for (size_t i = 0; i < info->flag_count; i++) {
        if (info->flags[i].required) {
            if (!hash_table_contains(cmd->flags, info->flags[i].name)) {
                parsed_command_destroy(cmd);
                return PARSE_ERROR_REQUIRED_FLAG_MISSING;
            }
        }
    }

    *output = cmd;
    return PARSE_SUCCESS;
}

ParseResult parse_command_string(const char* input,
                                const CommandRegistry* registry,
                                ParsedCommand** output) {
    if (!input || !registry || !output) {
        return PARSE_ERROR_EMPTY_COMMAND;
    }

    /* Tokenize input */
    Token* tokens = NULL;
    size_t token_count = 0;
    TokenizeResult tok_result = tokenize(input, &tokens, &token_count);

    if (tok_result != TOKENIZE_SUCCESS) {
        return PARSE_ERROR_EMPTY_COMMAND;
    }

    if (token_count == 0) {
        free_tokens(tokens, token_count);
        return PARSE_ERROR_EMPTY_COMMAND;
    }

    /* Parse command */
    ParseResult result = parse_command(tokens, token_count, registry, output);

    /* Store raw input if successful */
    if (result == PARSE_SUCCESS && *output) {
        (*output)->raw_input = strdup(input);
    }

    free_tokens(tokens, token_count);
    return result;
}

void parsed_command_destroy(ParsedCommand* cmd) {
    if (!cmd) return;

    free((void*)cmd->command_name);
    free(cmd->raw_input);

    if (cmd->flags) {
        hash_table_foreach(cmd->flags, destroy_argument_value_callback, NULL);
        hash_table_destroy(cmd->flags);
    }

    if (cmd->args) {
        for (size_t i = 0; i < cmd->arg_count; i++) {
            free(cmd->args[i]);
        }
        free(cmd->args);
    }

    free(cmd);
}

const ArgumentValue* parsed_command_get_flag(const ParsedCommand* cmd,
                                             const char* flag_name) {
    if (!cmd || !flag_name) return NULL;
    return (const ArgumentValue*)hash_table_get(cmd->flags, flag_name);
}

bool parsed_command_has_flag(const ParsedCommand* cmd, const char* flag_name) {
    if (!cmd || !flag_name) return false;
    return hash_table_contains(cmd->flags, flag_name);
}

const char* parsed_command_get_arg(const ParsedCommand* cmd, size_t index) {
    if (!cmd || index >= cmd->arg_count) return NULL;
    return cmd->args[index];
}

const char* parse_error_string(ParseResult result) {
    switch (result) {
        case PARSE_SUCCESS:
            return "Success";
        case PARSE_ERROR_EMPTY_COMMAND:
            return "Empty command";
        case PARSE_ERROR_UNKNOWN_COMMAND:
            return "Unknown command";
        case PARSE_ERROR_INVALID_FLAG:
            return "Invalid flag";
        case PARSE_ERROR_MISSING_FLAG_VALUE:
            return "Missing flag value";
        case PARSE_ERROR_INVALID_FLAG_VALUE:
            return "Invalid flag value";
        case PARSE_ERROR_TOO_FEW_ARGS:
            return "Too few arguments";
        case PARSE_ERROR_TOO_MANY_ARGS:
            return "Too many arguments";
        case PARSE_ERROR_REQUIRED_FLAG_MISSING:
            return "Required flag missing";
        case PARSE_ERROR_MEMORY:
            return "Memory allocation failed";
        default:
            return "Unknown error";
    }
}

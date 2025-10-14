#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

#include "tokenizer.h"
#include "registry.h"
#include <stddef.h>
#include <stdbool.h>

/**
 * Command Parser
 *
 * Parses tokenized command input into structured ParsedCommand.
 * Handles:
 * - Command name extraction
 * - Flag parsing (--flag, -f)
 * - Positional arguments
 * - Type validation against CommandInfo
 *
 * Usage:
 *   ParsedCommand* cmd = NULL;
 *   ParseResult result = parse_command(tokens, count, registry, &cmd);
 *   if (result == PARSE_SUCCESS) {
 *       // Use command...
 *       parsed_command_destroy(cmd);
 *   }
 */

/* Forward declaration */
typedef struct HashTable HashTable;

/* Parsed argument value (variant type) */
typedef struct {
    ArgumentType type;
    union {
        char* str_value;
        int int_value;
        float float_value;
        bool bool_value;
    } value;
} ArgumentValue;

/* Parsed command structure */
typedef struct ParsedCommand {
    const char* command_name;    /* Command name */
    const CommandInfo* info;     /* Command info from registry */
    HashTable* flags;            /* Flag name -> ArgumentValue* */
    char** args;                 /* Positional arguments array */
    size_t arg_count;            /* Number of positional arguments */
    char* raw_input;             /* Original input string */
} ParsedCommand;

/* Parse result codes */
typedef enum {
    PARSE_SUCCESS = 0,
    PARSE_ERROR_EMPTY_COMMAND,
    PARSE_ERROR_UNKNOWN_COMMAND,
    PARSE_ERROR_INVALID_FLAG,
    PARSE_ERROR_MISSING_FLAG_VALUE,
    PARSE_ERROR_INVALID_FLAG_VALUE,
    PARSE_ERROR_TOO_FEW_ARGS,
    PARSE_ERROR_TOO_MANY_ARGS,
    PARSE_ERROR_REQUIRED_FLAG_MISSING,
    PARSE_ERROR_MEMORY
} ParseResult;

/**
 * Parse command from tokens
 *
 * @param tokens Token array
 * @param token_count Number of tokens
 * @param registry Command registry
 * @param output Output parsed command (allocated by function)
 * @return ParseResult indicating success or error type
 */
ParseResult parse_command(const Token* tokens, size_t token_count,
                         const CommandRegistry* registry,
                         ParsedCommand** output);

/**
 * Parse command from raw string (convenience function)
 *
 * @param input Input string
 * @param registry Command registry
 * @param output Output parsed command (allocated by function)
 * @return ParseResult indicating success or error type
 */
ParseResult parse_command_string(const char* input,
                                const CommandRegistry* registry,
                                ParsedCommand** output);

/**
 * Destroy parsed command
 *
 * @param cmd Parsed command to destroy
 */
void parsed_command_destroy(ParsedCommand* cmd);

/**
 * Get flag value from parsed command
 *
 * @param cmd Parsed command
 * @param flag_name Flag name
 * @return ArgumentValue pointer or NULL if not present
 */
const ArgumentValue* parsed_command_get_flag(const ParsedCommand* cmd,
                                             const char* flag_name);

/**
 * Check if flag is present
 *
 * @param cmd Parsed command
 * @param flag_name Flag name
 * @return true if flag is present
 */
bool parsed_command_has_flag(const ParsedCommand* cmd, const char* flag_name);

/**
 * Get positional argument
 *
 * @param cmd Parsed command
 * @param index Argument index (0-based)
 * @return Argument string or NULL if out of bounds
 */
const char* parsed_command_get_arg(const ParsedCommand* cmd, size_t index);

/**
 * Get human-readable error message
 *
 * @param result ParseResult code
 * @return Error message string
 */
const char* parse_error_string(ParseResult result);

/**
 * Create ArgumentValue from string based on type
 *
 * @param str String to parse
 * @param type Target type
 * @return ArgumentValue pointer or NULL on error
 */
ArgumentValue* argument_value_create(const char* str, ArgumentType type);

/**
 * Destroy ArgumentValue
 *
 * @param value ArgumentValue to destroy
 */
void argument_value_destroy(ArgumentValue* value);

#endif /* COMMAND_PARSER_H */

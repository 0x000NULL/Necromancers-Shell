#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stddef.h>
#include <stdbool.h>

/**
 * Command Tokenizer
 *
 * Splits input strings into tokens, handling:
 * - Quoted strings (single and double quotes)
 * - Escape sequences (\n, \t, \\, \", \')
 * - Whitespace separation
 * - Empty token handling
 *
 * Usage:
 *   Token* tokens = NULL;
 *   size_t count = 0;
 *   TokenizeResult result = tokenize("echo \"hello world\"", &tokens, &count);
 *   if (result == TOKENIZE_SUCCESS) {
 *       // Use tokens...
 *       free_tokens(tokens, count);
 *   }
 */

/* Token structure - represents a single parsed token */
typedef struct {
    char* value;      /* Token string (dynamically allocated) */
    size_t length;    /* Length of token */
    bool is_quoted;   /* Whether token was quoted */
} Token;

/* Tokenization result codes */
typedef enum {
    TOKENIZE_SUCCESS = 0,
    TOKENIZE_ERROR_MEMORY,
    TOKENIZE_ERROR_UNCLOSED_QUOTE,
    TOKENIZE_ERROR_INVALID_ESCAPE,
    TOKENIZE_ERROR_EMPTY_INPUT
} TokenizeResult;

/**
 * Tokenize an input string
 *
 * @param input Input string to tokenize
 * @param tokens Output array of tokens (allocated by function)
 * @param count Output number of tokens
 * @return TokenizeResult indicating success or error type
 */
TokenizeResult tokenize(const char* input, Token** tokens, size_t* count);

/**
 * Free tokens array
 *
 * @param tokens Tokens array to free
 * @param count Number of tokens
 */
void free_tokens(Token* tokens, size_t count);

/**
 * Get human-readable error message
 *
 * @param result TokenizeResult code
 * @return Error message string
 */
const char* tokenize_error_string(TokenizeResult result);

#endif /* TOKENIZER_H */

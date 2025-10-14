/* POSIX features (strdup) */
#define _POSIX_C_SOURCE 200809L

#include "tokenizer.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Tokenizer state machine states */
typedef enum {
    STATE_INITIAL,
    STATE_IN_TOKEN,
    STATE_IN_SINGLE_QUOTE,
    STATE_IN_DOUBLE_QUOTE,
    STATE_ESCAPE,
    STATE_ESCAPE_IN_DOUBLE_QUOTE
} TokenizerState;

/* Dynamic string builder for token construction */
typedef struct {
    char* buffer;
    size_t length;
    size_t capacity;
} StringBuilder;

/* Initialize string builder */
static StringBuilder* sb_create(void) {
    StringBuilder* sb = malloc(sizeof(StringBuilder));
    if (!sb) return NULL;

    sb->capacity = 32;
    sb->buffer = malloc(sb->capacity);
    if (!sb->buffer) {
        free(sb);
        return NULL;
    }

    sb->buffer[0] = '\0';
    sb->length = 0;
    return sb;
}

/* Append character to string builder */
static bool sb_append(StringBuilder* sb, char c) {
    if (sb->length + 2 > sb->capacity) {
        size_t new_capacity = sb->capacity * 2;
        char* new_buffer = realloc(sb->buffer, new_capacity);
        if (!new_buffer) return false;

        sb->buffer = new_buffer;
        sb->capacity = new_capacity;
    }

    sb->buffer[sb->length++] = c;
    sb->buffer[sb->length] = '\0';
    return true;
}

/* Free string builder (returns buffer ownership to caller) */
/* Note: Currently unused but may be needed for future optimizations */
__attribute__((unused))
static char* sb_finalize(StringBuilder* sb) {
    char* result = sb->buffer;
    free(sb);
    return result;
}

/* Destroy string builder and its buffer */
static void sb_destroy(StringBuilder* sb) {
    if (sb) {
        free(sb->buffer);
        free(sb);
    }
}

/* Process escape sequence and return the actual character */
static char process_escape(char c) {
    switch (c) {
        case 'n':  return '\n';
        case 't':  return '\t';
        case 'r':  return '\r';
        case '\\': return '\\';
        case '"':  return '"';
        case '\'': return '\'';
        case '0':  return '\0';
        default:   return c; /* Unknown escapes pass through */
    }
}

/* Dynamic token array growth */
static bool add_token(Token** tokens, size_t* count, size_t* capacity,
                     const char* value, size_t length, bool is_quoted) {
    if (*count >= *capacity) {
        size_t new_capacity = (*capacity == 0) ? 4 : (*capacity * 2);
        Token* new_tokens = realloc(*tokens, new_capacity * sizeof(Token));
        if (!new_tokens) return false;

        *tokens = new_tokens;
        *capacity = new_capacity;
    }

    Token* token = &(*tokens)[*count];
    token->value = strdup(value);
    if (!token->value) return false;

    token->length = length;
    token->is_quoted = is_quoted;
    (*count)++;

    return true;
}

TokenizeResult tokenize(const char* input, Token** tokens, size_t* count) {
    if (!input || !tokens || !count) {
        return TOKENIZE_ERROR_EMPTY_INPUT;
    }

    /* Skip leading whitespace to check for empty input */
    const char* p = input;
    while (*p && isspace(*p)) p++;
    if (*p == '\0') {
        *tokens = NULL;
        *count = 0;
        return TOKENIZE_SUCCESS;
    }

    TokenizerState state = STATE_INITIAL;
    StringBuilder* sb = sb_create();
    if (!sb) return TOKENIZE_ERROR_MEMORY;

    *tokens = NULL;
    *count = 0;
    size_t capacity = 0;
    bool is_quoted = false;

    for (const char* c = input; *c != '\0'; c++) {
        switch (state) {
            case STATE_INITIAL:
                if (isspace(*c)) {
                    /* Skip whitespace */
                    continue;
                } else if (*c == '"') {
                    state = STATE_IN_DOUBLE_QUOTE;
                    is_quoted = true;
                } else if (*c == '\'') {
                    state = STATE_IN_SINGLE_QUOTE;
                    is_quoted = true;
                } else if (*c == '\\') {
                    state = STATE_ESCAPE;
                    is_quoted = false;
                } else {
                    if (!sb_append(sb, *c)) {
                        sb_destroy(sb);
                        return TOKENIZE_ERROR_MEMORY;
                    }
                    state = STATE_IN_TOKEN;
                    is_quoted = false;
                }
                break;

            case STATE_IN_TOKEN:
                if (isspace(*c)) {
                    /* End of token */
                    if (!add_token(tokens, count, &capacity,
                                 sb->buffer, sb->length, is_quoted)) {
                        sb_destroy(sb);
                        free_tokens(*tokens, *count);
                        return TOKENIZE_ERROR_MEMORY;
                    }
                    sb_destroy(sb);
                    sb = sb_create();
                    if (!sb) {
                        free_tokens(*tokens, *count);
                        return TOKENIZE_ERROR_MEMORY;
                    }
                    state = STATE_INITIAL;
                } else if (*c == '"') {
                    state = STATE_IN_DOUBLE_QUOTE;
                    is_quoted = true;
                } else if (*c == '\'') {
                    state = STATE_IN_SINGLE_QUOTE;
                    is_quoted = true;
                } else if (*c == '\\') {
                    state = STATE_ESCAPE;
                } else {
                    if (!sb_append(sb, *c)) {
                        sb_destroy(sb);
                        free_tokens(*tokens, *count);
                        return TOKENIZE_ERROR_MEMORY;
                    }
                }
                break;

            case STATE_IN_SINGLE_QUOTE:
                if (*c == '\'') {
                    state = STATE_IN_TOKEN;
                } else {
                    /* Single quotes: no escape processing */
                    if (!sb_append(sb, *c)) {
                        sb_destroy(sb);
                        free_tokens(*tokens, *count);
                        return TOKENIZE_ERROR_MEMORY;
                    }
                }
                break;

            case STATE_IN_DOUBLE_QUOTE:
                if (*c == '"') {
                    state = STATE_IN_TOKEN;
                } else if (*c == '\\') {
                    state = STATE_ESCAPE_IN_DOUBLE_QUOTE;
                } else {
                    if (!sb_append(sb, *c)) {
                        sb_destroy(sb);
                        free_tokens(*tokens, *count);
                        return TOKENIZE_ERROR_MEMORY;
                    }
                }
                break;

            case STATE_ESCAPE:
                {
                    char escaped = process_escape(*c);
                    if (!sb_append(sb, escaped)) {
                        sb_destroy(sb);
                        free_tokens(*tokens, *count);
                        return TOKENIZE_ERROR_MEMORY;
                    }
                    state = STATE_IN_TOKEN;
                }
                break;

            case STATE_ESCAPE_IN_DOUBLE_QUOTE:
                {
                    char escaped = process_escape(*c);
                    if (!sb_append(sb, escaped)) {
                        sb_destroy(sb);
                        free_tokens(*tokens, *count);
                        return TOKENIZE_ERROR_MEMORY;
                    }
                    state = STATE_IN_DOUBLE_QUOTE;
                }
                break;
        }
    }

    /* Handle final state */
    if (state == STATE_IN_SINGLE_QUOTE || state == STATE_IN_DOUBLE_QUOTE) {
        sb_destroy(sb);
        free_tokens(*tokens, *count);
        return TOKENIZE_ERROR_UNCLOSED_QUOTE;
    }

    if (state == STATE_ESCAPE || state == STATE_ESCAPE_IN_DOUBLE_QUOTE) {
        sb_destroy(sb);
        free_tokens(*tokens, *count);
        return TOKENIZE_ERROR_INVALID_ESCAPE;
    }

    /* Add final token if we have one */
    if (sb->length > 0) {
        if (!add_token(tokens, count, &capacity,
                     sb->buffer, sb->length, is_quoted)) {
            sb_destroy(sb);
            free_tokens(*tokens, *count);
            return TOKENIZE_ERROR_MEMORY;
        }
    }

    sb_destroy(sb);
    return TOKENIZE_SUCCESS;
}

void free_tokens(Token* tokens, size_t count) {
    if (!tokens) return;

    for (size_t i = 0; i < count; i++) {
        free(tokens[i].value);
    }
    free(tokens);
}

const char* tokenize_error_string(TokenizeResult result) {
    switch (result) {
        case TOKENIZE_SUCCESS:
            return "Success";
        case TOKENIZE_ERROR_MEMORY:
            return "Memory allocation failed";
        case TOKENIZE_ERROR_UNCLOSED_QUOTE:
            return "Unclosed quote";
        case TOKENIZE_ERROR_INVALID_ESCAPE:
            return "Invalid escape sequence";
        case TOKENIZE_ERROR_EMPTY_INPUT:
            return "Empty input";
        default:
            return "Unknown error";
    }
}

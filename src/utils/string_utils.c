#define _POSIX_C_SOURCE 200809L

#include "utils/string_utils.h"
#include "utils/logger.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>

/* String Builder */
#define DEFAULT_BUILDER_CAPACITY 256

struct StringBuilder {
    char* buffer;
    size_t length;
    size_t capacity;
};

/* Safe string copy */
size_t str_safe_copy(char* dst, size_t dst_size, const char* src) {
    if (!dst || dst_size == 0) return 0;
    if (!src) {
        dst[0] = '\0';
        return 0;
    }

    size_t i;
    for (i = 0; i < dst_size - 1 && src[i] != '\0'; i++) {
        dst[i] = src[i];
    }
    dst[i] = '\0';

    return i;
}

/* Safe string concatenation */
size_t str_safe_concat(char* dst, size_t dst_size, const char* src) {
    if (!dst || dst_size == 0) return 0;
    if (!src) return strlen(dst);

    size_t dst_len = strlen(dst);
    if (dst_len >= dst_size - 1) return dst_len;

    size_t remaining = dst_size - dst_len - 1;
    size_t i;
    for (i = 0; i < remaining && src[i] != '\0'; i++) {
        dst[dst_len + i] = src[i];
    }
    dst[dst_len + i] = '\0';

    return dst_len + i;
}

/* Get string length */
size_t str_length(const char* str) {
    return str ? strlen(str) : 0;
}

/* Compare strings */
int str_compare(const char* s1, const char* s2) {
    if (!s1 && !s2) return 0;
    if (!s1) return -1;
    if (!s2) return 1;
    return strcmp(s1, s2);
}

/* Compare strings ignoring case */
int str_compare_ignore_case(const char* s1, const char* s2) {
    if (!s1 && !s2) return 0;
    if (!s1) return -1;
    if (!s2) return 1;

    while (*s1 && *s2) {
        int diff = tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
        if (diff != 0) return diff;
        s1++;
        s2++;
    }

    return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
}

/* Check equality */
bool str_equals(const char* s1, const char* s2) {
    return str_compare(s1, s2) == 0;
}

/* Check equality ignoring case */
bool str_equals_ignore_case(const char* s1, const char* s2) {
    return str_compare_ignore_case(s1, s2) == 0;
}

/* Check if starts with prefix */
bool str_starts_with(const char* str, const char* prefix) {
    if (!str || !prefix) return false;

    while (*prefix) {
        if (*str != *prefix) return false;
        str++;
        prefix++;
    }

    return true;
}

/* Check if ends with suffix */
bool str_ends_with(const char* str, const char* suffix) {
    if (!str || !suffix) return false;

    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);

    if (suffix_len > str_len) return false;

    return strcmp(str + str_len - suffix_len, suffix) == 0;
}

/* Trim whitespace from start */
char* str_trim_left(char* str) {
    if (!str) return NULL;

    char* start = str;
    while (isspace((unsigned char)*start)) {
        start++;
    }

    /* Shift string to beginning if needed */
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }

    return str;
}

/* Trim whitespace from end */
char* str_trim_right(char* str) {
    if (!str) return NULL;

    size_t len = strlen(str);
    while (len > 0 && isspace((unsigned char)str[len - 1])) {
        len--;
    }
    str[len] = '\0';

    return str;
}

/* Trim whitespace from both ends */
char* str_trim(char* str) {
    if (!str) return NULL;

    /* Trim left */
    char* start = str;
    while (isspace((unsigned char)*start)) {
        start++;
    }

    /* Trim right */
    size_t len = strlen(start);
    char* end = start + len - 1;
    while (end > start && isspace((unsigned char)*end)) {
        end--;
    }
    *(end + 1) = '\0';

    /* Shift to beginning if needed */
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }

    return str;
}

/* Convert to lowercase */
char* str_to_lower(char* str) {
    if (!str) return NULL;

    for (char* p = str; *p; p++) {
        *p = tolower((unsigned char)*p);
    }

    return str;
}

/* Convert to uppercase */
char* str_to_upper(char* str) {
    if (!str) return NULL;

    for (char* p = str; *p; p++) {
        *p = toupper((unsigned char)*p);
    }

    return str;
}

/* Duplicate string */
char* str_duplicate(const char* str) {
    if (!str) return NULL;
    return strdup(str);
}

/* Find character */
char* str_find_char(const char* str, char ch) {
    if (!str) return NULL;
    return strchr(str, ch);
}

/* Find character (last occurrence) */
char* str_find_char_last(const char* str, char ch) {
    if (!str) return NULL;
    return strrchr(str, ch);
}

/* Find substring */
char* str_find_substring(const char* str, const char* substr) {
    if (!str || !substr) return NULL;
    return strstr(str, substr);
}

/* Check if empty or whitespace */
bool str_is_empty_or_whitespace(const char* str) {
    if (!str || *str == '\0') return true;

    while (*str) {
        if (!isspace((unsigned char)*str)) return false;
        str++;
    }

    return true;
}

/* Check if all digits */
bool str_is_digits(const char* str) {
    if (!str || *str == '\0') return false;

    while (*str) {
        if (!isdigit((unsigned char)*str)) return false;
        str++;
    }

    return true;
}

/* Check if alphanumeric */
bool str_is_alnum(const char* str) {
    if (!str || *str == '\0') return false;

    while (*str) {
        if (!isalnum((unsigned char)*str)) return false;
        str++;
    }

    return true;
}

/* Split string */
size_t str_split(char* str, char delimiter, char** tokens, size_t max_tokens) {
    if (!str || !tokens || max_tokens == 0) return 0;

    size_t count = 0;
    char* start = str;
    char* current = str;

    while (*current && count < max_tokens) {
        if (*current == delimiter) {
            *current = '\0';
            tokens[count++] = start;
            start = current + 1;
        }
        current++;
    }

    /* Add last token if there's room */
    if (count < max_tokens && *start) {
        tokens[count++] = start;
    }

    return count;
}

/* String Builder Implementation */

StringBuilder* str_builder_create(size_t initial_capacity) {
    StringBuilder* builder = malloc(sizeof(StringBuilder));
    if (!builder) {
        LOG_ERROR("Failed to allocate string builder");
        return NULL;
    }

    if (initial_capacity == 0) {
        initial_capacity = DEFAULT_BUILDER_CAPACITY;
    }

    builder->buffer = malloc(initial_capacity);
    if (!builder->buffer) {
        LOG_ERROR("Failed to allocate string builder buffer");
        free(builder);
        return NULL;
    }

    builder->buffer[0] = '\0';
    builder->length = 0;
    builder->capacity = initial_capacity;

    LOG_DEBUG("Created string builder with capacity %zu", initial_capacity);
    return builder;
}

void str_builder_destroy(StringBuilder* builder) {
    if (!builder) return;

    free(builder->buffer);
    free(builder);

    LOG_DEBUG("Destroyed string builder");
}

static bool str_builder_ensure_capacity(StringBuilder* builder, size_t required) {
    if (required <= builder->capacity) return true;

    /* Grow by 1.5x or required, whichever is larger */
    size_t new_capacity = builder->capacity * 3 / 2;
    if (new_capacity < required) {
        new_capacity = required;
    }

    char* new_buffer = realloc(builder->buffer, new_capacity);
    if (!new_buffer) {
        LOG_ERROR("Failed to grow string builder buffer");
        return false;
    }

    builder->buffer = new_buffer;
    builder->capacity = new_capacity;

    LOG_DEBUG("Grew string builder to capacity %zu", new_capacity);
    return true;
}

bool str_builder_append(StringBuilder* builder, const char* str) {
    if (!builder) return false;
    if (!str) return true;

    size_t str_len = strlen(str);
    size_t required = builder->length + str_len + 1;

    if (!str_builder_ensure_capacity(builder, required)) {
        return false;
    }

    memcpy(builder->buffer + builder->length, str, str_len + 1);
    builder->length += str_len;

    return true;
}

bool str_builder_append_char(StringBuilder* builder, char ch) {
    if (!builder) return false;

    size_t required = builder->length + 2;
    if (!str_builder_ensure_capacity(builder, required)) {
        return false;
    }

    builder->buffer[builder->length++] = ch;
    builder->buffer[builder->length] = '\0';

    return true;
}

bool str_builder_append_format(StringBuilder* builder, const char* fmt, ...) {
    if (!builder || !fmt) return false;

    va_list args1, args2;
    va_start(args1, fmt);
    va_copy(args2, args1);

    /* Calculate required size */
    int needed = vsnprintf(NULL, 0, fmt, args1);
    va_end(args1);

    if (needed < 0) {
        va_end(args2);
        return false;
    }

    size_t required = builder->length + needed + 1;
    if (!str_builder_ensure_capacity(builder, required)) {
        va_end(args2);
        return false;
    }

    /* Format into buffer */
    vsnprintf(builder->buffer + builder->length, needed + 1, fmt, args2);
    va_end(args2);

    builder->length += needed;

    return true;
}

const char* str_builder_get(const StringBuilder* builder) {
    return builder ? builder->buffer : NULL;
}

size_t str_builder_length(const StringBuilder* builder) {
    return builder ? builder->length : 0;
}

void str_builder_clear(StringBuilder* builder) {
    if (!builder) return;

    builder->buffer[0] = '\0';
    builder->length = 0;
}

char* str_builder_extract(StringBuilder* builder) {
    if (!builder || builder->length == 0) return NULL;

    char* result = builder->buffer;
    builder->buffer = malloc(DEFAULT_BUILDER_CAPACITY);
    if (!builder->buffer) {
        /* Restore old buffer on allocation failure */
        builder->buffer = result;
        return NULL;
    }

    builder->buffer[0] = '\0';
    builder->length = 0;
    builder->capacity = DEFAULT_BUILDER_CAPACITY;

    return result;
}

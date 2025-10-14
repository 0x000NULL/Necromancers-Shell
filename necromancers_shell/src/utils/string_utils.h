#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <stddef.h>
#include <stdbool.h>

/**
 * String Utilities
 *
 * Safe string operations and utilities for text processing.
 * All functions handle NULL pointers gracefully.
 *
 * Usage:
 *   char buffer[64];
 *   str_safe_copy(buffer, sizeof(buffer), "Hello");
 *   str_trim(buffer);
 *   if (str_equals_ignore_case(buffer, "hello")) { ... }
 */

/**
 * Safe string copy with bounds checking
 * Always null-terminates destination
 *
 * @param dst Destination buffer
 * @param dst_size Size of destination buffer
 * @param src Source string
 * @return Number of characters copied (excluding null terminator)
 */
size_t str_safe_copy(char* dst, size_t dst_size, const char* src);

/**
 * Safe string concatenation with bounds checking
 * Always null-terminates destination
 *
 * @param dst Destination buffer
 * @param dst_size Size of destination buffer
 * @param src Source string
 * @return Number of characters in result (excluding null terminator)
 */
size_t str_safe_concat(char* dst, size_t dst_size, const char* src);

/**
 * Get string length (NULL-safe)
 *
 * @param str String
 * @return Length or 0 if NULL
 */
size_t str_length(const char* str);

/**
 * Compare two strings (NULL-safe)
 *
 * @param s1 First string
 * @param s2 Second string
 * @return 0 if equal, <0 if s1 < s2, >0 if s1 > s2
 */
int str_compare(const char* s1, const char* s2);

/**
 * Compare strings ignoring case (NULL-safe)
 *
 * @param s1 First string
 * @param s2 Second string
 * @return 0 if equal, <0 if s1 < s2, >0 if s1 > s2
 */
int str_compare_ignore_case(const char* s1, const char* s2);

/**
 * Check if strings are equal (NULL-safe)
 *
 * @param s1 First string
 * @param s2 Second string
 * @return true if equal
 */
bool str_equals(const char* s1, const char* s2);

/**
 * Check if strings are equal, ignoring case (NULL-safe)
 *
 * @param s1 First string
 * @param s2 Second string
 * @return true if equal
 */
bool str_equals_ignore_case(const char* s1, const char* s2);

/**
 * Check if string starts with prefix (NULL-safe)
 *
 * @param str String
 * @param prefix Prefix to check
 * @return true if str starts with prefix
 */
bool str_starts_with(const char* str, const char* prefix);

/**
 * Check if string ends with suffix (NULL-safe)
 *
 * @param str String
 * @param suffix Suffix to check
 * @return true if str ends with suffix
 */
bool str_ends_with(const char* str, const char* suffix);

/**
 * Trim whitespace from start and end (in-place)
 * Modifies the string in-place
 *
 * @param str String to trim
 * @return Pointer to trimmed string (same as input)
 */
char* str_trim(char* str);

/**
 * Trim whitespace from start (in-place)
 *
 * @param str String to trim
 * @return Pointer to trimmed string (same as input)
 */
char* str_trim_left(char* str);

/**
 * Trim whitespace from end (in-place)
 *
 * @param str String to trim
 * @return Pointer to trimmed string (same as input)
 */
char* str_trim_right(char* str);

/**
 * Convert string to lowercase (in-place)
 *
 * @param str String to convert
 * @return Pointer to converted string (same as input)
 */
char* str_to_lower(char* str);

/**
 * Convert string to uppercase (in-place)
 *
 * @param str String to convert
 * @return Pointer to converted string (same as input)
 */
char* str_to_upper(char* str);

/**
 * Duplicate a string (NULL-safe)
 * Caller must free the returned string
 *
 * @param str String to duplicate
 * @return Duplicated string or NULL on failure
 */
char* str_duplicate(const char* str);

/**
 * Find first occurrence of character
 *
 * @param str String to search
 * @param ch Character to find
 * @return Pointer to first occurrence or NULL if not found
 */
char* str_find_char(const char* str, char ch);

/**
 * Find last occurrence of character
 *
 * @param str String to search
 * @param ch Character to find
 * @return Pointer to last occurrence or NULL if not found
 */
char* str_find_char_last(const char* str, char ch);

/**
 * Find substring
 *
 * @param str String to search
 * @param substr Substring to find
 * @return Pointer to first occurrence or NULL if not found
 */
char* str_find_substring(const char* str, const char* substr);

/**
 * Check if string contains only whitespace
 *
 * @param str String to check
 * @return true if empty or only whitespace
 */
bool str_is_empty_or_whitespace(const char* str);

/**
 * Check if string contains only digits
 *
 * @param str String to check
 * @return true if contains only digits
 */
bool str_is_digits(const char* str);

/**
 * Check if string contains only alphanumeric characters
 *
 * @param str String to check
 * @return true if contains only letters and digits
 */
bool str_is_alnum(const char* str);

/**
 * Split string by delimiter
 * Modifies the original string (replaces delimiters with null terminators)
 *
 * @param str String to split (will be modified)
 * @param delimiter Delimiter character
 * @param tokens Array to store token pointers
 * @param max_tokens Maximum number of tokens
 * @return Number of tokens found
 */
size_t str_split(char* str, char delimiter, char** tokens, size_t max_tokens);

/* String Builder - Dynamic string construction */

typedef struct StringBuilder StringBuilder;

/**
 * Create a string builder
 *
 * @param initial_capacity Initial capacity (0 for default)
 * @return StringBuilder pointer or NULL on failure
 */
StringBuilder* str_builder_create(size_t initial_capacity);

/**
 * Destroy a string builder
 *
 * @param builder String builder
 */
void str_builder_destroy(StringBuilder* builder);

/**
 * Append string to builder
 *
 * @param builder String builder
 * @param str String to append
 * @return true on success
 */
bool str_builder_append(StringBuilder* builder, const char* str);

/**
 * Append character to builder
 *
 * @param builder String builder
 * @param ch Character to append
 * @return true on success
 */
bool str_builder_append_char(StringBuilder* builder, char ch);

/**
 * Append formatted string to builder
 *
 * @param builder String builder
 * @param fmt Format string
 * @param ... Format arguments
 * @return true on success
 */
bool str_builder_append_format(StringBuilder* builder, const char* fmt, ...)
    __attribute__((format(printf, 2, 3)));

/**
 * Get string from builder (does not transfer ownership)
 * The returned string is valid until builder is modified or destroyed
 *
 * @param builder String builder
 * @return String pointer or NULL
 */
const char* str_builder_get(const StringBuilder* builder);

/**
 * Get string length
 *
 * @param builder String builder
 * @return Current length
 */
size_t str_builder_length(const StringBuilder* builder);

/**
 * Clear the builder
 *
 * @param builder String builder
 */
void str_builder_clear(StringBuilder* builder);

/**
 * Extract string from builder (transfers ownership)
 * Builder is cleared after extraction
 * Caller must free the returned string
 *
 * @param builder String builder
 * @return Extracted string or NULL
 */
char* str_builder_extract(StringBuilder* builder);

#endif /* STRING_UTILS_H */

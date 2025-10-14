/**
 * String Utilities Tests
 */

#include "utils/string_utils.h"
#include "utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Test results */
static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) \
    printf("Running test: %s\n", #name); \
    tests_run++; \
    if (test_##name()) { \
        tests_passed++; \
        printf("  ✓ PASSED\n"); \
    } else { \
        printf("  ✗ FAILED\n"); \
    }

/* Test: Safe copy */
static bool test_safe_copy(void) {
    char buffer[16];

    /* Normal copy */
    str_safe_copy(buffer, sizeof(buffer), "Hello");
    if (strcmp(buffer, "Hello") != 0) return false;

    /* Truncation */
    str_safe_copy(buffer, sizeof(buffer), "This is a very long string");
    if (strlen(buffer) != 15) return false;  /* 16 - 1 for null */
    if (buffer[15] != '\0') return false;

    /* NULL source */
    str_safe_copy(buffer, sizeof(buffer), NULL);
    if (buffer[0] != '\0') return false;

    return true;
}

/* Test: Safe concat */
static bool test_safe_concat(void) {
    char buffer[16] = "Hello";

    /* Normal concat */
    str_safe_concat(buffer, sizeof(buffer), " World");
    if (strcmp(buffer, "Hello World") != 0) return false;

    /* Truncation */
    str_safe_concat(buffer, sizeof(buffer), " More");
    if (strlen(buffer) != 15) return false;

    /* NULL source */
    size_t len = strlen(buffer);
    str_safe_concat(buffer, sizeof(buffer), NULL);
    if (strlen(buffer) != len) return false;

    return true;
}

/* Test: String comparison */
static bool test_comparison(void) {
    /* Equals */
    if (!str_equals("test", "test")) return false;
    if (str_equals("test", "TEST")) return false;
    if (str_equals("test", "other")) return false;

    /* Equals ignore case */
    if (!str_equals_ignore_case("test", "TEST")) return false;
    if (!str_equals_ignore_case("Hello", "hello")) return false;
    if (str_equals_ignore_case("test", "other")) return false;

    /* Compare */
    if (str_compare("abc", "abc") != 0) return false;
    if (str_compare("abc", "xyz") >= 0) return false;
    if (str_compare("xyz", "abc") <= 0) return false;

    return true;
}

/* Test: Starts/ends with */
static bool test_prefix_suffix(void) {
    const char* str = "hello_world.txt";

    /* Starts with */
    if (!str_starts_with(str, "hello")) return false;
    if (str_starts_with(str, "world")) return false;

    /* Ends with */
    if (!str_ends_with(str, ".txt")) return false;
    if (!str_ends_with(str, "txt")) return false;
    if (str_ends_with(str, "hello")) return false;

    return true;
}

/* Test: Trim operations */
static bool test_trim(void) {
    char buffer[32];

    /* Trim both */
    strcpy(buffer, "  hello  ");
    str_trim(buffer);
    if (strcmp(buffer, "hello") != 0) return false;

    /* Trim left */
    strcpy(buffer, "  hello");
    str_trim_left(buffer);
    if (strcmp(buffer, "hello") != 0) return false;

    /* Trim right */
    strcpy(buffer, "hello  ");
    str_trim_right(buffer);
    if (strcmp(buffer, "hello") != 0) return false;

    /* No trim needed */
    strcpy(buffer, "hello");
    str_trim(buffer);
    if (strcmp(buffer, "hello") != 0) return false;

    return true;
}

/* Test: Case conversion */
static bool test_case_conversion(void) {
    char buffer[32];

    /* To lower */
    strcpy(buffer, "Hello World");
    str_to_lower(buffer);
    if (strcmp(buffer, "hello world") != 0) return false;

    /* To upper */
    strcpy(buffer, "Hello World");
    str_to_upper(buffer);
    if (strcmp(buffer, "HELLO WORLD") != 0) return false;

    return true;
}

/* Test: String search */
static bool test_search(void) {
    const char* str = "hello world";

    /* Find char */
    char* pos = str_find_char(str, 'w');
    if (!pos || *pos != 'w') return false;

    /* Find char not present */
    pos = str_find_char(str, 'x');
    if (pos != NULL) return false;

    /* Find char last */
    pos = str_find_char_last(str, 'l');
    if (!pos || pos != &str[9]) return false;

    /* Find substring */
    pos = str_find_substring(str, "world");
    if (!pos || strcmp(pos, "world") != 0) return false;

    pos = str_find_substring(str, "notfound");
    if (pos != NULL) return false;

    return true;
}

/* Test: String validation */
static bool test_validation(void) {
    /* Empty or whitespace */
    if (!str_is_empty_or_whitespace("")) return false;
    if (!str_is_empty_or_whitespace("   ")) return false;
    if (str_is_empty_or_whitespace("text")) return false;

    /* Digits */
    if (!str_is_digits("12345")) return false;
    if (str_is_digits("123abc")) return false;
    if (str_is_digits("")) return false;

    /* Alphanumeric */
    if (!str_is_alnum("abc123")) return false;
    if (str_is_alnum("abc-123")) return false;
    if (str_is_alnum("")) return false;

    return true;
}

/* Test: String split */
static bool test_split(void) {
    char buffer[64];
    char* tokens[10];

    /* Split by comma */
    strcpy(buffer, "one,two,three");
    size_t count = str_split(buffer, ',', tokens, 10);

    if (count != 3) return false;
    if (strcmp(tokens[0], "one") != 0) return false;
    if (strcmp(tokens[1], "two") != 0) return false;
    if (strcmp(tokens[2], "three") != 0) return false;

    /* Split by space */
    strcpy(buffer, "hello world test");
    count = str_split(buffer, ' ', tokens, 10);

    if (count != 3) return false;
    if (strcmp(tokens[0], "hello") != 0) return false;
    if (strcmp(tokens[1], "world") != 0) return false;
    if (strcmp(tokens[2], "test") != 0) return false;

    return true;
}

/* Test: String builder basic */
static bool test_builder_basic(void) {
    StringBuilder* builder = str_builder_create(0);
    if (!builder) return false;

    /* Append strings */
    if (!str_builder_append(builder, "Hello")) {
        str_builder_destroy(builder);
        return false;
    }

    if (!str_builder_append(builder, " ")) {
        str_builder_destroy(builder);
        return false;
    }

    if (!str_builder_append(builder, "World")) {
        str_builder_destroy(builder);
        return false;
    }

    /* Check result */
    const char* result = str_builder_get(builder);
    if (!result || strcmp(result, "Hello World") != 0) {
        str_builder_destroy(builder);
        return false;
    }

    /* Check length */
    if (str_builder_length(builder) != 11) {
        str_builder_destroy(builder);
        return false;
    }

    str_builder_destroy(builder);
    return true;
}

/* Test: String builder append char */
static bool test_builder_append_char(void) {
    StringBuilder* builder = str_builder_create(0);
    if (!builder) return false;

    /* Append characters */
    for (char ch = 'A'; ch <= 'E'; ch++) {
        if (!str_builder_append_char(builder, ch)) {
            str_builder_destroy(builder);
            return false;
        }
    }

    const char* result = str_builder_get(builder);
    if (!result || strcmp(result, "ABCDE") != 0) {
        str_builder_destroy(builder);
        return false;
    }

    str_builder_destroy(builder);
    return true;
}

/* Test: String builder format */
static bool test_builder_format(void) {
    StringBuilder* builder = str_builder_create(0);
    if (!builder) return false;

    /* Append formatted */
    if (!str_builder_append_format(builder, "Number: %d", 42)) {
        str_builder_destroy(builder);
        return false;
    }

    if (!str_builder_append_format(builder, ", String: %s", "test")) {
        str_builder_destroy(builder);
        return false;
    }

    const char* result = str_builder_get(builder);
    if (!result || strcmp(result, "Number: 42, String: test") != 0) {
        str_builder_destroy(builder);
        return false;
    }

    str_builder_destroy(builder);
    return true;
}

/* Test: String builder clear */
static bool test_builder_clear(void) {
    StringBuilder* builder = str_builder_create(0);
    if (!builder) return false;

    str_builder_append(builder, "test");
    str_builder_clear(builder);

    if (str_builder_length(builder) != 0) {
        str_builder_destroy(builder);
        return false;
    }

    const char* result = str_builder_get(builder);
    if (!result || strcmp(result, "") != 0) {
        str_builder_destroy(builder);
        return false;
    }

    str_builder_destroy(builder);
    return true;
}

/* Test: String builder extract */
static bool test_builder_extract(void) {
    StringBuilder* builder = str_builder_create(0);
    if (!builder) return false;

    str_builder_append(builder, "extracted");

    char* extracted = str_builder_extract(builder);
    if (!extracted || strcmp(extracted, "extracted") != 0) {
        free(extracted);
        str_builder_destroy(builder);
        return false;
    }

    /* Builder should be empty now */
    if (str_builder_length(builder) != 0) {
        free(extracted);
        str_builder_destroy(builder);
        return false;
    }

    free(extracted);
    str_builder_destroy(builder);
    return true;
}

/* Test: String builder growth */
static bool test_builder_growth(void) {
    StringBuilder* builder = str_builder_create(8);  /* Small initial capacity */
    if (!builder) return false;

    /* Append many strings to trigger growth */
    for (int i = 0; i < 100; i++) {
        if (!str_builder_append(builder, "x")) {
            str_builder_destroy(builder);
            return false;
        }
    }

    if (str_builder_length(builder) != 100) {
        str_builder_destroy(builder);
        return false;
    }

    str_builder_destroy(builder);
    return true;
}

int main(void) {
    /* Initialize logger for tests */
    logger_init("test_string_utils.log", LOG_LEVEL_DEBUG);

    printf("=====================================\n");
    printf("String Utilities Tests\n");
    printf("=====================================\n\n");

    TEST(safe_copy);
    TEST(safe_concat);
    TEST(comparison);
    TEST(prefix_suffix);
    TEST(trim);
    TEST(case_conversion);
    TEST(search);
    TEST(validation);
    TEST(split);
    TEST(builder_basic);
    TEST(builder_append_char);
    TEST(builder_format);
    TEST(builder_clear);
    TEST(builder_extract);
    TEST(builder_growth);

    printf("\n=====================================\n");
    printf("Results: %d/%d tests passed\n", tests_passed, tests_run);
    printf("=====================================\n");

    logger_shutdown();

    return (tests_passed == tests_run) ? EXIT_SUCCESS : EXIT_FAILURE;
}

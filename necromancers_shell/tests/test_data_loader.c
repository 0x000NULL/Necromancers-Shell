#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../src/data/data_loader.h"

/**
 * @file test_data_loader.c
 * @brief Unit tests for data_loader.c
 *
 * Tests all aspects of the data parser:
 * - File loading
 * - Section parsing
 * - Type inference
 * - Value extraction
 * - Error handling
 */

#define TEST_DATA_FILE "tests/test_data.dat"

static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) \
    do { \
        printf("Running test: %s...", #name); \
        fflush(stdout); \
        tests_run++; \
        if (name()) { \
            printf(" PASSED\n"); \
            tests_passed++; \
        } else { \
            printf(" FAILED\n"); \
            tests_failed++; \
        } \
    } while (0)

/* Helper function to check test result */
#define ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf("\n  Assertion failed: %s\n", message); \
            return false; \
        } \
    } while (0)

/* Test file loading */
static bool test_file_load_success(void) {
    DataFile* file = data_file_load(TEST_DATA_FILE);
    ASSERT(file != NULL, "Failed to load test data file");
    ASSERT(data_file_is_valid(file), "Data file is not valid");
    ASSERT(data_file_get_section_count(file) > 0, "No sections loaded");
    data_file_destroy(file);
    return true;
}

static bool test_file_load_nonexistent(void) {
    DataFile* file = data_file_load("nonexistent_file.dat");
    ASSERT(file == NULL, "Should return NULL for nonexistent file");
    ASSERT(data_file_get_error() != NULL, "Error message should be set");
    return true;
}

static bool test_file_load_null_path(void) {
    DataFile* file = data_file_load(NULL);
    ASSERT(file == NULL, "Should return NULL for NULL path");
    return true;
}

/* Test section access */
static bool test_get_section_by_type_and_id(void) {
    DataFile* file = data_file_load(TEST_DATA_FILE);
    ASSERT(file != NULL, "Failed to load file");

    const DataSection* section = data_file_get_section(file, "TEST", "string_values");
    ASSERT(section != NULL, "Failed to get section");
    ASSERT(strcmp(section->section_type, "TEST") == 0, "Wrong section type");
    ASSERT(strcmp(section->section_id, "string_values") == 0, "Wrong section ID");
    ASSERT(section->property_count > 0, "Section has no properties");

    data_file_destroy(file);
    return true;
}

static bool test_get_sections_by_type(void) {
    DataFile* file = data_file_load(TEST_DATA_FILE);
    ASSERT(file != NULL, "Failed to load file");

    size_t count;
    const DataSection** sections = data_file_get_sections(file, "TEST", &count);
    ASSERT(sections != NULL, "Failed to get sections");
    ASSERT(count >= 8, "Expected at least 8 TEST sections");

    free((void*)sections);
    data_file_destroy(file);
    return true;
}

static bool test_get_nonexistent_section(void) {
    DataFile* file = data_file_load(TEST_DATA_FILE);
    ASSERT(file != NULL, "Failed to load file");

    const DataSection* section = data_file_get_section(file, "TEST", "nonexistent");
    ASSERT(section == NULL, "Should return NULL for nonexistent section");

    data_file_destroy(file);
    return true;
}

/* Test property access */
static bool test_get_property_from_section(void) {
    DataFile* file = data_file_load(TEST_DATA_FILE);
    ASSERT(file != NULL, "Failed to load file");

    const DataSection* section = data_file_get_section(file, "TEST", "string_values");
    ASSERT(section != NULL, "Failed to get section");

    const DataValue* value = data_section_get(section, "name");
    ASSERT(value != NULL, "Failed to get property");
    ASSERT(value->type == DATA_TYPE_STRING, "Wrong value type");
    ASSERT(strcmp(value->value.string_value, "Test Location") == 0, "Wrong value");

    data_file_destroy(file);
    return true;
}

static bool test_get_nonexistent_property(void) {
    DataFile* file = data_file_load(TEST_DATA_FILE);
    ASSERT(file != NULL, "Failed to load file");

    const DataSection* section = data_file_get_section(file, "TEST", "string_values");
    ASSERT(section != NULL, "Failed to get section");

    const DataValue* value = data_section_get(section, "nonexistent");
    ASSERT(value == NULL, "Should return NULL for nonexistent property");

    data_file_destroy(file);
    return true;
}

/* Test string values */
static bool test_string_value_extraction(void) {
    DataFile* file = data_file_load(TEST_DATA_FILE);
    ASSERT(file != NULL, "Failed to load file");

    const DataSection* section = data_file_get_section(file, "TEST", "string_values");
    ASSERT(section != NULL, "Failed to get section");

    const DataValue* name_val = data_section_get(section, "name");
    const char* name = data_value_get_string(name_val, "default");
    ASSERT(strcmp(name, "Test Location") == 0, "Wrong string value");

    const DataValue* desc_val = data_section_get(section, "description");
    const char* desc = data_value_get_string(desc_val, "default");
    ASSERT(strlen(desc) > 0, "Description should not be empty");

    data_file_destroy(file);
    return true;
}

static bool test_string_value_with_default(void) {
    DataFile* file = data_file_load(TEST_DATA_FILE);
    ASSERT(file != NULL, "Failed to load file");

    const DataSection* section = data_file_get_section(file, "TEST", "string_values");
    ASSERT(section != NULL, "Failed to get section");

    /* Nonexistent property should return default */
    const DataValue* val = data_section_get(section, "nonexistent");
    const char* result = data_value_get_string(val, "default_value");
    ASSERT(strcmp(result, "default_value") == 0, "Should return default");

    data_file_destroy(file);
    return true;
}

/* Test integer values */
static bool test_integer_value_extraction(void) {
    DataFile* file = data_file_load(TEST_DATA_FILE);
    ASSERT(file != NULL, "Failed to load file");

    const DataSection* section = data_file_get_section(file, "TEST", "numeric_values");
    ASSERT(section != NULL, "Failed to get section");

    const DataValue* pos_val = data_section_get(section, "integer_positive");
    ASSERT(pos_val != NULL && pos_val->type == DATA_TYPE_INT, "Wrong type");
    ASSERT(pos_val->value.int_value == 42, "Wrong positive integer");

    const DataValue* neg_val = data_section_get(section, "integer_negative");
    ASSERT(neg_val != NULL && neg_val->type == DATA_TYPE_INT, "Wrong type");
    ASSERT(neg_val->value.int_value == -100, "Wrong negative integer");

    const DataValue* zero_val = data_section_get(section, "integer_zero");
    ASSERT(zero_val != NULL && zero_val->type == DATA_TYPE_INT, "Wrong type");
    ASSERT(zero_val->value.int_value == 0, "Wrong zero integer");

    data_file_destroy(file);
    return true;
}

static bool test_integer_value_with_default(void) {
    DataFile* file = data_file_load(TEST_DATA_FILE);
    ASSERT(file != NULL, "Failed to load file");

    const DataSection* section = data_file_get_section(file, "TEST", "numeric_values");
    ASSERT(section != NULL, "Failed to get section");

    const DataValue* val = data_section_get(section, "nonexistent");
    int64_t result = data_value_get_int(val, 999);
    ASSERT(result == 999, "Should return default");

    data_file_destroy(file);
    return true;
}

/* Test float values */
static bool test_float_value_extraction(void) {
    DataFile* file = data_file_load(TEST_DATA_FILE);
    ASSERT(file != NULL, "Failed to load file");

    const DataSection* section = data_file_get_section(file, "TEST", "numeric_values");
    ASSERT(section != NULL, "Failed to get section");

    const DataValue* pos_val = data_section_get(section, "float_positive");
    ASSERT(pos_val != NULL && pos_val->type == DATA_TYPE_FLOAT, "Wrong type");
    ASSERT(pos_val->value.float_value > 3.14 && pos_val->value.float_value < 3.15,
           "Wrong positive float");

    const DataValue* neg_val = data_section_get(section, "float_negative");
    ASSERT(neg_val != NULL && neg_val->type == DATA_TYPE_FLOAT, "Wrong type");
    ASSERT(neg_val->value.float_value < -2.71 && neg_val->value.float_value > -2.72,
           "Wrong negative float");

    data_file_destroy(file);
    return true;
}

static bool test_float_value_with_default(void) {
    DataFile* file = data_file_load(TEST_DATA_FILE);
    ASSERT(file != NULL, "Failed to load file");

    const DataSection* section = data_file_get_section(file, "TEST", "numeric_values");
    ASSERT(section != NULL, "Failed to get section");

    const DataValue* val = data_section_get(section, "nonexistent");
    double result = data_value_get_float(val, 3.14);
    ASSERT(result > 3.13 && result < 3.15, "Should return default");

    data_file_destroy(file);
    return true;
}

/* Test boolean values */
static bool test_boolean_value_extraction(void) {
    DataFile* file = data_file_load(TEST_DATA_FILE);
    ASSERT(file != NULL, "Failed to load file");

    const DataSection* section = data_file_get_section(file, "TEST", "boolean_values");
    ASSERT(section != NULL, "Failed to get section");

    /* Test true variants */
    const DataValue* t1 = data_section_get(section, "bool_true_1");
    ASSERT(t1 != NULL && data_value_get_bool(t1, false) == true, "true should be true");

    const DataValue* t2 = data_section_get(section, "bool_true_2");
    ASSERT(t2 != NULL && data_value_get_bool(t2, false) == true, "TRUE should be true");

    const DataValue* t3 = data_section_get(section, "bool_true_3");
    ASSERT(t3 != NULL && data_value_get_bool(t3, false) == true, "yes should be true");

    const DataValue* t5 = data_section_get(section, "bool_true_5");
    ASSERT(t5 != NULL && data_value_get_bool(t5, false) == true, "1 should be true");

    /* Test false variants */
    const DataValue* f1 = data_section_get(section, "bool_false_1");
    ASSERT(f1 != NULL && data_value_get_bool(f1, true) == false, "false should be false");

    const DataValue* f3 = data_section_get(section, "bool_false_3");
    ASSERT(f3 != NULL && data_value_get_bool(f3, true) == false, "no should be false");

    data_file_destroy(file);
    return true;
}

static bool test_boolean_value_with_default(void) {
    DataFile* file = data_file_load(TEST_DATA_FILE);
    ASSERT(file != NULL, "Failed to load file");

    const DataSection* section = data_file_get_section(file, "TEST", "boolean_values");
    ASSERT(section != NULL, "Failed to get section");

    const DataValue* val = data_section_get(section, "nonexistent");
    bool result = data_value_get_bool(val, true);
    ASSERT(result == true, "Should return default");

    data_file_destroy(file);
    return true;
}

/* Test array values */
static bool test_array_value_extraction(void) {
    DataFile* file = data_file_load(TEST_DATA_FILE);
    ASSERT(file != NULL, "Failed to load file");

    const DataSection* section = data_file_get_section(file, "TEST", "array_values");
    ASSERT(section != NULL, "Failed to get section");

    size_t count;
    const DataValue* arr_val = data_section_get(section, "three_elements");
    ASSERT(arr_val != NULL && arr_val->type == DATA_TYPE_ARRAY, "Wrong type");

    const char** arr = data_value_get_array(arr_val, &count);
    ASSERT(arr != NULL, "Failed to get array");
    ASSERT(count == 3, "Wrong array count");
    ASSERT(strcmp(arr[0], "foo") == 0, "Wrong element 0");
    ASSERT(strcmp(arr[1], "bar") == 0, "Wrong element 1");
    ASSERT(strcmp(arr[2], "baz") == 0, "Wrong element 2");
    ASSERT(arr[3] == NULL, "Array should be NULL-terminated");

    data_file_destroy(file);
    return true;
}

static bool test_array_with_whitespace(void) {
    DataFile* file = data_file_load(TEST_DATA_FILE);
    ASSERT(file != NULL, "Failed to load file");

    const DataSection* section = data_file_get_section(file, "TEST", "array_values");
    ASSERT(section != NULL, "Failed to get section");

    size_t count;
    const DataValue* arr_val = data_section_get(section, "with_spaces");
    const char** arr = data_value_get_array(arr_val, &count);
    ASSERT(arr != NULL, "Failed to get array");
    ASSERT(count == 3, "Wrong array count");
    /* Whitespace should be trimmed */
    ASSERT(strcmp(arr[0], "foo") == 0, "Whitespace not trimmed");
    ASSERT(strcmp(arr[1], "bar") == 0, "Whitespace not trimmed");
    ASSERT(strcmp(arr[2], "baz") == 0, "Whitespace not trimmed");

    data_file_destroy(file);
    return true;
}

/* Test mixed types in one section */
static bool test_mixed_types_section(void) {
    DataFile* file = data_file_load(TEST_DATA_FILE);
    ASSERT(file != NULL, "Failed to load file");

    const DataSection* section = data_file_get_section(file, "TEST", "mixed_types");
    ASSERT(section != NULL, "Failed to get section");

    /* String */
    const char* name = data_value_get_string(data_section_get(section, "name"), "");
    ASSERT(strcmp(name, "Mixed Test") == 0, "Wrong string");

    /* Integer */
    int64_t count = data_value_get_int(data_section_get(section, "count"), 0);
    ASSERT(count == 10, "Wrong integer");

    /* Float */
    double ratio = data_value_get_float(data_section_get(section, "ratio"), 0.0);
    ASSERT(ratio > 0.74 && ratio < 0.76, "Wrong float");

    /* Boolean */
    bool enabled = data_value_get_bool(data_section_get(section, "enabled"), false);
    ASSERT(enabled == true, "Wrong boolean");

    /* Array */
    size_t arr_count;
    const char** tags = data_value_get_array(data_section_get(section, "tags"), &arr_count);
    ASSERT(tags != NULL && arr_count == 3, "Wrong array");

    data_file_destroy(file);
    return true;
}

/* Test memory management */
static bool test_memory_cleanup(void) {
    /* Load and destroy multiple times */
    for (int i = 0; i < 10; i++) {
        DataFile* file = data_file_load(TEST_DATA_FILE);
        ASSERT(file != NULL, "Failed to load file");
        data_file_destroy(file);
    }
    return true;
}

static bool test_destroy_null_file(void) {
    /* Should not crash */
    data_file_destroy(NULL);
    return true;
}

/* Main test runner */
int main(void) {
    printf("=== Data Loader Unit Tests ===\n\n");

    /* File loading tests */
    TEST(test_file_load_success);
    TEST(test_file_load_nonexistent);
    TEST(test_file_load_null_path);

    /* Section access tests */
    TEST(test_get_section_by_type_and_id);
    TEST(test_get_sections_by_type);
    TEST(test_get_nonexistent_section);

    /* Property access tests */
    TEST(test_get_property_from_section);
    TEST(test_get_nonexistent_property);

    /* String value tests */
    TEST(test_string_value_extraction);
    TEST(test_string_value_with_default);

    /* Integer value tests */
    TEST(test_integer_value_extraction);
    TEST(test_integer_value_with_default);

    /* Float value tests */
    TEST(test_float_value_extraction);
    TEST(test_float_value_with_default);

    /* Boolean value tests */
    TEST(test_boolean_value_extraction);
    TEST(test_boolean_value_with_default);

    /* Array value tests */
    TEST(test_array_value_extraction);
    TEST(test_array_with_whitespace);

    /* Mixed types test */
    TEST(test_mixed_types_section);

    /* Memory management tests */
    TEST(test_memory_cleanup);
    TEST(test_destroy_null_file);

    /* Print summary */
    printf("\n=== Test Summary ===\n");
    printf("Tests run:    %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_failed);

    if (tests_failed == 0) {
        printf("\n✓ All tests passed!\n");
        return 0;
    } else {
        printf("\n✗ Some tests failed.\n");
        return 1;
    }
}

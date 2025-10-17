/**
 * @file test_choice_prompt.c
 * @brief Unit tests for choice prompt system
 */

#include "../src/game/ui/choice_prompt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Test helper: count test results */
static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) \
    printf("Running %s...\n", name); \
    tests_run++;

#define ASSERT(condition, message) \
    if (!(condition)) { \
        printf("  FAIL: %s\n", message); \
        return false; \
    }

#define PASS() \
    tests_passed++; \
    printf("  PASS\n"); \
    return true;

/**
 * Test 1: Choice structure initialization
 */
bool test_choice_struct(void) {
    TEST("test_choice_struct");

    Choice choice = {
        .label = "Test Choice",
        .key = 't',
        .description = "A test choice description"
    };

    ASSERT(strcmp(choice.label, "Test Choice") == 0, "Label should match");
    ASSERT(choice.key == 't', "Key should be 't'");
    ASSERT(strcmp(choice.description, "A test choice description") == 0, "Description should match");

    PASS();
}

/**
 * Test 2: Binary choice setup
 */
bool test_binary_choice_setup(void) {
    TEST("test_binary_choice_setup");

    Choice choices[] = {
        {.label = "Yes", .key = 'y', .description = "Accept"},
        {.label = "No", .key = 'n', .description = "Decline"}
    };

    ASSERT(choices[0].key == 'y', "First choice key should be 'y'");
    ASSERT(choices[1].key == 'n', "Second choice key should be 'n'");
    ASSERT(strcmp(choices[0].label, "Yes") == 0, "First label should be 'Yes'");
    ASSERT(strcmp(choices[1].label, "No") == 0, "Second label should be 'No'");

    PASS();
}

/**
 * Test 3: Multi-choice setup
 */
bool test_multi_choice_setup(void) {
    TEST("test_multi_choice_setup");

    Choice choices[] = {
        {.label = "Option A", .key = 'a', .description = "First option"},
        {.label = "Option B", .key = 'b', .description = "Second option"},
        {.label = "Option C", .key = 'c', .description = "Third option"}
    };

    ASSERT(sizeof(choices) / sizeof(Choice) == 3, "Should have 3 choices");
    ASSERT(choices[0].key == 'a', "First key 'a'");
    ASSERT(choices[1].key == 'b', "Second key 'b'");
    ASSERT(choices[2].key == 'c', "Third key 'c'");

    PASS();
}

/**
 * Test 4: Choice validation - label length
 */
bool test_choice_label_length(void) {
    TEST("test_choice_label_length");

    Choice choice;

    /* Test normal length label */
    strncpy(choice.label, "Normal Label", sizeof(choice.label) - 1);
    choice.label[sizeof(choice.label) - 1] = '\0';

    ASSERT(strlen(choice.label) < sizeof(choice.label), "Label should fit in buffer");
    ASSERT(strlen(choice.label) == 12, "Label length should be 12");

    PASS();
}

/**
 * Test 5: Choice validation - description length
 */
bool test_choice_description_length(void) {
    TEST("test_choice_description_length");

    Choice choice;

    /* Test normal length description */
    strncpy(choice.description, "This is a test description", sizeof(choice.description) - 1);
    choice.description[sizeof(choice.description) - 1] = '\0';

    ASSERT(strlen(choice.description) < sizeof(choice.description), "Description should fit in buffer");

    PASS();
}

/**
 * Test 6: Choice array iteration
 */
bool test_choice_array_iteration(void) {
    TEST("test_choice_array_iteration");

    Choice choices[] = {
        {.label = "First", .key = '1', .description = "First choice"},
        {.label = "Second", .key = '2', .description = "Second choice"},
        {.label = "Third", .key = '3', .description = "Third choice"}
    };

    size_t count = sizeof(choices) / sizeof(Choice);
    ASSERT(count == 3, "Should have 3 choices");

    /* Verify each choice is accessible */
    for (size_t i = 0; i < count; i++) {
        ASSERT(choices[i].key == ('1' + (char)i), "Keys should be sequential");
        ASSERT(strlen(choices[i].label) > 0, "Each choice should have a label");
    }

    PASS();
}

/**
 * Test 7: Choice key uniqueness check
 */
bool test_choice_key_uniqueness(void) {
    TEST("test_choice_key_uniqueness");

    Choice choices[] = {
        {.label = "Harvest", .key = 'h', .description = "Harvest souls"},
        {.label = "Spare", .key = 's', .description = "Spare village"}
    };

    /* Verify keys are different */
    ASSERT(choices[0].key != choices[1].key, "Keys should be unique");

    PASS();
}

int main(void) {
    printf("\n=== Choice Prompt Unit Tests ===\n\n");

    /* Run all tests */
    test_choice_struct();
    test_binary_choice_setup();
    test_multi_choice_setup();
    test_choice_label_length();
    test_choice_description_length();
    test_choice_array_iteration();
    test_choice_key_uniqueness();

    /* Print results */
    printf("\n=== Test Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    if (tests_passed == tests_run) {
        printf("\n✓ All tests passed!\n\n");
        return 0;
    } else {
        printf("\n✗ Some tests failed!\n\n");
        return 1;
    }
}

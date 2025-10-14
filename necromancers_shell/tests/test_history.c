#include "../src/commands/history.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

static void test_add_and_get(void) {
    CommandHistory* history = command_history_create(10);
    assert(history != NULL);

    command_history_add(history, "help");
    command_history_add(history, "status");
    command_history_add(history, "quit");

    assert(command_history_size(history) == 3);

    /* Get most recent (index 0) */
    const char* cmd = command_history_get(history, 0);
    assert(cmd != NULL);
    assert(strcmp(cmd, "quit") == 0);

    /* Get second most recent (index 1) */
    cmd = command_history_get(history, 1);
    assert(cmd != NULL);
    assert(strcmp(cmd, "status") == 0);

    /* Get oldest (index 2) */
    cmd = command_history_get(history, 2);
    assert(cmd != NULL);
    assert(strcmp(cmd, "help") == 0);

    command_history_destroy(history);
    printf("[PASS] test_add_and_get\n");
}

static void test_duplicate_filtering(void) {
    CommandHistory* history = command_history_create(10);
    assert(history != NULL);

    command_history_add(history, "help");
    command_history_add(history, "help");  /* Should be ignored */
    command_history_add(history, "status");
    command_history_add(history, "status");  /* Should be ignored */

    assert(command_history_size(history) == 2);

    command_history_destroy(history);
    printf("[PASS] test_duplicate_filtering\n");
}

static void test_circular_buffer(void) {
    CommandHistory* history = command_history_create(3);
    assert(history != NULL);

    command_history_add(history, "cmd1");
    command_history_add(history, "cmd2");
    command_history_add(history, "cmd3");
    command_history_add(history, "cmd4");  /* Should evict cmd1 */

    assert(command_history_size(history) == 3);

    /* Most recent should be cmd4 */
    const char* cmd = command_history_get(history, 0);
    assert(strcmp(cmd, "cmd4") == 0);

    /* cmd1 should be gone */
    cmd = command_history_get(history, 2);
    assert(strcmp(cmd, "cmd2") == 0);

    command_history_destroy(history);
    printf("[PASS] test_circular_buffer\n");
}

static void test_search(void) {
    CommandHistory* history = command_history_create(10);
    assert(history != NULL);

    command_history_add(history, "help status");
    command_history_add(history, "status");
    command_history_add(history, "quit");
    command_history_add(history, "help");

    char** results = NULL;
    size_t count = 0;

    /* Search for "help" */
    assert(command_history_search(history, "help", &results, &count));
    assert(count == 2);
    command_history_free_search_results(results, count);

    /* Search for "status" */
    assert(command_history_search(history, "status", &results, &count));
    assert(count == 2);
    command_history_free_search_results(results, count);

    /* Search for non-existent */
    assert(command_history_search(history, "xyz", &results, &count));
    assert(count == 0);

    command_history_destroy(history);
    printf("[PASS] test_search\n");
}

static void test_clear(void) {
    CommandHistory* history = command_history_create(10);
    assert(history != NULL);

    command_history_add(history, "help");
    command_history_add(history, "status");

    assert(command_history_size(history) == 2);

    command_history_clear(history);

    assert(command_history_size(history) == 0);

    command_history_destroy(history);
    printf("[PASS] test_clear\n");
}

int main(void) {
    printf("Running history tests...\n\n");

    test_add_and_get();
    test_duplicate_filtering();
    test_circular_buffer();
    test_search();
    test_clear();

    printf("\nAll history tests passed!\n");
    return 0;
}

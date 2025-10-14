#include "../src/utils/trie.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

static void test_insert_and_contains(void) {
    Trie* trie = trie_create();
    assert(trie != NULL);

    assert(trie_insert(trie, "help"));
    assert(trie_insert(trie, "history"));
    assert(trie_insert(trie, "status"));

    assert(trie_contains(trie, "help"));
    assert(trie_contains(trie, "history"));
    assert(trie_contains(trie, "status"));
    assert(!trie_contains(trie, "quit"));

    assert(trie_size(trie) == 3);

    trie_destroy(trie);
    printf("[PASS] test_insert_and_contains\n");
}

static void test_prefix_matching(void) {
    Trie* trie = trie_create();
    assert(trie != NULL);

    trie_insert(trie, "help");
    trie_insert(trie, "history");
    trie_insert(trie, "status");
    trie_insert(trie, "stats");

    char** matches = NULL;
    size_t count = 0;

    /* Find with prefix "h" */
    assert(trie_find_with_prefix(trie, "h", &matches, &count));
    assert(count == 2);
    /* Matches should be "help" and "history" in some order */
    trie_free_matches(matches, count);

    /* Find with prefix "st" */
    assert(trie_find_with_prefix(trie, "st", &matches, &count));
    assert(count == 2);
    /* Matches should be "status" and "stats" */
    trie_free_matches(matches, count);

    /* Find with prefix "help" */
    assert(trie_find_with_prefix(trie, "help", &matches, &count));
    assert(count == 1);
    assert(strcmp(matches[0], "help") == 0);
    trie_free_matches(matches, count);

    /* Find with non-existent prefix */
    assert(trie_find_with_prefix(trie, "xyz", &matches, &count));
    assert(count == 0);

    trie_destroy(trie);
    printf("[PASS] test_prefix_matching\n");
}

static void test_remove(void) {
    Trie* trie = trie_create();
    assert(trie != NULL);

    trie_insert(trie, "help");
    trie_insert(trie, "history");

    assert(trie_size(trie) == 2);
    assert(trie_contains(trie, "help"));

    assert(trie_remove(trie, "help"));
    assert(!trie_contains(trie, "help"));
    assert(trie_size(trie) == 1);

    /* Try to remove non-existent */
    assert(!trie_remove(trie, "help"));

    trie_destroy(trie);
    printf("[PASS] test_remove\n");
}

static void test_clear(void) {
    Trie* trie = trie_create();
    assert(trie != NULL);

    trie_insert(trie, "help");
    trie_insert(trie, "history");
    trie_insert(trie, "status");

    assert(trie_size(trie) == 3);

    trie_clear(trie);

    assert(trie_size(trie) == 0);
    assert(!trie_contains(trie, "help"));

    trie_destroy(trie);
    printf("[PASS] test_clear\n");
}

int main(void) {
    printf("Running trie tests...\n\n");

    test_insert_and_contains();
    test_prefix_matching();
    test_remove();
    test_clear();

    printf("\nAll trie tests passed!\n");
    return 0;
}

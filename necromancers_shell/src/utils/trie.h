#ifndef TRIE_H
#define TRIE_H

#include <stdbool.h>
#include <stddef.h>

/**
 * Trie Data Structure
 *
 * Prefix tree for efficient string prefix matching.
 * Optimized for ASCII strings (128 character set).
 *
 * Usage:
 *   Trie* trie = trie_create();
 *   trie_insert(trie, "help");
 *   trie_insert(trie, "history");
 *   char** matches = NULL;
 *   size_t count = 0;
 *   trie_find_with_prefix(trie, "he", &matches, &count);
 *   // matches = ["help", "history"]
 *   trie_free_matches(matches, count);
 *   trie_destroy(trie);
 */

/* Opaque trie structure */
typedef struct Trie Trie;

/**
 * Create a new trie
 *
 * @return Trie pointer or NULL on failure
 */
Trie* trie_create(void);

/**
 * Destroy trie and free all memory
 *
 * @param trie Trie to destroy
 */
void trie_destroy(Trie* trie);

/**
 * Insert a string into the trie
 *
 * @param trie Trie
 * @param str String to insert
 * @return true on success
 */
bool trie_insert(Trie* trie, const char* str);

/**
 * Check if string exists in trie
 *
 * @param trie Trie
 * @param str String to search for
 * @return true if string exists
 */
bool trie_contains(const Trie* trie, const char* str);

/**
 * Remove string from trie
 *
 * @param trie Trie
 * @param str String to remove
 * @return true if string was found and removed
 */
bool trie_remove(Trie* trie, const char* str);

/**
 * Find all strings with given prefix
 *
 * @param trie Trie
 * @param prefix Prefix to search for
 * @param matches Output array of matching strings (allocated by function)
 * @param count Output number of matches
 * @return true on success
 */
bool trie_find_with_prefix(const Trie* trie, const char* prefix,
                          char*** matches, size_t* count);

/**
 * Free matches array returned by trie_find_with_prefix
 *
 * @param matches Matches array
 * @param count Number of matches
 */
void trie_free_matches(char** matches, size_t count);

/**
 * Get number of strings in trie
 *
 * @param trie Trie
 * @return Number of strings
 */
size_t trie_size(const Trie* trie);

/**
 * Clear all strings from trie
 *
 * @param trie Trie
 */
void trie_clear(Trie* trie);

#endif /* TRIE_H */

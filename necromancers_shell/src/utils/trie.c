/* strdup is a POSIX function, not C standard */
#define _POSIX_C_SOURCE 200809L

#include "trie.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define ALPHABET_SIZE 128  /* ASCII character set */

/* Trie node structure */
typedef struct TrieNode {
    struct TrieNode* children[ALPHABET_SIZE];
    bool is_end_of_word;
    char* word;  /* Store complete word at terminal nodes for easy retrieval */
} TrieNode;

/* Trie structure */
struct Trie {
    TrieNode* root;
    size_t size;
};

/* Helper: Create a new trie node */
static TrieNode* trie_node_create(void) {
    TrieNode* node = malloc(sizeof(TrieNode));
    if (!node) return NULL;

    for (int i = 0; i < ALPHABET_SIZE; i++) {
        node->children[i] = NULL;
    }

    node->is_end_of_word = false;
    node->word = NULL;

    return node;
}

/* Helper: Destroy a trie node and all its children (recursive) */
static void trie_node_destroy(TrieNode* node) {
    if (!node) return;

    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (node->children[i]) {
            trie_node_destroy(node->children[i]);
        }
    }

    free(node->word);
    free(node);
}

/* Helper: Collect all words in subtrie (recursive DFS) */
static void collect_words(TrieNode* node, char*** matches, size_t* count, size_t* capacity) {
    if (!node) return;

    if (node->is_end_of_word && node->word) {
        /* Grow array if needed */
        if (*count >= *capacity) {
            size_t new_capacity = (*capacity == 0) ? 8 : (*capacity * 2);
            char** new_matches = realloc(*matches, new_capacity * sizeof(char*));
            if (!new_matches) return;  /* Out of memory, stop collecting */

            *matches = new_matches;
            *capacity = new_capacity;
        }

        /* Add word to results */
        (*matches)[*count] = strdup(node->word);
        if ((*matches)[*count]) {
            (*count)++;
        }
    }

    /* Recursively collect from children */
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (node->children[i]) {
            collect_words(node->children[i], matches, count, capacity);
        }
    }
}

Trie* trie_create(void) {
    Trie* trie = malloc(sizeof(Trie));
    if (!trie) return NULL;

    trie->root = trie_node_create();
    if (!trie->root) {
        free(trie);
        return NULL;
    }

    trie->size = 0;
    return trie;
}

void trie_destroy(Trie* trie) {
    if (!trie) return;

    trie_node_destroy(trie->root);
    free(trie);
}

bool trie_insert(Trie* trie, const char* str) {
    if (!trie || !str) return false;

    TrieNode* node = trie->root;

    for (const char* c = str; *c != '\0'; c++) {
        unsigned char index = (unsigned char)*c;
        if (index >= ALPHABET_SIZE) {
            /* Non-ASCII characters not supported */
            continue;
        }

        if (!node->children[index]) {
            node->children[index] = trie_node_create();
            if (!node->children[index]) return false;
        }

        node = node->children[index];
    }

    /* Mark as end of word */
    if (!node->is_end_of_word) {
        node->is_end_of_word = true;
        node->word = strdup(str);
        if (!node->word) return false;
        trie->size++;
    }

    return true;
}

bool trie_contains(const Trie* trie, const char* str) {
    if (!trie || !str) return false;

    TrieNode* node = trie->root;

    for (const char* c = str; *c != '\0'; c++) {
        unsigned char index = (unsigned char)*c;
        if (index >= ALPHABET_SIZE || !node->children[index]) {
            return false;
        }
        node = node->children[index];
    }

    return node->is_end_of_word;
}

bool trie_remove(Trie* trie, const char* str) {
    if (!trie || !str) return false;

    /* Find the node */
    TrieNode* node = trie->root;

    for (const char* c = str; *c != '\0'; c++) {
        unsigned char index = (unsigned char)*c;
        if (index >= ALPHABET_SIZE || !node->children[index]) {
            return false;  /* String not found */
        }
        node = node->children[index];
    }

    if (!node->is_end_of_word) {
        return false;  /* String not found */
    }

    /* Mark as not end of word */
    node->is_end_of_word = false;
    free(node->word);
    node->word = NULL;
    trie->size--;

    /* Note: We don't actually remove nodes, just mark them as not terminal.
     * Full node removal would require parent tracking. This is simpler and
     * still efficient for our use case. */

    return true;
}

bool trie_find_with_prefix(const Trie* trie, const char* prefix,
                          char*** matches, size_t* count) {
    if (!trie || !prefix || !matches || !count) return false;

    *matches = NULL;
    *count = 0;

    /* Find the node corresponding to the prefix */
    TrieNode* node = trie->root;

    for (const char* c = prefix; *c != '\0'; c++) {
        unsigned char index = (unsigned char)*c;
        if (index >= ALPHABET_SIZE || !node->children[index]) {
            /* Prefix not found, return empty results */
            return true;
        }
        node = node->children[index];
    }

    /* Collect all words in the subtrie */
    size_t capacity = 0;
    collect_words(node, matches, count, &capacity);

    return true;
}

void trie_free_matches(char** matches, size_t count) {
    if (!matches) return;

    for (size_t i = 0; i < count; i++) {
        free(matches[i]);
    }
    free(matches);
}

size_t trie_size(const Trie* trie) {
    if (!trie) return 0;
    return trie->size;
}

void trie_clear(Trie* trie) {
    if (!trie) return;

    trie_node_destroy(trie->root);
    trie->root = trie_node_create();
    trie->size = 0;
}

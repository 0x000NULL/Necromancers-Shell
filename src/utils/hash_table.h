#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stddef.h>
#include <stdbool.h>

/**
 * Hash Table - String-keyed dictionary
 *
 * Generic hash table with string keys and void* values.
 * Uses open addressing with linear probing for collision resolution.
 *
 * Usage:
 *   HashTable* table = hash_table_create(100);
 *   hash_table_put(table, "command", cmd_ptr);
 *   void* value = hash_table_get(table, "command");
 *   hash_table_destroy(table);
 */

/* Opaque hash table structure */
typedef struct HashTable HashTable;

/* Iterator callback function */
typedef void (*HashTableIterator)(const char* key, void* value, void* userdata);

/**
 * Create a hash table
 *
 * @param initial_capacity Initial capacity (will grow as needed)
 * @return Hash table pointer or NULL on failure
 */
HashTable* hash_table_create(size_t initial_capacity);

/**
 * Destroy a hash table
 * Does NOT free the values (caller's responsibility)
 *
 * @param table Hash table to destroy
 */
void hash_table_destroy(HashTable* table);

/**
 * Insert or update a key-value pair
 *
 * @param table Hash table
 * @param key String key (will be copied)
 * @param value Value pointer
 * @return true on success
 */
bool hash_table_put(HashTable* table, const char* key, void* value);

/**
 * Get a value by key
 *
 * @param table Hash table
 * @param key String key
 * @return Value pointer or NULL if not found
 */
void* hash_table_get(const HashTable* table, const char* key);

/**
 * Check if key exists
 *
 * @param table Hash table
 * @param key String key
 * @return true if key exists
 */
bool hash_table_contains(const HashTable* table, const char* key);

/**
 * Remove a key-value pair
 *
 * @param table Hash table
 * @param key String key
 * @return Value pointer or NULL if not found
 */
void* hash_table_remove(HashTable* table, const char* key);

/**
 * Clear all entries
 *
 * @param table Hash table
 */
void hash_table_clear(HashTable* table);

/**
 * Get number of entries
 *
 * @param table Hash table
 * @return Entry count
 */
size_t hash_table_size(const HashTable* table);

/**
 * Get capacity
 *
 * @param table Hash table
 * @return Current capacity
 */
size_t hash_table_capacity(const HashTable* table);

/**
 * Get load factor
 *
 * @param table Hash table
 * @return Load factor (0.0 - 1.0)
 */
float hash_table_load_factor(const HashTable* table);

/**
 * Iterate over all entries
 *
 * @param table Hash table
 * @param iterator Callback function
 * @param userdata User data passed to callback
 */
void hash_table_foreach(const HashTable* table, HashTableIterator iterator, void* userdata);

#endif /* HASH_TABLE_H */

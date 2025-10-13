#define _POSIX_C_SOURCE 200809L

#include "utils/hash_table.h"
#include "utils/logger.h"
#include <stdlib.h>
#include <string.h>

/* Hash table entry */
typedef struct {
    char* key;          /* Owned string */
    void* value;        /* User value */
    bool occupied;      /* Slot occupied */
    bool deleted;       /* Tombstone for removed entries */
} HashEntry;

/* Hash table structure */
struct HashTable {
    HashEntry* entries;
    size_t capacity;
    size_t size;
    size_t deleted_count;
};

/* Constants */
#define INITIAL_CAPACITY 16
#define MAX_LOAD_FACTOR 0.7f
#define MIN_CAPACITY 8

/* FNV-1a hash function for strings */
static size_t hash_string(const char* str) {
    size_t hash = 2166136261u;

    while (*str) {
        hash ^= (unsigned char)(*str++);
        hash *= 16777619u;
    }

    return hash;
}

/* Find entry slot for key */
static HashEntry* find_entry(HashEntry* entries, size_t capacity, const char* key) {
    size_t index = hash_string(key) % capacity;
    HashEntry* tombstone = NULL;

    /* Linear probing */
    for (size_t i = 0; i < capacity; i++) {
        HashEntry* entry = &entries[index];

        if (!entry->occupied) {
            /* Found empty slot */
            if (!entry->deleted) {
                /* Return tombstone if we found one, otherwise this empty slot */
                return tombstone ? tombstone : entry;
            } else {
                /* Remember first tombstone */
                if (!tombstone) {
                    tombstone = entry;
                }
            }
        } else if (strcmp(entry->key, key) == 0) {
            /* Found existing key */
            return entry;
        }

        /* Try next slot */
        index = (index + 1) % capacity;
    }

    /* Table is full (should not happen due to resizing) */
    return tombstone;
}

/* Resize and rehash */
static bool resize_table(HashTable* table, size_t new_capacity) {
    /* Allocate new entries */
    HashEntry* new_entries = calloc(new_capacity, sizeof(HashEntry));
    if (!new_entries) {
        LOG_ERROR("Failed to allocate hash table entries");
        return false;
    }

    /* Rehash existing entries */
    size_t new_size = 0;
    for (size_t i = 0; i < table->capacity; i++) {
        HashEntry* old_entry = &table->entries[i];
        if (old_entry->occupied && !old_entry->deleted) {
            HashEntry* new_entry = find_entry(new_entries, new_capacity, old_entry->key);
            new_entry->key = old_entry->key;  /* Transfer ownership */
            new_entry->value = old_entry->value;
            new_entry->occupied = true;
            new_entry->deleted = false;
            new_size++;
        } else if (old_entry->occupied) {
            /* Free deleted entries */
            free(old_entry->key);
        }
    }

    /* Replace old table */
    free(table->entries);
    table->entries = new_entries;
    table->capacity = new_capacity;
    table->size = new_size;
    table->deleted_count = 0;

    LOG_DEBUG("Hash table resized to capacity %zu", new_capacity);
    return true;
}

HashTable* hash_table_create(size_t initial_capacity) {
    if (initial_capacity < MIN_CAPACITY) {
        initial_capacity = MIN_CAPACITY;
    }

    HashTable* table = malloc(sizeof(HashTable));
    if (!table) {
        LOG_ERROR("Failed to allocate hash table");
        return NULL;
    }

    table->entries = calloc(initial_capacity, sizeof(HashEntry));
    if (!table->entries) {
        LOG_ERROR("Failed to allocate hash table entries");
        free(table);
        return NULL;
    }

    table->capacity = initial_capacity;
    table->size = 0;
    table->deleted_count = 0;

    LOG_DEBUG("Created hash table with capacity %zu", initial_capacity);
    return table;
}

void hash_table_destroy(HashTable* table) {
    if (!table) return;

    /* Free all keys */
    for (size_t i = 0; i < table->capacity; i++) {
        if (table->entries[i].occupied) {
            free(table->entries[i].key);
        }
    }

    free(table->entries);
    free(table);

    LOG_DEBUG("Destroyed hash table");
}

bool hash_table_put(HashTable* table, const char* key, void* value) {
    if (!table || !key) return false;

    /* Check if resize needed */
    float load = (float)(table->size + table->deleted_count) / table->capacity;
    if (load > MAX_LOAD_FACTOR) {
        if (!resize_table(table, table->capacity * 2)) {
            return false;
        }
    }

    /* Find or create entry */
    HashEntry* entry = find_entry(table->entries, table->capacity, key);
    if (!entry) {
        LOG_ERROR("Hash table full (should not happen)");
        return false;
    }

    bool is_new = !entry->occupied || entry->deleted;

    if (is_new) {
        /* New entry */
        entry->key = strdup(key);
        if (!entry->key) {
            LOG_ERROR("Failed to duplicate key string");
            return false;
        }
        entry->occupied = true;
        entry->deleted = false;
        table->size++;
        if (entry->deleted) {
            table->deleted_count--;
        }
    } else {
        /* Update existing */
        LOG_DEBUG("Updated existing key: %s", key);
    }

    entry->value = value;
    return true;
}

void* hash_table_get(const HashTable* table, const char* key) {
    if (!table || !key) return NULL;

    HashEntry* entry = find_entry(table->entries, table->capacity, key);
    if (entry && entry->occupied && !entry->deleted) {
        return entry->value;
    }

    return NULL;
}

bool hash_table_contains(const HashTable* table, const char* key) {
    if (!table || !key) return false;

    HashEntry* entry = find_entry(table->entries, table->capacity, key);
    return entry && entry->occupied && !entry->deleted;
}

void* hash_table_remove(HashTable* table, const char* key) {
    if (!table || !key) return NULL;

    HashEntry* entry = find_entry(table->entries, table->capacity, key);
    if (!entry || !entry->occupied || entry->deleted) {
        return NULL;
    }

    /* Mark as deleted (tombstone) */
    void* value = entry->value;
    entry->deleted = true;
    table->size--;
    table->deleted_count++;

    LOG_DEBUG("Removed key: %s", key);

    /* Check if should shrink */
    if (table->size < table->capacity / 4 && table->capacity > MIN_CAPACITY) {
        resize_table(table, table->capacity / 2);
    }

    return value;
}

void hash_table_clear(HashTable* table) {
    if (!table) return;

    /* Free all keys */
    for (size_t i = 0; i < table->capacity; i++) {
        if (table->entries[i].occupied) {
            free(table->entries[i].key);
            table->entries[i].key = NULL;
            table->entries[i].occupied = false;
            table->entries[i].deleted = false;
        }
    }

    table->size = 0;
    table->deleted_count = 0;

    LOG_DEBUG("Cleared hash table");
}

size_t hash_table_size(const HashTable* table) {
    return table ? table->size : 0;
}

size_t hash_table_capacity(const HashTable* table) {
    return table ? table->capacity : 0;
}

float hash_table_load_factor(const HashTable* table) {
    if (!table || table->capacity == 0) return 0.0f;
    return (float)table->size / table->capacity;
}

void hash_table_foreach(const HashTable* table, HashTableIterator iterator, void* userdata) {
    if (!table || !iterator) return;

    for (size_t i = 0; i < table->capacity; i++) {
        HashEntry* entry = &table->entries[i];
        if (entry->occupied && !entry->deleted) {
            iterator(entry->key, entry->value, userdata);
        }
    }
}

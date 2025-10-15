/**
 * @file memory_fragment.h
 * @brief Memory fragment system for player backstory
 *
 * Memory fragments are pieces of the player's forgotten past, discovered
 * through gameplay. They reveal the player's history, motivations, and
 * connections to NPCs and locations.
 */

#ifndef MEMORY_FRAGMENT_H
#define MEMORY_FRAGMENT_H

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#define MAX_FRAGMENT_CROSS_REFS 8

/**
 * @brief Memory fragment (piece of player's forgotten past)
 */
typedef struct {
    char id[64];                    /**< Unique fragment ID */
    char title[128];                /**< Fragment title */
    char content[1024];             /**< Memory text/content */

    /* Discovery tracking */
    bool discovered;                /**< Has player found this memory? */
    time_t discovery_time;          /**< When it was discovered */
    char discovery_location[64];    /**< Where it was discovered */
    char discovery_method[64];      /**< How: "quest", "explore", "dialogue", etc. */

    /* Categorization */
    char category[64];              /**< Category: "past_life", "death", "family", etc. */
    int chronological_order;        /**< 0 = earliest, higher = more recent */

    /* Cross-references */
    char related_fragments[MAX_FRAGMENT_CROSS_REFS][64];  /**< Related memory IDs */
    size_t related_count;

    char related_npcs[MAX_FRAGMENT_CROSS_REFS][64];       /**< Related NPC IDs */
    size_t npc_count;

    char related_locations[MAX_FRAGMENT_CROSS_REFS][64];  /**< Related location IDs */
    size_t location_count;

    /* Flags */
    bool key_memory;                /**< Critical to main story */
    bool hidden;                    /**< Don't show in list until discovered */
} MemoryFragment;

/**
 * @brief Create a new memory fragment
 *
 * @param id Unique fragment ID
 * @param title Fragment title
 * @param content Memory content/text
 * @return Newly allocated MemoryFragment, or NULL on failure
 */
MemoryFragment* memory_fragment_create(const char* id, const char* title, const char* content);

/**
 * @brief Destroy memory fragment and free memory
 *
 * @param fragment Fragment to destroy (can be NULL)
 */
void memory_fragment_destroy(MemoryFragment* fragment);

/**
 * @brief Mark fragment as discovered
 *
 * @param fragment Memory fragment
 * @param location Where it was discovered
 * @param method How it was discovered
 */
void memory_fragment_discover(MemoryFragment* fragment, const char* location, const char* method);

/**
 * @brief Add related fragment cross-reference
 *
 * @param fragment Memory fragment
 * @param fragment_id Related fragment ID
 */
void memory_fragment_add_related(MemoryFragment* fragment, const char* fragment_id);

/**
 * @brief Add related NPC
 *
 * @param fragment Memory fragment
 * @param npc_id NPC ID mentioned in this memory
 */
void memory_fragment_add_npc(MemoryFragment* fragment, const char* npc_id);

/**
 * @brief Add related location
 *
 * @param fragment Memory fragment
 * @param location_id Location ID mentioned in this memory
 */
void memory_fragment_add_location(MemoryFragment* fragment, const char* location_id);

#endif /* MEMORY_FRAGMENT_H */

/**
 * @file memory_manager.h
 * @brief Manager for all memory fragments
 *
 * Manages collection of memory fragments, discovery, and cross-referencing.
 */

#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include "memory_fragment.h"
#include <stdbool.h>

/**
 * @brief Memory manager - manages all memory fragments
 */
typedef struct {
    MemoryFragment** fragments;     /**< Array of fragment pointers */
    size_t fragment_count;          /**< Number of fragments */
    size_t fragment_capacity;       /**< Allocated capacity */
} MemoryManager;

/**
 * @brief Create memory manager
 *
 * @return Newly allocated MemoryManager, or NULL on failure
 */
MemoryManager* memory_manager_create(void);

/**
 * @brief Destroy memory manager and all fragments
 *
 * @param manager Manager to destroy (can be NULL)
 */
void memory_manager_destroy(MemoryManager* manager);

/**
 * @brief Add fragment to manager
 *
 * Manager takes ownership of the fragment.
 *
 * @param manager Memory manager
 * @param fragment Fragment to add (ownership transferred)
 */
void memory_manager_add_fragment(MemoryManager* manager, MemoryFragment* fragment);

/**
 * @brief Get fragment by ID
 *
 * @param manager Memory manager
 * @param fragment_id Fragment ID to find
 * @return Fragment pointer, or NULL if not found
 */
MemoryFragment* memory_manager_get_fragment(const MemoryManager* manager, const char* fragment_id);

/**
 * @brief Get all discovered fragments
 *
 * Returns array of fragment pointers. Caller must free the array
 * (but NOT the fragments themselves).
 *
 * @param manager Memory manager
 * @param count_out Output: number of fragments
 * @return Array of fragment pointers, or NULL if none discovered
 */
MemoryFragment** memory_manager_get_discovered(const MemoryManager* manager, size_t* count_out);

/**
 * @brief Get fragments by category
 *
 * Returns array of fragment pointers. Caller must free the array
 * (but NOT the fragments themselves).
 *
 * @param manager Memory manager
 * @param category Category to filter (e.g., "past_life", "death")
 * @param count_out Output: number of fragments
 * @return Array of fragment pointers, or NULL if none found
 */
MemoryFragment** memory_manager_get_by_category(const MemoryManager* manager,
                                                 const char* category,
                                                 size_t* count_out);

/**
 * @brief Get fragments in chronological order
 *
 * Returns discovered fragments sorted by chronological_order field.
 * Caller must free the array (but NOT the fragments themselves).
 *
 * @param manager Memory manager
 * @param count_out Output: number of fragments
 * @return Array of fragment pointers sorted by chronological order
 */
MemoryFragment** memory_manager_get_chronological(const MemoryManager* manager, size_t* count_out);

/**
 * @brief Get related fragments (cross-references)
 *
 * Returns fragments referenced by the given fragment.
 * Caller must free the array (but NOT the fragments themselves).
 *
 * @param manager Memory manager
 * @param fragment_id Source fragment ID
 * @param count_out Output: number of related fragments
 * @return Array of related fragment pointers, or NULL if none
 */
MemoryFragment** memory_manager_get_related(const MemoryManager* manager,
                                             const char* fragment_id,
                                             size_t* count_out);

/**
 * @brief Discover a fragment
 *
 * Marks fragment as discovered and records location/method.
 *
 * @param manager Memory manager
 * @param fragment_id Fragment ID to discover
 * @param location Where it was discovered
 * @param method How it was discovered ("quest", "explore", etc.)
 */
void memory_manager_discover_fragment(MemoryManager* manager,
                                      const char* fragment_id,
                                      const char* location,
                                      const char* method);

/**
 * @brief Load fragments from data file
 *
 * Uses data_loader.h to parse memory_fragments.dat file.
 *
 * @param manager Memory manager
 * @param filepath Path to data file
 * @return true on success, false on failure
 */
bool memory_manager_load_from_file(MemoryManager* manager, const char* filepath);

#endif /* MEMORY_MANAGER_H */

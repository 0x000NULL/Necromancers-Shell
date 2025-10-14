#ifndef MINION_MANAGER_H
#define MINION_MANAGER_H

#include "minion.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @file minion_manager.h
 * @brief Minion army management system
 *
 * Manages a collection of minions with querying and filtering capabilities.
 */

/**
 * @brief Opaque minion manager structure
 *
 * Manages a dynamic collection of minions.
 * Implementation details are hidden in the .c file.
 */
typedef struct MinionManager MinionManager;

/**
 * @brief Create a new minion manager
 *
 * @param initial_capacity Initial capacity for minion storage
 * @return Pointer to new minion manager, or NULL on failure
 */
MinionManager* minion_manager_create(size_t initial_capacity);

/**
 * @brief Destroy minion manager and all contained minions
 *
 * @param manager Pointer to minion manager (can be NULL)
 */
void minion_manager_destroy(MinionManager* manager);

/**
 * @brief Add a minion to the manager
 *
 * The manager takes ownership of the minion.
 * The minion pointer should not be used after this call unless retrieved again.
 *
 * @param manager Pointer to minion manager
 * @param minion Pointer to minion to add
 * @return true on success, false on failure
 */
bool minion_manager_add(MinionManager* manager, Minion* minion);

/**
 * @brief Remove a minion from the manager by ID
 *
 * Transfers ownership back to caller - caller must destroy the returned minion.
 *
 * @param manager Pointer to minion manager
 * @param minion_id ID of minion to remove
 * @return Pointer to removed minion, or NULL if not found
 */
Minion* minion_manager_remove(MinionManager* manager, uint32_t minion_id);

/**
 * @brief Get a minion by ID
 *
 * Returns a pointer to the minion in the manager's collection.
 * The pointer remains valid until the minion is removed or manager is destroyed.
 *
 * @param manager Pointer to minion manager
 * @param minion_id ID of minion to get
 * @return Pointer to minion, or NULL if not found
 */
Minion* minion_manager_get(MinionManager* manager, uint32_t minion_id);

/**
 * @brief Get minion at specific index
 *
 * Returns a pointer to the minion at the given index.
 * Useful for iterating through all minions.
 *
 * @param manager Pointer to minion manager
 * @param index Index of minion (0 to count-1)
 * @return Pointer to minion, or NULL if index out of bounds
 */
Minion* minion_manager_get_at(MinionManager* manager, size_t index);

/**
 * @brief Get all minions at a specific location
 *
 * Returns an array of pointers to minions at the given location.
 * The caller must free the returned array (but not the minions themselves).
 *
 * @param manager Pointer to minion manager
 * @param location_id Location ID to filter by
 * @param count_out Pointer to store count of returned minions
 * @return Array of minion pointers, or NULL on failure. Must be freed by caller.
 */
Minion** minion_manager_get_at_location(MinionManager* manager, uint32_t location_id, size_t* count_out);

/**
 * @brief Get total number of minions
 *
 * @param manager Pointer to minion manager
 * @return Total count of minions, or 0 if manager is NULL
 */
size_t minion_manager_count(MinionManager* manager);

/**
 * @brief Get count of minions by type
 *
 * @param manager Pointer to minion manager
 * @param type Minion type to count
 * @return Count of minions of the given type
 */
size_t minion_manager_count_by_type(MinionManager* manager, MinionType type);

/**
 * @brief Get count of minions at a specific location
 *
 * @param manager Pointer to minion manager
 * @param location_id Location ID to count
 * @return Count of minions at the location
 */
size_t minion_manager_count_at_location(MinionManager* manager, uint32_t location_id);

/**
 * @brief Clear all minions from the manager
 *
 * Destroys all minions in the collection.
 *
 * @param manager Pointer to minion manager
 */
void minion_manager_clear(MinionManager* manager);

#endif /* MINION_MANAGER_H */

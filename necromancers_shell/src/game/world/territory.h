/**
 * @file territory.h
 * @brief Territory manager for tracking all game locations
 *
 * Manages collection of locations, discovery, and spatial relationships.
 */

#ifndef NECROMANCER_TERRITORY_H
#define NECROMANCER_TERRITORY_H

#include "location.h"
#include <stddef.h>
#include <stdbool.h>

/**
 * @brief Opaque territory manager structure
 */
typedef struct TerritoryManager TerritoryManager;

/**
 * @brief Create a new territory manager
 *
 * @return Newly allocated TerritoryManager, or NULL on failure
 */
TerritoryManager* territory_manager_create(void);

/**
 * @brief Destroy territory manager and all its locations
 *
 * @param manager Manager to destroy (can be NULL)
 */
void territory_manager_destroy(TerritoryManager* manager);

/**
 * @brief Add a location to the territory
 *
 * The manager takes ownership of the location.
 *
 * @param manager Territory manager
 * @param location Location to add
 * @return true on success, false on failure
 */
bool territory_manager_add_location(TerritoryManager* manager, Location* location);

/**
 * @brief Get location by ID
 *
 * @param manager Territory manager
 * @param id Location ID
 * @return Pointer to location, or NULL if not found
 */
Location* territory_manager_get_location(const TerritoryManager* manager, uint32_t id);

/**
 * @brief Get location by name
 *
 * @param manager Territory manager
 * @param name Location name
 * @return Pointer to location, or NULL if not found
 */
Location* territory_manager_get_location_by_name(const TerritoryManager* manager,
                                                  const char* name);

/**
 * @brief Get all discovered locations
 *
 * @param manager Territory manager
 * @param results Pointer to array of location pointers (caller allocates)
 * @param count Pointer to store number of results
 * @return true on success, false on failure
 */
bool territory_manager_get_discovered(const TerritoryManager* manager,
                                       Location*** results,
                                       size_t* count);

/**
 * @brief Get total number of locations
 *
 * @param manager Territory manager
 * @return Number of locations
 */
size_t territory_manager_count(const TerritoryManager* manager);

/**
 * @brief Get number of discovered locations
 *
 * @param manager Territory manager
 * @return Number of discovered locations
 */
size_t territory_manager_count_discovered(const TerritoryManager* manager);

/**
 * @brief Free results array from territory_manager_get_discovered
 *
 * @param results Results array to free
 */
void territory_manager_free_results(Location** results);

/**
 * @brief Load locations from file (STUB implementation)
 *
 * Currently creates hardcoded locations for testing.
 * Future: Parse JSON file.
 *
 * @param manager Territory manager
 * @param filepath Path to locations file (ignored for now)
 * @return Number of locations loaded
 */
size_t territory_manager_load_from_file(TerritoryManager* manager,
                                         const char* filepath);

/**
 * @brief Clear all locations from manager
 *
 * @param manager Territory manager
 */
void territory_manager_clear(TerritoryManager* manager);

#endif /* NECROMANCER_TERRITORY_H */

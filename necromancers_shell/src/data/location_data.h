#ifndef LOCATION_DATA_H
#define LOCATION_DATA_H

#include <stddef.h>
#include "../game/world/location.h"
#include "../game/world/territory.h"
#include "data_loader.h"

/**
 * @file location_data.h
 * @brief Load location definitions from data files
 *
 * Reads locations from data/locations.dat and creates Location objects
 * that can be added to the TerritoryManager.
 */

/**
 * @brief Load all locations from data file into territory manager
 *
 * Reads all [LOCATION:*] sections from the data file and creates
 * Location objects. Also builds the location graph connections.
 *
 * @param territory Territory manager to populate
 * @param data_file Loaded data file
 * @return Number of locations loaded, or 0 on error
 */
size_t location_data_load_all(TerritoryManager* territory, const DataFile* data_file);

/**
 * @brief Create a single Location from a data section
 *
 * Parses all properties from the section and creates a Location object.
 * Does not add it to the territory manager.
 *
 * @param section Data section with location properties
 * @return Newly created Location, or NULL on error
 */
Location* location_data_create_from_section(const DataSection* section);

/**
 * @brief Build location graph connections from data
 *
 * Parses the "connections" property from each location and builds
 * the graph connections in the territory manager.
 *
 * Must be called after all locations are loaded.
 *
 * @param territory Territory manager with loaded locations
 * @param data_file Data file to read connection data from
 * @return Number of connections created
 */
size_t location_data_build_connections(TerritoryManager* territory, const DataFile* data_file);

#endif /* LOCATION_DATA_H */

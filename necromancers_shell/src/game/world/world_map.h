/**
 * @file world_map.h
 * @brief World map visualization and management system
 *
 * Provides ASCII map rendering, coordinate mapping, and region management
 * for the game world. Wraps TerritoryManager and LocationGraph for enhanced
 * visualization and navigation.
 */

#ifndef NECROMANCER_WORLD_MAP_H
#define NECROMANCER_WORLD_MAP_H

#include "territory.h"
#include "location_graph.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @brief 2D map coordinates
 */
typedef struct {
    int16_t x;  /**< X coordinate (-1000 to 1000) */
    int16_t y;  /**< Y coordinate (-1000 to 1000) */
} MapCoordinates;

/**
 * @brief Map regions for grouping locations
 */
typedef enum {
    MAP_REGION_STARTING_GROUNDS = 0,  /**< Safe starting area */
    MAP_REGION_EASTERN_WASTES,        /**< Barren battlefields */
    MAP_REGION_SOUTHERN_VILLAGES,     /**< Populated settlements */
    MAP_REGION_WESTERN_CRYPTS,        /**< Ancient burial grounds */
    MAP_REGION_NORTHERN_RITUAL,       /**< Corrupted magical sites */
    MAP_REGION_CENTRAL_NEXUS,         /**< Death Network hub */
    MAP_REGION_COUNT
} MapRegion;

/**
 * @brief Location metadata for map display
 */
typedef struct {
    uint32_t location_id;       /**< Associated location ID */
    MapCoordinates coords;      /**< Map coordinates */
    MapRegion region;           /**< Region this location belongs to */
    char symbol;                /**< ASCII symbol for this location */
    bool discovered;            /**< Whether location is discovered */
} LocationMapData;

/**
 * @brief World map structure (opaque)
 */
typedef struct WorldMap WorldMap;

/**
 * @brief Map rendering options
 */
typedef struct {
    uint16_t width;                     /**< Map width in characters (default: 60) */
    uint16_t height;                    /**< Map height in characters (default: 24) */
    bool show_undiscovered;             /**< Show undiscovered locations as '?' */
    bool show_connections;              /**< Draw lines between locations */
    bool show_region_labels;            /**< Display region names */
    bool show_legend;                   /**< Display symbol legend */
    uint32_t highlight_location_id;     /**< Location to highlight (0 = none) */
    uint32_t* highlight_path;           /**< Path to highlight (NULL = none) */
    size_t highlight_path_length;       /**< Length of highlight path */
} MapRenderOptions;

/**
 * @brief Create default map render options
 *
 * @return Default MapRenderOptions structure
 */
MapRenderOptions map_render_options_default(void);

/**
 * @brief Create world map instance
 *
 * Wraps existing TerritoryManager and LocationGraph for visualization.
 * The WorldMap does NOT take ownership of these structures.
 *
 * @param territory Territory manager (not owned)
 * @param graph Location graph (not owned)
 * @return Newly allocated WorldMap, or NULL on failure
 */
WorldMap* world_map_create(TerritoryManager* territory, LocationGraph* graph);

/**
 * @brief Destroy world map and free resources
 *
 * Does NOT destroy the underlying TerritoryManager or LocationGraph.
 *
 * @param map World map to destroy (can be NULL)
 */
void world_map_destroy(WorldMap* map);

/**
 * @brief Set map coordinates for a location
 *
 * @param map World map
 * @param location_id Location ID
 * @param x X coordinate
 * @param y Y coordinate
 * @return true on success, false if location not found
 */
bool world_map_set_coordinates(WorldMap* map, uint32_t location_id,
                                int16_t x, int16_t y);

/**
 * @brief Get map coordinates for a location
 *
 * @param map World map
 * @param location_id Location ID
 * @param coords Output coordinates (can be NULL to just check existence)
 * @return true if coordinates exist, false otherwise
 */
bool world_map_get_coordinates(const WorldMap* map, uint32_t location_id,
                                MapCoordinates* coords);

/**
 * @brief Set region for a location
 *
 * @param map World map
 * @param location_id Location ID
 * @param region Region to assign
 * @return true on success, false if location not found
 */
bool world_map_set_region(WorldMap* map, uint32_t location_id, MapRegion region);

/**
 * @brief Get region for a location
 *
 * @param map World map
 * @param location_id Location ID
 * @return Region, or MAP_REGION_STARTING_GROUNDS if not found
 */
MapRegion world_map_get_region(const WorldMap* map, uint32_t location_id);

/**
 * @brief Set custom symbol for a location
 *
 * By default, symbols are based on location type:
 * - G = Graveyard
 * - B = Battlefield
 * - V = Village
 * - C = Crypt
 * - R = Ritual Site
 * - @ = Current location
 * - ? = Undiscovered
 *
 * @param map World map
 * @param location_id Location ID
 * @param symbol ASCII symbol to use
 * @return true on success, false if location not found
 */
bool world_map_set_symbol(WorldMap* map, uint32_t location_id, char symbol);

/**
 * @brief Get all locations in a region
 *
 * @param map World map
 * @param region Region to query
 * @param results Output array of location IDs (caller allocates)
 * @param max_results Maximum number of results
 * @return Number of locations found
 */
size_t world_map_get_locations_in_region(const WorldMap* map, MapRegion region,
                                          uint32_t* results, size_t max_results);

/**
 * @brief Get locations within Manhattan distance radius
 *
 * @param map World map
 * @param center_id Center location ID
 * @param radius Maximum distance
 * @param results Output array of location IDs (caller allocates)
 * @param max_results Maximum number of results
 * @return Number of locations found
 */
size_t world_map_get_locations_in_radius(const WorldMap* map,
                                          uint32_t center_id,
                                          uint16_t radius,
                                          uint32_t* results,
                                          size_t max_results);

/**
 * @brief Render ASCII map to string buffer
 *
 * Generates a text-based visualization of the world map with:
 * - Location symbols based on type
 * - Box-drawing characters for connections (─│┌┐└┘├┤┬┴┼)
 * - Region boundaries
 * - Current location highlighting (@)
 * - Optional path highlighting
 * - Optional legend
 *
 * @param map World map
 * @param current_location_id Current player location (highlighted as @)
 * @param options Rendering options
 * @param buffer Output buffer (caller allocates)
 * @param buffer_size Size of output buffer
 * @return Number of characters written (excluding null terminator), or 0 on failure
 */
size_t world_map_render(const WorldMap* map,
                         uint32_t current_location_id,
                         const MapRenderOptions* options,
                         char* buffer,
                         size_t buffer_size);

/**
 * @brief Get legend string for map symbols
 *
 * @param buffer Output buffer (caller allocates)
 * @param buffer_size Size of output buffer
 * @return Number of characters written
 */
size_t world_map_get_legend(char* buffer, size_t buffer_size);

/**
 * @brief Get string name of map region
 *
 * @param region Map region enum
 * @return String name
 */
const char* world_map_region_name(MapRegion region);

/**
 * @brief Auto-layout locations using force-directed graph layout
 *
 * Automatically assigns coordinates to all locations based on their
 * connections, attempting to minimize edge crossings.
 *
 * @param map World map
 * @param iterations Number of layout iterations (default: 100)
 * @return true on success
 */
bool world_map_auto_layout(WorldMap* map, uint32_t iterations);

/**
 * @brief Calculate bounding box of all locations
 *
 * @param map World map
 * @param min_x Output minimum X coordinate
 * @param max_x Output maximum X coordinate
 * @param min_y Output minimum Y coordinate
 * @param max_y Output maximum Y coordinate
 * @return true if locations exist, false if empty
 */
bool world_map_get_bounds(const WorldMap* map,
                           int16_t* min_x, int16_t* max_x,
                           int16_t* min_y, int16_t* max_y);

#endif /* NECROMANCER_WORLD_MAP_H */

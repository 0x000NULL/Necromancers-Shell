/**
 * @file location_graph.h
 * @brief Location graph system for world connectivity
 *
 * Manages connections between locations as a weighted directed graph.
 * Provides pathfinding algorithms for navigation between locations.
 *
 * Integrates with existing Location and Territory systems from Phase 2.
 */

#ifndef NECROMANCER_LOCATION_GRAPH_H
#define NECROMANCER_LOCATION_GRAPH_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Connection between two locations
 *
 * Represents an edge in the location graph with metadata about
 * the connection (distance, danger level, unlock requirements).
 */
typedef struct {
    uint32_t from_location_id;      /**< Source location ID */
    uint32_t to_location_id;        /**< Destination location ID */
    uint8_t travel_time_hours;      /**< Travel time in game hours (1-10) */
    uint8_t danger_level;           /**< Danger level 0-100 (affects encounters) */
    bool requires_unlock;           /**< Whether connection is locked */
    char unlock_requirement[64];    /**< Required quest/skill ID to unlock */
} LocationConnection;

/**
 * @brief Pathfinding result
 *
 * Contains the calculated path between two locations and metadata.
 */
typedef struct {
    uint32_t* path;                 /**< Array of location IDs forming the path */
    size_t path_length;             /**< Number of locations in path */
    uint32_t total_travel_time;     /**< Sum of travel times in hours */
    uint32_t total_danger;          /**< Sum of danger levels along path */
    bool path_found;                /**< Whether a valid path exists */
} PathfindingResult;

/**
 * @brief Location graph structure
 *
 * Stores all connections between locations in the game world.
 * Uses adjacency list representation for efficient pathfinding.
 */
typedef struct LocationGraph LocationGraph;

/**
 * @brief Create a new location graph
 *
 * @return Newly allocated LocationGraph, or NULL on failure
 */
LocationGraph* location_graph_create(void);

/**
 * @brief Destroy location graph and free all resources
 *
 * @param graph Graph to destroy (can be NULL)
 */
void location_graph_destroy(LocationGraph* graph);

/**
 * @brief Add a connection between two locations
 *
 * Creates a directed edge from 'from' to 'to' with the specified metadata.
 * For bidirectional connections, call twice (once each direction).
 *
 * @param graph Location graph
 * @param from_id Source location ID
 * @param to_id Destination location ID
 * @param travel_time Travel time in hours
 * @param danger_level Danger level (0-100)
 * @return true on success, false on failure
 */
bool location_graph_add_connection(LocationGraph* graph,
                                    uint32_t from_id,
                                    uint32_t to_id,
                                    uint8_t travel_time,
                                    uint8_t danger_level);

/**
 * @brief Add a bidirectional connection between two locations
 *
 * Convenience function that adds connections in both directions.
 *
 * @param graph Location graph
 * @param location_a First location ID
 * @param location_b Second location ID
 * @param travel_time Travel time in hours (same both directions)
 * @param danger_level Danger level (0-100, same both directions)
 * @return true on success, false on failure
 */
bool location_graph_add_bidirectional(LocationGraph* graph,
                                       uint32_t location_a,
                                       uint32_t location_b,
                                       uint8_t travel_time,
                                       uint8_t danger_level);

/**
 * @brief Set unlock requirement for a connection
 *
 * @param graph Location graph
 * @param from_id Source location ID
 * @param to_id Destination location ID
 * @param requirement Quest ID or skill name required to unlock
 * @return true on success, false if connection not found
 */
bool location_graph_set_unlock_requirement(LocationGraph* graph,
                                            uint32_t from_id,
                                            uint32_t to_id,
                                            const char* requirement);

/**
 * @brief Check if a connection exists between two locations
 *
 * @param graph Location graph
 * @param from_id Source location ID
 * @param to_id Destination location ID
 * @return true if direct connection exists, false otherwise
 */
bool location_graph_has_connection(const LocationGraph* graph,
                                    uint32_t from_id,
                                    uint32_t to_id);

/**
 * @brief Get all neighbors of a location
 *
 * Returns all locations directly connected from the given location.
 *
 * @param graph Location graph
 * @param location_id Location to query
 * @param neighbors Output array of neighbor location IDs (caller allocates)
 * @param max_neighbors Maximum number of neighbors to return
 * @return Number of neighbors found (may be less than max_neighbors)
 */
size_t location_graph_get_neighbors(const LocationGraph* graph,
                                     uint32_t location_id,
                                     uint32_t* neighbors,
                                     size_t max_neighbors);

/**
 * @brief Get connection information between two locations
 *
 * @param graph Location graph
 * @param from_id Source location ID
 * @param to_id Destination location ID
 * @param connection Output connection data (can be NULL to just check existence)
 * @return true if connection exists, false otherwise
 */
bool location_graph_get_connection(const LocationGraph* graph,
                                    uint32_t from_id,
                                    uint32_t to_id,
                                    LocationConnection* connection);

/**
 * @brief Find shortest path between two locations using Dijkstra's algorithm
 *
 * Uses travel time as edge weight. Ignores locked connections unless
 * player has met unlock requirements.
 *
 * The returned PathfindingResult must be freed with pathfinding_result_free().
 *
 * @param graph Location graph
 * @param from_id Starting location ID
 * @param to_id Target location ID
 * @param result Output pathfinding result
 * @return true on success (path may or may not exist), false on error
 */
bool location_graph_find_path(const LocationGraph* graph,
                               uint32_t from_id,
                               uint32_t to_id,
                               PathfindingResult* result);

/**
 * @brief Free pathfinding result
 *
 * Frees the path array allocated during pathfinding.
 *
 * @param result Result to free (can be NULL)
 */
void pathfinding_result_free(PathfindingResult* result);

/**
 * @brief Check if location is reachable from starting location
 *
 * Uses breadth-first search to determine reachability.
 *
 * @param graph Location graph
 * @param from_id Starting location ID
 * @param to_id Target location ID
 * @return true if path exists (regardless of unlock status), false otherwise
 */
bool location_graph_is_reachable(const LocationGraph* graph,
                                  uint32_t from_id,
                                  uint32_t to_id);

/**
 * @brief Get total number of connections in the graph
 *
 * @param graph Location graph
 * @return Number of directed connections
 */
size_t location_graph_get_connection_count(const LocationGraph* graph);

/**
 * @brief Get all unique location IDs in the graph
 *
 * Returns all locations that have at least one connection (incoming or outgoing).
 *
 * @param graph Location graph
 * @param locations Output array of location IDs (caller allocates)
 * @param max_locations Maximum number of locations to return
 * @return Number of unique locations found
 */
size_t location_graph_get_all_locations(const LocationGraph* graph,
                                         uint32_t* locations,
                                         size_t max_locations);

/**
 * @brief Validate graph connectivity
 *
 * Ensures all locations in the graph are reachable from a starting location.
 *
 * @param graph Location graph
 * @param starting_location_id Starting location for validation
 * @return true if all locations are reachable, false otherwise
 */
bool location_graph_validate_connectivity(const LocationGraph* graph,
                                           uint32_t starting_location_id);

#endif /* NECROMANCER_LOCATION_GRAPH_H */

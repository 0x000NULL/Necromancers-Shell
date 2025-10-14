/**
 * @file location.h
 * @brief Location system for game world
 *
 * Represents discoverable locations in the game world. Each location has a type,
 * resources (corpses), control level, and connections to other locations.
 */

#ifndef NECROMANCER_LOCATION_H
#define NECROMANCER_LOCATION_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Types of locations in the game world
 */
typedef enum {
    LOCATION_TYPE_GRAVEYARD = 0,    /**< Cemetery with many corpses */
    LOCATION_TYPE_BATTLEFIELD,      /**< War site with warrior souls */
    LOCATION_TYPE_VILLAGE,          /**< Living settlement (innocent souls) */
    LOCATION_TYPE_CRYPT,            /**< Ancient tomb with powerful souls */
    LOCATION_TYPE_RITUAL_SITE,      /**< Magical site with corrupted energy */
    LOCATION_TYPE_COUNT
} LocationType;

/**
 * @brief Status of location discovery and control
 */
typedef enum {
    LOCATION_STATUS_UNDISCOVERED,   /**< Not yet discovered */
    LOCATION_STATUS_DISCOVERED,     /**< Discovered but not controlled */
    LOCATION_STATUS_CONTROLLED,     /**< Under player control */
    LOCATION_STATUS_HOSTILE         /**< Hostile forces present */
} LocationStatus;

/**
 * @brief Represents a location in the game world
 */
typedef struct Location {
    uint32_t id;                    /**< Unique location ID */
    char name[64];                  /**< Location name */
    LocationType type;              /**< Type of location */
    LocationStatus status;          /**< Current status */
    char description[512];          /**< Descriptive text */
    uint32_t corpse_count;          /**< Available corpses for harvesting */
    uint32_t soul_quality_avg;      /**< Average soul quality (0-100) */
    uint8_t control_level;          /**< Control percentage (0-100) */
    uint32_t defense_strength;      /**< Defense level */
    uint32_t* connected_ids;        /**< Array of connected location IDs */
    size_t connection_count;        /**< Number of connections */
    size_t connection_capacity;     /**< Capacity of connections array */
    bool discovered;                /**< Whether location has been discovered */
    uint64_t discovered_timestamp;  /**< When location was discovered (game time) */
} Location;

/**
 * @brief Create a new location
 *
 * @param id Unique location ID
 * @param name Location name (max 63 chars)
 * @param type Location type
 * @return Newly allocated Location, or NULL on failure
 */
Location* location_create(uint32_t id, const char* name, LocationType type);

/**
 * @brief Destroy a location and free its memory
 *
 * @param location Location to destroy (can be NULL)
 */
void location_destroy(Location* location);

/**
 * @brief Get string name of location type
 *
 * @param type Location type
 * @return String name of type
 */
const char* location_type_name(LocationType type);

/**
 * @brief Get string name of location status
 *
 * @param status Location status
 * @return String name of status
 */
const char* location_status_name(LocationStatus status);

/**
 * @brief Add a connection to another location
 *
 * @param location Location to modify
 * @param connected_id ID of location to connect to
 * @return true on success, false on failure
 */
bool location_add_connection(Location* location, uint32_t connected_id);

/**
 * @brief Check if location is connected to another
 *
 * @param location Location to check
 * @param other_id ID of other location
 * @return true if connected, false otherwise
 */
bool location_is_connected(const Location* location, uint32_t other_id);

/**
 * @brief Mark location as discovered
 *
 * @param location Location to discover
 * @param timestamp Current game time
 */
void location_discover(Location* location, uint64_t timestamp);

/**
 * @brief Harvest corpses from location
 *
 * Removes up to max_count corpses from the location.
 *
 * @param location Location to harvest from
 * @param max_count Maximum corpses to harvest
 * @return Actual number of corpses harvested
 */
uint32_t location_harvest_corpses(Location* location, uint32_t max_count);

/**
 * @brief Increase control level of location
 *
 * @param location Location to modify
 * @param amount Amount to increase (0-100)
 * @return New control level
 */
uint8_t location_increase_control(Location* location, uint8_t amount);

/**
 * @brief Decrease control level of location
 *
 * @param location Location to modify
 * @param amount Amount to decrease (0-100)
 * @return New control level
 */
uint8_t location_decrease_control(Location* location, uint8_t amount);

/**
 * @brief Get descriptive string for location
 *
 * @param location Location to describe
 * @param buffer Buffer to write to
 * @param buffer_size Size of buffer
 * @return Number of characters written
 */
size_t location_get_description_formatted(const Location* location, char* buffer, size_t buffer_size);

#endif /* NECROMANCER_LOCATION_H */

/**
 * @file death_network.h
 * @brief Death Network system for corpse generation and death tracking
 *
 * The Death Network simulates the flow of death energy across the game world.
 * Each location has a "death signature" that determines corpse availability,
 * quality, and regeneration rate. The network creates dynamic gameplay where
 * corpses are generated based on location type, recent events, and time passage.
 *
 * Key concepts:
 * - Death Nodes: Each location has a node tracking death activity
 * - Death Flow: Energy flows between connected locations
 * - Corpse Generation: Automatic generation based on node strength
 * - Death Events: Battles, plagues, natural deaths affecting the network
 * - Scanning: Players can scan the network to find rich corpse sources
 */

#ifndef NECROMANCER_DEATH_NETWORK_H
#define NECROMANCER_DEATH_NETWORK_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Death signature strength (0-100)
 *
 * Represents the intensity of death energy at a location:
 * - 0-20: Dormant (very few corpses)
 * - 21-40: Weak (occasional deaths)
 * - 41-60: Moderate (steady supply)
 * - 61-80: Strong (frequent deaths)
 * - 81-100: Overwhelming (mass death events)
 */
typedef uint8_t DeathSignature;

/**
 * @brief Types of death events
 */
typedef enum {
    DEATH_EVENT_NATURAL = 0,    /**< Natural causes (age, disease) */
    DEATH_EVENT_BATTLE,         /**< Combat deaths */
    DEATH_EVENT_PLAGUE,         /**< Mass disease outbreak */
    DEATH_EVENT_EXECUTION,      /**< Judicial killings */
    DEATH_EVENT_SACRIFICE,      /**< Ritual sacrifices */
    DEATH_EVENT_ACCIDENT,       /**< Accidents (construction, mining) */
    DEATH_EVENT_MURDER,         /**< Criminal killings */
    DEATH_EVENT_COUNT
} DeathEventType;

/**
 * @brief Death quality affecting soul value
 */
typedef enum {
    DEATH_QUALITY_POOR = 0,     /**< Common souls (10 energy) */
    DEATH_QUALITY_AVERAGE,      /**< Average souls (20 energy) */
    DEATH_QUALITY_GOOD,         /**< Quality souls (35 energy) */
    DEATH_QUALITY_EXCELLENT,    /**< Warrior/mage souls (50 energy) */
    DEATH_QUALITY_LEGENDARY     /**< Ancient souls (100 energy) */
} DeathQuality;

/**
 * @brief Death node representing a location's death activity
 */
typedef struct {
    uint32_t location_id;           /**< Associated location ID */
    DeathSignature signature;       /**< Current death energy (0-100) */
    DeathSignature base_signature;  /**< Base signature for this location type */

    /* Corpse tracking */
    uint32_t available_corpses;     /**< Current corpses available */
    uint32_t max_corpses;           /**< Maximum corpse capacity */
    uint32_t total_deaths;          /**< Total deaths since game start */

    /* Quality distribution */
    uint8_t quality_poor;           /**< % chance for poor quality (0-100) */
    uint8_t quality_average;        /**< % chance for average quality */
    uint8_t quality_good;           /**< % chance for good quality */
    uint8_t quality_excellent;      /**< % chance for excellent quality */
    uint8_t quality_legendary;      /**< % chance for legendary quality */

    /* Regeneration */
    uint32_t hours_since_harvest;   /**< Hours since last harvest */
    uint8_t regen_rate;             /**< Corpses regenerated per day (0-10) */

    /* Recent events */
    DeathEventType last_event_type; /**< Most recent death event */
    uint32_t hours_since_event;     /**< Hours since last major event */

    /* Network flow */
    bool is_active;                 /**< Whether node is actively generating */
    uint8_t flow_strength;          /**< Energy flow to connected nodes (0-100) */
} DeathNode;

/**
 * @brief Death event for network propagation
 */
typedef struct {
    uint32_t location_id;       /**< Location where event occurred */
    DeathEventType type;        /**< Type of death event */
    uint32_t death_count;       /**< Number of deaths */
    DeathQuality avg_quality;   /**< Average quality of deaths */
    uint32_t timestamp_hours;   /**< Game time when event occurred */
} DeathEvent;

/**
 * @brief Death Network structure (opaque)
 */
typedef struct DeathNetwork DeathNetwork;

/**
 * @brief Create a new death network
 *
 * @return Newly allocated DeathNetwork, or NULL on failure
 */
DeathNetwork* death_network_create(void);

/**
 * @brief Destroy death network and free all resources
 *
 * @param network Network to destroy (can be NULL)
 */
void death_network_destroy(DeathNetwork* network);

/**
 * @brief Add a location to the death network
 *
 * Creates a death node for the location with appropriate default values
 * based on location type (graveyard, battlefield, etc.).
 *
 * @param network Death network
 * @param location_id Location ID to add
 * @param base_signature Base death energy for this location (0-100)
 * @param max_corpses Maximum corpse capacity
 * @param regen_rate Daily corpse regeneration rate
 * @return true on success, false on failure
 */
bool death_network_add_location(DeathNetwork* network,
                                 uint32_t location_id,
                                 DeathSignature base_signature,
                                 uint32_t max_corpses,
                                 uint8_t regen_rate);

/**
 * @brief Get death node for a location
 *
 * @param network Death network
 * @param location_id Location ID
 * @return Pointer to death node, or NULL if not found
 */
const DeathNode* death_network_get_node(const DeathNetwork* network,
                                         uint32_t location_id);

/**
 * @brief Update death network (called each game hour)
 *
 * Processes:
 * - Corpse regeneration based on time passage
 * - Death signature decay
 * - Network flow between connected locations
 * - Random death events
 *
 * @param network Death network
 * @param hours_passed Number of game hours passed
 */
void death_network_update(DeathNetwork* network, uint32_t hours_passed);

/**
 * @brief Trigger a death event at a location
 *
 * Manually trigger a death event (from quests, combat, etc.).
 * Updates death signature, generates corpses, and propagates energy.
 *
 * @param network Death network
 * @param event Death event details
 * @return true on success, false if location not found
 */
bool death_network_trigger_event(DeathNetwork* network, const DeathEvent* event);

/**
 * @brief Harvest corpses from a location
 *
 * Reduces available corpses and returns quality distribution.
 *
 * @param network Death network
 * @param location_id Location to harvest from
 * @param count Number of corpses to harvest
 * @param qualities Output array for quality of each corpse (caller allocates)
 * @return Number of corpses actually harvested (may be less than requested)
 */
uint32_t death_network_harvest_corpses(DeathNetwork* network,
                                        uint32_t location_id,
                                        uint32_t count,
                                        DeathQuality* qualities);

/**
 * @brief Scan death network around a location
 *
 * Returns all locations within network range sorted by death signature.
 * Used for the 'scan' command to find corpse-rich locations.
 *
 * @param network Death network
 * @param center_location_id Location to scan from
 * @param results Output array of location IDs (caller allocates)
 * @param max_results Maximum number of results
 * @return Number of locations found
 */
size_t death_network_scan(const DeathNetwork* network,
                          uint32_t center_location_id,
                          uint32_t* results,
                          size_t max_results);

/**
 * @brief Get locations with highest death signatures
 *
 * @param network Death network
 * @param results Output array of location IDs (caller allocates)
 * @param max_results Maximum number of results
 * @return Number of locations found
 */
size_t death_network_get_hotspots(const DeathNetwork* network,
                                   uint32_t* results,
                                   size_t max_results);

/**
 * @brief Calculate death quality from node
 *
 * Randomly selects a quality tier based on node's quality distribution.
 *
 * @param node Death node
 * @return Quality tier for generated corpse
 */
DeathQuality death_network_roll_quality(const DeathNode* node);

/**
 * @brief Get soul energy value for death quality
 *
 * @param quality Death quality tier
 * @return Soul energy value
 */
uint32_t death_quality_to_soul_energy(DeathQuality quality);

/**
 * @brief Get string name for death event type
 *
 * @param type Death event type
 * @return String name
 */
const char* death_event_type_name(DeathEventType type);

/**
 * @brief Get string name for death quality
 *
 * @param quality Death quality tier
 * @return String name
 */
const char* death_quality_name(DeathQuality quality);

/**
 * @brief Get death signature strength description
 *
 * @param signature Death signature value (0-100)
 * @return Description string ("Dormant", "Weak", etc.)
 */
const char* death_signature_description(DeathSignature signature);

/**
 * @brief Set quality distribution for a location
 *
 * Sets the probability distribution for corpse quality at a location.
 * Percentages must sum to 100.
 *
 * @param network Death network
 * @param location_id Location ID
 * @param poor Percentage for poor quality (0-100)
 * @param average Percentage for average quality (0-100)
 * @param good Percentage for good quality (0-100)
 * @param excellent Percentage for excellent quality (0-100)
 * @param legendary Percentage for legendary quality (0-100)
 * @return true on success, false if percentages don't sum to 100
 */
bool death_network_set_quality_distribution(DeathNetwork* network,
                                             uint32_t location_id,
                                             uint8_t poor,
                                             uint8_t average,
                                             uint8_t good,
                                             uint8_t excellent,
                                             uint8_t legendary);

/**
 * @brief Get total network activity level
 *
 * Calculates average death signature across all nodes.
 *
 * @param network Death network
 * @return Average signature (0-100), or 0 if network is empty
 */
DeathSignature death_network_get_activity_level(const DeathNetwork* network);

/**
 * @brief Get total corpses available in network
 *
 * @param network Death network
 * @return Total corpse count
 */
uint32_t death_network_get_total_corpses(const DeathNetwork* network);

/**
 * @brief Get statistics for death network
 *
 * @param network Death network
 * @param total_locations Output: total locations in network
 * @param total_corpses Output: total available corpses
 * @param total_deaths Output: total deaths tracked
 * @param avg_signature Output: average death signature
 */
void death_network_get_stats(const DeathNetwork* network,
                             size_t* total_locations,
                             uint32_t* total_corpses,
                             uint32_t* total_deaths,
                             DeathSignature* avg_signature);

#endif /* NECROMANCER_DEATH_NETWORK_H */

/**
 * @file territory_status.h
 * @brief Territory control, faction influence, and alert level system
 *
 * Manages dynamic territory control mechanics including:
 * - Player vs hostile faction control percentages
 * - Alert levels that affect detection and enemy response
 * - Territory stability and defense strength
 * - Resource generation modifiers based on control
 */

#ifndef NECROMANCER_TERRITORY_STATUS_H
#define NECROMANCER_TERRITORY_STATUS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Alert levels for territories
 *
 * Alert levels increase when player performs hostile actions and decrease
 * over time. Higher alerts mean stronger enemy response and reduced rewards.
 */
typedef enum {
    ALERT_NONE = 0,       /**< No alert, normal operations */
    ALERT_LOW,            /**< Minor suspicion, +10% detection */
    ALERT_MEDIUM,         /**< Moderate alert, +25% detection, -10% rewards */
    ALERT_HIGH,           /**< High alert, +50% detection, -25% rewards */
    ALERT_CRITICAL,       /**< Critical alert, +100% detection, -50% rewards, reinforcements */
    ALERT_LEVEL_COUNT
} AlertLevel;

/**
 * @brief Faction types that can control territory
 */
typedef enum {
    FACTION_PLAYER = 0,   /**< Player-controlled undead forces */
    FACTION_LIVING,       /**< Living defenders (villages, guards) */
    FACTION_HUNTERS,      /**< Necromancer hunters */
    FACTION_RIVAL,        /**< Rival necromancers */
    FACTION_NEUTRAL,      /**< Neutral or abandoned */
    FACTION_COUNT
} FactionType;

/**
 * @brief Territory stability status
 */
typedef enum {
    STABILITY_CHAOTIC = 0,    /**< <25% control, unstable */
    STABILITY_CONTESTED,      /**< 25-49% control, contested */
    STABILITY_CONTROLLED,     /**< 50-74% control, stable */
    STABILITY_DOMINATED,      /**< 75-100% control, dominated */
    STABILITY_COUNT
} StabilityLevel;

/**
 * @brief Territory control information for a location
 */
typedef struct {
    uint32_t location_id;           /**< Associated location ID */
    uint8_t control_percentage;     /**< Player control (0-100%) */
    FactionType dominant_faction;   /**< Current dominant faction */
    AlertLevel alert_level;         /**< Current alert level */
    StabilityLevel stability;       /**< Territory stability */
    uint32_t defense_strength;      /**< Defense level (0-1000) */
    uint64_t last_activity_time;    /**< Last hostile activity timestamp */
    uint64_t alert_decay_time;      /**< When alert will decay next */
    float resource_modifier;        /**< Resource generation multiplier (0.5-2.0) */
    bool under_siege;               /**< Whether location is under siege */
    bool reinforcements_called;     /**< Whether reinforcements are incoming */
    uint32_t garrison_strength;     /**< Strength of defending forces */
} TerritoryStatus;

/**
 * @brief Territory status manager (opaque)
 */
typedef struct TerritoryStatusManager TerritoryStatusManager;

/**
 * @brief Create territory status manager
 *
 * @return Newly allocated manager, or NULL on failure
 */
TerritoryStatusManager* territory_status_create(void);

/**
 * @brief Destroy territory status manager
 *
 * @param manager Manager to destroy (can be NULL)
 */
void territory_status_destroy(TerritoryStatusManager* manager);

/**
 * @brief Get or create status for a location
 *
 * @param manager Territory status manager
 * @param location_id Location ID
 * @return Status structure, or NULL on failure
 */
TerritoryStatus* territory_status_get(TerritoryStatusManager* manager, uint32_t location_id);

/**
 * @brief Update control percentage for a location
 *
 * @param manager Territory status manager
 * @param location_id Location ID
 * @param control_percentage New control percentage (0-100)
 * @return true on success
 */
bool territory_status_set_control(TerritoryStatusManager* manager,
                                   uint32_t location_id,
                                   uint8_t control_percentage);

/**
 * @brief Increase alert level due to hostile action
 *
 * @param manager Territory status manager
 * @param location_id Location ID
 * @param amount Amount to increase (1-3 levels)
 * @param current_time Current game time for decay tracking
 * @return New alert level
 */
AlertLevel territory_status_raise_alert(TerritoryStatusManager* manager,
                                         uint32_t location_id,
                                         uint8_t amount,
                                         uint64_t current_time);

/**
 * @brief Decrease alert level over time
 *
 * @param manager Territory status manager
 * @param location_id Location ID
 * @param current_time Current game time
 * @return New alert level
 */
AlertLevel territory_status_decay_alert(TerritoryStatusManager* manager,
                                         uint32_t location_id,
                                         uint64_t current_time);

/**
 * @brief Update all territory statuses based on game time
 *
 * Decays alerts, updates stability, checks for reinforcements
 *
 * @param manager Territory status manager
 * @param current_time Current game time
 */
void territory_status_update_all(TerritoryStatusManager* manager, uint64_t current_time);

/**
 * @brief Calculate resource generation modifier
 *
 * Based on control percentage, stability, and alert level
 *
 * @param status Territory status
 * @return Multiplier (0.5 = half resources, 2.0 = double resources)
 */
float territory_status_resource_modifier(const TerritoryStatus* status);

/**
 * @brief Calculate detection chance modifier
 *
 * @param status Territory status
 * @return Detection multiplier (1.0 = normal, 2.0 = double chance)
 */
float territory_status_detection_modifier(const TerritoryStatus* status);

/**
 * @brief Check if location needs reinforcements
 *
 * @param status Territory status
 * @return true if reinforcements should be called
 */
bool territory_status_needs_reinforcements(const TerritoryStatus* status);

/**
 * @brief Get string name of alert level
 *
 * @param level Alert level
 * @return String name
 */
const char* territory_status_alert_name(AlertLevel level);

/**
 * @brief Get string name of faction
 *
 * @param faction Faction type
 * @return String name
 */
const char* territory_status_faction_name(FactionType faction);

/**
 * @brief Get string name of stability level
 *
 * @param stability Stability level
 * @return String name
 */
const char* territory_status_stability_name(StabilityLevel stability);

/**
 * @brief Calculate stability level from control percentage
 *
 * @param control_percentage Control percentage (0-100)
 * @return Stability level
 */
StabilityLevel territory_status_calculate_stability(uint8_t control_percentage);

/**
 * @brief Get all locations with a specific alert level
 *
 * @param manager Territory status manager
 * @param alert_level Alert level to filter by
 * @param results Output array of location IDs (caller allocates)
 * @param max_results Maximum number of results
 * @return Number of locations found
 */
size_t territory_status_get_by_alert(const TerritoryStatusManager* manager,
                                      AlertLevel alert_level,
                                      uint32_t* results,
                                      size_t max_results);

/**
 * @brief Get all locations under player control (>50%)
 *
 * @param manager Territory status manager
 * @param results Output array of location IDs (caller allocates)
 * @param max_results Maximum number of results
 * @return Number of locations found
 */
size_t territory_status_get_controlled(const TerritoryStatusManager* manager,
                                        uint32_t* results,
                                        size_t max_results);

#endif /* NECROMANCER_TERRITORY_STATUS_H */

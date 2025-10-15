/**
 * @file alliance.h
 * @brief Alliance system for multi-necromancer relationships
 *
 * Manages alliances between player and Regional Council necromancers.
 * Supports 5 alliance types from hostile to full alliance, with phylactery
 * oath binding and resource/knowledge sharing mechanics.
 */

#ifndef NECROMANCERS_ALLIANCE_H
#define NECROMANCERS_ALLIANCE_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

/**
 * @brief Alliance type progression
 *
 * Alliance types form a progression:
 * HOSTILE → NEUTRAL → NON_AGGRESSION → INFO_EXCHANGE → FULL_ALLIANCE
 */
typedef enum {
    ALLIANCE_HOSTILE,           /**< Actively antagonistic, may attack */
    ALLIANCE_NEUTRAL,           /**< Indifferent, no interaction */
    ALLIANCE_NON_AGGRESSION,    /**< Peaceful coexistence, no hostility */
    ALLIANCE_INFO_EXCHANGE,     /**< Share knowledge and information */
    ALLIANCE_FULL               /**< Full cooperation, resource sharing */
} AllianceType;

/**
 * @brief Alliance structure
 *
 * Represents an alliance relationship between player and another necromancer.
 * Tracks alliance type, phylactery oaths (binding contracts), shared resources,
 * and trust level.
 */
typedef struct {
    char npc_id[64];                /**< Unique ID of allied necromancer */
    AllianceType type;              /**< Current alliance type */
    bool phylactery_oath;           /**< Binding oath, cannot break alliance */
    uint32_t knowledge_shared;      /**< Number of research/spells shared */
    uint32_t resources_shared;      /**< Amount of soul energy shared */
    float trust_level;              /**< Trust level (0-100%) */
    uint32_t days_since_formed;     /**< Days since alliance formed */
    uint32_t interactions;          /**< Number of interactions */
} Alliance;

/**
 * @brief Create a new alliance
 *
 * @param npc_id Unique identifier of the necromancer
 * @param type Initial alliance type
 * @return Newly allocated Alliance or NULL on failure
 */
Alliance* alliance_create(const char* npc_id, AllianceType type);

/**
 * @brief Destroy an alliance and free memory
 *
 * @param alliance Alliance to destroy (can be NULL)
 */
void alliance_destroy(Alliance* alliance);

/**
 * @brief Upgrade alliance to next level
 *
 * Advances alliance type one step (e.g., NEUTRAL → NON_AGGRESSION).
 * Cannot upgrade beyond FULL_ALLIANCE.
 *
 * @param alliance Alliance to upgrade
 * @return true on success, false if already at max or alliance is NULL
 */
bool alliance_upgrade(Alliance* alliance);

/**
 * @brief Downgrade alliance to previous level
 *
 * Decreases alliance type one step (e.g., INFO_EXCHANGE → NON_AGGRESSION).
 * Cannot downgrade beyond HOSTILE. Phylactery oaths prevent downgrading.
 *
 * @param alliance Alliance to downgrade
 * @return true on success, false if at minimum, oath prevents, or alliance is NULL
 */
bool alliance_downgrade(Alliance* alliance);

/**
 * @brief Set alliance to specific type
 *
 * Directly sets alliance type. Phylactery oaths prevent setting to lower types.
 *
 * @param alliance Alliance to modify
 * @param type New alliance type
 * @return true on success, false if oath prevents or alliance is NULL
 */
bool alliance_set_type(Alliance* alliance, AllianceType type);

/**
 * @brief Bind alliance with phylactery oath
 *
 * Creates a binding magical contract that prevents alliance downgrade.
 * Oath cannot be removed once set.
 *
 * @param alliance Alliance to bind
 * @return true on success, false if already bound or alliance is NULL
 */
bool alliance_bind_phylactery_oath(Alliance* alliance);

/**
 * @brief Add shared knowledge
 *
 * Records knowledge sharing (research, spells, techniques).
 * Increases trust level.
 *
 * @param alliance Alliance to update
 * @param count Number of knowledge items shared
 * @return true on success, false if alliance is NULL
 */
bool alliance_add_knowledge(Alliance* alliance, uint32_t count);

/**
 * @brief Add shared resources
 *
 * Records resource sharing (soul energy, materials).
 * Increases trust level.
 *
 * @param alliance Alliance to update
 * @param amount Amount of resources shared
 * @return true on success, false if alliance is NULL
 */
bool alliance_add_resources(Alliance* alliance, uint32_t amount);

/**
 * @brief Modify trust level
 *
 * Directly adjusts trust level by specified amount (can be negative).
 * Trust is clamped to [0, 100].
 *
 * @param alliance Alliance to update
 * @param change Amount to change trust (positive or negative)
 * @return true on success, false if alliance is NULL
 */
bool alliance_modify_trust(Alliance* alliance, float change);

/**
 * @brief Record an interaction
 *
 * Increments interaction counter. Used for tracking relationship depth.
 *
 * @param alliance Alliance to update
 */
void alliance_record_interaction(Alliance* alliance);

/**
 * @brief Advance time for alliance
 *
 * Updates days_since_formed counter. Should be called daily.
 *
 * @param alliance Alliance to update
 * @param days Number of days to advance
 */
void alliance_advance_time(Alliance* alliance, uint32_t days);

/**
 * @brief Check if alliance can be upgraded
 *
 * @param alliance Alliance to check
 * @return true if upgrade is possible, false otherwise
 */
bool alliance_can_upgrade(const Alliance* alliance);

/**
 * @brief Check if alliance can be downgraded
 *
 * @param alliance Alliance to check
 * @return true if downgrade is possible, false if oath prevents or at minimum
 */
bool alliance_can_downgrade(const Alliance* alliance);

/**
 * @brief Check if alliance type requires minimum trust
 *
 * Some alliance types require minimum trust levels to maintain.
 *
 * @param alliance Alliance to check
 * @return true if trust level meets requirements, false otherwise
 */
bool alliance_meets_trust_requirement(const Alliance* alliance);

/**
 * @brief Get alliance type name
 *
 * @param type Alliance type
 * @return String name (e.g., "Hostile", "Full Alliance")
 */
const char* alliance_type_name(AllianceType type);

/**
 * @brief Get alliance description
 *
 * Returns a descriptive string for the current alliance state.
 *
 * @param alliance Alliance to describe
 * @return String describing the alliance
 */
const char* alliance_get_description(const Alliance* alliance);

/**
 * @brief Check if alliances are compatible for council coordination
 *
 * Some alliance types allow joint operations (Purge defense, territory sharing).
 *
 * @param type Alliance type
 * @return true if type supports coordination, false otherwise
 */
bool alliance_supports_coordination(AllianceType type);

#endif /* NECROMANCERS_ALLIANCE_H */

/**
 * @file alliance_manager.h
 * @brief Manager for multiple alliances with Regional Council
 *
 * Manages a collection of alliances between player and Regional Council
 * necromancers. Supports alliance lookup, council coordination, and
 * collective operations (Purge defense, resource pooling).
 */

#ifndef NECROMANCERS_ALLIANCE_MANAGER_H
#define NECROMANCERS_ALLIANCE_MANAGER_H

#include "alliance.h"
#include <stdbool.h>
#include <stddef.h>

/* Maximum number of concurrent alliances */
#define MAX_ALLIANCES 20

/**
 * @brief Alliance manager structure
 *
 * Manages collection of alliances with Regional Council necromancers.
 */
typedef struct {
    Alliance* alliances[MAX_ALLIANCES]; /**< Array of alliance pointers */
    size_t alliance_count;              /**< Number of active alliances */
} AllianceManager;

/**
 * @brief Create a new alliance manager
 *
 * @return Newly allocated AllianceManager or NULL on failure
 */
AllianceManager* alliance_manager_create(void);

/**
 * @brief Destroy alliance manager and all alliances
 *
 * @param manager Manager to destroy (can be NULL)
 */
void alliance_manager_destroy(AllianceManager* manager);

/**
 * @brief Add a new alliance
 *
 * Creates alliance with specified necromancer. Fails if alliance already exists.
 *
 * @param manager Alliance manager
 * @param npc_id Unique ID of necromancer
 * @param type Initial alliance type
 * @return true on success, false if alliance exists or manager is full
 */
bool alliance_manager_add(AllianceManager* manager, const char* npc_id, AllianceType type);

/**
 * @brief Remove an alliance
 *
 * Removes and destroys alliance with specified necromancer.
 *
 * @param manager Alliance manager
 * @param npc_id Unique ID of necromancer
 * @return true on success, false if alliance not found
 */
bool alliance_manager_remove(AllianceManager* manager, const char* npc_id);

/**
 * @brief Find alliance by NPC ID
 *
 * @param manager Alliance manager
 * @param npc_id Unique ID of necromancer
 * @return Pointer to alliance, or NULL if not found
 */
Alliance* alliance_manager_find(const AllianceManager* manager, const char* npc_id);

/**
 * @brief Get alliance by index
 *
 * @param manager Alliance manager
 * @param index Alliance index (0 to alliance_count-1)
 * @return Pointer to alliance, or NULL if index out of range
 */
Alliance* alliance_manager_get(const AllianceManager* manager, size_t index);

/**
 * @brief Count alliances of specific type
 *
 * @param manager Alliance manager
 * @param type Alliance type to count
 * @return Number of alliances of specified type
 */
size_t alliance_manager_count_by_type(const AllianceManager* manager, AllianceType type);

/**
 * @brief Get all allies available for coordination
 *
 * Returns count of allies with INFO_EXCHANGE or FULL alliance
 * (eligible for joint operations).
 *
 * @param manager Alliance manager
 * @return Number of allies available for coordination
 */
size_t alliance_manager_coordination_allies(const AllianceManager* manager);

/**
 * @brief Get all hostile necromancers
 *
 * @param manager Alliance manager
 * @return Number of hostile necromancers
 */
size_t alliance_manager_hostile_count(const AllianceManager* manager);

/**
 * @brief Calculate total trust across all alliances
 *
 * Returns average trust level across all alliances.
 *
 * @param manager Alliance manager
 * @return Average trust level (0-100), or 0 if no alliances
 */
float alliance_manager_average_trust(const AllianceManager* manager);

/**
 * @brief Advance time for all alliances
 *
 * Updates days_since_formed for all alliances.
 *
 * @param manager Alliance manager
 * @param days Number of days to advance
 */
void alliance_manager_advance_time(AllianceManager* manager, uint32_t days);

/**
 * @brief Check if council coordination is available
 *
 * Returns true if at least one ally supports coordination.
 *
 * @param manager Alliance manager
 * @return true if coordination available, false otherwise
 */
bool alliance_manager_has_coordination(const AllianceManager* manager);

/**
 * @brief Get strongest alliance type
 *
 * Returns the highest alliance type among all allies.
 *
 * @param manager Alliance manager
 * @return Highest alliance type, or ALLIANCE_HOSTILE if none
 */
AllianceType alliance_manager_strongest_alliance(const AllianceManager* manager);

/**
 * @brief Count alliances with phylactery oaths
 *
 * @param manager Alliance manager
 * @return Number of oath-bound alliances
 */
size_t alliance_manager_oath_count(const AllianceManager* manager);

#endif /* NECROMANCERS_ALLIANCE_MANAGER_H */

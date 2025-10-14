#ifndef MINION_H
#define MINION_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @file minion.h
 * @brief Core minion system for Necromancer's Shell
 *
 * Minions are undead creatures raised and controlled by the player.
 * Each minion has a type, stats, and can be bound to a soul for bonuses.
 */

/**
 * @brief Types of minions available in the game
 *
 * Different minion types have different base stats and costs.
 */
typedef enum {
    MINION_TYPE_ZOMBIE = 0,      /**< Tanky, slow (HP 100, Atk 15, Def 20, Spd 5) */
    MINION_TYPE_SKELETON,         /**< Fast, fragile (HP 50, Atk 25, Def 10, Spd 15) */
    MINION_TYPE_GHOUL,            /**< Aggressive melee (HP 80, Atk 30, Def 15, Spd 10) */
    MINION_TYPE_WRAITH,           /**< Ethereal magic (HP 60, Atk 35, Def 8, Spd 20) */
    MINION_TYPE_WIGHT,            /**< Elite warrior (HP 120, Atk 40, Def 25, Spd 12) */
    MINION_TYPE_REVENANT,         /**< Powerful, intelligent (HP 150, Atk 50, Def 30, Spd 15) */
    MINION_TYPE_COUNT
} MinionType;

/**
 * @brief Minion statistics structure
 */
typedef struct {
    uint32_t health;              /**< Current health */
    uint32_t health_max;          /**< Maximum health */
    uint32_t attack;              /**< Attack power */
    uint32_t defense;             /**< Defense rating */
    uint32_t speed;               /**< Speed rating */
    uint8_t loyalty;              /**< Loyalty level (0-100) */
} MinionStats;

/**
 * @brief Core minion structure
 */
typedef struct Minion {
    uint32_t id;                  /**< Unique minion identifier */
    char name[64];                /**< Minion name */
    MinionType type;              /**< Type of minion */
    MinionStats stats;            /**< Current stats */
    uint32_t bound_soul_id;       /**< ID of bound soul (0 if none) */
    uint32_t location_id;         /**< Current location ID */
    uint64_t raised_timestamp;    /**< When minion was raised */
    uint32_t experience;          /**< Experience points */
    uint8_t level;                /**< Minion level */
} Minion;

/**
 * @brief Create a new minion
 *
 * Allocates and initializes a new minion with base stats for its type.
 * If name is NULL, generates a default name based on type.
 *
 * @param type Minion type
 * @param name Minion name (NULL for auto-generated name)
 * @param soul_id Soul ID to bind (0 for no soul)
 * @return Pointer to newly allocated minion, or NULL on failure
 */
Minion* minion_create(MinionType type, const char* name, uint32_t soul_id);

/**
 * @brief Destroy a minion and free its memory
 *
 * @param minion Pointer to minion to destroy (can be NULL)
 */
void minion_destroy(Minion* minion);

/**
 * @brief Get the name of a minion type
 *
 * @param type Minion type
 * @return String name of the minion type (e.g., "Zombie", "Skeleton")
 */
const char* minion_type_name(MinionType type);

/**
 * @brief Get base stats for a minion type
 *
 * Fills the provided stats structure with base values for the type.
 *
 * @param type Minion type
 * @param stats Pointer to stats structure to fill
 */
void minion_get_base_stats(MinionType type, MinionStats* stats);

/**
 * @brief Calculate soul energy cost to raise a minion
 *
 * @param type Minion type
 * @return Soul energy cost to raise this type
 */
uint32_t minion_calculate_raise_cost(MinionType type);

/**
 * @brief Bind a soul to a minion
 *
 * Marks the minion as having a soul bound to it.
 * This does not modify stats - the caller should do that separately.
 *
 * @param minion Pointer to minion
 * @param soul_id ID of soul to bind
 * @return true on success, false if minion is NULL or already has a soul
 */
bool minion_bind_soul(Minion* minion, uint32_t soul_id);

/**
 * @brief Unbind soul from a minion
 *
 * Removes the soul binding from the minion.
 *
 * @param minion Pointer to minion
 * @return ID of unbound soul, or 0 if no soul was bound
 */
uint32_t minion_unbind_soul(Minion* minion);

/**
 * @brief Move minion to a different location
 *
 * @param minion Pointer to minion
 * @param location_id New location ID
 */
void minion_move_to_location(Minion* minion, uint32_t location_id);

/**
 * @brief Add experience to a minion
 *
 * Adds experience points and levels up if threshold is reached.
 * Each level requires level * 100 XP (level 1->2 needs 100, 2->3 needs 200, etc.)
 *
 * @param minion Pointer to minion
 * @param xp Experience points to add
 * @return true if minion leveled up, false otherwise
 */
bool minion_add_experience(Minion* minion, uint32_t xp);

/**
 * @brief Apply damage to a minion
 *
 * Reduces minion health by damage amount (clamped to 0).
 *
 * @param minion Pointer to minion
 * @param damage Damage amount
 * @return true if minion is still alive (health > 0), false if dead
 */
bool minion_take_damage(Minion* minion, uint32_t damage);

/**
 * @brief Heal a minion
 *
 * Restores health (capped at health_max).
 *
 * @param minion Pointer to minion
 * @param amount Amount to heal
 */
void minion_heal(Minion* minion, uint32_t amount);

/**
 * @brief Get a formatted description of the minion
 *
 * Returns a human-readable description including name, type, level, and stats.
 *
 * @param minion Pointer to minion
 * @param buffer Buffer to write description into
 * @param buffer_size Size of buffer
 */
void minion_get_description(const Minion* minion, char* buffer, size_t buffer_size);

#endif /* MINION_H */

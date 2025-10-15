#ifndef COMBATANT_H
#define COMBATANT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @file combatant.h
 * @brief Combat participant wrapper for combat system
 *
 * Wraps minions and enemies into a unified combatant interface for combat.
 * Provides common combat statistics and behavior.
 */

/* Forward declarations */
struct CombatState;
typedef struct Combatant Combatant;

/**
 * @brief Combat participant type
 */
typedef enum {
    COMBATANT_TYPE_MINION,    /**< Player-controlled minion */
    COMBATANT_TYPE_ENEMY,     /**< AI-controlled enemy */
    COMBATANT_TYPE_PLAYER     /**< Player character (future) */
} CombatantType;

/**
 * @brief AI function pointer type
 *
 * @param self The combatant making the decision
 * @param combat Current combat state
 */
typedef void (*AIFunction)(Combatant* self, struct CombatState* combat);

/**
 * @brief Combat participant structure
 *
 * Unified interface for all combat participants (minions, enemies, player).
 */
struct Combatant {
    char id[64];                  /**< Unique identifier */
    char name[128];               /**< Display name */
    CombatantType type;           /**< Type of combatant */

    /* Combat stats */
    uint32_t health;              /**< Current health */
    uint32_t health_max;          /**< Maximum health */
    uint32_t attack;              /**< Attack power */
    uint32_t defense;             /**< Defense rating */
    uint32_t speed;               /**< Speed rating */
    uint8_t initiative;           /**< Initiative roll (0-255) */

    /* Control */
    bool is_player_controlled;    /**< If true, player commands this unit */
    AIFunction ai_func;           /**< AI function (NULL for player-controlled) */

    /* Entity reference */
    void* entity;                 /**< Points to underlying Minion or Enemy */

    /* Combat state */
    bool has_acted_this_turn;     /**< Whether combatant acted this turn */
    bool is_defending;            /**< Defensive stance (+50% defense) */
};

/**
 * @brief Create a combatant from a minion
 *
 * Wraps an existing minion for combat. The minion's stats are copied.
 *
 * @param minion_entity Pointer to Minion structure
 * @param is_player_controlled Whether this minion is controlled by player
 * @return Newly allocated Combatant, or NULL on failure
 */
Combatant* combatant_create_from_minion(void* minion_entity, bool is_player_controlled);

/**
 * @brief Create a combatant from an enemy
 *
 * Wraps an existing enemy for combat. The enemy's stats are copied.
 *
 * @param enemy_entity Pointer to Enemy structure
 * @param ai_func AI function for this enemy
 * @return Newly allocated Combatant, or NULL on failure
 */
Combatant* combatant_create_from_enemy(void* enemy_entity, AIFunction ai_func);

/**
 * @brief Destroy a combatant
 *
 * Frees the combatant wrapper. Does NOT free the underlying entity.
 *
 * @param combatant Combatant to destroy (can be NULL)
 */
void combatant_destroy(Combatant* combatant);

/**
 * @brief Apply damage to a combatant
 *
 * Reduces health by damage amount, clamped to 0.
 *
 * @param combatant Combatant to damage
 * @param damage Damage amount
 * @return true if combatant is still alive, false if dead (health == 0)
 */
bool combatant_take_damage(Combatant* combatant, uint32_t damage);

/**
 * @brief Heal a combatant
 *
 * Restores health, capped at health_max.
 *
 * @param combatant Combatant to heal
 * @param amount Healing amount
 */
void combatant_heal(Combatant* combatant, uint32_t amount);

/**
 * @brief Check if combatant is alive
 *
 * @param combatant Combatant to check
 * @return true if health > 0, false otherwise
 */
bool combatant_is_alive(const Combatant* combatant);

/**
 * @brief Set combatant to defending stance
 *
 * Applies +50% defense modifier for this turn.
 *
 * @param combatant Combatant to set defending
 */
void combatant_set_defending(Combatant* combatant, bool defending);

/**
 * @brief Get effective defense (considering defending stance)
 *
 * @param combatant Combatant
 * @return Effective defense value
 */
uint32_t combatant_get_effective_defense(const Combatant* combatant);

/**
 * @brief Reset turn-based flags
 *
 * Clears has_acted_this_turn, is_defending flags for new turn.
 *
 * @param combatant Combatant to reset
 */
void combatant_reset_turn_flags(Combatant* combatant);

/**
 * @brief Roll initiative for combatant
 *
 * Generates initiative value based on speed + random component.
 *
 * @param combatant Combatant to roll for
 */
void combatant_roll_initiative(Combatant* combatant);

/**
 * @brief Sync stats back to underlying entity
 *
 * Updates the wrapped Minion or Enemy with current health/stats.
 *
 * @param combatant Combatant to sync
 */
void combatant_sync_to_entity(Combatant* combatant);

/**
 * @brief Get formatted description of combatant
 *
 * @param combatant Combatant to describe
 * @param buffer Buffer to write into
 * @param buffer_size Size of buffer
 */
void combatant_get_description(const Combatant* combatant, char* buffer, size_t buffer_size);

#endif /* COMBATANT_H */

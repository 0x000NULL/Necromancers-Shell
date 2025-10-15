#ifndef ENEMY_H
#define ENEMY_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @file enemy.h
 * @brief Enemy system for combat encounters
 *
 * Defines enemy types, stats, and behaviors for combat encounters.
 * Enemies can be Church forces, rival necromancers, or civilians.
 */

/**
 * @brief Types of enemies
 */
typedef enum {
    ENEMY_TYPE_PALADIN = 0,     /**< Holy tank, high defense (HP 120, Atk 35, Def 40, Spd 8) */
    ENEMY_TYPE_PRIEST,           /**< Healer support (HP 60, Atk 20, Def 15, Spd 10) */
    ENEMY_TYPE_INQUISITOR,       /**< Anti-undead specialist (HP 80, Atk 50, Def 20, Spd 12) */
    ENEMY_TYPE_VILLAGER,         /**< Weak civilian (HP 30, Atk 5, Def 5, Spd 6) */
    ENEMY_TYPE_GUARD,            /**< Basic soldier (HP 60, Atk 25, Def 15, Spd 9) */
    ENEMY_TYPE_RIVAL_NECROMANCER,/**< Competing necromancer (HP 70, Atk 40, Def 18, Spd 11) */
    ENEMY_TYPE_COUNT
} EnemyType;

/**
 * @brief AI behavior patterns
 */
typedef enum {
    AI_BEHAVIOR_AGGRESSIVE,  /**< Always attack lowest HP target */
    AI_BEHAVIOR_DEFENSIVE,   /**< Defend allies, attack weakest */
    AI_BEHAVIOR_BALANCED,    /**< Mix of offense and defense */
    AI_BEHAVIOR_SUPPORT,     /**< Prioritize healing/buffing */
    AI_BEHAVIOR_TACTICAL     /**< Focus fire on single target */
} AIBehavior;

/**
 * @brief Enemy structure
 */
typedef struct {
    uint32_t id;                /**< Unique enemy ID */
    char name[64];              /**< Enemy name/title */
    EnemyType type;             /**< Enemy type */

    /* Combat stats */
    uint32_t health;            /**< Current health */
    uint32_t health_max;        /**< Maximum health */
    uint32_t attack;            /**< Attack power */
    uint32_t defense;           /**< Defense rating */
    uint32_t speed;             /**< Speed rating */

    /* AI and behavior */
    AIBehavior ai_behavior;     /**< AI pattern */

    /* Rewards */
    uint32_t experience_reward; /**< XP given on defeat */
    uint32_t soul_energy_reward;/**< Soul energy given on defeat */

    /* Metadata */
    char description[256];      /**< Flavor text */
} Enemy;

/**
 * @brief Create a new enemy
 *
 * Allocates and initializes an enemy with base stats for its type.
 *
 * @param type Enemy type
 * @param id Unique enemy ID
 * @return Newly allocated Enemy, or NULL on failure
 */
Enemy* enemy_create(EnemyType type, uint32_t id);

/**
 * @brief Destroy an enemy
 *
 * @param enemy Enemy to destroy (can be NULL)
 */
void enemy_destroy(Enemy* enemy);

/**
 * @brief Get enemy type name
 *
 * @param type Enemy type
 * @return Human-readable type name (e.g., "Paladin", "Priest")
 */
const char* enemy_type_name(EnemyType type);

/**
 * @brief Get base stats for enemy type
 *
 * Fills provided pointers with base stats for the type.
 *
 * @param type Enemy type
 * @param hp Pointer to fill with max HP (can be NULL)
 * @param atk Pointer to fill with attack (can be NULL)
 * @param def Pointer to fill with defense (can be NULL)
 * @param spd Pointer to fill with speed (can be NULL)
 */
void enemy_get_base_stats(EnemyType type, uint32_t* hp, uint32_t* atk, uint32_t* def, uint32_t* spd);

/**
 * @brief Get default AI behavior for enemy type
 *
 * @param type Enemy type
 * @return Default AI behavior for this type
 */
AIBehavior enemy_get_default_ai(EnemyType type);

/**
 * @brief Get experience reward for defeating this enemy
 *
 * @param enemy Enemy
 * @return Experience points
 */
uint32_t enemy_get_experience_reward(const Enemy* enemy);

/**
 * @brief Get soul energy reward for defeating this enemy
 *
 * @param enemy Enemy
 * @return Soul energy amount
 */
uint32_t enemy_get_soul_energy_reward(const Enemy* enemy);

/**
 * @brief Apply damage to enemy
 *
 * @param enemy Enemy to damage
 * @param damage Damage amount
 * @return true if enemy is still alive, false if dead
 */
bool enemy_take_damage(Enemy* enemy, uint32_t damage);

/**
 * @brief Heal an enemy
 *
 * @param enemy Enemy to heal
 * @param amount Healing amount
 */
void enemy_heal(Enemy* enemy, uint32_t amount);

/**
 * @brief Check if enemy is alive
 *
 * @param enemy Enemy to check
 * @return true if health > 0, false otherwise
 */
bool enemy_is_alive(const Enemy* enemy);

/**
 * @brief Get formatted description of enemy
 *
 * @param enemy Enemy to describe
 * @param buffer Buffer to write into
 * @param buffer_size Size of buffer
 */
void enemy_get_description(const Enemy* enemy, char* buffer, size_t buffer_size);

/**
 * @brief Get AI behavior name
 *
 * @param behavior AI behavior
 * @return Human-readable behavior name
 */
const char* ai_behavior_name(AIBehavior behavior);

#endif /* ENEMY_H */

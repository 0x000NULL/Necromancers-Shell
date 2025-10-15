/**
 * @file damage.h
 * @brief Damage calculation and application system
 */

#ifndef DAMAGE_H
#define DAMAGE_H

#include <stdint.h>
#include <stdbool.h>

/* Forward declarations */
typedef struct Combatant Combatant;
typedef struct CombatState CombatState;

/**
 * @brief Damage type enumeration
 */
typedef enum {
    DAMAGE_TYPE_PHYSICAL,     /* Normal attack damage */
    DAMAGE_TYPE_NECROTIC,     /* Death magic damage */
    DAMAGE_TYPE_HOLY,         /* Anti-undead damage */
    DAMAGE_TYPE_PURE          /* Ignores defense */
} DamageType;

/**
 * @brief Attack result information
 */
typedef struct {
    uint32_t damage_dealt;        /* Final damage applied */
    uint32_t base_damage;         /* Pre-mitigation damage */
    uint32_t mitigated_damage;    /* Damage blocked by defense */
    bool is_critical;             /* Critical hit? */
    float critical_multiplier;    /* 1.5x for crits */
    bool was_blocked;             /* Defense blocked hit? */
    DamageType damage_type;       /* Type of damage */
} AttackResult;

/**
 * @brief Calculate damage from attacker to defender
 *
 * Formula:
 * - Base Damage = attacker->attack
 * - Critical Check: 10% chance for 1.5x damage
 * - Defense Mitigation: damage -= (defender->defense / 2)
 * - Defending Bonus: +50% defense if defender is defending
 * - Minimum Damage: Always at least 1 damage
 *
 * @param attacker Attacking combatant
 * @param defender Defending combatant
 * @param damage_type Type of damage to deal
 * @return Attack result with all damage information
 */
AttackResult damage_calculate_attack(
    const Combatant* attacker,
    const Combatant* defender,
    DamageType damage_type
);

/**
 * @brief Apply damage to a combatant and log result
 *
 * Updates combatant health, checks for death, and logs message to combat log.
 *
 * @param combat Combat state (for logging)
 * @param target Target combatant
 * @param result Attack result to apply
 * @return true if target is still alive, false if killed
 */
bool damage_apply_attack(
    CombatState* combat,
    Combatant* target,
    const AttackResult* result
);

/**
 * @brief Calculate healing amount
 *
 * Formula:
 * - Base Heal = healer->attack / 2
 * - Minimum: 10 HP
 * - Maximum: target->health_max - target->health
 *
 * @param healer Healing combatant
 * @param target Target to heal
 * @return Amount of healing to apply
 */
uint32_t damage_calculate_heal(
    const Combatant* healer,
    const Combatant* target
);

/**
 * @brief Apply healing and log result
 *
 * @param combat Combat state (for logging)
 * @param target Target to heal
 * @param amount Healing amount
 */
void damage_apply_heal(
    CombatState* combat,
    Combatant* target,
    uint32_t amount
);

/**
 * @brief Get critical hit chance (default 10%)
 *
 * @return Critical hit chance (0.0 to 1.0)
 */
float damage_get_crit_chance(void);

/**
 * @brief Get critical hit multiplier (default 1.5x)
 *
 * @return Critical damage multiplier
 */
float damage_get_crit_multiplier(void);

/**
 * @brief Roll for critical hit
 *
 * @return true if critical hit, false otherwise
 */
bool damage_roll_critical(void);

/**
 * @brief Get damage type name
 *
 * @param type Damage type
 * @return Human-readable damage type name
 */
const char* damage_type_name(DamageType type);

#endif /* DAMAGE_H */

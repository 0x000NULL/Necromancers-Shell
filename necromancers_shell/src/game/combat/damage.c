/**
 * @file damage.c
 * @brief Damage calculation and application system implementation
 */

#include "damage.h"
#include "combatant.h"
#include "combat.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Critical hit constants */
#define CRIT_CHANCE 0.10f    /* 10% chance */
#define CRIT_MULTIPLIER 1.5f /* 1.5x damage */

/**
 * @brief Roll for critical hit
 */
bool damage_roll_critical(void) {
    return ((float)rand() / (float)RAND_MAX) < CRIT_CHANCE;
}

/**
 * @brief Get critical hit chance
 */
float damage_get_crit_chance(void) {
    return CRIT_CHANCE;
}

/**
 * @brief Get critical hit multiplier
 */
float damage_get_crit_multiplier(void) {
    return CRIT_MULTIPLIER;
}

/**
 * @brief Get damage type name
 */
const char* damage_type_name(DamageType type) {
    switch (type) {
        case DAMAGE_TYPE_PHYSICAL: return "Physical";
        case DAMAGE_TYPE_NECROTIC: return "Necrotic";
        case DAMAGE_TYPE_HOLY:     return "Holy";
        case DAMAGE_TYPE_PURE:     return "Pure";
        default:                   return "Unknown";
    }
}

/**
 * @brief Calculate damage from attacker to defender
 */
AttackResult damage_calculate_attack(
    const Combatant* attacker,
    const Combatant* defender,
    DamageType damage_type
) {
    AttackResult result = {0};

    if (!attacker || !defender) {
        return result;
    }

    result.damage_type = damage_type;

    /* Base damage from attacker's attack stat */
    uint32_t base_damage = attacker->attack;
    result.base_damage = base_damage;

    /* Roll for critical hit */
    result.is_critical = damage_roll_critical();
    result.critical_multiplier = result.is_critical ? CRIT_MULTIPLIER : 1.0f;

    /* Apply critical multiplier */
    uint32_t damage = (uint32_t)((float)base_damage * result.critical_multiplier);

    /* Apply defense mitigation (except for PURE damage) */
    if (damage_type != DAMAGE_TYPE_PURE) {
        /* Get effective defense (includes defending bonus) */
        uint32_t effective_defense = combatant_get_effective_defense(defender);

        /* Mitigation: damage reduced by half of defense */
        uint32_t mitigation = effective_defense / 2;

        if (damage > mitigation) {
            result.mitigated_damage = mitigation;
            damage -= mitigation;
        } else {
            /* Defense blocked most/all damage */
            result.mitigated_damage = damage;
            damage = 1;  /* Minimum damage */
            result.was_blocked = true;
        }
    } else {
        /* Pure damage ignores defense */
        result.mitigated_damage = 0;
        result.was_blocked = false;
    }

    /* Ensure minimum damage of 1 */
    if (damage < 1) {
        damage = 1;
    }

    result.damage_dealt = damage;
    return result;
}

/**
 * @brief Apply damage to a combatant and log result
 */
bool damage_apply_attack(
    CombatState* combat,
    Combatant* target,
    const AttackResult* result
) {
    if (!combat || !target || !result) {
        return false;
    }

    /* Apply damage */
    bool alive = combatant_take_damage(target, result->damage_dealt);

    /* Log the attack with color coding */
    char log_msg[512];

    if (result->is_critical) {
        snprintf(log_msg, sizeof(log_msg),
            "\033[1;33m%s deals %u damage to %s (%u base - %u mitigated) - CRITICAL HIT!\033[0m",
            target->name, result->damage_dealt, target->name,
            result->base_damage, result->mitigated_damage);
    } else if (result->was_blocked) {
        snprintf(log_msg, sizeof(log_msg),
            "\033[0;36m%s's attack is mostly blocked! Only %u damage dealt.\033[0m",
            target->name, result->damage_dealt);
    } else {
        snprintf(log_msg, sizeof(log_msg),
            "%s takes %u %s damage (%u base - %u mitigated)",
            target->name, result->damage_dealt, damage_type_name(result->damage_type),
            result->base_damage, result->mitigated_damage);
    }

    combat_log_message(combat, "%s", log_msg);

    /* Log death if killed */
    if (!alive) {
        combat_log_message(combat, "\033[1;31m%s has been slain!\033[0m", target->name);
    } else {
        /* Log remaining HP */
        combat_log_message(combat, "%s has %u/%u HP remaining",
            target->name, target->health, target->health_max);
    }

    return alive;
}

/**
 * @brief Calculate healing amount
 */
uint32_t damage_calculate_heal(
    const Combatant* healer,
    const Combatant* target
) {
    if (!healer || !target) {
        return 0;
    }

    /* Base heal = half of healer's attack */
    uint32_t heal_amount = healer->attack / 2;

    /* Minimum heal of 10 */
    if (heal_amount < 10) {
        heal_amount = 10;
    }

    /* Cap at target's missing HP */
    uint32_t missing_hp = target->health_max - target->health;
    if (heal_amount > missing_hp) {
        heal_amount = missing_hp;
    }

    return heal_amount;
}

/**
 * @brief Apply healing and log result
 */
void damage_apply_heal(
    CombatState* combat,
    Combatant* target,
    uint32_t amount
) {
    if (!combat || !target || amount == 0) {
        return;
    }

    /* Apply healing */
    combatant_heal(target, amount);

    /* Log with color */
    combat_log_message(combat, "\033[1;32m%s heals for %u HP\033[0m",
        target->name, amount);
    combat_log_message(combat, "%s now has %u/%u HP",
        target->name, target->health, target->health_max);
}

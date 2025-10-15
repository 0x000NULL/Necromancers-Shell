#include "enemy_ai.h"
#include <stdlib.h>
#include <stdio.h>

/* === AI Helper Functions === */

Combatant* ai_find_lowest_hp_player(const CombatState* combat) {
    if (!combat) return NULL;

    Combatant* lowest = NULL;
    uint32_t lowest_hp = UINT32_MAX;

    for (uint8_t i = 0; i < combat->player_force_count; i++) {
        Combatant* c = combat->player_forces[i];
        if (combatant_is_alive(c) && c->health < lowest_hp) {
            lowest = c;
            lowest_hp = c->health;
        }
    }

    return lowest;
}

Combatant* ai_find_highest_hp_player(const CombatState* combat) {
    if (!combat) return NULL;

    Combatant* highest = NULL;
    uint32_t highest_hp = 0;

    for (uint8_t i = 0; i < combat->player_force_count; i++) {
        Combatant* c = combat->player_forces[i];
        if (combatant_is_alive(c) && c->health > highest_hp) {
            highest = c;
            highest_hp = c->health;
        }
    }

    return highest;
}

Combatant* ai_find_random_player(const CombatState* combat) {
    if (!combat) return NULL;

    /* Count living players */
    uint8_t alive_count = 0;
    for (uint8_t i = 0; i < combat->player_force_count; i++) {
        if (combatant_is_alive(combat->player_forces[i])) {
            alive_count++;
        }
    }

    if (alive_count == 0) return NULL;

    /* Pick random index */
    uint8_t target_index = rand() % alive_count;

    /* Find that player */
    uint8_t alive_seen = 0;
    for (uint8_t i = 0; i < combat->player_force_count; i++) {
        if (combatant_is_alive(combat->player_forces[i])) {
            if (alive_seen == target_index) {
                return combat->player_forces[i];
            }
            alive_seen++;
        }
    }

    return NULL;
}

Combatant* ai_find_lowest_hp_ally(const CombatState* combat) {
    if (!combat) return NULL;

    Combatant* lowest = NULL;
    uint32_t lowest_hp = UINT32_MAX;

    for (uint8_t i = 0; i < combat->enemy_force_count; i++) {
        Combatant* c = combat->enemy_forces[i];
        if (combatant_is_alive(c)) {
            uint32_t hp_percent = (c->health * 100) / c->health_max;

            /* Only consider if below 80% health */
            if (hp_percent < 80 && c->health < lowest_hp) {
                lowest = c;
                lowest_hp = c->health;
            }
        }
    }

    return lowest;
}

void ai_perform_attack(Combatant* attacker, Combatant* target, CombatState* combat) {
    if (!attacker || !target || !combat) return;

    /* Simple damage formula: attack - (defense / 2) */
    uint32_t defense = combatant_get_effective_defense(target);
    uint32_t damage = attacker->attack;

    if (damage > defense / 2) {
        damage -= defense / 2;
    } else {
        damage = 1;  /* Minimum damage */
    }

    /* Apply damage */
    bool alive = combatant_take_damage(target, damage);

    /* Log */
    if (alive) {
        combat_log_message(combat, "%s attacks %s for %u damage!",
                          attacker->name, target->name, damage);
    } else {
        combat_log_message(combat, "%s attacks %s for %u damage! %s was defeated!",
                          attacker->name, target->name, damage, target->name);
    }
}

void ai_perform_heal(Combatant* healer, Combatant* target, CombatState* combat) {
    if (!healer || !target || !combat) return;

    /* Heal amount = healer's attack / 2 */
    uint32_t heal_amount = healer->attack / 2;
    if (heal_amount < 10) heal_amount = 10;

    uint32_t old_hp = target->health;
    combatant_heal(target, heal_amount);
    uint32_t actual_heal = target->health - old_hp;

    combat_log_message(combat, "%s heals %s for %u HP!",
                      healer->name, target->name, actual_heal);
}

/* === AI Behavior Implementations === */

void ai_aggressive_turn(Combatant* self, CombatState* combat) {
    if (!self || !combat) return;

    /* Attack lowest HP player */
    Combatant* target = ai_find_lowest_hp_player(combat);
    if (target) {
        ai_perform_attack(self, target, combat);
    }
}

void ai_defensive_turn(Combatant* self, CombatState* combat) {
    if (!self || !combat) return;

    /* Check if any ally needs healing */
    Combatant* wounded_ally = ai_find_lowest_hp_ally(combat);
    if (wounded_ally && self->attack >= 20) {
        /* Can heal - do so if ally is below 50% */
        uint32_t hp_percent = (wounded_ally->health * 100) / wounded_ally->health_max;
        if (hp_percent < 50) {
            ai_perform_heal(self, wounded_ally, combat);
            return;
        }
    }

    /* Otherwise defend and attack weakest */
    combatant_set_defending(self, true);
    Combatant* target = ai_find_lowest_hp_player(combat);
    if (target) {
        ai_perform_attack(self, target, combat);
    }
}

void ai_balanced_turn(Combatant* self, CombatState* combat) {
    if (!self || !combat) return;

    /* Decide based on own HP */
    uint32_t hp_percent = (self->health * 100) / self->health_max;

    if (hp_percent < 30) {
        /* Low HP - defend */
        combatant_set_defending(self, true);
        combat_log_message(combat, "%s takes a defensive stance!", self->name);
    } else {
        /* Attack random target */
        Combatant* target = ai_find_random_player(combat);
        if (target) {
            ai_perform_attack(self, target, combat);
        }
    }
}

void ai_support_turn(Combatant* self, CombatState* combat) {
    if (!self || !combat) return;

    /* Always prioritize healing */
    Combatant* wounded_ally = ai_find_lowest_hp_ally(combat);
    if (wounded_ally) {
        ai_perform_heal(self, wounded_ally, combat);
        return;
    }

    /* If no one needs healing, attack lowest HP player */
    Combatant* target = ai_find_lowest_hp_player(combat);
    if (target) {
        ai_perform_attack(self, target, combat);
    }
}

void ai_tactical_turn(Combatant* self, CombatState* combat) {
    if (!self || !combat) return;

    /* Focus fire - target lowest HP player repeatedly */
    Combatant* target = ai_find_lowest_hp_player(combat);
    if (target) {
        ai_perform_attack(self, target, combat);
    }
}

AIFunction ai_get_function(AIBehavior behavior) {
    switch (behavior) {
        case AI_BEHAVIOR_AGGRESSIVE:  return ai_aggressive_turn;
        case AI_BEHAVIOR_DEFENSIVE:   return ai_defensive_turn;
        case AI_BEHAVIOR_BALANCED:    return ai_balanced_turn;
        case AI_BEHAVIOR_SUPPORT:     return ai_support_turn;
        case AI_BEHAVIOR_TACTICAL:    return ai_tactical_turn;
        default:                      return ai_balanced_turn;
    }
}

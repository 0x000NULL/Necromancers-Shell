#ifndef ENEMY_AI_H
#define ENEMY_AI_H

#include "combatant.h"
#include "combat.h"
#include "enemy.h"

/**
 * @file enemy_ai.h
 * @brief Enemy AI decision-making system
 *
 * Provides AI functions for different enemy behavior patterns.
 * Each behavior implements different tactical priorities.
 */

/**
 * @brief Aggressive AI behavior
 *
 * Always attacks the target with lowest HP. Focuses on eliminating weak targets.
 *
 * @param self AI-controlled combatant
 * @param combat Current combat state
 */
void ai_aggressive_turn(Combatant* self, CombatState* combat);

/**
 * @brief Defensive AI behavior
 *
 * Prioritizes defending and protecting allies. Attacks only when necessary.
 *
 * @param self AI-controlled combatant
 * @param combat Current combat state
 */
void ai_defensive_turn(Combatant* self, CombatState* combat);

/**
 * @brief Balanced AI behavior
 *
 * Mix of offense and defense. Makes decisions based on tactical situation.
 *
 * @param self AI-controlled combatant
 * @param combat Current combat state
 */
void ai_balanced_turn(Combatant* self, CombatState* combat);

/**
 * @brief Support AI behavior
 *
 * Prioritizes healing and buffing allies. Attacks as last resort.
 *
 * @param self AI-controlled combatant
 * @param combat Current combat state
 */
void ai_support_turn(Combatant* self, CombatState* combat);

/**
 * @brief Tactical AI behavior
 *
 * Focus fires on single target until eliminated. Prefers high-threat targets.
 *
 * @param self AI-controlled combatant
 * @param combat Current combat state
 */
void ai_tactical_turn(Combatant* self, CombatState* combat);

/**
 * @brief Get AI function for behavior type
 *
 * Returns the appropriate AI function pointer for a given behavior.
 *
 * @param behavior AI behavior type
 * @return AI function pointer
 */
AIFunction ai_get_function(AIBehavior behavior);

/* === Helper Functions === */

/**
 * @brief Find lowest HP player target
 *
 * @param combat Combat state
 * @return Pointer to lowest HP player combatant, or NULL if none alive
 */
Combatant* ai_find_lowest_hp_player(const CombatState* combat);

/**
 * @brief Find highest HP player target
 *
 * @param combat Combat state
 * @return Pointer to highest HP player combatant, or NULL if none alive
 */
Combatant* ai_find_highest_hp_player(const CombatState* combat);

/**
 * @brief Find random living player target
 *
 * @param combat Combat state
 * @return Pointer to random player combatant, or NULL if none alive
 */
Combatant* ai_find_random_player(const CombatState* combat);

/**
 * @brief Find lowest HP ally (for healing)
 *
 * @param combat Combat state
 * @return Pointer to lowest HP ally, or NULL if none need healing
 */
Combatant* ai_find_lowest_hp_ally(const CombatState* combat);

/**
 * @brief Perform basic attack on target
 *
 * Calculates damage and applies it to target. Logs combat message.
 *
 * @param attacker Attacking combatant
 * @param target Target combatant
 * @param combat Combat state (for logging)
 */
void ai_perform_attack(Combatant* attacker, Combatant* target, CombatState* combat);

/**
 * @brief Perform heal on target
 *
 * Heals target and logs combat message.
 *
 * @param healer Healing combatant
 * @param target Target combatant
 * @param combat Combat state (for logging)
 */
void ai_perform_heal(Combatant* healer, Combatant* target, CombatState* combat);

#endif /* ENEMY_AI_H */

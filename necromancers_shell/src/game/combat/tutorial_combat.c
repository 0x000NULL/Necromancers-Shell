/**
 * @file tutorial_combat.c
 * @brief Tutorial combat implementation
 */

#include "tutorial_combat.h"
#include "combat.h"
#include "combatant.h"
#include "enemy.h"
#include "enemy_ai.h"
#include "../game_state.h"
#include "../minions/minion.h"
#include "../minions/minion_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Check if tutorial is completed
 */
bool tutorial_combat_is_completed(const GameState* game_state) {
    if (!game_state) {
        return false;
    }

    /* TODO: Add tutorial_combat_complete field to GameState */
    /* For now, return false to allow repeated tutorials */
    return false;
}

/**
 * @brief Mark tutorial as completed
 */
void tutorial_combat_mark_completed(GameState* game_state) {
    if (!game_state) {
        return;
    }

    /* TODO: Set tutorial_combat_complete = true in GameState */
    /* For now, this is a no-op */
}

/**
 * @brief Get welcome message
 */
const char* tutorial_combat_get_welcome_message(void) {
    return "\n"
           "╔═══════════════════════════════════════════════════════════════╗\n"
           "║                    COMBAT TUTORIAL                            ║\n"
           "╚═══════════════════════════════════════════════════════════════╝\n"
           "\n"
           "Welcome to combat! You'll learn the basics of fighting.\n"
           "\n"
           "A weak villager stands before you. Time to test your skills.\n";
}

/**
 * @brief Get instructions
 */
const char* tutorial_combat_get_instructions(void) {
    return "\n"
           "TUTORIAL COMMANDS:\n"
           "  attack E1  - Attack the enemy villager\n"
           "  defend     - Enter defensive stance (+50%% defense)\n"
           "  flee       - Attempt to escape (not recommended for tutorial!)\n"
           "  status     - View detailed combat status\n"
           "\n"
           "TIP: Try using 'attack E1' to strike the villager!\n";
}

/**
 * @brief Award completion bonus
 */
void tutorial_combat_award_bonus(GameState* game_state) {
    if (!game_state) {
        return;
    }

    /* Award 50 soul energy bonus */
    game_state->resources.soul_energy += 50;

    printf("\n\033[1;32m");  /* Bright green */
    printf("Tutorial completed! You've earned 50 bonus soul energy!\n");
    printf("\033[0m");  /* Reset */
}

/**
 * @brief Create tutorial combat
 */
CombatState* tutorial_combat_create(GameState* game_state) {
    if (!game_state) {
        return NULL;
    }

    /* Create tutorial zombie minion if player has none */
    Minion* tutorial_zombie = NULL;
    if (game_state->minions && minion_manager_count(game_state->minions) == 0) {
        tutorial_zombie = minion_create(
            MINION_TYPE_ZOMBIE,
            "Tutorial Zombie",
            0  /* No bound soul */
        );

        if (tutorial_zombie) {
            minion_manager_add(game_state->minions, tutorial_zombie);
        }
    }

    /* Get first minion (tutorial zombie or existing) */
    Minion* player_minion = NULL;
    if (game_state->minions) {
        player_minion = minion_manager_get_at(game_state->minions, 0);
    }

    if (!player_minion) {
        return NULL;
    }

    /* Create weak villager enemy */
    Enemy* villager = enemy_create(1, ENEMY_TYPE_VILLAGER);
    if (!villager) {
        return NULL;
    }

    /* Weaken the villager for tutorial (50% stats) */
    villager->health = villager->health / 2;
    villager->health_max = villager->health_max / 2;
    villager->attack = villager->attack / 2;
    villager->defense = villager->defense / 2;

    /* Create combat state */
    CombatState* combat = combat_state_create();
    if (!combat) {
        enemy_destroy(villager);
        return NULL;
    }

    /* Create player combatant from minion */
    Combatant* player_combatant = combatant_create_from_minion(player_minion, true);
    if (!player_combatant) {
        combat_state_destroy(combat);
        enemy_destroy(villager);
        return NULL;
    }

    /* Create enemy combatant with balanced AI */
    Combatant* enemy_combatant = combatant_create_from_enemy(villager, ai_balanced_turn);
    if (!enemy_combatant) {
        combatant_destroy(player_combatant);
        combat_state_destroy(combat);
        enemy_destroy(villager);
        return NULL;
    }

    /* Add combatants to combat */
    if (!combat_add_player_combatant(combat, player_combatant)) {
        combatant_destroy(enemy_combatant);
        combatant_destroy(player_combatant);
        combat_state_destroy(combat);
        enemy_destroy(villager);
        return NULL;
    }

    if (!combat_add_enemy_combatant(combat, enemy_combatant)) {
        /* Player combatant already added to combat, will be freed by combat_state_destroy */
        combatant_destroy(enemy_combatant);
        combat_state_destroy(combat);
        enemy_destroy(villager);
        return NULL;
    }

    /* Initialize combat */
    combat_initialize(combat);

    /* Log tutorial start */
    combat_log_message(combat, "=== TUTORIAL COMBAT ===");
    combat_log_message(combat, "Learn the basics by defeating a weak villager!");
    combat_log_message(combat, "Commands: attack E1, defend, flee, status");

    return combat;
}

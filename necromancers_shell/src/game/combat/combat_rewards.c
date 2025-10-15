/**
 * @file combat_rewards.c
 * @brief Combat rewards system implementation
 */

#include "combat_rewards.h"
#include "combat.h"
#include "combatant.h"
#include "enemy.h"
#include "../game_state.h"
#include "../souls/soul_manager.h"
#include "../resources/corruption.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * @brief Calculate rewards from combat victory
 */
CombatRewards* combat_calculate_rewards(const CombatState* combat) {
    if (!combat) {
        return NULL;
    }

    CombatRewards* rewards = calloc(1, sizeof(CombatRewards));
    if (!rewards) {
        return NULL;
    }

    /* Calculate totals from defeated enemies */
    for (uint8_t i = 0; i < combat->enemy_force_count; i++) {
        Combatant* enemy_combatant = combat->enemy_forces[i];

        /* Only count dead enemies */
        if (combatant_is_alive(enemy_combatant)) {
            continue;
        }

        /* Get underlying enemy entity */
        if (enemy_combatant->type == COMBATANT_TYPE_ENEMY && enemy_combatant->entity) {
            Enemy* enemy = (Enemy*)enemy_combatant->entity;

            /* Add experience */
            rewards->experience_total += enemy_get_experience_reward(enemy);

            /* Add soul energy */
            rewards->soul_energy_gained += enemy_get_soul_energy_reward(enemy);

            /* Check for corruption increase (villagers are innocent) */
            if (enemy->type == ENEMY_TYPE_VILLAGER) {
                rewards->corruption_change += 5;  /* +5 corruption per villager killed */
            }
        }
    }

    /* Harvest souls from corpses (simplified - one soul per enemy) */
    uint8_t dead_count = 0;
    for (uint8_t i = 0; i < combat->enemy_force_count; i++) {
        if (!combatant_is_alive(combat->enemy_forces[i])) {
            dead_count++;
        }
    }

    if (dead_count > 0) {
        rewards->souls_harvested = calloc(dead_count, sizeof(Soul*));
        if (rewards->souls_harvested) {
            rewards->soul_count = dead_count;

            /* Create souls based on enemy types */
            uint8_t soul_idx = 0;
            for (uint8_t i = 0; i < combat->enemy_force_count; i++) {
                Combatant* enemy_combatant = combat->enemy_forces[i];

                if (combatant_is_alive(enemy_combatant)) {
                    continue;
                }

                /* Determine soul type based on enemy */
                SoulType soul_type = SOUL_TYPE_COMMON;
                uint8_t soul_quality = 50;  /* Base quality */

                if (enemy_combatant->type == COMBATANT_TYPE_ENEMY && enemy_combatant->entity) {
                    Enemy* enemy = (Enemy*)enemy_combatant->entity;

                    switch (enemy->type) {
                        case ENEMY_TYPE_PALADIN:
                        case ENEMY_TYPE_GUARD:
                            soul_type = SOUL_TYPE_WARRIOR;
                            soul_quality = 60;
                            break;

                        case ENEMY_TYPE_PRIEST:
                        case ENEMY_TYPE_INQUISITOR:
                            soul_type = SOUL_TYPE_MAGE;
                            soul_quality = 65;
                            break;

                        case ENEMY_TYPE_VILLAGER:
                            soul_type = SOUL_TYPE_INNOCENT;
                            soul_quality = 40;
                            break;

                        case ENEMY_TYPE_RIVAL_NECROMANCER:
                            soul_type = SOUL_TYPE_CORRUPTED;
                            soul_quality = 70;
                            break;

                        default:
                            soul_type = SOUL_TYPE_COMMON;
                            soul_quality = 50;
                            break;
                    }
                }

                /* Create soul */
                Soul* soul = soul_create(soul_type, soul_quality);
                if (soul) {
                    rewards->souls_harvested[soul_idx++] = soul;
                }
            }
        }
    }

    return rewards;
}

/**
 * @brief Apply rewards to game state
 */
void combat_apply_rewards(const CombatRewards* rewards, GameState* game_state) {
    if (!rewards || !game_state) {
        return;
    }

    /* Add experience */
    game_state->player_experience += rewards->experience_total;

    /* Check for level up (simple: every 1000 XP) */
    while (game_state->player_experience >= 1000 * game_state->player_level) {
        game_state->player_level++;
        /* Could add stat bonuses here in future */
    }

    /* Add soul energy */
    game_state->resources.soul_energy += rewards->soul_energy_gained;

    /* Increase corruption */
    if (rewards->corruption_change > 0) {
        corruption_add(&game_state->corruption, rewards->corruption_change,
                      "Killed innocent enemies", game_state->resources.day_count);
    }

    /* Add souls to soul manager */
    if (rewards->souls_harvested && rewards->soul_count > 0) {
        for (uint8_t i = 0; i < rewards->soul_count; i++) {
            if (rewards->souls_harvested[i]) {
                /* Create a copy to add to manager */
                Soul* soul_copy = soul_create(
                    rewards->souls_harvested[i]->type,
                    rewards->souls_harvested[i]->quality
                );
                if (soul_copy) {
                    soul_manager_add(game_state->souls, soul_copy);
                }
            }
        }
    }
}

/**
 * @brief Free rewards structure
 */
void combat_rewards_destroy(CombatRewards* rewards) {
    if (!rewards) {
        return;
    }

    /* Free harvested souls */
    if (rewards->souls_harvested) {
        for (uint8_t i = 0; i < rewards->soul_count; i++) {
            soul_destroy(rewards->souls_harvested[i]);
        }
        free(rewards->souls_harvested);
    }

    free(rewards);
}

/**
 * @brief Format rewards as human-readable string
 */
void combat_rewards_format(const CombatRewards* rewards, char* buf, size_t buf_size) {
    if (!rewards || !buf || buf_size == 0) {
        return;
    }

    char* ptr = buf;
    size_t remaining = buf_size;
    int written;

    written = snprintf(ptr, remaining, "\n=== VICTORY REWARDS ===\n\n");
    ptr += written;
    remaining -= written;

    /* Experience */
    if (rewards->experience_total > 0) {
        written = snprintf(ptr, remaining, "Experience: +%u\n", rewards->experience_total);
        ptr += written;
        remaining -= written;
    }

    /* Soul energy */
    if (rewards->soul_energy_gained > 0) {
        written = snprintf(ptr, remaining, "Soul Energy: +%u\n", rewards->soul_energy_gained);
        ptr += written;
        remaining -= written;
    }

    /* Souls harvested */
    if (rewards->soul_count > 0) {
        written = snprintf(ptr, remaining, "Souls Harvested: %u\n", rewards->soul_count);
        ptr += written;
        remaining -= written;

        /* Break down by type */
        uint8_t counts[SOUL_TYPE_COUNT] = {0};
        for (uint8_t i = 0; i < rewards->soul_count; i++) {
            if (rewards->souls_harvested[i]) {
                counts[rewards->souls_harvested[i]->type]++;
            }
        }

        for (uint8_t t = 0; t < SOUL_TYPE_COUNT; t++) {
            if (counts[t] > 0) {
                written = snprintf(ptr, remaining, "  - %s: %u\n",
                    soul_type_name(t), counts[t]);
                ptr += written;
                remaining -= written;
            }
        }
    }

    /* Corruption */
    if (rewards->corruption_change > 0) {
        written = snprintf(ptr, remaining, "\nCorruption: +%u (innocent blood spilled)\n",
            rewards->corruption_change);
        ptr += written;
        remaining -= written;
    }
}

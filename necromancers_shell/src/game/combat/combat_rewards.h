/**
 * @file combat_rewards.h
 * @brief Combat victory reward calculation and distribution
 */

#ifndef COMBAT_REWARDS_H
#define COMBAT_REWARDS_H

#include <stdint.h>
#include "../souls/soul.h"
#include "../game_state.h"

/* Forward declarations */
typedef struct CombatState CombatState;

/**
 * @brief Combat rewards structure
 */
typedef struct {
    uint32_t experience_total;       /* Total XP from enemies */
    uint32_t soul_energy_gained;     /* Soul energy from kills */
    uint8_t corruption_change;       /* +corruption if innocent enemies */
    Soul** souls_harvested;          /* New souls from corpses */
    uint8_t soul_count;             /* Number of souls */
} CombatRewards;

/**
 * @brief Calculate rewards from combat victory
 *
 * @param combat Combat state
 * @return Allocated rewards structure (caller must free with combat_rewards_destroy)
 */
CombatRewards* combat_calculate_rewards(const CombatState* combat);

/**
 * @brief Apply rewards to game state
 *
 * Updates player experience, soul energy, corruption, and adds souls
 *
 * @param rewards Rewards to apply
 * @param game_state Game state to update
 */
void combat_apply_rewards(const CombatRewards* rewards, GameState* game_state);

/**
 * @brief Free rewards structure
 *
 * @param rewards Rewards to free
 */
void combat_rewards_destroy(CombatRewards* rewards);

/**
 * @brief Format rewards as human-readable string
 *
 * @param rewards Rewards to format
 * @param buf Output buffer
 * @param buf_size Buffer size
 * @return Formatted string in buf
 */
void combat_rewards_format(const CombatRewards* rewards, char* buf, size_t buf_size);

#endif /* COMBAT_REWARDS_H */

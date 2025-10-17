/**
 * @file ending_system.h
 * @brief Ending determination and validation system
 *
 * Analyzes player choices, corruption level, trial performance, and game state
 * to determine which of the six ending paths the player qualifies for.
 *
 * Six Ending Paths:
 * 1. Revenant Route (Redemption) - Resurrect yourself, restore humanity, corruption <30%
 * 2. Lich Lord Route (Apotheosis) - Embrace undeath, immortal tyrant, corruption 100%
 * 3. Reaper Route (Service) - Become death's administrator, corruption 40-69%
 * 4. Archon Route (Revolution) - Rewrite Death Network protocols, corruption 30-60%
 * 5. Wraith Route (Freedom) - Distributed consciousness, escape the system, corruption <40%
 * 6. Morningstar Route (Transcendence) - Become 8th god, exactly 50% corruption (hardest)
 */

#ifndef ENDING_SYSTEM_H
#define ENDING_SYSTEM_H

#include "ending_types.h"
#include <stdbool.h>
#include <stdint.h>

/* Forward declaration */
typedef struct GameState GameState;

/**
 * @brief Ending requirement check result
 */
typedef struct {
    bool qualified;                 /**< True if player meets all requirements */
    char reason[256];               /**< Explanation if not qualified */
    float corruption_req;           /**< Required corruption range (min or exact) */
    float corruption_req_max;       /**< Max corruption (-1 if single value) */
    bool divine_approval_req;       /**< Requires Divine Council approval */
    int trial_score_req;            /**< Required average trial score (0-100) */
    int trials_passed_req;          /**< Minimum trials passed */
} EndingRequirement;

/**
 * @brief Ending achievement tracking
 */
typedef struct {
    EndingType ending;              /**< Which ending was achieved */
    uint32_t day_achieved;          /**< Game day when ending triggered */
    float final_corruption;         /**< Corruption percentage at ending */
    int trials_passed;              /**< Number of trials passed (0-7) */
    float avg_trial_score;          /**< Average trial score across all trials */
    bool divine_approval;           /**< Divine Council approved */
    uint32_t civilian_kills;        /**< Total civilians killed */
    bool saved_maya;                /**< Trial 6 choice: saved Maya (true) */
    bool achieved_goals;            /**< Path-specific goals met */
} EndingAchievement;

/**
 * @brief Determine which ending the player qualifies for
 *
 * Analyzes the complete game state to determine the appropriate ending.
 * Priority order if multiple endings qualify:
 * 1. Morningstar (hardest, most specific)
 * 2. Archon (requires Divine approval + trials)
 * 3. Revenant (redemption path)
 * 4. Wraith (escape path)
 * 5. Reaper (service path)
 * 6. Lich Lord (corruption path, fallback)
 *
 * @param state Game state with all player choices
 * @return Ending type achieved
 *
 * @note Returns ENDING_NONE if game is not complete
 */
EndingType determine_ending(const GameState* state);

/**
 * @brief Check if player meets requirements for specific ending
 *
 * Validates all requirements for a single ending path.
 *
 * @param state Game state
 * @param ending Ending type to check
 * @param req_out Output parameter for requirement details (can be NULL)
 * @return True if player qualifies for this ending
 */
bool check_ending_requirements(
    const GameState* state,
    EndingType ending,
    EndingRequirement* req_out
);

/**
 * @brief Get human-readable ending name
 *
 * @param ending Ending type
 * @return Ending name string (e.g., "Revenant Route - Redemption")
 */
const char* ending_get_name(EndingType ending);

/**
 * @brief Get brief ending description
 *
 * @param ending Ending type
 * @return One-sentence description
 */
const char* ending_get_description(EndingType ending);

/**
 * @brief Get ending category (thematic grouping)
 *
 * @param ending Ending type
 * @return Category string ("Redemption", "Power", "Service", "Revolution", "Freedom", "Transcendence")
 */
const char* ending_get_category(EndingType ending);

/**
 * @brief Calculate ending achievement statistics
 *
 * Populates EndingAchievement structure with all relevant metrics.
 *
 * @param state Game state
 * @param achievement Output parameter for achievement data
 */
void calculate_ending_achievement(
    const GameState* state,
    EndingAchievement* achievement
);

/**
 * @brief Validate game completion state
 *
 * Checks if the game has reached a state where an ending can be determined.
 * Requirements:
 * - Divine Judgment completed
 * - All 7 trials attempted
 * - Day >= 155 (minimum story day for judgment)
 *
 * @param state Game state
 * @return True if game is ready for ending determination
 */
bool is_game_complete(const GameState* state);

/**
 * @brief Get all endings player qualifies for
 *
 * Some players may qualify for multiple endings. This returns all valid options
 * so the player or system can choose (usually highest priority wins).
 *
 * @param state Game state
 * @param endings_out Output array (size 6)
 * @return Number of endings player qualifies for (0-6)
 */
int get_qualified_endings(
    const GameState* state,
    EndingType endings_out[6]
);

/**
 * @brief Get ending difficulty rating
 *
 * @param ending Ending type
 * @return Difficulty (1=Easy, 5=Hardest)
 */
int ending_get_difficulty(EndingType ending);

/**
 * @brief Get historical success rate (lore)
 *
 * According to story lore, some endings are rarer than others.
 *
 * @param ending Ending type
 * @return Success rate as string (e.g., "0.3%", "12%", "Unknown")
 */
const char* ending_get_success_rate(EndingType ending);

#endif /* ENDING_SYSTEM_H */

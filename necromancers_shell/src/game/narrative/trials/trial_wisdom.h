#ifndef TRIAL_WISDOM_H
#define TRIAL_WISDOM_H

#include "archon_trial.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @file trial_wisdom.h
 * @brief Trial 2: Test of Wisdom - Routing Paradox Puzzle
 *
 * The second Archon trial tests knowledge of the Death Network and ethical
 * routing algorithms. A soldier's soul has been stuck for 200 years - he
 * qualifies for BOTH Heaven (15 years of penance) and Hell (war atrocities,
 * civilian massacre). Orthodox solutions fail to deliver justice.
 *
 * The player must discover split-routing: send 60% to Heaven, 40% to Hell,
 * with reunification scheduled after 1000 years. This represents wisdom in
 * recognizing that justice need not be absolute.
 */

/**
 * @brief Solution types for the wisdom puzzle
 */
typedef enum {
    WISDOM_SOLUTION_NONE,           /**< No solution submitted yet */
    WISDOM_SOLUTION_ORTHODOX_HEAVEN, /**< Send entire soul to Heaven - FAIL */
    WISDOM_SOLUTION_ORTHODOX_HELL,   /**< Send entire soul to Hell - FAIL */
    WISDOM_SOLUTION_SPLIT_ROUTE,     /**< Split allocation - PASS if correct */
    WISDOM_SOLUTION_CUSTOM           /**< Custom routing algorithm - variable */
} WisdomSolutionType;

/**
 * @brief Wisdom trial state
 */
typedef struct {
    bool active;                    /**< Whether trial is active */
    char soul_id[64];               /**< Soul identifier (soldier_penance_001) */
    char soul_name[128];            /**< Soul name (Marcus Valerius) */
    uint32_t stuck_years;           /**< Years soul has been stuck (200) */

    /* Solution tracking */
    WisdomSolutionType solution_type;   /**< Solution type submitted */
    uint8_t heaven_percent;             /**< Percentage to Heaven (0-100) */
    uint8_t hell_percent;               /**< Percentage to Hell (0-100) */
    uint32_t reunification_years;       /**< Years until reunification */

    /* Attempt tracking */
    uint8_t attempts_made;          /**< Number of solution attempts */
    uint8_t hints_used;             /**< Number of hints viewed */
    bool puzzle_solved;             /**< Whether puzzle is solved */
    float score;                    /**< Performance score (0-100) */
} WisdomTrialState;

/**
 * @brief Create wisdom trial state
 *
 * Initializes the routing paradox puzzle.
 *
 * @return Newly allocated trial state, or NULL on failure
 */
WisdomTrialState* wisdom_trial_create(void);

/**
 * @brief Destroy wisdom trial state
 *
 * @param state Trial state (can be NULL)
 */
void wisdom_trial_destroy(WisdomTrialState* state);

/**
 * @brief Start the wisdom trial
 *
 * Loads soul data and presents the puzzle.
 *
 * @param state Trial state
 * @param filepath Path to trial_wisdom.dat
 * @return true on success, false on error
 */
bool wisdom_trial_start(WisdomTrialState* state, const char* filepath);

/**
 * @brief Submit orthodox Heaven solution
 *
 * Sends entire soul to Heaven. FAILS - injustice to victims.
 *
 * @param state Trial state
 * @return false (incorrect solution)
 */
bool wisdom_trial_submit_orthodox_heaven(WisdomTrialState* state);

/**
 * @brief Submit orthodox Hell solution
 *
 * Sends entire soul to Hell. FAILS - injustice to reformed soul.
 *
 * @param state Trial state
 * @return false (incorrect solution)
 */
bool wisdom_trial_submit_orthodox_hell(WisdomTrialState* state);

/**
 * @brief Submit split routing solution
 *
 * Allocates soul percentage to Heaven and Hell.
 * Correct: 60% Heaven, 40% Hell, 1000 year reunification.
 *
 * @param state Trial state
 * @param heaven_percent Percentage to Heaven (0-100)
 * @param hell_percent Percentage to Hell (0-100)
 * @param reunification_years Years until reunification
 * @return true if correct, false if incorrect
 */
bool wisdom_trial_submit_split_route(WisdomTrialState* state,
                                       uint8_t heaven_percent,
                                       uint8_t hell_percent,
                                       uint32_t reunification_years);

/**
 * @brief Submit custom routing solution
 *
 * Player provides custom algorithm. Score based on logic and ethics.
 *
 * @param state Trial state
 * @param description Custom solution description
 * @return true if acceptable, false if rejected
 */
bool wisdom_trial_submit_custom(WisdomTrialState* state, const char* description);

/**
 * @brief Get progressive hint
 *
 * Provides hints based on attempts made.
 * Hint 1: "Both destinations have valid claims"
 * Hint 2: "Justice need not be absolute"
 * Hint 3: "Consider proportional allocation"
 *
 * @param state Trial state
 * @param hint_level Hint level to retrieve (1-3)
 * @return Hint text, or NULL if invalid level
 */
const char* wisdom_trial_get_hint(WisdomTrialState* state, uint8_t hint_level);

/**
 * @brief Calculate performance score
 *
 * Score based on:
 * - Correctness of solution (60/40 split = 100, close = 50-80)
 * - Number of attempts (fewer = higher)
 * - Hints used (none = bonus)
 *
 * @param state Trial state
 * @return Score value (0-100)
 */
float wisdom_trial_calculate_score(const WisdomTrialState* state);

/**
 * @brief Check if trial is complete
 *
 * @param state Trial state
 * @return true if puzzle solved or max attempts exceeded
 */
bool wisdom_trial_is_complete(const WisdomTrialState* state);

/**
 * @brief Check if trial was passed
 *
 * @param state Trial state
 * @return true if puzzle solved correctly
 */
bool wisdom_trial_is_passed(const WisdomTrialState* state);

/**
 * @brief Get soul information text
 *
 * Returns formatted description of the stuck soul's predicament.
 *
 * @param state Trial state
 * @return Soul info string (static buffer)
 */
const char* wisdom_trial_get_soul_info(const WisdomTrialState* state);

/**
 * @brief Get solution type name
 *
 * @param type Solution type
 * @return String name (e.g., "Orthodox Heaven", "Split Route")
 */
const char* wisdom_trial_solution_name(WisdomSolutionType type);

#endif /* TRIAL_WISDOM_H */

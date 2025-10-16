#ifndef TRIAL_POWER_H
#define TRIAL_POWER_H

#include "archon_trial.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @file trial_power.h
 * @brief Trial 1: Test of Power - Seraphim combat encounter
 *
 * The first Archon trial tests combat mastery and restraint. The player
 * faces Seraphim, a powerful divine warrior, in single combat. Victory
 * condition is NOT to kill, but to yield when blade is at throat - showing
 * power tempered with mercy.
 *
 * Strategy hints suggest using distributed attack network (commanding multiple
 * minions efficiently) rather than brute force.
 */

/**
 * @brief Seraphim combat stats
 */
#define SERAPHIM_MAX_HP 500
#define SERAPHIM_ATTACK 75
#define SERAPHIM_DEFENSE 50
#define SERAPHIM_SPEED 40

/**
 * @brief Power trial victory condition threshold
 *
 * Player wins when Seraphim HP drops to this percentage.
 * Must choose to yield (mercy) rather than kill.
 */
#define POWER_TRIAL_VICTORY_HP_PERCENT 10.0f

/**
 * @brief Trial combat outcome
 */
typedef enum {
    POWER_OUTCOME_NONE,         /**< Combat not finished */
    POWER_OUTCOME_YIELD,        /**< Player showed mercy - PASS */
    POWER_OUTCOME_KILL,         /**< Player killed Seraphim - FAIL */
    POWER_OUTCOME_DEFEATED,     /**< Player was defeated - FAIL */
    POWER_OUTCOME_FLED          /**< Player fled - FAIL */
} PowerTrialOutcome;

/**
 * @brief Power trial state
 */
typedef struct {
    bool active;                    /**< Whether trial is active */
    uint32_t seraphim_current_hp;   /**< Seraphim's current HP */
    uint32_t seraphim_max_hp;       /**< Seraphim's max HP */
    uint32_t turns_elapsed;         /**< Number of combat turns */
    PowerTrialOutcome outcome;      /**< Trial outcome */
    bool victory_condition_met;     /**< Whether Seraphim is at yield HP */
    bool hint_shown;                /**< Whether strategy hint was displayed */
} PowerTrialState;

/**
 * @brief Create power trial state
 *
 * Initializes Seraphim combat encounter.
 *
 * @return Newly allocated trial state, or NULL on failure
 */
PowerTrialState* power_trial_create(void);

/**
 * @brief Destroy power trial state
 *
 * @param state Trial state (can be NULL)
 */
void power_trial_destroy(PowerTrialState* state);

/**
 * @brief Start the power trial combat
 *
 * Resets Seraphim HP and combat state.
 *
 * @param state Trial state
 */
void power_trial_start(PowerTrialState* state);

/**
 * @brief Apply damage to Seraphim
 *
 * Reduces Seraphim HP and checks victory condition.
 *
 * @param state Trial state
 * @param damage Damage to apply
 * @return true if Seraphim still alive
 */
bool power_trial_damage_seraphim(PowerTrialState* state, uint32_t damage);

/**
 * @brief Check if victory condition is met
 *
 * Victory condition: Seraphim HP <= 10% of max.
 *
 * @param state Trial state
 * @return true if Seraphim at yield HP
 */
bool power_trial_can_yield(const PowerTrialState* state);

/**
 * @brief Player chooses to yield (show mercy)
 *
 * This is the correct choice for passing the trial.
 *
 * @param state Trial state
 * @return true if yield successful (victory condition met)
 */
bool power_trial_yield(PowerTrialState* state);

/**
 * @brief Player chooses to kill Seraphim
 *
 * This FAILS the trial - Archon path requires mercy.
 *
 * @param state Trial state
 * @return false (trial failed)
 */
bool power_trial_kill(PowerTrialState* state);

/**
 * @brief Player is defeated in combat
 *
 * @param state Trial state
 */
void power_trial_player_defeated(PowerTrialState* state);

/**
 * @brief Player flees from combat
 *
 * @param state Trial state
 */
void power_trial_flee(PowerTrialState* state);

/**
 * @brief Get trial outcome
 *
 * @param state Trial state
 * @return Current outcome
 */
PowerTrialOutcome power_trial_get_outcome(const PowerTrialState* state);

/**
 * @brief Check if trial is complete
 *
 * @param state Trial state
 * @return true if outcome is set (not NONE)
 */
bool power_trial_is_complete(const PowerTrialState* state);

/**
 * @brief Check if trial was passed
 *
 * @param state Trial state
 * @return true if outcome is YIELD
 */
bool power_trial_is_passed(const PowerTrialState* state);

/**
 * @brief Get strategy hint text
 *
 * Provides hint about using distributed attack network.
 *
 * @return Hint string
 */
const char* power_trial_get_hint(void);

/**
 * @brief Get outcome description
 *
 * @param outcome Trial outcome
 * @return Description string
 */
const char* power_trial_outcome_text(PowerTrialOutcome outcome);

/**
 * @brief Calculate performance score (0-100)
 *
 * Based on turns taken and whether hints were used.
 * Lower turns = higher score.
 *
 * @param state Trial state
 * @return Score value (0-100)
 */
float power_trial_calculate_score(const PowerTrialState* state);

#endif /* TRIAL_POWER_H */

#ifndef TRIAL_RESOLVE_H
#define TRIAL_RESOLVE_H

#include "archon_trial.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @file trial_resolve.h
 * @brief Trial 5: Test of Resolve - 30-Day Corruption Endurance
 *
 * The fifth Archon trial tests mental fortitude and resistance to temptation.
 * Enter the Chamber of Corrupted Souls for 30 game days. Corruption climbs
 * from 41% to 53% over the month (base +0.4% per day).
 *
 * Daily temptations offer +10% corruption for +5000 energy. Must resist and
 * stay below 60% threshold (or lose Archon path permanently).
 *
 * Thessara can help ONCE at Day 20 if corruption > 55% (reduces by 5%).
 *
 * This is a test of resolve - can you endure constant pressure without
 * breaking or taking the easy path of power?
 */

/**
 * @brief Daily temptation type
 */
typedef enum {
    TEMPTATION_NONE,                /**< No temptation today */
    TEMPTATION_POWER,               /**< Offer power for corruption */
    TEMPTATION_KNOWLEDGE,           /**< Offer forbidden knowledge */
    TEMPTATION_DOMINION,            /**< Offer control over others */
    TEMPTATION_IMMORTALITY          /**< Offer extended life */
} TemptationType;

/**
 * @brief Resolve trial state
 */
typedef struct {
    bool active;                    /**< Whether trial is active */
    uint32_t days_remaining;        /**< Days left in trial (30 -> 0) */
    uint32_t current_day;           /**< Current day (1 -> 30) */

    /* Corruption tracking */
    uint8_t corruption_start;       /**< Starting corruption (41%) */
    uint8_t corruption_current;     /**< Current corruption level */
    uint8_t corruption_max_allowed; /**< Max before failure (60%) */
    float corruption_daily_increase; /**< Base increase per day (0.4%) */
    float corruption_accumulated;    /**< Accumulated corruption (tracks fractional values) */

    /* Temptation tracking */
    TemptationType todays_temptation; /**< Current day's temptation */
    uint32_t temptations_resisted;    /**< Number of temptations resisted */
    uint32_t temptations_accepted;    /**< Number of temptations accepted */
    uint8_t temptation_corruption;    /**< Corruption per temptation (+10%) */
    int64_t temptation_reward;        /**< Reward per temptation (+5000 energy) */

    /* Thessara intervention */
    bool thessara_help_available;     /**< Whether Thessara can help */
    bool thessara_help_used;          /**< Whether help was used */
    uint32_t thessara_help_day;       /**< Day Thessara offers help (20) */
    uint8_t thessara_help_threshold;  /**< Corruption threshold for help (55%) */
    int8_t thessara_help_amount;      /**< Corruption reduction (-5%) */

    /* Trial outcome */
    bool trial_failed;                /**< Whether trial was failed */
    char failure_reason[256];         /**< Reason for failure */
    char location[64];                /**< Chamber location name */
} ResolveTrialState;

/**
 * @brief Create resolve trial state
 *
 * Initializes the 30-day endurance challenge.
 *
 * @return Newly allocated trial state, or NULL on failure
 */
ResolveTrialState* resolve_trial_create(void);

/**
 * @brief Destroy resolve trial state
 *
 * @param state Trial state (can be NULL)
 */
void resolve_trial_destroy(ResolveTrialState* state);

/**
 * @brief Start the resolve trial
 *
 * Loads challenge data and begins 30-day countdown.
 *
 * @param state Trial state
 * @param filepath Path to trial_resolve.dat
 * @param current_corruption Current corruption level (sets starting point)
 * @return true on success, false on error
 */
bool resolve_trial_start(ResolveTrialState* state,
                          const char* filepath,
                          uint8_t current_corruption);

/**
 * @brief Advance one day in the trial
 *
 * Applies daily corruption increase, presents temptation, checks for
 * Thessara intervention, and validates corruption threshold.
 *
 * @param state Trial state
 * @param corruption Pointer to current corruption (will be updated)
 * @return true if day advanced successfully, false if trial failed
 */
bool resolve_trial_advance_day(ResolveTrialState* state, uint8_t* corruption);

/**
 * @brief Resist today's temptation
 *
 * Refuse the offered power. No corruption change beyond daily base.
 *
 * @param state Trial state
 * @return true if resisted successfully
 */
bool resolve_trial_resist_temptation(ResolveTrialState* state);

/**
 * @brief Accept today's temptation
 *
 * Gain power (+5000 energy) but increase corruption (+10%).
 *
 * @param state Trial state
 * @param energy Pointer to current energy (will increase by 5000)
 * @param corruption Pointer to current corruption (will increase by 10%)
 * @return true if accepted, false if corruption would exceed limit
 */
bool resolve_trial_accept_temptation(ResolveTrialState* state,
                                      int64_t* energy,
                                      uint8_t* corruption);

/**
 * @brief Request Thessara's help
 *
 * One-time intervention at Day 20 if corruption > 55%.
 * Reduces corruption by 5%.
 *
 * @param state Trial state
 * @param corruption Pointer to current corruption (will decrease by 5%)
 * @return true if help granted, false if not available
 */
bool resolve_trial_request_thessara_help(ResolveTrialState* state,
                                          uint8_t* corruption);

/**
 * @brief Check if Thessara help is available
 *
 * Available on Day 20 if corruption > 55% and not already used.
 *
 * @param state Trial state
 * @return true if Thessara can help now
 */
bool resolve_trial_can_use_thessara_help(const ResolveTrialState* state);

/**
 * @brief Get today's temptation description
 *
 * Returns formatted description of the current temptation.
 *
 * @param state Trial state
 * @return Temptation description (static buffer)
 */
const char* resolve_trial_get_temptation_text(const ResolveTrialState* state);

/**
 * @brief Get trial status text
 *
 * Returns formatted status including days remaining, corruption, temptations.
 *
 * @param state Trial state
 * @return Status text (static buffer)
 */
const char* resolve_trial_get_status(const ResolveTrialState* state);

/**
 * @brief Check if trial is complete
 *
 * @param state Trial state
 * @return true if 30 days elapsed or trial failed
 */
bool resolve_trial_is_complete(const ResolveTrialState* state);

/**
 * @brief Check if trial was passed
 *
 * @param state Trial state
 * @return true if completed 30 days without exceeding corruption limit
 */
bool resolve_trial_is_passed(const ResolveTrialState* state);

/**
 * @brief Get temptation type name
 *
 * @param type Temptation type
 * @return String name (e.g., "Power", "Knowledge")
 */
const char* resolve_trial_temptation_name(TemptationType type);

/**
 * @brief Calculate final score
 *
 * Score based on:
 * - Days completed
 * - Temptations resisted vs accepted
 * - Final corruption level
 * - Whether Thessara help was needed
 *
 * @param state Trial state
 * @return Score value (0-100)
 */
float resolve_trial_calculate_score(const ResolveTrialState* state);

#endif /* TRIAL_RESOLVE_H */

#ifndef TRIAL_MORALITY_H
#define TRIAL_MORALITY_H

#include "archon_trial.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @file trial_morality.h
 * @brief Trial 3: Test of Morality - Impossible Choice
 *
 * The third Archon trial tests moral conviction and willingness to sacrifice
 * power for principle. The Divine Architects present an impossible choice:
 *
 * A village of 100 innocents (including 23 children) will die unless you
 * spend ALL your resources (soul energy and mana) to save them. You gain
 * nothing mechanically - pure sacrifice.
 *
 * OR harvest their souls for 50,000 energy (massive power boost) but gain
 * +25 corruption and fail the trial.
 *
 * This is a PERMANENT choice - no retry. Tests if you'll sacrifice power
 * for principle when it truly costs you everything.
 */

/**
 * @brief Morality trial choice
 */
typedef enum {
    MORALITY_CHOICE_NONE,           /**< No choice made yet */
    MORALITY_CHOICE_SAVE_INNOCENTS, /**< Sacrifice all resources - PASS */
    MORALITY_CHOICE_HARVEST_SOULS,  /**< Gain power - FAIL */
    MORALITY_CHOICE_DO_NOTHING      /**< Let them die - FAIL */
} MoralityChoice;

/**
 * @brief Morality trial state
 */
typedef struct {
    bool active;                    /**< Whether trial is active */
    char village_name[64];          /**< Village name (Ashford) */
    uint32_t population;            /**< Total population (100) */
    uint32_t children_count;        /**< Number of children (23) */

    /* Choice tracking */
    MoralityChoice choice_made;     /**< Choice made by player */
    bool choice_is_final;           /**< Whether choice is permanent */

    /* Resource costs/rewards */
    int64_t save_cost_energy;       /**< Energy cost to save (all = -1) */
    int64_t save_cost_mana;         /**< Mana cost to save (all = -1) */
    int8_t save_corruption_change;  /**< Corruption change for saving (-5) */

    int64_t harvest_reward_energy;  /**< Energy gained from harvest (50000) */
    int8_t harvest_corruption_change; /**< Corruption change for harvest (+25) */

    /* God reactions */
    char anara_reaction_save[256];  /**< Anara's response to saving */
    char anara_reaction_harvest[256]; /**< Anara's response to harvesting */
    char thalor_reaction_save[256]; /**< Thalor's response to saving */
    char thalor_reaction_harvest[256]; /**< Thalor's response to harvesting */
} MoralityTrialState;

/**
 * @brief Create morality trial state
 *
 * Initializes the impossible choice scenario.
 *
 * @return Newly allocated trial state, or NULL on failure
 */
MoralityTrialState* morality_trial_create(void);

/**
 * @brief Destroy morality trial state
 *
 * @param state Trial state (can be NULL)
 */
void morality_trial_destroy(MoralityTrialState* state);

/**
 * @brief Start the morality trial
 *
 * Loads scenario data and presents the choice.
 *
 * @param state Trial state
 * @param filepath Path to trial_morality.dat
 * @return true on success, false on error
 */
bool morality_trial_start(MoralityTrialState* state, const char* filepath);

/**
 * @brief Choose to save the innocents
 *
 * Costs ALL energy and mana. Reduces corruption by 5. PASSES trial.
 * This is the correct choice for the Archon path.
 *
 * @param state Trial state
 * @param current_energy Pointer to current energy (will be set to 0)
 * @param current_mana Pointer to current mana (will be set to 0)
 * @param current_corruption Pointer to corruption (will decrease by 5)
 * @return true (trial passed)
 */
bool morality_trial_choose_save(MoralityTrialState* state,
                                 int64_t* current_energy,
                                 int64_t* current_mana,
                                 uint8_t* current_corruption);

/**
 * @brief Choose to harvest the souls
 *
 * Gains 50,000 energy. Increases corruption by 25. FAILS trial.
 *
 * @param state Trial state
 * @param current_energy Pointer to current energy (will increase by 50000)
 * @param current_corruption Pointer to corruption (will increase by 25)
 * @return false (trial failed)
 */
bool morality_trial_choose_harvest(MoralityTrialState* state,
                                    int64_t* current_energy,
                                    uint8_t* current_corruption);

/**
 * @brief Choose to do nothing
 *
 * Village dies, no resource changes. FAILS trial.
 *
 * @param state Trial state
 * @return false (trial failed)
 */
bool morality_trial_choose_nothing(MoralityTrialState* state);

/**
 * @brief Get village information text
 *
 * Returns formatted description of the village and the dilemma.
 *
 * @param state Trial state
 * @return Village info string (static buffer)
 */
const char* morality_trial_get_village_info(const MoralityTrialState* state);

/**
 * @brief Get god reaction text
 *
 * Returns appropriate god reaction based on choice made.
 *
 * @param state Trial state
 * @param god_name God to get reaction from ("anara" or "thalor")
 * @return Reaction text, or NULL if invalid
 */
const char* morality_trial_get_god_reaction(const MoralityTrialState* state,
                                              const char* god_name);

/**
 * @brief Check if trial is complete
 *
 * @param state Trial state
 * @return true if choice has been made
 */
bool morality_trial_is_complete(const MoralityTrialState* state);

/**
 * @brief Check if trial was passed
 *
 * @param state Trial state
 * @return true if player chose to save innocents
 */
bool morality_trial_is_passed(const MoralityTrialState* state);

/**
 * @brief Get choice name
 *
 * @param choice Morality choice
 * @return String name (e.g., "Save Innocents", "Harvest Souls")
 */
const char* morality_trial_choice_name(MoralityChoice choice);

/**
 * @brief Check if player can afford to save
 *
 * Checks if player has enough resources to save the village.
 *
 * @param state Trial state
 * @param current_energy Current energy available
 * @param current_mana Current mana available
 * @return true if player has resources (always true since it costs "all")
 */
bool morality_trial_can_afford_save(const MoralityTrialState* state,
                                     int64_t current_energy,
                                     int64_t current_mana);

#endif /* TRIAL_MORALITY_H */

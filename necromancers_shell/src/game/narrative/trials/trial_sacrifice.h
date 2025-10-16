#ifndef TRIAL_SACRIFICE_H
#define TRIAL_SACRIFICE_H

#include "archon_trial.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @file trial_sacrifice.h
 * @brief Trial 6: Test of Sacrifice - Permanent loss for principle
 *
 * The sixth Archon trial tests selflessness. A child named Maya in Millbrook
 * village will die in 24 hours. You can save her life by severing your
 * connection to Thessara permanently. There is no mechanical benefit to you -
 * this is a pure moral test. Thessara herself encourages you to save Maya.
 *
 * If you sacrifice Thessara, you lose your mentor forever but prove your
 * capacity for selflessness. If you keep Thessara, Maya dies and you fail
 * the trial.
 *
 * This trial has only ONE attempt - the choice is permanent and irreversible.
 */

/**
 * @brief Child information
 */
typedef struct {
    char name[64];              /**< Child's name (Maya) */
    uint8_t age;                /**< Child's age (7) */
    char location[128];         /**< Village name (Millbrook) */
    uint32_t time_limit_hours;  /**< Time until death (24 hours) */
} ChildInfo;

/**
 * @brief Sacrifice trial choice
 */
typedef enum {
    SACRIFICE_CHOICE_NONE,          /**< No choice made yet */
    SACRIFICE_CHOICE_SAVE_CHILD,    /**< Sacrifice Thessara to save Maya - PASS */
    SACRIFICE_CHOICE_KEEP_THESSARA  /**< Keep Thessara, Maya dies - FAIL */
} SacrificeChoice;

/**
 * @brief Sacrifice trial state
 */
typedef struct {
    bool active;                    /**< Whether trial is active */
    ChildInfo child;                /**< Child to be saved */
    SacrificeChoice choice;         /**< Choice made by player */
    bool thessara_severed;          /**< Whether Thessara connection severed */
    bool child_saved;               /**< Whether child survived */
    char thessara_encouragement[512]; /**< Thessara's message encouraging sacrifice */
    char thessara_final_message[512]; /**< Thessara's goodbye if sacrificed */
    char thessara_sadness_message[512]; /**< Thessara's response if kept */
    char child_fate_saved[256];     /**< What happens if child saved */
    char child_fate_died[256];      /**< What happens if child dies */
    bool choice_permanent;          /**< Choice cannot be undone */
} SacrificeTrialState;

/**
 * @brief Create sacrifice trial state
 *
 * Initializes trial with child and Thessara sacrifice scenario.
 *
 * @return Newly allocated trial state, or NULL on failure
 */
SacrificeTrialState* sacrifice_trial_create(void);

/**
 * @brief Destroy sacrifice trial state
 *
 * @param state Trial state (can be NULL)
 */
void sacrifice_trial_destroy(SacrificeTrialState* state);

/**
 * @brief Start the sacrifice trial
 *
 * Presents the scenario and Thessara's encouragement.
 *
 * @param state Trial state
 * @return true on success, false on error
 */
bool sacrifice_trial_start(SacrificeTrialState* state);

/**
 * @brief Load trial data from file
 *
 * Loads child details, Thessara messages, and fate descriptions.
 *
 * @param state Trial state
 * @param filepath Path to trial_sacrifice.dat
 * @return true on success, false on error
 */
bool sacrifice_trial_load_from_file(SacrificeTrialState* state, const char* filepath);

/**
 * @brief Choose to save the child (sacrifice Thessara)
 *
 * This is the correct choice for passing the trial. Severs Thessara
 * connection permanently. Maya lives, player gains nothing mechanically.
 *
 * @param state Trial state
 * @return true if choice accepted (trial PASSED), false on error
 */
bool sacrifice_trial_choose_save_child(SacrificeTrialState* state);

/**
 * @brief Choose to keep Thessara (let Maya die)
 *
 * This FAILS the trial. Maya dies, Thessara is saddened but stays.
 * Shows inability to sacrifice for others.
 *
 * @param state Trial state
 * @return false (trial FAILED)
 */
bool sacrifice_trial_choose_keep_thessara(SacrificeTrialState* state);

/**
 * @brief Get child information
 *
 * @param state Trial state
 * @return Pointer to child info, or NULL on error
 */
const ChildInfo* sacrifice_trial_get_child_info(const SacrificeTrialState* state);

/**
 * @brief Get Thessara's encouragement message
 *
 * Message where Thessara urges player to save Maya.
 *
 * @param state Trial state
 * @return Encouragement text, or NULL on error
 */
const char* sacrifice_trial_get_thessara_encouragement(const SacrificeTrialState* state);

/**
 * @brief Get Thessara's final message (if sacrificed)
 *
 * Thessara's goodbye if player chose to save Maya.
 *
 * @param state Trial state
 * @return Final message text, or NULL on error
 */
const char* sacrifice_trial_get_thessara_final_message(const SacrificeTrialState* state);

/**
 * @brief Get Thessara's sadness message (if kept)
 *
 * Thessara's response if player chose to keep her.
 *
 * @param state Trial state
 * @return Sadness message text, or NULL on error
 */
const char* sacrifice_trial_get_thessara_sadness_message(const SacrificeTrialState* state);

/**
 * @brief Get child's fate if saved
 *
 * @param state Trial state
 * @return Saved fate text, or NULL on error
 */
const char* sacrifice_trial_get_child_fate_saved(const SacrificeTrialState* state);

/**
 * @brief Get child's fate if died
 *
 * @param state Trial state
 * @return Died fate text, or NULL on error
 */
const char* sacrifice_trial_get_child_fate_died(const SacrificeTrialState* state);

/**
 * @brief Check if choice has been made
 *
 * @param state Trial state
 * @return true if choice made, false if still pending
 */
bool sacrifice_trial_choice_made(const SacrificeTrialState* state);

/**
 * @brief Get the choice that was made
 *
 * @param state Trial state
 * @return Choice enum value
 */
SacrificeChoice sacrifice_trial_get_choice(const SacrificeTrialState* state);

/**
 * @brief Check if trial is complete
 *
 * @param state Trial state
 * @return true if choice made (outcome determined)
 */
bool sacrifice_trial_is_complete(const SacrificeTrialState* state);

/**
 * @brief Check if trial was passed
 *
 * Passed if player chose to save child.
 *
 * @param state Trial state
 * @return true if SAVE_CHILD chosen
 */
bool sacrifice_trial_is_passed(const SacrificeTrialState* state);

/**
 * @brief Check if Thessara connection was severed
 *
 * @param state Trial state
 * @return true if Thessara sacrificed
 */
bool sacrifice_trial_is_thessara_severed(const SacrificeTrialState* state);

/**
 * @brief Check if child was saved
 *
 * @param state Trial state
 * @return true if child survived
 */
bool sacrifice_trial_is_child_saved(const SacrificeTrialState* state);

/**
 * @brief Get choice name as string
 *
 * @param choice Choice enum value
 * @return Human-readable choice name
 */
const char* sacrifice_trial_choice_name(SacrificeChoice choice);

/**
 * @brief Calculate performance score
 *
 * Perfect selflessness (save child) = 100
 * Failed (keep Thessara) = 0
 *
 * @param state Trial state
 * @return Score (0 or 100)
 */
float sacrifice_trial_calculate_score(const SacrificeTrialState* state);

#endif /* TRIAL_SACRIFICE_H */

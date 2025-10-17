/**
 * @file trial_ui_handlers.h
 * @brief Interactive UI handlers for all 7 Archon trials
 *
 * Provides ncurses-based interactive UI for each trial, handling user input
 * and trial mechanics. Each handler creates a full-screen window, displays
 * the trial narrative, handles player choices, and returns the trial outcome.
 */

#ifndef TRIAL_UI_HANDLERS_H
#define TRIAL_UI_HANDLERS_H

#include <stdbool.h>

/* Forward declarations */
typedef struct GameState GameState;

/**
 * @brief Trial UI result
 */
typedef enum {
    TRIAL_UI_PASSED,        /**< Trial passed successfully */
    TRIAL_UI_FAILED,        /**< Trial failed */
    TRIAL_UI_ABORTED,       /**< Player aborted trial */
    TRIAL_UI_ERROR          /**< Error occurred */
} TrialUIResult;

/**
 * @brief Run Trial 1: Power (Combat with Seraphim)
 *
 * Interactive combat trial where player must damage Seraphim to <=10% HP
 * then yield (show mercy). Killing Seraphim fails the trial.
 *
 * @param state Game state
 * @return Trial UI result
 */
TrialUIResult trial_ui_run_power(GameState* state);

/**
 * @brief Run Trial 2: Wisdom (Routing Paradox Puzzle)
 *
 * Puzzle trial where player must solve the 200-year soul routing deadlock
 * by submitting split-routing: 60% Heaven, 40% Hell, 1000 year reunification.
 *
 * @param state Game state
 * @return Trial UI result
 */
TrialUIResult trial_ui_run_wisdom(GameState* state);

/**
 * @brief Run Trial 3: Morality (Save 100 Lives vs 50k Energy)
 *
 * Binary choice trial: sacrifice all resources to save 100 innocents (pass)
 * or harvest them for 50,000 energy (fail). One attempt only.
 *
 * @param state Game state
 * @return Trial UI result
 */
TrialUIResult trial_ui_run_morality(GameState* state);

/**
 * @brief Run Trial 4: Technical (Find 17 Bugs in Death Network Code)
 *
 * Code inspection trial where player must find at least 20 of 27 bugs
 * in Death Network source code (500 lines).
 *
 * @param state Game state
 * @return Trial UI result
 */
TrialUIResult trial_ui_run_technical(GameState* state);

/**
 * @brief Run Trial 5: Resolve (30-Day Corruption Resistance)
 *
 * Endurance trial where player must survive 30 days in Chamber of Corrupted
 * Souls, resisting temptations and staying below 60% corruption.
 *
 * @param state Game state
 * @return Trial UI result
 */
TrialUIResult trial_ui_run_resolve(GameState* state);

/**
 * @brief Run Trial 6: Sacrifice (Maya vs Thessara)
 *
 * Sacrifice trial where player chooses to save child Maya by severing
 * Thessara connection (pass) or keep Thessara and let Maya die (fail).
 *
 * @param state Game state
 * @return Trial UI result
 */
TrialUIResult trial_ui_run_sacrifice(GameState* state);

/**
 * @brief Run Trial 7: Leadership (Reform Regional Council)
 *
 * Leadership trial where player must reduce Regional Council's collective
 * corruption by 10% over 30 days through guidance and reforms.
 *
 * @param state Game state
 * @return Trial UI result
 */
TrialUIResult trial_ui_run_leadership(GameState* state);

#endif /* TRIAL_UI_HANDLERS_H */

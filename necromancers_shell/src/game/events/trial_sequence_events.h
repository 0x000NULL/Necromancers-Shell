/**
 * @file trial_sequence_events.h
 * @brief Trial sequence event system - Manages progression through 7 Archon trials
 *
 * Provides event-driven progression through the Archon trial sequence:
 * - Trial 1: Power (Combat with Seraphim)
 * - Trial 2: Wisdom (Routing paradox puzzle)
 * - Trial 3: Morality (Save 100 lives vs 50k energy)
 * - Trial 4: Technical (Fix 17 Death Network bugs in 24 hours)
 * - Trial 5: Resolve (Resist corruption for 30 days)
 * - Trial 6: Sacrifice (Choose between Maya and Thessara)
 * - Trial 7: Leadership (Reform Regional Council)
 *
 * Each trial completion triggers the next trial unlock automatically.
 */

#ifndef NECROMANCER_TRIAL_SEQUENCE_EVENTS_H
#define NECROMANCER_TRIAL_SEQUENCE_EVENTS_H

#include <stdint.h>
#include <stdbool.h>

/* Forward declarations */
typedef struct GameState GameState;
typedef struct EventScheduler EventScheduler;

/**
 * @brief Trial sequence state
 */
typedef enum {
    TRIAL_SEQ_INACTIVE,          /**< Trials not started */
    TRIAL_SEQ_ACTIVE,            /**< At least one trial unlocked */
    TRIAL_SEQ_COMPLETED,         /**< All 7 trials passed */
    TRIAL_SEQ_FAILED             /**< One or more trials permanently failed */
} TrialSequenceState;

/**
 * @brief Trial sequence progress tracking
 */
typedef struct {
    TrialSequenceState state;
    uint8_t trials_unlocked;     /**< Bitmask of unlocked trials (1-7) */
    uint8_t trials_completed;    /**< Bitmask of completed trials (1-7) */
    uint8_t trials_failed;       /**< Bitmask of failed trials (1-7) */
    uint32_t last_completion_day; /**< Day of most recent trial completion */
    bool judgment_triggered;     /**< Whether Divine Judgment event fired */
} TrialSequenceProgress;

/**
 * @brief Register all trial sequence events
 *
 * Sets up event callbacks for trial completions that automatically
 * unlock the next trial in the sequence.
 *
 * @param scheduler Event scheduler
 * @param state Game state
 * @return Number of trial events registered
 */
uint32_t trial_sequence_register_events(EventScheduler* scheduler, GameState* state);

/**
 * @brief Event callback for trial completion
 *
 * Called when a trial is successfully completed. Unlocks the next trial
 * and provides narrative transition text.
 *
 * @param state Game state
 * @param trial_number Trial that was completed (1-7)
 * @return true on success
 */
bool trial_sequence_on_completion(GameState* state, uint32_t trial_number);

/**
 * @brief Unlock next trial in sequence
 *
 * Automatically unlocks the next trial after previous one is completed.
 * Trial 7 completion triggers Divine Judgment event.
 *
 * @param state Game state
 * @param completed_trial Trial that was just completed (1-7)
 * @return true if next trial unlocked, false if sequence complete
 */
bool trial_sequence_unlock_next(GameState* state, uint32_t completed_trial);

/**
 * @brief Trigger Divine Judgment event
 *
 * Called automatically after Trial 7 completion. Initiates the
 * final judgment by the Seven Divine Architects.
 *
 * @param state Game state
 * @return true on success
 */
bool trial_sequence_trigger_judgment(GameState* state);

/**
 * @brief Get trial sequence state
 *
 * @param state Game state
 * @return Current sequence state
 */
TrialSequenceState trial_sequence_get_state(const GameState* state);

/**
 * @brief Get trial sequence progress
 *
 * @param state Game state
 * @return Progress structure (copied)
 */
TrialSequenceProgress trial_sequence_get_progress(const GameState* state);

/**
 * @brief Check if trial is unlocked
 *
 * @param state Game state
 * @param trial_number Trial to check (1-7)
 * @return true if unlocked
 */
bool trial_sequence_is_unlocked(const GameState* state, uint32_t trial_number);

/**
 * @brief Check if trial is completed
 *
 * @param state Game state
 * @param trial_number Trial to check (1-7)
 * @return true if completed
 */
bool trial_sequence_is_completed(const GameState* state, uint32_t trial_number);

/**
 * @brief Check if trial is failed
 *
 * @param state Game state
 * @param trial_number Trial to check (1-7)
 * @return true if permanently failed
 */
bool trial_sequence_is_failed(const GameState* state, uint32_t trial_number);

/**
 * @brief Get number of trials completed
 *
 * @param state Game state
 * @return Count of completed trials (0-7)
 */
uint32_t trial_sequence_count_completed(const GameState* state);

/**
 * @brief Get number of trials failed
 *
 * @param state Game state
 * @return Count of failed trials
 */
uint32_t trial_sequence_count_failed(const GameState* state);

/**
 * @brief Check if all trials are complete
 *
 * @param state Game state
 * @return true if all 7 trials passed
 */
bool trial_sequence_all_completed(const GameState* state);

/**
 * @brief Display trial progress summary
 *
 * Shows which trials are unlocked, completed, or failed.
 *
 * @param state Game state
 */
void trial_sequence_display_progress(const GameState* state);

/**
 * @brief Reset for testing
 *
 * Resets the global trial sequence state to initial values.
 * Should only be used in unit tests.
 */
void trial_sequence_reset_for_testing(void);

#endif /* NECROMANCER_TRIAL_SEQUENCE_EVENTS_H */

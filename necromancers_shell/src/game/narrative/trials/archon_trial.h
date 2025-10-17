#ifndef ARCHON_TRIAL_H
#define ARCHON_TRIAL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @file archon_trial.h
 * @brief Archon Trial System - Seven trials for Archon transformation ending
 *
 * The Archon path represents transcendence - rising above both living and undead
 * to become a custodian of cosmic balance. Requires 30-60% corruption and
 * completion of all 7 trials.
 *
 * The seven trials test:
 * 1. Power - Combat mastery (defeat Seraphim without killing)
 * 2. Wisdom - Knowledge and understanding (divine puzzle)
 * 3. Sacrifice - Willingness to give up power for principle
 * 4. Leadership - Command without cruelty
 * 5. Mercy - Restraint and compassion
 * 6. Resolve - Conviction under pressure
 * 7. Balance - Harmony between opposing forces
 */

/**
 * @brief Maximum number of trials in the Archon path
 */
#define MAX_ARCHON_TRIALS 7

/**
 * @brief Maximum attempts allowed per trial (0 = unlimited)
 */
#define MAX_TRIAL_ATTEMPTS 3

/**
 * @brief Trial completion status
 */
typedef enum {
    TRIAL_STATUS_LOCKED,        /**< Not yet unlocked */
    TRIAL_STATUS_AVAILABLE,     /**< Unlocked but not started */
    TRIAL_STATUS_IN_PROGRESS,   /**< Currently active */
    TRIAL_STATUS_PASSED,        /**< Successfully completed */
    TRIAL_STATUS_FAILED         /**< Failed (exhausted attempts or disqualified) */
} TrialStatus;

/**
 * @brief Trial type classification
 */
typedef enum {
    TRIAL_TYPE_COMBAT,      /**< Combat/power trial */
    TRIAL_TYPE_PUZZLE,      /**< Logic/wisdom trial */
    TRIAL_TYPE_MORAL,       /**< Ethical choice trial */
    TRIAL_TYPE_TECHNICAL,   /**< Skill/technique trial */
    TRIAL_TYPE_SACRIFICE,   /**< Give up something valuable */
    TRIAL_TYPE_LEADERSHIP,  /**< Command/influence trial */
    TRIAL_TYPE_RESOLVE      /**< Endurance/determination trial */
} TrialType;

/**
 * @brief Individual Archon trial
 */
typedef struct {
    uint32_t id;                    /**< Unique trial ID */
    uint32_t trial_number;          /**< Trial number (1-7) */
    char name[64];                  /**< Trial name (e.g., "Test of Power") */
    char description[256];          /**< Short description */
    TrialType type;                 /**< Trial classification */
    TrialStatus status;             /**< Current status */
    uint8_t attempts_made;          /**< Number of attempts so far */
    uint8_t max_attempts;           /**< Maximum attempts allowed (0 = unlimited) */
    float best_score;               /**< Best performance score (0-100) */
    bool score_matters;             /**< Whether score affects outcome */
    char completion_text[512];      /**< Text shown on completion */

    /* Unlock conditions */
    uint8_t unlock_corruption_min;  /**< Minimum corruption required */
    uint8_t unlock_corruption_max;  /**< Maximum corruption allowed */
    uint8_t unlock_consciousness_min; /**< Minimum consciousness required */
    uint32_t prerequisite_trial_id; /**< Trial that must be passed first (0 = none) */
} ArchonTrial;

/**
 * @brief Archon trial manager
 */
struct ArchonTrialManager {
    ArchonTrial trials[MAX_ARCHON_TRIALS]; /**< All 7 trials */
    size_t trial_count;                     /**< Number of trials loaded */
    uint32_t current_trial_id;              /**< ID of active trial (0 = none) */
    bool path_active;                       /**< Whether Archon path is active */
    bool path_completed;                    /**< Whether all trials are passed */
    bool path_failed;                       /**< Whether path is permanently failed */
};
typedef struct ArchonTrialManager ArchonTrialManager;

/**
 * @brief Create and initialize trial manager
 *
 * Allocates manager and loads trial definitions from data file.
 *
 * @return Newly allocated trial manager, or NULL on failure
 */
ArchonTrialManager* archon_trial_manager_create(void);

/**
 * @brief Destroy trial manager
 *
 * @param manager Trial manager (can be NULL)
 */
void archon_trial_manager_destroy(ArchonTrialManager* manager);

/**
 * @brief Load trial definitions from data file
 *
 * Parses archon_trials.dat and populates trial structures.
 *
 * @param manager Trial manager
 * @param filepath Path to trial data file
 * @return true on success, false on error
 */
bool archon_trial_load_from_file(ArchonTrialManager* manager, const char* filepath);

/**
 * @brief Check if trial can be unlocked based on player state
 *
 * Checks corruption, consciousness, and prerequisite requirements.
 *
 * @param manager Trial manager
 * @param trial_id Trial to check
 * @param corruption Current corruption level (0-100)
 * @param consciousness Current consciousness level (0-100)
 * @return true if trial meets unlock conditions
 */
bool archon_trial_can_unlock(const ArchonTrialManager* manager,
                              uint32_t trial_id,
                              uint8_t corruption,
                              float consciousness);

/**
 * @brief Unlock a trial
 *
 * Changes status from LOCKED to AVAILABLE if conditions met.
 *
 * @param manager Trial manager
 * @param trial_id Trial to unlock
 * @param corruption Current corruption level
 * @param consciousness Current consciousness level
 * @return true if unlocked, false if conditions not met or already unlocked
 */
bool archon_trial_unlock(ArchonTrialManager* manager,
                         uint32_t trial_id,
                         uint8_t corruption,
                         float consciousness);

/**
 * @brief Start a trial
 *
 * Changes status from AVAILABLE to IN_PROGRESS.
 *
 * @param manager Trial manager
 * @param trial_id Trial to start
 * @return true on success, false if not available or already in progress
 */
bool archon_trial_start(ArchonTrialManager* manager, uint32_t trial_id);

/**
 * @brief Complete a trial successfully
 *
 * Changes status to PASSED and records score.
 *
 * @param manager Trial manager
 * @param trial_id Trial to complete
 * @param score Performance score (0-100, optional)
 * @return true on success, false if not in progress
 */
bool archon_trial_complete(ArchonTrialManager* manager,
                           uint32_t trial_id,
                           float score);

/**
 * @brief Fail a trial
 *
 * Increments attempt counter. If max attempts exceeded, marks as FAILED.
 * Otherwise returns to AVAILABLE.
 *
 * @param manager Trial manager
 * @param trial_id Trial that was failed
 * @return true if can retry, false if permanently failed
 */
bool archon_trial_fail(ArchonTrialManager* manager, uint32_t trial_id);

/**
 * @brief Get trial by ID
 *
 * @param manager Trial manager
 * @param trial_id Trial ID
 * @return Pointer to trial, or NULL if not found
 */
const ArchonTrial* archon_trial_get(const ArchonTrialManager* manager,
                                    uint32_t trial_id);

/**
 * @brief Get trial by number (1-7)
 *
 * @param manager Trial manager
 * @param trial_number Trial number (1-7)
 * @return Pointer to trial, or NULL if not found
 */
const ArchonTrial* archon_trial_get_by_number(const ArchonTrialManager* manager,
                                               uint32_t trial_number);

/**
 * @brief Get current active trial
 *
 * @param manager Trial manager
 * @return Pointer to active trial, or NULL if none active
 */
const ArchonTrial* archon_trial_get_current(const ArchonTrialManager* manager);

/**
 * @brief Check if all trials are completed
 *
 * @param manager Trial manager
 * @return true if all 7 trials are PASSED
 */
bool archon_trial_all_completed(const ArchonTrialManager* manager);

/**
 * @brief Get number of trials passed
 *
 * @param manager Trial manager
 * @return Count of PASSED trials
 */
size_t archon_trial_count_passed(const ArchonTrialManager* manager);

/**
 * @brief Get number of trials failed
 *
 * @param manager Trial manager
 * @return Count of FAILED trials
 */
size_t archon_trial_count_failed(const ArchonTrialManager* manager);

/**
 * @brief Check if Archon path is still available
 *
 * Path fails if any trial is permanently failed or if player
 * corruption moves outside 30-60% range after trials start.
 *
 * @param manager Trial manager
 * @param corruption Current corruption level
 * @return true if path can still be completed
 */
bool archon_trial_path_available(const ArchonTrialManager* manager,
                                  uint8_t corruption);

/**
 * @brief Activate the Archon path
 *
 * Marks path as active and unlocks Trial 1 if conditions met.
 *
 * @param manager Trial manager
 * @param corruption Current corruption level
 * @param consciousness Current consciousness level
 * @return true on success, false if conditions not met
 */
bool archon_trial_activate_path(ArchonTrialManager* manager,
                                 uint8_t corruption,
                                 float consciousness);

/**
 * @brief Get trial status name
 *
 * @param status Trial status
 * @return String name (e.g., "Available", "Passed")
 */
const char* archon_trial_status_name(TrialStatus status);

/**
 * @brief Get trial type name
 *
 * @param type Trial type
 * @return String name (e.g., "Combat", "Wisdom")
 */
const char* archon_trial_type_name(TrialType type);

#endif /* ARCHON_TRIAL_H */

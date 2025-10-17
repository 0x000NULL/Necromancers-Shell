/**
 * @file ending_system.h
 * @brief Ending system for six possible game conclusions
 *
 * Manages the six possible endings based on player choices and corruption:
 * 1. Revenant Route - Return to life (corruption <30%)
 * 2. Lich Lord Route - Embrace eternal undeath (corruption >70%)
 * 3. Reaper Route - Eternal service (corruption 40-69%)
 * 4. Archon/Morningstar Route - Become a god (consciousness decay critical)
 * 5. Wraith Route - Distributed consciousness
 * 6. Error Route - Failed Morningstar attempt
 */

#ifndef NECROMANCER_ENDING_SYSTEM_H
#define NECROMANCER_ENDING_SYSTEM_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "../narrative/endings/ending_types.h"

/* Forward declaration */
typedef struct GameState GameState;

/**
 * @brief Ending requirement structure
 */
typedef struct {
    uint8_t min_corruption;      /**< Minimum corruption required */
    uint8_t max_corruption;      /**< Maximum corruption required */
    float min_consciousness;     /**< Minimum consciousness required */
    uint32_t min_day;            /**< Minimum day required */
    bool requires_archon;        /**< Requires Archon status */
    bool requires_morningstar;   /**< Requires Morningstar attempt */
} EndingRequirements;

/**
 * @brief Ending data structure
 */
typedef struct {
    EndingType type;
    char name[64];
    char description[256];
    char achievement[64];
    EndingRequirements requirements;
    bool unlocked;               /**< Has player met requirements? */
    bool chosen;                 /**< Has player chosen this ending? */
} Ending;

/**
 * @brief Ending system state
 */
typedef struct EndingSystem {
    Ending endings[6];           /**< Six possible endings */
    EndingType chosen_ending;    /**< Which ending was chosen */
    bool game_ended;             /**< Has game ended? */
    uint32_t ending_day;         /**< Day when ending was triggered */

    /* Morningstar trial progress */
    bool morningstar_attempted;
    uint8_t trials_completed;    /**< 0-7 trials */
    bool trial_combat_vorathos;
    bool trial_kael_fragment;
    bool trial_master_magic;
    bool trial_perfect_corruption;
    bool trial_apotheosis_engine;
    bool trial_death_transcend;
    bool trial_claim_domain;
} EndingSystem;

/**
 * @brief Create ending system
 *
 * @return Newly allocated EndingSystem or NULL on failure
 */
EndingSystem* ending_system_create(void);

/**
 * @brief Destroy ending system and free memory
 *
 * @param system Ending system to destroy (can be NULL)
 */
void ending_system_destroy(EndingSystem* system);

/**
 * @brief Check which endings are available based on current game state
 *
 * Updates the unlocked status of all endings.
 *
 * @param system Ending system
 * @param state Game state
 */
void ending_system_check_availability(EndingSystem* system, const GameState* state);

/**
 * @brief Get list of available endings
 *
 * @param system Ending system
 * @param count_out Output: number of available endings
 * @return Array of available endings (do not free)
 */
const Ending** ending_system_get_available(const EndingSystem* system, size_t* count_out);

/**
 * @brief Trigger an ending
 *
 * @param system Ending system
 * @param state Game state
 * @param ending_type Which ending to trigger
 * @return true on success, false if ending not available
 */
bool ending_system_trigger(EndingSystem* system, GameState* state, EndingType ending_type);

/**
 * @brief Check if game has ended
 *
 * @param system Ending system
 * @return true if game ended, false otherwise
 */
bool ending_system_has_ended(const EndingSystem* system);

/**
 * @brief Get chosen ending type
 *
 * @param system Ending system
 * @return Chosen ending type, or ENDING_NONE if not chosen
 */
EndingType ending_system_get_chosen(const EndingSystem* system);

/**
 * @brief Get ending by type
 *
 * @param system Ending system
 * @param type Ending type
 * @return Pointer to ending or NULL if not found
 */
const Ending* ending_system_get_ending(const EndingSystem* system, EndingType type);

/**
 * @brief Start Morningstar attempt
 *
 * @param system Ending system
 * @return true on success
 */
bool ending_system_start_morningstar(EndingSystem* system);

/**
 * @brief Complete a Morningstar trial
 *
 * @param system Ending system
 * @param trial_index Trial number (0-6)
 * @return true on success
 */
bool ending_system_complete_trial(EndingSystem* system, uint8_t trial_index);

/**
 * @brief Get Morningstar trial progress
 *
 * @param system Ending system
 * @return Number of trials completed (0-7)
 */
uint8_t ending_system_get_trial_progress(const EndingSystem* system);

/**
 * @brief Check if specific ending requirement is met
 *
 * @param requirements Ending requirements
 * @param corruption Current corruption level
 * @param consciousness Current consciousness level
 * @param day_count Current day
 * @param is_archon Is player an Archon?
 * @return true if requirements met
 */
bool ending_check_requirements(const EndingRequirements* requirements,
                               uint8_t corruption, float consciousness,
                               uint32_t day_count, bool is_archon);

#endif /* NECROMANCER_ENDING_SYSTEM_H */

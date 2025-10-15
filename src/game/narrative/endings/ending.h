/**
 * @file ending.h
 * @brief Extended ending system for Necromancer's Shell
 *
 * Defines the 6 major endings (plus 1 failure state) based on player choices,
 * corruption level, consciousness, divine favor, and key events.
 */

#ifndef NECROMANCERS_ENDING_H
#define NECROMANCERS_ENDING_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

/* Maximum text sizes */
#define MAX_ENDING_NAME 128
#define MAX_ENDING_DESCRIPTION 2048
#define MAX_EPILOGUE_TEXT 4096
#define MAX_CONDITION_DESC 256

/**
 * @brief Ending types
 *
 * Seven possible endings based on player path and choices.
 */
typedef enum {
    ENDING_NONE,                    /**< No ending yet */
    ENDING_REVENANT,                /**< Resurrect to human life */
    ENDING_WRAITH,                  /**< Ascend as consciousness administrator */
    ENDING_MORNINGSTAR,             /**< Become eighth Divine Architect */
    ENDING_ARCHON,                  /**< Become death's system administrator */
    ENDING_LICH_LORD,               /**< Conquer the living */
    ENDING_OBLIVION,                /**< Voluntary dissolution */
    ENDING_DIVINE_DESTRUCTION       /**< Failure: destroyed by gods */
} EndingType;

/**
 * @brief Ending achievement status
 *
 * Tracks whether ending conditions have been met.
 */
typedef enum {
    ACHIEVEMENT_LOCKED,             /**< Ending unavailable (path locked) */
    ACHIEVEMENT_AVAILABLE,          /**< Requirements met */
    ACHIEVEMENT_UNLOCKED            /**< Ending achieved */
} EndingAchievement;

/**
 * @brief Ending requirements structure
 *
 * Defines the conditions needed to achieve each ending.
 */
typedef struct {
    /* Corruption requirements */
    uint8_t min_corruption;         /**< Minimum corruption % (0-100) */
    uint8_t max_corruption;         /**< Maximum corruption % (0-100) */

    /* Consciousness requirements */
    uint8_t min_consciousness;      /**< Minimum consciousness % (0-100) */

    /* Divine favor requirements */
    int16_t min_divine_favor;       /**< Minimum favor with at least one god */
    uint8_t gods_favor_required;    /**< Number of gods with positive favor needed */

    /* Key event flags */
    bool requires_ashbrook_spared;  /**< Must have spared Ashbrook */
    bool forbids_full_harvest;      /**< Cannot have done full village harvest */
    bool requires_thessara_trust;   /**< Must have high trust with Thessara */
    bool requires_council_verdict;  /**< Must face Divine Council judgment */
    bool requires_void_battle;      /**< Must defeat Vorathos in combat */

    /* Special requirements */
    bool requires_all_trials;       /**< Must complete all 7 trials */
    uint32_t min_souls_harvested;   /**< Minimum total souls harvested */
    uint8_t min_alliances;          /**< Minimum full alliances with council */
} EndingRequirements;

/**
 * @brief Ending data structure
 *
 * Complete information about an ending including requirements and narrative.
 */
typedef struct {
    EndingType type;
    char name[MAX_ENDING_NAME];
    char description[MAX_ENDING_DESCRIPTION];
    char epilogue[MAX_EPILOGUE_TEXT];

    EndingRequirements requirements;
    EndingAchievement achievement;

    bool path_locked;               /**< True if actions have locked this path */
    char lock_reason[MAX_CONDITION_DESC];

    uint32_t day_achieved;          /**< Day when ending was achieved (0 if not) */
} Ending;

/**
 * @brief Ending manager structure
 *
 * Tracks all possible endings and player progress toward them.
 */
typedef struct {
    Ending endings[7];              /**< All 7 possible endings */
    EndingType current_ending;      /**< Active ending if game is over */
    bool game_over;                 /**< Has game ended? */
    uint32_t ending_day;            /**< Day when game ended */
} EndingManager;

/**
 * @brief Create ending manager
 *
 * Initializes all ending definitions with requirements.
 *
 * @return Newly allocated EndingManager or NULL on failure
 */
EndingManager* ending_manager_create(void);

/**
 * @brief Destroy ending manager
 *
 * @param manager Manager to destroy (can be NULL)
 */
void ending_manager_destroy(EndingManager* manager);

/**
 * @brief Lock an ending path
 *
 * Prevents achievement of specific ending due to player action.
 *
 * @param manager Ending manager
 * @param type Ending to lock
 * @param reason Description of why it was locked
 * @return true on success, false if manager is NULL
 */
bool ending_manager_lock_path(EndingManager* manager, EndingType type, const char* reason);

/**
 * @brief Check if ending path is available
 *
 * @param manager Ending manager
 * @param type Ending to check
 * @return true if available (not locked and requirements potentially achievable)
 */
bool ending_manager_is_available(const EndingManager* manager, EndingType type);

/**
 * @brief Evaluate ending requirements
 *
 * Check if player currently meets requirements for ending.
 *
 * @param manager Ending manager
 * @param type Ending to evaluate
 * @param corruption Current corruption % (0-100)
 * @param consciousness Current consciousness % (0-100)
 * @param divine_favor Array of 7 god favor values (-100 to +100)
 * @param ashbrook_spared True if Ashbrook was spared
 * @param did_full_harvest True if any full harvest occurred
 * @param thessara_trust Thessara trust level (0-100)
 * @param council_verdict_received True if Divine Council gave verdict
 * @param void_battle_won True if defeated Vorathos
 * @param trials_completed Number of trials completed (0-7)
 * @param total_souls_harvested Total souls harvested across game
 * @param full_alliances Number of full alliances with Regional Council
 * @return true if requirements met, false otherwise
 */
bool ending_manager_check_requirements(EndingManager* manager, EndingType type,
                                       uint8_t corruption, uint8_t consciousness,
                                       const int16_t divine_favor[7],
                                       bool ashbrook_spared, bool did_full_harvest,
                                       float thessara_trust, bool council_verdict_received,
                                       bool void_battle_won, uint8_t trials_completed,
                                       uint32_t total_souls_harvested, uint8_t full_alliances);

/**
 * @brief Trigger ending
 *
 * Sets game_over and current_ending if requirements are met.
 *
 * @param manager Ending manager
 * @param type Ending to trigger
 * @param day Current game day
 * @return true if ending triggered, false if requirements not met or already game over
 */
bool ending_manager_trigger_ending(EndingManager* manager, EndingType type, uint32_t day);

/**
 * @brief Get ending name
 *
 * @param type Ending type
 * @return String name of ending
 */
const char* ending_type_name(EndingType type);

/**
 * @brief Get ending by type
 *
 * @param manager Ending manager
 * @param type Ending type
 * @return Pointer to Ending structure or NULL if not found
 */
const Ending* ending_manager_get_ending(const EndingManager* manager, EndingType type);

/**
 * @brief Get all available endings
 *
 * Returns array of ending types that are not locked.
 *
 * @param manager Ending manager
 * @param count_out Output: number of available endings
 * @return Array of available ending types (caller should not free)
 */
const EndingType* ending_manager_get_available_endings(const EndingManager* manager, size_t* count_out);

#endif /* NECROMANCERS_ENDING_H */

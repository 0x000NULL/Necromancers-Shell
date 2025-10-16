/**
 * @file null_space.h
 * @brief Null Space - Gap in Death Network topology
 *
 * Null space is a special location that exists between routing protocols,
 * neither in the Mortal Realm nor in any afterlife. It's where Thessara
 * resides and where necromancers can do deep consciousness work.
 *
 * Features:
 * - No time passage (time is frozen in null space)
 * - Direct consciousness-to-consciousness communication
 * - Access to Death Network debugging tools
 * - Safe from corruption effects while inside
 * - Meeting place with Thessara
 */

#ifndef NECROMANCER_NULL_SPACE_H
#define NECROMANCER_NULL_SPACE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Forward declarations */
typedef struct GameState GameState;

/**
 * @brief Null space activity types
 *
 * Different activities available in null space.
 */
typedef enum {
    NULL_SPACE_IDLE,              /**< Just observing */
    NULL_SPACE_MEETING_THESSARA,  /**< Meeting with Thessara */
    NULL_SPACE_NETWORK_DEBUG,     /**< Debugging Death Network */
    NULL_SPACE_MEDITATION,        /**< Consciousness stabilization */
    NULL_SPACE_RESEARCH,          /**< Accelerated research */
    NULL_SPACE_SOUL_ANALYSIS      /**< Deep soul structure analysis */
} NullSpaceActivity;

/**
 * @brief Visit record for null space
 */
typedef struct {
    uint32_t visit_day;           /**< Day of visit */
    NullSpaceActivity activity;   /**< What was done */
    uint32_t duration_hours;      /**< Subjective time spent (doesn't advance game time) */
    char notes[256];              /**< Description of what happened */
} NullSpaceVisit;

/**
 * @brief Null space state
 *
 * Tracks player's interaction with null space.
 */
typedef struct {
    /* Discovery */
    bool discovered;              /**< Has player found null space? */
    uint32_t discovery_day;       /**< Day when first discovered */

    /* Access */
    bool can_access;              /**< Can player currently access null space? */
    uint32_t access_unlock_day;   /**< Day when access was unlocked */

    /* Current state */
    bool currently_inside;        /**< Is player currently in null space? */
    NullSpaceActivity current_activity; /**< Current activity if inside */
    uint32_t entry_day;           /**< Day when entered (for current visit) */
    uint32_t subjective_hours;    /**< Subjective hours spent in current visit */

    /* Visit history */
    #define MAX_NULL_SPACE_VISITS 100
    NullSpaceVisit visits[MAX_NULL_SPACE_VISITS];
    size_t visit_count;

    /* Statistics */
    uint32_t total_visits;        /**< Total number of visits */
    uint32_t total_subjective_hours; /**< Total subjective time spent */
    uint32_t thessara_meetings;   /**< Number of Thessara meetings */
    uint32_t network_debug_count; /**< Times debugged Death Network */
    uint32_t meditation_count;    /**< Times meditated */

    /* Special unlocks */
    bool archon_workspace;        /**< Has Archon-level workspace? */
    bool wraith_observation;      /**< Can observe distributed consciousness? */
    bool network_root_access;     /**< Full Death Network access? */
} NullSpaceState;

/**
 * @brief Create null space state
 *
 * @return Newly allocated NullSpaceState or NULL on failure
 */
NullSpaceState* null_space_create(void);

/**
 * @brief Destroy null space state and free memory
 *
 * @param null_space Null space to destroy (can be NULL)
 */
void null_space_destroy(NullSpaceState* null_space);

/**
 * @brief Discover null space
 *
 * Marks null space as discovered. Usually triggered by reaching certain
 * corruption level or completing specific research.
 *
 * @param null_space Null space state
 * @param day Current game day
 * @return true on success, false if already discovered or null_space is NULL
 */
bool null_space_discover(NullSpaceState* null_space, uint32_t day);

/**
 * @brief Unlock access to null space
 *
 * Grants ability to enter/exit null space at will.
 *
 * @param null_space Null space state
 * @param day Current game day
 * @return true on success, false if already unlocked or null_space is NULL
 */
bool null_space_unlock_access(NullSpaceState* null_space, uint32_t day);

/**
 * @brief Enter null space
 *
 * Transitions player into null space. Time will be frozen until exit.
 *
 * @param null_space Null space state
 * @param day Current game day
 * @param activity Initial activity to perform
 * @return true on success, false if already inside, no access, or null_space is NULL
 */
bool null_space_enter(NullSpaceState* null_space, uint32_t day, NullSpaceActivity activity);

/**
 * @brief Exit null space
 *
 * Returns player to normal world. Records visit.
 *
 * @param null_space Null space state
 * @param notes Optional notes about the visit (can be NULL)
 * @return true on success, false if not inside or null_space is NULL
 */
bool null_space_exit(NullSpaceState* null_space, const char* notes);

/**
 * @brief Change current activity in null space
 *
 * Switch to different activity while inside null space.
 *
 * @param null_space Null space state
 * @param activity New activity
 * @return true on success, false if not inside or null_space is NULL
 */
bool null_space_change_activity(NullSpaceState* null_space, NullSpaceActivity activity);

/**
 * @brief Add subjective time spent in null space
 *
 * Increment subjective hours (doesn't advance game time).
 *
 * @param null_space Null space state
 * @param hours Subjective hours to add
 * @return true on success, false if not inside or null_space is NULL
 */
bool null_space_add_time(NullSpaceState* null_space, uint32_t hours);

/**
 * @brief Unlock Archon workspace
 *
 * Grants access to Archon-level null space features.
 *
 * @param null_space Null space state
 * @return true on success, false if already unlocked or null_space is NULL
 */
bool null_space_unlock_archon_workspace(NullSpaceState* null_space);

/**
 * @brief Unlock Wraith observation
 *
 * Allows observation of distributed consciousness patterns.
 *
 * @param null_space Null space state
 * @return true on success, false if already unlocked or null_space is NULL
 */
bool null_space_unlock_wraith_observation(NullSpaceState* null_space);

/**
 * @brief Unlock network root access
 *
 * Grants full Death Network administrative access.
 *
 * @param null_space Null space state
 * @return true on success, false if already unlocked or null_space is NULL
 */
bool null_space_unlock_network_root_access(NullSpaceState* null_space);

/**
 * @brief Check if player is inside null space
 *
 * @param null_space Null space state
 * @return true if inside, false otherwise
 */
bool null_space_is_inside(const NullSpaceState* null_space);

/**
 * @brief Check if player can access null space
 *
 * @param null_space Null space state
 * @return true if access is unlocked, false otherwise
 */
bool null_space_has_access(const NullSpaceState* null_space);

/**
 * @brief Get visit by index
 *
 * @param null_space Null space state
 * @param index Visit index (0 to visit_count-1)
 * @return Pointer to visit, or NULL if index out of range
 */
const NullSpaceVisit* null_space_get_visit(const NullSpaceState* null_space, size_t index);

/**
 * @brief Get activity name
 *
 * @param activity Activity type
 * @return String name of activity
 */
const char* null_space_activity_name(NullSpaceActivity activity);

#endif /* NECROMANCER_NULL_SPACE_H */

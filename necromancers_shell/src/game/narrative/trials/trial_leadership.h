#ifndef TRIAL_LEADERSHIP_H
#define TRIAL_LEADERSHIP_H

#include "archon_trial.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @file trial_leadership.h
 * @brief Trial 7: Test of Leadership - Council reformation over 30 days
 *
 * The seventh and final Archon trial tests leadership and diplomacy. The player
 * leads the Regional Council of 6 necromancers for 30 days. Success requires
 * reducing collective corruption by 10%+ through guidance, reforms, and
 * strategic diplomacy.
 *
 * Council members have different corruption levels, resistance to change, and
 * attitudes toward the player. Success requires balancing individual needs
 * with collective goals - proving responsible necromancy leadership.
 */

/**
 * @brief Maximum council size
 */
#define MAX_COUNCIL_MEMBERS 6

/**
 * @brief Trial duration in days
 */
#define LEADERSHIP_TRIAL_DURATION 30

/**
 * @brief Minimum council meeting interval (days)
 */
#define COUNCIL_MEETING_COOLDOWN 3

/**
 * @brief Member resistance level to change
 */
typedef enum {
    RESISTANCE_LOW,      /**< Easily guided (-1% corruption per approach) */
    RESISTANCE_MEDIUM,   /**< Moderate resistance (-2% diplomatic) */
    RESISTANCE_HIGH      /**< Strong resistance (-3% harsh) */
} ResistanceLevel;

/**
 * @brief Member attitude toward player
 */
typedef enum {
    ATTITUDE_HOSTILE,    /**< Opposes player actively */
    ATTITUDE_NEUTRAL,    /**< Indifferent, transactional */
    ATTITUDE_SUPPORTIVE  /**< Supports player's leadership */
} MemberAttitude;

/**
 * @brief Guidance approach type
 */
typedef enum {
    GUIDANCE_HARSH,          /**< -3% corruption, -10 attitude (high resistance) */
    GUIDANCE_DIPLOMATIC,     /**< -2% corruption, +5 attitude (medium resistance) */
    GUIDANCE_INSPIRATIONAL   /**< -1% corruption, +15 attitude (low resistance) */
} GuidanceApproach;

/**
 * @brief Reform policy type
 */
typedef enum {
    REFORM_CODE_OF_CONDUCT,      /**< All -1%, except Mordak/Vorgath resist */
    REFORM_SOUL_ETHICS,          /**< Low corruption -2%, high -0.5% */
    REFORM_CORRUPTION_LIMITS,    /**< All -1.5%, attitude -5 (enforced) */
    REFORM_PEER_ACCOUNTABILITY   /**< Supportive members influence others */
} ReformType;

/**
 * @brief Council member progress tracking
 */
typedef struct {
    char npc_id[64];                /**< NPC identifier */
    char name[128];                 /**< Member name */
    float corruption_start;         /**< Starting corruption (0-100) */
    float corruption_current;       /**< Current corruption (0-100) */
    float corruption_target;        /**< Target corruption for success */
    ResistanceLevel resistance;     /**< Resistance to change */
    int32_t attitude_score;         /**< Attitude score (-100 to +100) */
    MemberAttitude attitude;        /**< Current attitude */
    uint8_t reforms_applied;        /**< Number of reforms affecting this member */
    uint8_t meetings_held;          /**< Individual meetings with player */
    uint8_t last_interaction_day;   /**< Last day of interaction */
    char specialty[64];             /**< Member's specialty area */
} CouncilMemberProgress;

/**
 * @brief Leadership trial state
 */
typedef struct {
    bool active;                            /**< Whether trial is active */
    uint8_t current_day;                    /**< Current day (1-30) */
    uint8_t days_remaining;                 /**< Days left in trial */
    CouncilMemberProgress members[MAX_COUNCIL_MEMBERS]; /**< 6 council members */
    size_t member_count;                    /**< Number of members (should be 6) */
    float collective_corruption_start;      /**< Starting average corruption */
    float collective_corruption_current;    /**< Current average corruption */
    float collective_corruption_target;     /**< Target corruption (10% reduction) */
    uint8_t reforms_successful;             /**< Number of successful reforms */
    uint8_t council_meetings_held;          /**< Full council meetings */
    uint8_t last_council_meeting_day;       /**< Day of last council meeting */
    bool target_met;                        /**< Whether 10% reduction achieved */
} LeadershipTrialState;

/**
 * @brief Create leadership trial state
 *
 * Initializes council with 6 members.
 *
 * @return Newly allocated trial state, or NULL on failure
 */
LeadershipTrialState* leadership_trial_create(void);

/**
 * @brief Destroy leadership trial state
 *
 * @param state Trial state (can be NULL)
 */
void leadership_trial_destroy(LeadershipTrialState* state);

/**
 * @brief Start the leadership trial
 *
 * Begins 30-day leadership period. Resets day counter and member states.
 *
 * @param state Trial state
 * @return true on success, false on error
 */
bool leadership_trial_start(LeadershipTrialState* state);

/**
 * @brief Load trial data from file
 *
 * Loads council members and reform definitions.
 *
 * @param state Trial state
 * @param filepath Path to trial_leadership.dat
 * @return true on success, false on error
 */
bool leadership_trial_load_from_file(LeadershipTrialState* state, const char* filepath);

/**
 * @brief Advance one day in the trial
 *
 * Applies daily corruption drift, checks for events, updates state.
 *
 * @param state Trial state
 * @return true if day advanced, false if trial complete or error
 */
bool leadership_trial_advance_day(LeadershipTrialState* state);

/**
 * @brief Meet with individual council member
 *
 * One-on-one meeting to discuss reforms and apply guidance.
 *
 * @param state Trial state
 * @param member_index Member index (0-5)
 * @param approach Guidance approach to use
 * @return true on success, false on error
 */
bool leadership_trial_meet_with_member(LeadershipTrialState* state,
                                        size_t member_index,
                                        GuidanceApproach approach);

/**
 * @brief Hold full council meeting
 *
 * Address all 6 members simultaneously. Can only be called every 3 days.
 *
 * @param state Trial state
 * @return true on success, false if on cooldown or error
 */
bool leadership_trial_council_meeting(LeadershipTrialState* state);

/**
 * @brief Implement a reform policy
 *
 * Apply council-wide reform. Different reforms affect members differently.
 *
 * @param state Trial state
 * @param reform Reform type to implement
 * @return true on success, false on error
 */
bool leadership_trial_implement_reform(LeadershipTrialState* state, ReformType reform);

/**
 * @brief Calculate current collective corruption
 *
 * Average corruption across all 6 members.
 *
 * @param state Trial state
 * @return Average corruption (0-100)
 */
float leadership_trial_calculate_collective_corruption(const LeadershipTrialState* state);

/**
 * @brief Check if target reduction is met
 *
 * Target: 10% reduction from starting collective corruption.
 *
 * @param state Trial state
 * @return true if target achieved
 */
bool leadership_trial_is_target_met(const LeadershipTrialState* state);

/**
 * @brief Get member progress by index
 *
 * @param state Trial state
 * @param member_index Member index (0-5)
 * @return Pointer to member progress, or NULL on error
 */
const CouncilMemberProgress* leadership_trial_get_member(
    const LeadershipTrialState* state,
    size_t member_index);

/**
 * @brief Get member progress by NPC ID
 *
 * @param state Trial state
 * @param npc_id NPC identifier
 * @return Pointer to member progress, or NULL if not found
 */
const CouncilMemberProgress* leadership_trial_get_member_by_id(
    const LeadershipTrialState* state,
    const char* npc_id);

/**
 * @brief Check if trial is complete
 *
 * Complete when 30 days elapsed.
 *
 * @param state Trial state
 * @return true if 30 days reached
 */
bool leadership_trial_is_complete(const LeadershipTrialState* state);

/**
 * @brief Check if trial was passed
 *
 * Passed if collective corruption reduced by 10%+ after 30 days.
 *
 * @param state Trial state
 * @return true if target met and complete
 */
bool leadership_trial_is_passed(const LeadershipTrialState* state);

/**
 * @brief Get summary statistics
 *
 * Returns formatted summary of trial progress (current day, corruption,
 * members at target, reforms applied, etc.)
 *
 * @param state Trial state
 * @param buffer Buffer to store summary
 * @param buffer_size Size of buffer
 * @return Number of characters written
 */
int leadership_trial_get_summary(const LeadershipTrialState* state,
                                  char* buffer,
                                  size_t buffer_size);

/**
 * @brief Get resistance level name
 *
 * @param resistance Resistance level
 * @return String name (e.g., "Low", "Medium", "High")
 */
const char* leadership_trial_resistance_name(ResistanceLevel resistance);

/**
 * @brief Get attitude name
 *
 * @param attitude Attitude enum
 * @return String name (e.g., "Hostile", "Neutral", "Supportive")
 */
const char* leadership_trial_attitude_name(MemberAttitude attitude);

/**
 * @brief Get guidance approach name
 *
 * @param approach Guidance approach
 * @return String name (e.g., "Harsh", "Diplomatic", "Inspirational")
 */
const char* leadership_trial_guidance_name(GuidanceApproach approach);

/**
 * @brief Get reform type name
 *
 * @param reform Reform type
 * @return String name (e.g., "Code of Conduct", "Soul Ethics")
 */
const char* leadership_trial_reform_name(ReformType reform);

/**
 * @brief Calculate performance score (0-100)
 *
 * Based on:
 * - Number of members who met individual targets
 * - Total corruption reduction achieved
 * - Reforms successfully implemented
 * - Member attitudes at end
 *
 * @param state Trial state
 * @return Score (0-100)
 */
float leadership_trial_calculate_score(const LeadershipTrialState* state);

#endif /* TRIAL_LEADERSHIP_H */

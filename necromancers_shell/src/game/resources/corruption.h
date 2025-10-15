#ifndef CORRUPTION_H
#define CORRUPTION_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @file corruption.h
 * @brief Corruption tracking system
 *
 * Manages player corruption level, which affects gameplay mechanics,
 * available choices, and determines ending paths.
 */

/**
 * @brief Maximum number of corruption events to track
 */
#define MAX_CORRUPTION_EVENTS 50

/**
 * @brief Corruption tier system (10 detailed tiers)
 *
 * Corruption is tracked as a value from 0-100 with 10 distinct tiers.
 * Tier 7 (70%) is the irreversible threshold - soul becomes unrouteable.
 * Path restrictions apply based on corruption level.
 */
typedef enum {
    CORRUPTION_TIER_0 = 0,   /**< 0-10%:   Pristine - Still thinks of corpses as people */
    CORRUPTION_TIER_1 = 11,  /**< 11-20%:  Tainted - Guilt fading, rationalization begins */
    CORRUPTION_TIER_2 = 21,  /**< 21-30%:  Corrupted - Resource mentality emerging */
    CORRUPTION_TIER_3 = 31,  /**< 31-40%:  Tainted - Humanity cracking */
    CORRUPTION_TIER_4 = 41,  /**< 41-50%:  Vile - Barely human */
    CORRUPTION_TIER_5 = 51,  /**< 51-60%:  Vile - Can still feel some emotions */
    CORRUPTION_TIER_6 = 61,  /**< 61-69%:  Abyssal - 9 points from point of no return */
    CORRUPTION_TIER_7 = 70,  /**< 70%:     IRREVERSIBLE - Soul becomes unrouteable */
    CORRUPTION_TIER_8 = 71,  /**< 71-89%:  Damned - Locked into Lich/Reaper paths only */
    CORRUPTION_TIER_9 = 90,  /**< 90-99%:  Void-Touched - Near total loss */
    CORRUPTION_TIER_10 = 100 /**< 100%:    Lich Lord - Transformation complete */
} CorruptionTier;

/**
 * @brief Corruption event record
 *
 * Records a specific action that changed corruption level.
 * Used for narrative callbacks and player feedback.
 */
typedef struct {
    char description[128];  /**< Description of the corrupting action */
    int8_t change;          /**< Change in corruption (positive or negative) */
    uint32_t day;           /**< Day when event occurred */
} CorruptionEvent;

/**
 * @brief Corruption state structure
 *
 * Tracks current corruption level and history of corrupting events.
 */
typedef struct {
    uint8_t corruption;                         /**< Current corruption value (0-100) */
    CorruptionEvent events[MAX_CORRUPTION_EVENTS]; /**< History of corruption events */
    size_t event_count;                         /**< Number of events recorded */
} CorruptionState;

/**
 * @brief Initialize corruption state
 *
 * Sets corruption to 0 and clears event history.
 *
 * @param state Pointer to corruption state
 */
void corruption_init(CorruptionState* state);

/**
 * @brief Add corruption
 *
 * Increases corruption by the specified amount (capped at 100).
 * Records the event with a description.
 *
 * @param state Pointer to corruption state
 * @param amount Amount to increase (0-100)
 * @param description Description of the corrupting action
 * @param day Current game day
 * @return true on success, false if state is NULL
 */
bool corruption_add(CorruptionState* state, uint8_t amount, const char* description, uint32_t day);

/**
 * @brief Reduce corruption
 *
 * Decreases corruption by the specified amount (minimum 0).
 * Records the event with a description.
 *
 * @param state Pointer to corruption state
 * @param amount Amount to decrease (0-100)
 * @param description Description of the redeeming action
 * @param day Current game day
 * @return true on success, false if state is NULL
 */
bool corruption_reduce(CorruptionState* state, uint8_t amount, const char* description, uint32_t day);

/**
 * @brief Get current corruption tier (0-10)
 *
 * Returns the tier corresponding to current corruption percentage.
 *
 * @param state Pointer to corruption state
 * @return Current corruption tier
 */
CorruptionTier corruption_get_tier(const CorruptionState* state);

/**
 * @brief Get name of a corruption tier
 *
 * @param tier Corruption tier
 * @return String name (e.g., "Pristine", "Lich Lord")
 */
const char* corruption_tier_name(CorruptionTier tier);

/**
 * @brief Get description of current corruption state
 *
 * Returns a descriptive string based on current corruption level.
 *
 * @param state Pointer to corruption state
 * @return String describing the corruption state
 */
const char* corruption_get_description(const CorruptionState* state);

/**
 * @brief Calculate gameplay penalty based on corruption
 *
 * Returns a penalty multiplier (0.0 to 1.0) based on corruption.
 * Higher corruption may reduce effectiveness of certain actions.
 *
 * Formula: 1.0 - (corruption / 200.0)
 * This gives penalties from 0% (at 0 corruption) to 50% (at 100 corruption)
 *
 * @param state Pointer to corruption state
 * @return Penalty multiplier (0.0 to 1.0)
 */
float corruption_calculate_penalty(const CorruptionState* state);

/**
 * @brief Check if corruption has crossed irreversible threshold (>= 70%)
 *
 * Once corruption reaches 70%, soul becomes unrouteable and redemption
 * paths (Revenant, Wraith, Archon) are permanently locked.
 *
 * @param state Pointer to corruption state
 * @return true if corruption >= 70%, false otherwise
 */
bool corruption_is_irreversible(const CorruptionState* state);

/**
 * @brief Check if corruption is at Lich Lord level (>= 90%)
 *
 * @param state Pointer to corruption state
 * @return true if corruption >= 90, false otherwise
 */
bool corruption_is_damned(const CorruptionState* state);

/**
 * @brief Check if corruption is pristine (< 11%)
 *
 * @param state Pointer to corruption state
 * @return true if corruption < 11, false otherwise
 */
bool corruption_is_pure(const CorruptionState* state);

/**
 * @brief Check if Revenant path is available (< 30%)
 *
 * Revenant path requires low corruption for resurrection to mortal life.
 *
 * @param state Pointer to corruption state
 * @return true if Revenant path is available, false otherwise
 */
bool corruption_revenant_available(const CorruptionState* state);

/**
 * @brief Check if Wraith path is available (< 40%)
 *
 * Wraith path requires relatively low corruption for fragmentation.
 *
 * @param state Pointer to corruption state
 * @return true if Wraith path is available, false otherwise
 */
bool corruption_wraith_available(const CorruptionState* state);

/**
 * @brief Check if Archon path is available (30-60%)
 *
 * Archon path requires balanced corruption - not too pure, not too corrupted.
 *
 * @param state Pointer to corruption state
 * @return true if Archon path is available, false otherwise
 */
bool corruption_archon_available(const CorruptionState* state);

/**
 * @brief Get most recent corruption event
 *
 * @param state Pointer to corruption state
 * @return Pointer to most recent event, or NULL if no events
 */
const CorruptionEvent* corruption_get_latest_event(const CorruptionState* state);

/**
 * @brief Get corruption event by index
 *
 * @param state Pointer to corruption state
 * @param index Event index (0 = oldest)
 * @return Pointer to event, or NULL if index out of range
 */
const CorruptionEvent* corruption_get_event(const CorruptionState* state, size_t index);

#endif /* CORRUPTION_H */

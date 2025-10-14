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
 * @brief Corruption level thresholds
 *
 * Corruption is tracked as a value from 0-100.
 * Different thresholds unlock different mechanics and story branches.
 */
typedef enum {
    CORRUPTION_PURE = 0,         /**< 0-19% - Pure, untainted by dark magic */
    CORRUPTION_TAINTED = 20,     /**< 20-39% - Tainted, beginning to stray */
    CORRUPTION_COMPROMISED = 40, /**< 40-59% - Compromised, morality weakening */
    CORRUPTION_CORRUPTED = 60,   /**< 60-79% - Corrupted, embracing darkness */
    CORRUPTION_DAMNED = 80       /**< 80-100% - Damned, consumed by evil */
} CorruptionLevel;

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
 * @brief Get current corruption level category
 *
 * Returns the level threshold corresponding to current corruption.
 *
 * @param state Pointer to corruption state
 * @return Current corruption level
 */
CorruptionLevel corruption_get_level(const CorruptionState* state);

/**
 * @brief Get name of a corruption level
 *
 * @param level Corruption level
 * @return String name (e.g., "Pure", "Damned")
 */
const char* corruption_level_name(CorruptionLevel level);

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
 * @brief Check if corruption is at damned level
 *
 * @param state Pointer to corruption state
 * @return true if corruption >= 80, false otherwise
 */
bool corruption_is_damned(const CorruptionState* state);

/**
 * @brief Check if corruption is pure
 *
 * @param state Pointer to corruption state
 * @return true if corruption < 20, false otherwise
 */
bool corruption_is_pure(const CorruptionState* state);

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

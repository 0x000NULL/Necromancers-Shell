#ifndef CONSCIOUSNESS_H
#define CONSCIOUSNESS_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @file consciousness.h
 * @brief Consciousness decay tracking system
 *
 * Tracks identity/awareness stability separate from corruption.
 * Consciousness decays over time and affects ending availability.
 */

/**
 * @brief Consciousness state structure
 *
 * Tracks player consciousness stability, decay rate, and fragmentation.
 * Separate from corruption - consciousness is about identity/awareness,
 * while corruption is about morality.
 */
typedef struct {
    float stability;              /**< Current consciousness (0-100%) */
    float decay_rate;             /**< Decay per month (default -0.1%) */
    uint32_t months_until_critical; /**< Months until < 10% (calculated) */
    float fragmentation_level;    /**< Fragmentation progress for Wraith path (0-100%) */
    bool approaching_wraith;      /**< Warning flag when fragmentation significant */
    uint32_t last_decay_month;    /**< Last month when decay was applied */
} ConsciousnessState;

/**
 * @brief Initialize consciousness state
 *
 * Sets consciousness to 100%, decay rate to -0.1% per month.
 *
 * @param state Pointer to consciousness state
 */
void consciousness_init(ConsciousnessState* state);

/**
 * @brief Apply monthly decay to consciousness
 *
 * Reduces consciousness by decay_rate. Updates critical threshold calculations.
 * Should be called when game time crosses a month boundary.
 *
 * @param state Pointer to consciousness state
 * @param current_month Current month (to prevent double-decay)
 */
void consciousness_apply_decay(ConsciousnessState* state, uint32_t current_month);

/**
 * @brief Restore consciousness
 *
 * Increases consciousness by specified amount (harvesting souls, certain actions).
 *
 * @param state Pointer to consciousness state
 * @param amount Amount to restore (0-100)
 * @return true on success, false if state is NULL
 */
bool consciousness_restore(ConsciousnessState* state, float amount);

/**
 * @brief Reduce consciousness
 *
 * Decreases consciousness by specified amount (special events, damage).
 *
 * @param state Pointer to consciousness state
 * @param amount Amount to reduce (0-100)
 * @return true on success, false if state is NULL
 */
bool consciousness_reduce(ConsciousnessState* state, float amount);

/**
 * @brief Set decay rate
 *
 * Changes the monthly decay rate (e.g., from -0.1% to -0.2%).
 *
 * @param state Pointer to consciousness state
 * @param rate New decay rate per month
 */
void consciousness_set_decay_rate(ConsciousnessState* state, float rate);

/**
 * @brief Increase fragmentation (for Wraith path)
 *
 * Increases fragmentation level toward Wraith transformation.
 *
 * @param state Pointer to consciousness state
 * @param amount Amount to increase (0-100)
 */
void consciousness_add_fragmentation(ConsciousnessState* state, float amount);

/**
 * @brief Check if consciousness is critical (< 10%)
 *
 * @param state Pointer to consciousness state
 * @return true if consciousness < 10%, false otherwise
 */
bool consciousness_is_critical(const ConsciousnessState* state);

/**
 * @brief Check if consciousness is stable (>= 80%)
 *
 * @param state Pointer to consciousness state
 * @return true if consciousness >= 80%, false otherwise
 */
bool consciousness_is_stable(const ConsciousnessState* state);

/**
 * @brief Calculate months until critical threshold
 *
 * Calculates how many months remain until consciousness drops below 10%.
 * Returns 0 if already critical or decay_rate is non-negative.
 *
 * @param state Pointer to consciousness state
 * @return Number of months until critical (0 if already critical)
 */
uint32_t consciousness_months_until_critical(const ConsciousnessState* state);

/**
 * @brief Get description of consciousness state
 *
 * Returns a descriptive string based on current consciousness level.
 *
 * @param state Pointer to consciousness state
 * @return String describing consciousness state
 */
const char* consciousness_get_description(const ConsciousnessState* state);

/**
 * @brief Check if fragmentation is high enough for Wraith warnings
 *
 * @param state Pointer to consciousness state
 * @return true if fragmentation >= 50%, false otherwise
 */
bool consciousness_is_fragmenting(const ConsciousnessState* state);

#endif /* CONSCIOUSNESS_H */

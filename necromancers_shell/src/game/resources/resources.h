#ifndef RESOURCES_H
#define RESOURCES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @file resources.h
 * @brief Core resource management system
 *
 * Manages primary game resources including soul energy, mana,
 * and time tracking.
 */

/**
 * @brief Core resources structure
 *
 * Tracks all primary game resources and time progression.
 */
typedef struct {
    uint32_t soul_energy;    /**< Current soul energy available */
    uint32_t mana;           /**< Current mana */
    uint32_t mana_max;       /**< Maximum mana capacity */
    uint32_t day_count;      /**< Number of days elapsed since start */
    uint32_t time_hours;     /**< Current time in hours (0-23) */
} Resources;

/**
 * @brief Initialize resources to default values
 *
 * Sets initial values:
 * - soul_energy: 0
 * - mana: 100
 * - mana_max: 100
 * - day_count: 0
 * - time_hours: 0 (midnight)
 *
 * @param resources Pointer to resources structure
 */
void resources_init(Resources* resources);

/**
 * @brief Add soul energy
 *
 * @param resources Pointer to resources structure
 * @param amount Amount of soul energy to add
 * @return true on success, false if resources is NULL
 */
bool resources_add_soul_energy(Resources* resources, uint32_t amount);

/**
 * @brief Spend soul energy
 *
 * Attempts to spend the specified amount of soul energy.
 * Only succeeds if enough energy is available.
 *
 * @param resources Pointer to resources structure
 * @param amount Amount of soul energy to spend
 * @return true if successful, false if insufficient energy or resources is NULL
 */
bool resources_spend_soul_energy(Resources* resources, uint32_t amount);

/**
 * @brief Check if player has sufficient soul energy
 *
 * @param resources Pointer to resources structure
 * @param amount Amount to check for
 * @return true if player has at least this much energy, false otherwise
 */
bool resources_has_soul_energy(const Resources* resources, uint32_t amount);

/**
 * @brief Add mana (up to maximum)
 *
 * @param resources Pointer to resources structure
 * @param amount Amount of mana to add
 * @return true on success, false if resources is NULL
 */
bool resources_add_mana(Resources* resources, uint32_t amount);

/**
 * @brief Spend mana
 *
 * Attempts to spend the specified amount of mana.
 * Only succeeds if enough mana is available.
 *
 * @param resources Pointer to resources structure
 * @param amount Amount of mana to spend
 * @return true if successful, false if insufficient mana or resources is NULL
 */
bool resources_spend_mana(Resources* resources, uint32_t amount);

/**
 * @brief Check if player has sufficient mana
 *
 * @param resources Pointer to resources structure
 * @param amount Amount to check for
 * @return true if player has at least this much mana, false otherwise
 */
bool resources_has_mana(const Resources* resources, uint32_t amount);

/**
 * @brief Regenerate mana over time
 *
 * Regenerates mana based on time passed (typically called each hour).
 * Amount regenerated depends on mana_max and current mana.
 *
 * @param resources Pointer to resources structure
 * @param amount Amount of mana to regenerate (capped at mana_max)
 */
void resources_regenerate_mana(Resources* resources, uint32_t amount);

/**
 * @brief Increase maximum mana capacity
 *
 * @param resources Pointer to resources structure
 * @param amount Amount to increase maximum mana by
 * @return true on success, false if resources is NULL
 */
bool resources_increase_max_mana(Resources* resources, uint32_t amount);

/**
 * @brief Advance time by a number of hours
 *
 * Advances the game clock. When crossing midnight (24 hours),
 * increments day_count and wraps time_hours.
 *
 * @param resources Pointer to resources structure
 * @param hours Number of hours to advance
 */
void resources_advance_time(Resources* resources, uint32_t hours);

/**
 * @brief Format time as a readable string
 *
 * Formats the current time as "Day X, HH:00"
 *
 * @param resources Pointer to resources structure
 * @param buffer Buffer to write formatted string
 * @param buffer_size Size of buffer
 * @return Number of characters written (excluding null terminator)
 */
int resources_format_time(const Resources* resources, char* buffer, size_t buffer_size);

/**
 * @brief Get a descriptive string for time of day
 *
 * Returns strings like "midnight", "morning", "afternoon", "evening", "night"
 *
 * @param resources Pointer to resources structure
 * @return String describing the time of day
 */
const char* resources_get_time_of_day(const Resources* resources);

#endif /* RESOURCES_H */

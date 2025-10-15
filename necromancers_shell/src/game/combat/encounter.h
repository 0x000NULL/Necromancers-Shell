#ifndef ENCOUNTER_H
#define ENCOUNTER_H

#include "enemy.h"
#include "../world/location.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @file encounter.h
 * @brief Combat encounter generation system
 *
 * Generates combat encounters based on difficulty level and location type.
 * Encounters define which enemies appear and in what numbers.
 */

#define MAX_ENCOUNTER_ENEMIES 8

/**
 * @brief Encounter template structure
 *
 * Defines a reusable encounter template with enemy types and counts.
 */
typedef struct {
    char id[64];                            /**< Encounter ID */
    char description[256];                  /**< Encounter description */
    EnemyType enemy_types[MAX_ENCOUNTER_ENEMIES];  /**< Enemy types */
    uint8_t enemy_counts[MAX_ENCOUNTER_ENEMIES];   /**< Count per type */
    uint8_t enemy_type_count;               /**< Number of different types */
    uint8_t difficulty_rating;              /**< Difficulty (1-10) */
} EncounterTemplate;

/**
 * @brief Generate random encounter for location
 *
 * Creates an encounter appropriate for the location type and difficulty.
 *
 * @param difficulty Difficulty level (1-10)
 * @param location_type Type of location
 * @return Newly allocated encounter template, or NULL on failure
 */
EncounterTemplate* encounter_generate(uint8_t difficulty, LocationType location_type);

/**
 * @brief Create encounter from template ID
 *
 * Looks up a predefined encounter template by ID.
 *
 * @param template_id Encounter template ID
 * @return Newly allocated encounter template, or NULL if not found
 */
EncounterTemplate* encounter_create_from_template(const char* template_id);

/**
 * @brief Destroy encounter template
 *
 * @param encounter Encounter to destroy (can be NULL)
 */
void encounter_destroy(EncounterTemplate* encounter);

/**
 * @brief Spawn enemies from encounter template
 *
 * Creates actual Enemy instances based on the template.
 * Caller is responsible for freeing returned enemy array and each enemy.
 *
 * @param template Encounter template
 * @param count_out Output parameter for number of enemies spawned
 * @return Array of Enemy pointers, or NULL on failure
 */
Enemy** encounter_spawn_enemies(const EncounterTemplate* template, uint8_t* count_out);

/**
 * @brief Get total enemy count in encounter
 *
 * @param template Encounter template
 * @return Total number of enemies
 */
uint8_t encounter_get_total_enemies(const EncounterTemplate* template);

/**
 * @brief Calculate total threat level of encounter
 *
 * Sum of enemy stats weighted by count.
 *
 * @param template Encounter template
 * @return Threat level value
 */
uint32_t encounter_calculate_threat(const EncounterTemplate* template);

#endif /* ENCOUNTER_H */

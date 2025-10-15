#ifndef MINION_DATA_H
#define MINION_DATA_H

#include <stddef.h>
#include "../game/minions/minion.h"
#include "data_loader.h"

/**
 * @file minion_data.h
 * @brief Load minion type definitions from data files
 *
 * Reads minion types from data/minions.dat and provides base stats
 * for creating minions.
 */

/**
 * @brief Minion type definition loaded from data file
 *
 * Contains all the base properties for a minion type that can be
 * used to create instances of that minion.
 */
typedef struct {
    MinionType type;              /**< Minion type enum */
    char name[64];                /**< Display name */
    char description[512];        /**< Lore and gameplay description */
    uint32_t base_health;         /**< Base health points */
    uint32_t base_attack;         /**< Base attack power */
    uint32_t base_defense;        /**< Base defense rating */
    uint32_t base_speed;          /**< Base speed rating */
    uint8_t base_loyalty;         /**< Starting loyalty (0-100) */
    uint32_t raise_cost;          /**< Soul energy cost to raise */
    uint8_t unlock_level;         /**< Level required to unlock */
    char role[32];                /**< Role (tank, scout, striker, etc.) */
    char specialization[64];      /**< Combat specialization */
} MinionTypeDefinition;

/**
 * @brief Load minion type definitions from data file
 *
 * Reads all [MINION:*] sections from the data file and stores the
 * base stats for each minion type. This should be called once at
 * game initialization.
 *
 * @param data_file Loaded data file
 * @param definitions Array to store loaded definitions (must be size MINION_TYPE_COUNT)
 * @return Number of definitions loaded, or 0 on error
 */
size_t minion_data_load_definitions(const DataFile* data_file, MinionTypeDefinition* definitions);

/**
 * @brief Create a MinionTypeDefinition from a data section
 *
 * Parses all properties from the section and creates a definition.
 * The minion type is inferred from the section ID.
 *
 * @param section Data section with minion properties
 * @return 0 on success, -1 on error (stores in provided definition)
 */
int minion_data_create_definition(const DataSection* section, MinionTypeDefinition* def);

/**
 * @brief Get the MinionType enum value from a string ID
 *
 * Maps string IDs (zombie, skeleton, etc.) to enum values.
 *
 * @param type_str String type identifier
 * @return MinionType enum value, or MINION_TYPE_COUNT on error
 */
MinionType minion_data_parse_type(const char* type_str);

/**
 * @brief Apply base stats from definition to a MinionStats structure
 *
 * Fills the stats structure with base values from the definition.
 *
 * @param def Minion type definition
 * @param stats Stats structure to fill
 */
void minion_data_apply_base_stats(const MinionTypeDefinition* def, MinionStats* stats);

#endif /* MINION_DATA_H */

#ifndef SPELL_DATA_H
#define SPELL_DATA_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "data_loader.h"

/**
 * @file spell_data.h
 * @brief Load spell definitions from data files
 *
 * Reads spells from data/spells.dat and provides spell definitions
 * that can be used in the spell system.
 */

/**
 * @brief Spell target types
 */
typedef enum {
    SPELL_TARGET_SELF,        /**< Targets the caster */
    SPELL_TARGET_SINGLE,      /**< Targets a single enemy/ally */
    SPELL_TARGET_AREA,        /**< Targets an area/multiple targets */
    SPELL_TARGET_LOCATION,    /**< Targets a location */
    SPELL_TARGET_COUNT
} SpellTargetType;

/**
 * @brief Spell schools/categories
 */
typedef enum {
    SPELL_SCHOOL_NECROMANCY,  /**< Raising undead, soul manipulation */
    SPELL_SCHOOL_SHADOW,      /**< Dark energy, curses */
    SPELL_SCHOOL_DEATH,       /**< Death magic, decay */
    SPELL_SCHOOL_BLOOD,       /**< Life force, vampiric */
    SPELL_SCHOOL_BONE,        /**< Bone magic, conjuration */
    SPELL_SCHOOL_COUNT
} SpellSchool;

/**
 * @brief Spell definition loaded from data file
 */
typedef struct {
    char id[64];              /**< Spell identifier (e.g., "raise_dead") */
    char name[64];            /**< Display name */
    char description[512];    /**< What the spell does */
    uint32_t mana_cost;       /**< Mana required to cast */
    uint32_t soul_cost;       /**< Soul energy cost */
    uint8_t cooldown;         /**< Turns before recast */
    SpellTargetType target_type;  /**< How the spell targets */
    uint8_t unlock_level;     /**< Level required to unlock */
    SpellSchool school;       /**< Magic school */
    char effect_type[64];     /**< Type of effect (damage, summon, etc.) */
    uint32_t effect_value;    /**< Numerical effect value */
} SpellDefinition;

/**
 * @brief Load spell definitions from data file
 *
 * Reads all [SPELL:*] sections from the data file and creates
 * spell definitions.
 *
 * @param data_file Loaded data file
 * @param spells Array to store loaded spells (caller allocates)
 * @param max_spells Maximum number of spells to load
 * @return Number of spells loaded, or 0 on error
 */
size_t spell_data_load_definitions(const DataFile* data_file, SpellDefinition* spells, size_t max_spells);

/**
 * @brief Create a spell definition from a data section
 *
 * Parses all properties from the section and creates a spell definition.
 *
 * @param section Data section with spell properties
 * @param spell Pointer to spell definition to fill
 * @return 0 on success, -1 on error
 */
int spell_data_create_definition(const DataSection* section, SpellDefinition* spell);

/**
 * @brief Parse spell target type from string
 *
 * @param type_str String type identifier
 * @return SpellTargetType enum value, or SPELL_TARGET_COUNT on error
 */
SpellTargetType spell_data_parse_target_type(const char* type_str);

/**
 * @brief Parse spell school from string
 *
 * @param school_str String school identifier
 * @return SpellSchool enum value, or SPELL_SCHOOL_COUNT on error
 */
SpellSchool spell_data_parse_school(const char* school_str);

/**
 * @brief Get spell school name
 *
 * @param school Spell school enum
 * @return String name of school
 */
const char* spell_data_school_name(SpellSchool school);

/**
 * @brief Get spell target type name
 *
 * @param type Target type enum
 * @return String name of target type
 */
const char* spell_data_target_type_name(SpellTargetType type);

#endif /* SPELL_DATA_H */

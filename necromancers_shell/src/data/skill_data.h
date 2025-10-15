#ifndef SKILL_DATA_H
#define SKILL_DATA_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "data_loader.h"

/**
 * @file skill_data.h
 * @brief Load skill definitions from data files
 *
 * Reads skills from data/skills.dat and provides skill definitions
 * for the progression system.
 */

/**
 * @brief Skill categories
 */
typedef enum {
    SKILL_CATEGORY_COMBAT,      /**< Combat effectiveness */
    SKILL_CATEGORY_NECROMANCY,  /**< Minion bonuses */
    SKILL_CATEGORY_UTILITY,     /**< Resource management */
    SKILL_CATEGORY_MASTERY,     /**< Late-game powerful bonuses */
    SKILL_CATEGORY_COUNT
} SkillCategory;

/**
 * @brief Skill definition loaded from data file
 */
typedef struct {
    char id[64];              /**< Skill identifier (e.g., "shadow_affinity") */
    char name[64];            /**< Display name */
    char description[512];    /**< What the skill does */
    uint8_t max_rank;         /**< Maximum investment level */
    uint8_t unlock_level;     /**< Player level required for rank 1 */
    SkillCategory category;   /**< Skill category */
    char prerequisite[64];    /**< Required skill ID ("none" if no prerequisite) */
    uint32_t effect_per_rank; /**< Numerical effect per rank */
    char effect_type[64];     /**< Type of effect */
} SkillDefinition;

/**
 * @brief Load skill definitions from data file
 *
 * Reads all [SKILL:*] sections from the data file and creates
 * skill definitions.
 *
 * @param data_file Loaded data file
 * @param skills Array to store loaded skills (caller allocates)
 * @param max_skills Maximum number of skills to load
 * @return Number of skills loaded, or 0 on error
 */
size_t skill_data_load_definitions(const DataFile* data_file, SkillDefinition* skills, size_t max_skills);

/**
 * @brief Create a skill definition from a data section
 *
 * Parses all properties from the section and creates a skill definition.
 *
 * @param section Data section with skill properties
 * @param skill Pointer to skill definition to fill
 * @return 0 on success, -1 on error
 */
int skill_data_create_definition(const DataSection* section, SkillDefinition* skill);

/**
 * @brief Parse skill category from string
 *
 * @param category_str String category identifier
 * @return SkillCategory enum value, or SKILL_CATEGORY_COUNT on error
 */
SkillCategory skill_data_parse_category(const char* category_str);

/**
 * @brief Get skill category name
 *
 * @param category Skill category enum
 * @return String name of category
 */
const char* skill_data_category_name(SkillCategory category);

#endif /* SKILL_DATA_H */

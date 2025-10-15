#define _POSIX_C_SOURCE 200809L

#include "skill_data.h"
#include "../utils/logger.h"
#include <stdlib.h>
#include <string.h>

/**
 * @file skill_data.c
 * @brief Implementation of skill data loader
 */

/**
 * @brief Parse skill category from string
 */
SkillCategory skill_data_parse_category(const char* category_str) {
    if (strcmp(category_str, "combat") == 0) {
        return SKILL_CATEGORY_COMBAT;
    } else if (strcmp(category_str, "necromancy") == 0) {
        return SKILL_CATEGORY_NECROMANCY;
    } else if (strcmp(category_str, "utility") == 0) {
        return SKILL_CATEGORY_UTILITY;
    } else if (strcmp(category_str, "mastery") == 0) {
        return SKILL_CATEGORY_MASTERY;
    }

    LOG_WARN("Unknown skill category: %s", category_str);
    return SKILL_CATEGORY_COUNT;
}

/**
 * @brief Get skill category name
 */
const char* skill_data_category_name(SkillCategory category) {
    switch (category) {
        case SKILL_CATEGORY_COMBAT: return "Combat";
        case SKILL_CATEGORY_NECROMANCY: return "Necromancy";
        case SKILL_CATEGORY_UTILITY: return "Utility";
        case SKILL_CATEGORY_MASTERY: return "Mastery";
        default: return "Unknown";
    }
}

/**
 * @brief Create skill definition from data section
 */
int skill_data_create_definition(const DataSection* section, SkillDefinition* skill) {
    if (!section || !skill) {
        LOG_ERROR("skill_data_create_definition: NULL parameter");
        return -1;
    }

    /* Clear skill */
    memset(skill, 0, sizeof(SkillDefinition));

    /* Store skill ID from section ID */
    size_t id_len = strlen(section->section_id);
    if (id_len >= sizeof(skill->id)) {
        id_len = sizeof(skill->id) - 1;
    }
    memcpy(skill->id, section->section_id, id_len);
    skill->id[id_len] = '\0';

    /* Extract required properties */
    const char* name = data_value_get_string(
        data_section_get(section, "name"), "Unknown Skill");

    const char* description = data_value_get_string(
        data_section_get(section, "description"), "No description available.");

    const char* category_str = data_value_get_string(
        data_section_get(section, "category"), "utility");

    const char* prerequisite = data_value_get_string(
        data_section_get(section, "prerequisite"), "none");

    const char* effect_type = data_value_get_string(
        data_section_get(section, "effect_type"), "none");

    /* Copy strings with bounds checking */
    size_t name_len = strlen(name);
    if (name_len >= sizeof(skill->name)) {
        name_len = sizeof(skill->name) - 1;
    }
    memcpy(skill->name, name, name_len);
    skill->name[name_len] = '\0';

    size_t desc_len = strlen(description);
    if (desc_len >= sizeof(skill->description)) {
        desc_len = sizeof(skill->description) - 1;
    }
    memcpy(skill->description, description, desc_len);
    skill->description[desc_len] = '\0';

    size_t prereq_len = strlen(prerequisite);
    if (prereq_len >= sizeof(skill->prerequisite)) {
        prereq_len = sizeof(skill->prerequisite) - 1;
    }
    memcpy(skill->prerequisite, prerequisite, prereq_len);
    skill->prerequisite[prereq_len] = '\0';

    size_t effect_len = strlen(effect_type);
    if (effect_len >= sizeof(skill->effect_type)) {
        effect_len = sizeof(skill->effect_type) - 1;
    }
    memcpy(skill->effect_type, effect_type, effect_len);
    skill->effect_type[effect_len] = '\0';

    /* Parse category */
    skill->category = skill_data_parse_category(category_str);

    /* Extract numeric properties */
    skill->max_rank = (uint8_t)data_value_get_int(
        data_section_get(section, "max_rank"), 1);

    skill->unlock_level = (uint8_t)data_value_get_int(
        data_section_get(section, "unlock_level"), 0);

    skill->effect_per_rank = (uint32_t)data_value_get_int(
        data_section_get(section, "effect_per_rank"), 5);

    LOG_DEBUG("Created skill definition: %s (max rank %u, unlock lvl %u)",
              skill->name, skill->max_rank, skill->unlock_level);

    return 0;
}

/**
 * @brief Load all skill definitions from data file
 */
size_t skill_data_load_definitions(const DataFile* data_file, SkillDefinition* skills, size_t max_skills) {
    if (!data_file || !skills || max_skills == 0) {
        LOG_ERROR("skill_data_load_definitions: Invalid parameter");
        return 0;
    }

    /* Get all SKILL sections */
    size_t section_count = 0;
    const DataSection** sections = data_file_get_sections(data_file, "SKILL", &section_count);

    if (!sections || section_count == 0) {
        LOG_WARN("No SKILL sections found in data file");
        return 0;
    }

    LOG_INFO("Loading %zu skill definitions from data file", section_count);

    /* Create definitions for each section */
    size_t loaded_count = 0;
    for (size_t i = 0; i < section_count && loaded_count < max_skills; i++) {
        if (skill_data_create_definition(sections[i], &skills[loaded_count]) == 0) {
            loaded_count++;
        }
    }

    free((void*)sections);

    LOG_INFO("Loaded %zu/%zu skill definitions successfully", loaded_count, section_count);
    return loaded_count;
}

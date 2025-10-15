#define _POSIX_C_SOURCE 200809L

#include "spell_data.h"
#include "../utils/logger.h"
#include <stdlib.h>
#include <string.h>

/**
 * @file spell_data.c
 * @brief Implementation of spell data loader
 */

/**
 * @brief Parse spell target type from string
 */
SpellTargetType spell_data_parse_target_type(const char* type_str) {
    if (strcmp(type_str, "self") == 0) {
        return SPELL_TARGET_SELF;
    } else if (strcmp(type_str, "single") == 0) {
        return SPELL_TARGET_SINGLE;
    } else if (strcmp(type_str, "area") == 0) {
        return SPELL_TARGET_AREA;
    } else if (strcmp(type_str, "location") == 0) {
        return SPELL_TARGET_LOCATION;
    }

    LOG_WARN("Unknown spell target type: %s", type_str);
    return SPELL_TARGET_COUNT;
}

/**
 * @brief Parse spell school from string
 */
SpellSchool spell_data_parse_school(const char* school_str) {
    if (strcmp(school_str, "necromancy") == 0) {
        return SPELL_SCHOOL_NECROMANCY;
    } else if (strcmp(school_str, "shadow") == 0) {
        return SPELL_SCHOOL_SHADOW;
    } else if (strcmp(school_str, "death") == 0) {
        return SPELL_SCHOOL_DEATH;
    } else if (strcmp(school_str, "blood") == 0) {
        return SPELL_SCHOOL_BLOOD;
    } else if (strcmp(school_str, "bone") == 0) {
        return SPELL_SCHOOL_BONE;
    }

    LOG_WARN("Unknown spell school: %s", school_str);
    return SPELL_SCHOOL_COUNT;
}

/**
 * @brief Get spell school name
 */
const char* spell_data_school_name(SpellSchool school) {
    switch (school) {
        case SPELL_SCHOOL_NECROMANCY: return "Necromancy";
        case SPELL_SCHOOL_SHADOW: return "Shadow";
        case SPELL_SCHOOL_DEATH: return "Death";
        case SPELL_SCHOOL_BLOOD: return "Blood";
        case SPELL_SCHOOL_BONE: return "Bone";
        default: return "Unknown";
    }
}

/**
 * @brief Get spell target type name
 */
const char* spell_data_target_type_name(SpellTargetType type) {
    switch (type) {
        case SPELL_TARGET_SELF: return "Self";
        case SPELL_TARGET_SINGLE: return "Single Target";
        case SPELL_TARGET_AREA: return "Area";
        case SPELL_TARGET_LOCATION: return "Location";
        default: return "Unknown";
    }
}

/**
 * @brief Create spell definition from data section
 */
int spell_data_create_definition(const DataSection* section, SpellDefinition* spell) {
    if (!section || !spell) {
        LOG_ERROR("spell_data_create_definition: NULL parameter");
        return -1;
    }

    /* Clear spell */
    memset(spell, 0, sizeof(SpellDefinition));

    /* Store spell ID from section ID */
    size_t id_len = strlen(section->section_id);
    if (id_len >= sizeof(spell->id)) {
        id_len = sizeof(spell->id) - 1;
    }
    memcpy(spell->id, section->section_id, id_len);
    spell->id[id_len] = '\0';

    /* Extract required properties */
    const char* name = data_value_get_string(
        data_section_get(section, "name"), "Unknown Spell");

    const char* description = data_value_get_string(
        data_section_get(section, "description"), "No description available.");

    const char* target_type_str = data_value_get_string(
        data_section_get(section, "target_type"), "self");

    const char* school_str = data_value_get_string(
        data_section_get(section, "school"), "necromancy");

    const char* effect_type = data_value_get_string(
        data_section_get(section, "effect_type"), "none");

    /* Copy strings with bounds checking */
    size_t name_len = strlen(name);
    if (name_len >= sizeof(spell->name)) {
        name_len = sizeof(spell->name) - 1;
    }
    memcpy(spell->name, name, name_len);
    spell->name[name_len] = '\0';

    size_t desc_len = strlen(description);
    if (desc_len >= sizeof(spell->description)) {
        desc_len = sizeof(spell->description) - 1;
    }
    memcpy(spell->description, description, desc_len);
    spell->description[desc_len] = '\0';

    size_t effect_len = strlen(effect_type);
    if (effect_len >= sizeof(spell->effect_type)) {
        effect_len = sizeof(spell->effect_type) - 1;
    }
    memcpy(spell->effect_type, effect_type, effect_len);
    spell->effect_type[effect_len] = '\0';

    /* Parse enums */
    spell->target_type = spell_data_parse_target_type(target_type_str);
    spell->school = spell_data_parse_school(school_str);

    /* Extract numeric properties */
    spell->mana_cost = (uint32_t)data_value_get_int(
        data_section_get(section, "mana_cost"), 10);

    spell->soul_cost = (uint32_t)data_value_get_int(
        data_section_get(section, "soul_cost"), 0);

    spell->cooldown = (uint8_t)data_value_get_int(
        data_section_get(section, "cooldown"), 0);

    spell->unlock_level = (uint8_t)data_value_get_int(
        data_section_get(section, "unlock_level"), 0);

    spell->effect_value = (uint32_t)data_value_get_int(
        data_section_get(section, "effect_value"), 0);

    LOG_DEBUG("Created spell definition: %s (mana %u, soul %u)",
              spell->name, spell->mana_cost, spell->soul_cost);

    return 0;
}

/**
 * @brief Load all spell definitions from data file
 */
size_t spell_data_load_definitions(const DataFile* data_file, SpellDefinition* spells, size_t max_spells) {
    if (!data_file || !spells || max_spells == 0) {
        LOG_ERROR("spell_data_load_definitions: Invalid parameter");
        return 0;
    }

    /* Get all SPELL sections */
    size_t section_count = 0;
    const DataSection** sections = data_file_get_sections(data_file, "SPELL", &section_count);

    if (!sections || section_count == 0) {
        LOG_WARN("No SPELL sections found in data file");
        return 0;
    }

    LOG_INFO("Loading %zu spell definitions from data file", section_count);

    /* Create definitions for each section */
    size_t loaded_count = 0;
    for (size_t i = 0; i < section_count && loaded_count < max_spells; i++) {
        if (spell_data_create_definition(sections[i], &spells[loaded_count]) == 0) {
            loaded_count++;
        }
    }

    free((void*)sections);

    LOG_INFO("Loaded %zu/%zu spell definitions successfully", loaded_count, section_count);
    return loaded_count;
}

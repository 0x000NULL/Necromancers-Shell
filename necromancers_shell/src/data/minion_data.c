#define _POSIX_C_SOURCE 200809L

#include "minion_data.h"
#include "../utils/logger.h"
#include <stdlib.h>
#include <string.h>

/**
 * @file minion_data.c
 * @brief Implementation of minion data loader
 */

/**
 * @brief Parse minion type from string identifier
 */
MinionType minion_data_parse_type(const char* type_str) {
    if (strcmp(type_str, "zombie") == 0) {
        return MINION_TYPE_ZOMBIE;
    } else if (strcmp(type_str, "skeleton") == 0) {
        return MINION_TYPE_SKELETON;
    } else if (strcmp(type_str, "ghoul") == 0) {
        return MINION_TYPE_GHOUL;
    } else if (strcmp(type_str, "wraith") == 0) {
        return MINION_TYPE_WRAITH;
    } else if (strcmp(type_str, "wight") == 0) {
        return MINION_TYPE_WIGHT;
    } else if (strcmp(type_str, "revenant") == 0) {
        return MINION_TYPE_REVENANT;
    }

    LOG_WARN("Unknown minion type: %s", type_str);
    return MINION_TYPE_COUNT;
}

/**
 * @brief Create MinionTypeDefinition from data section
 */
int minion_data_create_definition(const DataSection* section, MinionTypeDefinition* def) {
    if (!section || !def) {
        LOG_ERROR("minion_data_create_definition: NULL parameter");
        return -1;
    }

    /* Clear definition */
    memset(def, 0, sizeof(MinionTypeDefinition));

    /* Parse minion type from section ID */
    def->type = minion_data_parse_type(section->section_id);
    if (def->type == MINION_TYPE_COUNT) {
        LOG_ERROR("Invalid minion type in section: %s", section->section_id);
        return -1;
    }

    /* Extract required properties */
    const char* name = data_value_get_string(
        data_section_get(section, "name"), "Unknown Minion");

    const char* description = data_value_get_string(
        data_section_get(section, "description"), "No description available.");

    const char* role = data_value_get_string(
        data_section_get(section, "role"), "unknown");

    const char* specialization = data_value_get_string(
        data_section_get(section, "specialization"), "none");

    /* Copy strings with bounds checking */
    size_t name_len = strlen(name);
    if (name_len >= sizeof(def->name)) {
        name_len = sizeof(def->name) - 1;
    }
    memcpy(def->name, name, name_len);
    def->name[name_len] = '\0';

    size_t desc_len = strlen(description);
    if (desc_len >= sizeof(def->description)) {
        desc_len = sizeof(def->description) - 1;
    }
    memcpy(def->description, description, desc_len);
    def->description[desc_len] = '\0';

    size_t role_len = strlen(role);
    if (role_len >= sizeof(def->role)) {
        role_len = sizeof(def->role) - 1;
    }
    memcpy(def->role, role, role_len);
    def->role[role_len] = '\0';

    size_t spec_len = strlen(specialization);
    if (spec_len >= sizeof(def->specialization)) {
        spec_len = sizeof(def->specialization) - 1;
    }
    memcpy(def->specialization, specialization, spec_len);
    def->specialization[spec_len] = '\0';

    /* Extract numeric properties */
    def->base_health = (uint32_t)data_value_get_int(
        data_section_get(section, "base_health"), 50);

    def->base_attack = (uint32_t)data_value_get_int(
        data_section_get(section, "base_attack"), 10);

    def->base_defense = (uint32_t)data_value_get_int(
        data_section_get(section, "base_defense"), 10);

    def->base_speed = (uint32_t)data_value_get_int(
        data_section_get(section, "base_speed"), 10);

    def->base_loyalty = (uint8_t)data_value_get_int(
        data_section_get(section, "base_loyalty"), 50);

    def->raise_cost = (uint32_t)data_value_get_int(
        data_section_get(section, "raise_cost"), 100);

    def->unlock_level = (uint8_t)data_value_get_int(
        data_section_get(section, "unlock_level"), 0);

    LOG_DEBUG("Created minion definition: %s (type %d, cost %u)",
              def->name, def->type, def->raise_cost);

    return 0;
}

/**
 * @brief Load all minion type definitions from data file
 */
size_t minion_data_load_definitions(const DataFile* data_file, MinionTypeDefinition* definitions) {
    if (!data_file || !definitions) {
        LOG_ERROR("minion_data_load_definitions: NULL parameter");
        return 0;
    }

    /* Get all MINION sections */
    size_t section_count = 0;
    const DataSection** sections = data_file_get_sections(data_file, "MINION", &section_count);

    if (!sections || section_count == 0) {
        LOG_WARN("No MINION sections found in data file");
        return 0;
    }

    LOG_INFO("Loading %zu minion type definitions from data file", section_count);

    /* Create definitions for each section */
    size_t loaded_count = 0;
    for (size_t i = 0; i < section_count; i++) {
        MinionTypeDefinition def;
        if (minion_data_create_definition(sections[i], &def) == 0) {
            /* Validate type is within bounds */
            if (def.type < MINION_TYPE_COUNT) {
                /* Store in the appropriate slot */
                definitions[def.type] = def;
                loaded_count++;
            } else {
                LOG_ERROR("Minion type out of bounds: %d", def.type);
            }
        }
    }

    free((void*)sections);

    LOG_INFO("Loaded %zu/%zu minion type definitions successfully", loaded_count, section_count);
    return loaded_count;
}

/**
 * @brief Apply base stats from definition to MinionStats
 */
void minion_data_apply_base_stats(const MinionTypeDefinition* def, MinionStats* stats) {
    if (!def || !stats) {
        LOG_WARN("minion_data_apply_base_stats: NULL parameter");
        return;
    }

    stats->health = def->base_health;
    stats->health_max = def->base_health;
    stats->attack = def->base_attack;
    stats->defense = def->base_defense;
    stats->speed = def->base_speed;
    stats->loyalty = def->base_loyalty;
}

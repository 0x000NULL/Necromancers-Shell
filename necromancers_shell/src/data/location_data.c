#define _POSIX_C_SOURCE 200809L

#include "location_data.h"
#include "../utils/logger.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/**
 * @file location_data.c
 * @brief Implementation of location data loader
 */

/**
 * @brief Simple hash function for string IDs
 *
 * Converts string IDs to uint32_t for use in Location objects.
 */
static uint32_t hash_string_id(const char* str) {
    uint32_t hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + (uint32_t)c; /* hash * 33 + c */
    }

    return hash;
}

/**
 * @brief Parse location type from string
 */
static LocationType parse_location_type(const char* type_str) {
    if (strcmp(type_str, "graveyard") == 0) {
        return LOCATION_TYPE_GRAVEYARD;
    } else if (strcmp(type_str, "battlefield") == 0) {
        return LOCATION_TYPE_BATTLEFIELD;
    } else if (strcmp(type_str, "village") == 0) {
        return LOCATION_TYPE_VILLAGE;
    } else if (strcmp(type_str, "crypt") == 0) {
        return LOCATION_TYPE_CRYPT;
    } else if (strcmp(type_str, "ritual_site") == 0) {
        return LOCATION_TYPE_RITUAL_SITE;
    }

    LOG_WARN("Unknown location type: %s, defaulting to graveyard", type_str);
    return LOCATION_TYPE_GRAVEYARD;
}

/**
 * @brief Create Location from data section
 */
Location* location_data_create_from_section(const DataSection* section) {
    if (!section) {
        LOG_ERROR("location_data_create_from_section: section is NULL");
        return NULL;
    }

    /* Extract required properties */
    const char* name = data_value_get_string(
        data_section_get(section, "name"), "Unnamed Location");

    const char* type_str = data_value_get_string(
        data_section_get(section, "type"), "graveyard");
    LocationType type = parse_location_type(type_str);

    const char* description = data_value_get_string(
        data_section_get(section, "description"), "No description available.");

    /* Convert string ID to numeric hash */
    uint32_t id = hash_string_id(section->section_id);

    /* Create location */
    Location* loc = location_create(id, name, type);
    if (!loc) {
        LOG_ERROR("Failed to create location: %s", section->section_id);
        return NULL;
    }

    /* Set description */
    strncpy(loc->description, description, sizeof(loc->description) - 1);
    loc->description[sizeof(loc->description) - 1] = '\0';

    /* Set optional properties */
    loc->corpse_count = (uint32_t)data_value_get_int(
        data_section_get(section, "corpse_count"), 0);

    loc->soul_quality_avg = (uint32_t)data_value_get_int(
        data_section_get(section, "soul_quality_avg"), 50);

    loc->control_level = (uint8_t)data_value_get_int(
        data_section_get(section, "control_level"), 0);

    loc->defense_strength = (uint32_t)data_value_get_int(
        data_section_get(section, "defense_strength"), 0);

    loc->discovered = data_value_get_bool(
        data_section_get(section, "discovered"), false);

    LOG_DEBUG("Created location: %s (ID: %u, type: %s)",
              loc->name, loc->id, location_type_name(type));

    return loc;
}

/**
 * @brief Load all locations from data file
 */
size_t location_data_load_all(TerritoryManager* territory, const DataFile* data_file) {
    if (!territory || !data_file) {
        LOG_ERROR("location_data_load_all: NULL parameter");
        return 0;
    }

    /* Get all LOCATION sections */
    size_t section_count = 0;
    const DataSection** sections = data_file_get_sections(data_file, "LOCATION", &section_count);

    if (!sections || section_count == 0) {
        LOG_WARN("No LOCATION sections found in data file");
        return 0;
    }

    LOG_INFO("Loading %zu locations from data file", section_count);

    /* Create locations */
    size_t loaded_count = 0;
    for (size_t i = 0; i < section_count; i++) {
        Location* loc = location_data_create_from_section(sections[i]);
        if (loc) {
            if (territory_manager_add_location(territory, loc)) {
                loaded_count++;
            } else {
                LOG_ERROR("Failed to add location to territory: %s", loc->id);
                location_destroy(loc);
            }
        }
    }

    free((void*)sections);

    LOG_INFO("Loaded %zu/%zu locations successfully", loaded_count, section_count);
    return loaded_count;
}

/**
 * @brief Build location graph connections
 */
size_t location_data_build_connections(TerritoryManager* territory, const DataFile* data_file) {
    if (!territory || !data_file) {
        LOG_ERROR("location_data_build_connections: NULL parameter");
        return 0;
    }

    /* Get all LOCATION sections */
    size_t section_count = 0;
    const DataSection** sections = data_file_get_sections(data_file, "LOCATION", &section_count);

    if (!sections || section_count == 0) {
        return 0;
    }

    LOG_INFO("Building location connections from data file");

    size_t connection_count = 0;

    /* For each location, parse connections */
    for (size_t i = 0; i < section_count; i++) {
        const DataSection* section = sections[i];
        const char* from_str_id = section->section_id;
        uint32_t from_id = hash_string_id(from_str_id);

        /* Get connections array */
        size_t conn_count = 0;
        const char** connections = data_value_get_array(
            data_section_get(section, "connections"), &conn_count);

        if (!connections || conn_count == 0) {
            continue;
        }

        /* Get source location */
        Location* from_loc = territory_manager_get_location(territory, from_id);
        if (!from_loc) {
            LOG_WARN("Source location not found: %s (ID: %u)", from_str_id, from_id);
            continue;
        }

        /* Add each connection */
        for (size_t j = 0; j < conn_count; j++) {
            const char* to_str_id = connections[j];
            uint32_t to_id = hash_string_id(to_str_id);

            /* Get destination location */
            Location* to_loc = territory_manager_get_location(territory, to_id);
            if (!to_loc) {
                LOG_WARN("Destination location not found: %s (ID: %u) from %s",
                         to_str_id, to_id, from_str_id);
                continue;
            }

            /* Add connection using location_add_connection */
            if (location_add_connection(from_loc, to_id)) {
                connection_count++;
                LOG_DEBUG("Connected: %s -> %s", from_str_id, to_str_id);
            } else {
                LOG_WARN("Failed to add connection: %s -> %s", from_str_id, to_str_id);
            }
        }
    }

    free((void*)sections);

    LOG_INFO("Created %zu connections between locations", connection_count);
    return connection_count;
}

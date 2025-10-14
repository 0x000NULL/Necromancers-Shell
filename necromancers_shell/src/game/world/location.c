/**
 * @file location.c
 * @brief Implementation of location system
 */

#include "location.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define INITIAL_CONNECTION_CAPACITY 4

Location* location_create(uint32_t id, const char* name, LocationType type) {
    if (!name) {
        return NULL;
    }

    Location* location = calloc(1, sizeof(Location));
    if (!location) {
        return NULL;
    }

    location->id = id;
    strncpy(location->name, name, sizeof(location->name) - 1);
    location->name[sizeof(location->name) - 1] = '\0';
    location->type = type;
    location->status = LOCATION_STATUS_UNDISCOVERED;
    location->description[0] = '\0';
    location->corpse_count = 0;
    location->soul_quality_avg = 50; /* Default quality */
    location->control_level = 0;
    location->defense_strength = 0;
    location->discovered = false;
    location->discovered_timestamp = 0;

    /* Allocate initial connection array */
    location->connected_ids = malloc(INITIAL_CONNECTION_CAPACITY * sizeof(uint32_t));
    if (!location->connected_ids) {
        free(location);
        return NULL;
    }
    location->connection_count = 0;
    location->connection_capacity = INITIAL_CONNECTION_CAPACITY;

    return location;
}

void location_destroy(Location* location) {
    if (!location) {
        return;
    }
    free(location->connected_ids);
    free(location);
}

const char* location_type_name(LocationType type) {
    switch (type) {
        case LOCATION_TYPE_GRAVEYARD:
            return "Graveyard";
        case LOCATION_TYPE_BATTLEFIELD:
            return "Battlefield";
        case LOCATION_TYPE_VILLAGE:
            return "Village";
        case LOCATION_TYPE_CRYPT:
            return "Crypt";
        case LOCATION_TYPE_RITUAL_SITE:
            return "Ritual Site";
        default:
            return "Unknown";
    }
}

const char* location_status_name(LocationStatus status) {
    switch (status) {
        case LOCATION_STATUS_UNDISCOVERED:
            return "Undiscovered";
        case LOCATION_STATUS_DISCOVERED:
            return "Discovered";
        case LOCATION_STATUS_CONTROLLED:
            return "Controlled";
        case LOCATION_STATUS_HOSTILE:
            return "Hostile";
        default:
            return "Unknown";
    }
}

bool location_add_connection(Location* location, uint32_t connected_id) {
    if (!location) {
        return false;
    }

    /* Check if already connected */
    for (size_t i = 0; i < location->connection_count; i++) {
        if (location->connected_ids[i] == connected_id) {
            return true; /* Already connected */
        }
    }

    /* Expand array if needed */
    if (location->connection_count >= location->connection_capacity) {
        size_t new_capacity = location->connection_capacity * 2;
        uint32_t* new_array = realloc(location->connected_ids,
                                       new_capacity * sizeof(uint32_t));
        if (!new_array) {
            return false;
        }
        location->connected_ids = new_array;
        location->connection_capacity = new_capacity;
    }

    /* Add connection */
    location->connected_ids[location->connection_count++] = connected_id;
    return true;
}

bool location_is_connected(const Location* location, uint32_t other_id) {
    if (!location) {
        return false;
    }

    for (size_t i = 0; i < location->connection_count; i++) {
        if (location->connected_ids[i] == other_id) {
            return true;
        }
    }
    return false;
}

void location_discover(Location* location, uint64_t timestamp) {
    if (!location) {
        return;
    }

    if (!location->discovered) {
        location->discovered = true;
        location->discovered_timestamp = timestamp;
        if (location->status == LOCATION_STATUS_UNDISCOVERED) {
            location->status = LOCATION_STATUS_DISCOVERED;
        }
    }
}

uint32_t location_harvest_corpses(Location* location, uint32_t max_count) {
    if (!location) {
        return 0;
    }

    uint32_t harvested = (max_count < location->corpse_count)
                         ? max_count
                         : location->corpse_count;
    location->corpse_count -= harvested;
    return harvested;
}

uint8_t location_increase_control(Location* location, uint8_t amount) {
    if (!location) {
        return 0;
    }

    uint32_t new_control = (uint32_t)location->control_level + amount;
    if (new_control > 100) {
        new_control = 100;
    }
    location->control_level = (uint8_t)new_control;

    /* Update status based on control level */
    if (location->control_level >= 75) {
        location->status = LOCATION_STATUS_CONTROLLED;
    } else if (location->control_level > 0) {
        location->status = LOCATION_STATUS_DISCOVERED;
    }

    return location->control_level;
}

uint8_t location_decrease_control(Location* location, uint8_t amount) {
    if (!location) {
        return 0;
    }

    if (amount > location->control_level) {
        location->control_level = 0;
    } else {
        location->control_level -= amount;
    }

    /* Update status based on control level */
    if (location->control_level < 75) {
        if (location->status == LOCATION_STATUS_CONTROLLED) {
            location->status = LOCATION_STATUS_DISCOVERED;
        }
    }

    return location->control_level;
}

size_t location_get_description_formatted(const Location* location,
                                           char* buffer,
                                           size_t buffer_size) {
    if (!location || !buffer || buffer_size == 0) {
        return 0;
    }

    int written = snprintf(buffer, buffer_size,
        "%s (%s)\n"
        "Status: %s\n"
        "Description: %s\n"
        "Corpses: %u\n"
        "Soul Quality: %u\n"
        "Control: %u%%\n"
        "Defense: %u\n"
        "Connections: %zu",
        location->name,
        location_type_name(location->type),
        location_status_name(location->status),
        location->description[0] ? location->description : "No description available",
        location->corpse_count,
        location->soul_quality_avg,
        location->control_level,
        location->defense_strength,
        location->connection_count);

    return (written > 0) ? (size_t)written : 0;
}

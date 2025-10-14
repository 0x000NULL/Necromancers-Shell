/**
 * @file territory.c
 * @brief Implementation of territory manager
 */

#include "territory.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define INITIAL_CAPACITY 16

/**
 * @brief Territory manager structure
 */
struct TerritoryManager {
    Location** locations;       /**< Dynamic array of location pointers */
    size_t count;               /**< Number of locations */
    size_t capacity;            /**< Capacity of array */
};

TerritoryManager* territory_manager_create(void) {
    TerritoryManager* manager = calloc(1, sizeof(TerritoryManager));
    if (!manager) {
        return NULL;
    }

    manager->locations = malloc(INITIAL_CAPACITY * sizeof(Location*));
    if (!manager->locations) {
        free(manager);
        return NULL;
    }

    manager->count = 0;
    manager->capacity = INITIAL_CAPACITY;

    return manager;
}

void territory_manager_destroy(TerritoryManager* manager) {
    if (!manager) {
        return;
    }

    /* Destroy all locations */
    for (size_t i = 0; i < manager->count; i++) {
        location_destroy(manager->locations[i]);
    }
    free(manager->locations);
    free(manager);
}

bool territory_manager_add_location(TerritoryManager* manager, Location* location) {
    if (!manager || !location) {
        return false;
    }

    /* Check for duplicate ID */
    for (size_t i = 0; i < manager->count; i++) {
        if (manager->locations[i]->id == location->id) {
            return false; /* Duplicate ID */
        }
    }

    /* Expand array if needed */
    if (manager->count >= manager->capacity) {
        size_t new_capacity = manager->capacity * 2;
        Location** new_array = realloc(manager->locations,
                                        new_capacity * sizeof(Location*));
        if (!new_array) {
            return false;
        }
        manager->locations = new_array;
        manager->capacity = new_capacity;
    }

    /* Add location */
    manager->locations[manager->count++] = location;
    return true;
}

Location* territory_manager_get_location(const TerritoryManager* manager, uint32_t id) {
    if (!manager) {
        return NULL;
    }

    for (size_t i = 0; i < manager->count; i++) {
        if (manager->locations[i]->id == id) {
            return manager->locations[i];
        }
    }
    return NULL;
}

Location* territory_manager_get_location_by_name(const TerritoryManager* manager,
                                                  const char* name) {
    if (!manager || !name) {
        return NULL;
    }

    for (size_t i = 0; i < manager->count; i++) {
        if (strcmp(manager->locations[i]->name, name) == 0) {
            return manager->locations[i];
        }
    }
    return NULL;
}

bool territory_manager_get_discovered(const TerritoryManager* manager,
                                       Location*** results,
                                       size_t* count) {
    if (!manager || !results || !count) {
        return false;
    }

    /* Count discovered locations */
    size_t discovered_count = 0;
    for (size_t i = 0; i < manager->count; i++) {
        if (manager->locations[i]->discovered) {
            discovered_count++;
        }
    }

    if (discovered_count == 0) {
        *results = NULL;
        *count = 0;
        return true;
    }

    /* Allocate results array */
    Location** discovered = malloc(discovered_count * sizeof(Location*));
    if (!discovered) {
        return false;
    }

    /* Fill results */
    size_t idx = 0;
    for (size_t i = 0; i < manager->count; i++) {
        if (manager->locations[i]->discovered) {
            discovered[idx++] = manager->locations[i];
        }
    }

    *results = discovered;
    *count = discovered_count;
    return true;
}

size_t territory_manager_count(const TerritoryManager* manager) {
    return manager ? manager->count : 0;
}

size_t territory_manager_count_discovered(const TerritoryManager* manager) {
    if (!manager) {
        return 0;
    }

    size_t count = 0;
    for (size_t i = 0; i < manager->count; i++) {
        if (manager->locations[i]->discovered) {
            count++;
        }
    }
    return count;
}

void territory_manager_free_results(Location** results) {
    free(results);
}

size_t territory_manager_load_from_file(TerritoryManager* manager,
                                         const char* filepath) {
    if (!manager) {
        return 0;
    }

    /* Ignore filepath for now - create hardcoded locations */
    (void)filepath;

    size_t loaded = 0;

    /* Location 1: Forgotten Graveyard (starting location) */
    Location* loc1 = location_create(1, "Forgotten Graveyard", LOCATION_TYPE_GRAVEYARD);
    if (loc1) {
        loc1->corpse_count = 50;
        loc1->soul_quality_avg = 40;
        loc1->control_level = 100;
        loc1->discovered = true;
        loc1->status = LOCATION_STATUS_CONTROLLED;
        strncpy(loc1->description,
                "A moss-covered cemetery, long abandoned by the living. "
                "Ancient tombstones lean at odd angles, and the earth is "
                "soft from recent rains. This is your domain.",
                sizeof(loc1->description) - 1);
        location_add_connection(loc1, 2);
        location_add_connection(loc1, 3);
        if (territory_manager_add_location(manager, loc1)) {
            loaded++;
        } else {
            location_destroy(loc1);
        }
    }

    /* Location 2: Old Battlefield */
    Location* loc2 = location_create(2, "Old Battlefield", LOCATION_TYPE_BATTLEFIELD);
    if (loc2) {
        loc2->corpse_count = 200;
        loc2->soul_quality_avg = 60;
        loc2->control_level = 0;
        loc2->defense_strength = 30;
        strncpy(loc2->description,
                "A scarred field where armies clashed decades ago. Rusted "
                "weapons still protrude from the ground, and the air carries "
                "echoes of ancient battle cries. Many warrior souls linger here.",
                sizeof(loc2->description) - 1);
        location_add_connection(loc2, 1);
        location_add_connection(loc2, 4);
        if (territory_manager_add_location(manager, loc2)) {
            loaded++;
        } else {
            location_destroy(loc2);
        }
    }

    /* Location 3: Sleepy Hollow Village */
    Location* loc3 = location_create(3, "Sleepy Hollow", LOCATION_TYPE_VILLAGE);
    if (loc3) {
        loc3->corpse_count = 30;
        loc3->soul_quality_avg = 70;
        loc3->control_level = 0;
        loc3->defense_strength = 50;
        loc3->status = LOCATION_STATUS_HOSTILE;
        strncpy(loc3->description,
                "A quiet village nestled in a valley. Lanterns glow in windows "
                "as families gather for evening meals. The living go about their "
                "peaceful lives, unaware of the dark forces nearby.",
                sizeof(loc3->description) - 1);
        location_add_connection(loc3, 1);
        location_add_connection(loc3, 5);
        if (territory_manager_add_location(manager, loc3)) {
            loaded++;
        } else {
            location_destroy(loc3);
        }
    }

    /* Location 4: Ancient Crypt */
    Location* loc4 = location_create(4, "Ancient Crypt", LOCATION_TYPE_CRYPT);
    if (loc4) {
        loc4->corpse_count = 80;
        loc4->soul_quality_avg = 75;
        loc4->control_level = 0;
        loc4->defense_strength = 60;
        strncpy(loc4->description,
                "A stone structure carved into the hillside, its entrance guarded "
                "by weathered statues. Inside, ancient nobles rest in ornate "
                "sarcophagi. Powerful souls await, but so do protective wards.",
                sizeof(loc4->description) - 1);
        location_add_connection(loc4, 2);
        location_add_connection(loc4, 5);
        if (territory_manager_add_location(manager, loc4)) {
            loaded++;
        } else {
            location_destroy(loc4);
        }
    }

    /* Location 5: Cursed Ritual Site */
    Location* loc5 = location_create(5, "Cursed Ritual Site", LOCATION_TYPE_RITUAL_SITE);
    if (loc5) {
        loc5->corpse_count = 40;
        loc5->soul_quality_avg = 85;
        loc5->control_level = 0;
        loc5->defense_strength = 80;
        strncpy(loc5->description,
                "A circle of standing stones pulses with dark energy. The ground "
                "is scorched black, and strange symbols glow faintly in the moonlight. "
                "This place remembers ancient sacrifices and forbidden rituals.",
                sizeof(loc5->description) - 1);
        location_add_connection(loc5, 3);
        location_add_connection(loc5, 4);
        if (territory_manager_add_location(manager, loc5)) {
            loaded++;
        } else {
            location_destroy(loc5);
        }
    }

    return loaded;
}

void territory_manager_clear(TerritoryManager* manager) {
    if (!manager) {
        return;
    }

    for (size_t i = 0; i < manager->count; i++) {
        location_destroy(manager->locations[i]);
    }
    manager->count = 0;
}

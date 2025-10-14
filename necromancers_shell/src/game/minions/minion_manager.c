#include "minion_manager.h"
#include <stdlib.h>
#include <string.h>

/**
 * @brief Growth factor for dynamic array
 */
#define GROWTH_FACTOR 2

/**
 * @brief Internal minion manager structure
 */
struct MinionManager {
    Minion** minions;    /**< Dynamic array of minion pointers */
    size_t count;        /**< Current number of minions */
    size_t capacity;     /**< Capacity of minions array */
};

MinionManager* minion_manager_create(size_t initial_capacity) {
    if (initial_capacity == 0) {
        initial_capacity = 50;  /* Default capacity */
    }

    MinionManager* manager = (MinionManager*)malloc(sizeof(MinionManager));
    if (!manager) {
        return NULL;
    }

    manager->minions = (Minion**)malloc(initial_capacity * sizeof(Minion*));
    if (!manager->minions) {
        free(manager);
        return NULL;
    }

    manager->count = 0;
    manager->capacity = initial_capacity;

    return manager;
}

void minion_manager_destroy(MinionManager* manager) {
    if (!manager) {
        return;
    }

    /* Destroy all minions */
    for (size_t i = 0; i < manager->count; i++) {
        minion_destroy(manager->minions[i]);
    }

    free(manager->minions);
    free(manager);
}

bool minion_manager_add(MinionManager* manager, Minion* minion) {
    if (!manager || !minion) {
        return false;
    }

    /* Grow array if needed */
    if (manager->count >= manager->capacity) {
        size_t new_capacity = manager->capacity * GROWTH_FACTOR;
        Minion** new_minions = (Minion**)realloc(manager->minions, new_capacity * sizeof(Minion*));
        if (!new_minions) {
            return false;
        }
        manager->minions = new_minions;
        manager->capacity = new_capacity;
    }

    /* Add minion to array */
    manager->minions[manager->count++] = minion;
    return true;
}

Minion* minion_manager_remove(MinionManager* manager, uint32_t minion_id) {
    if (!manager) {
        return NULL;
    }

    /* Find minion by ID */
    for (size_t i = 0; i < manager->count; i++) {
        if (manager->minions[i]->id == minion_id) {
            /* Store pointer to return */
            Minion* minion = manager->minions[i];

            /* Shift remaining minions down */
            for (size_t j = i; j < manager->count - 1; j++) {
                manager->minions[j] = manager->minions[j + 1];
            }

            manager->count--;
            return minion;  /* Transfer ownership to caller */
        }
    }

    return NULL;  /* Not found */
}

Minion* minion_manager_get(MinionManager* manager, uint32_t minion_id) {
    if (!manager) {
        return NULL;
    }

    /* Find minion by ID */
    for (size_t i = 0; i < manager->count; i++) {
        if (manager->minions[i]->id == minion_id) {
            return manager->minions[i];
        }
    }

    return NULL;
}

Minion* minion_manager_get_at(MinionManager* manager, size_t index) {
    if (!manager || index >= manager->count) {
        return NULL;
    }

    return manager->minions[index];
}

Minion** minion_manager_get_at_location(MinionManager* manager, uint32_t location_id, size_t* count_out) {
    if (!manager || !count_out) {
        return NULL;
    }

    /* Count minions at location first */
    size_t count = 0;
    for (size_t i = 0; i < manager->count; i++) {
        if (manager->minions[i]->location_id == location_id) {
            count++;
        }
    }

    *count_out = count;

    if (count == 0) {
        return NULL;
    }

    /* Allocate result array */
    Minion** result = (Minion**)malloc(count * sizeof(Minion*));
    if (!result) {
        return NULL;
    }

    /* Fill result array */
    size_t result_index = 0;
    for (size_t i = 0; i < manager->count; i++) {
        if (manager->minions[i]->location_id == location_id) {
            result[result_index++] = manager->minions[i];
        }
    }

    return result;
}

size_t minion_manager_count(MinionManager* manager) {
    if (!manager) {
        return 0;
    }
    return manager->count;
}

size_t minion_manager_count_by_type(MinionManager* manager, MinionType type) {
    if (!manager) {
        return 0;
    }

    size_t count = 0;
    for (size_t i = 0; i < manager->count; i++) {
        if (manager->minions[i]->type == type) {
            count++;
        }
    }

    return count;
}

size_t minion_manager_count_at_location(MinionManager* manager, uint32_t location_id) {
    if (!manager) {
        return 0;
    }

    size_t count = 0;
    for (size_t i = 0; i < manager->count; i++) {
        if (manager->minions[i]->location_id == location_id) {
            count++;
        }
    }

    return count;
}

void minion_manager_clear(MinionManager* manager) {
    if (!manager) {
        return;
    }

    /* Destroy all minions */
    for (size_t i = 0; i < manager->count; i++) {
        minion_destroy(manager->minions[i]);
    }

    manager->count = 0;
}

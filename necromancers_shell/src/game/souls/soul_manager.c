#include "soul_manager.h"
#include <stdlib.h>
#include <string.h>

/**
 * @brief Initial capacity for soul array
 */
#define INITIAL_CAPACITY 100

/**
 * @brief Growth factor for dynamic array
 */
#define GROWTH_FACTOR 2

/**
 * @brief Internal soul manager structure
 */
struct SoulManager {
    Soul** souls;       /**< Dynamic array of soul pointers */
    size_t count;       /**< Current number of souls */
    size_t capacity;    /**< Capacity of souls array */
};

/* Forward declarations for sort comparators */
static int compare_by_id(const void* a, const void* b);
static int compare_by_type(const void* a, const void* b);
static int compare_by_quality(const void* a, const void* b);
static int compare_by_quality_desc(const void* a, const void* b);
static int compare_by_energy(const void* a, const void* b);
static int compare_by_energy_desc(const void* a, const void* b);

SoulManager* soul_manager_create(void) {
    SoulManager* manager = (SoulManager*)malloc(sizeof(SoulManager));
    if (!manager) {
        return NULL;
    }

    manager->souls = (Soul**)malloc(INITIAL_CAPACITY * sizeof(Soul*));
    if (!manager->souls) {
        free(manager);
        return NULL;
    }

    manager->count = 0;
    manager->capacity = INITIAL_CAPACITY;

    return manager;
}

void soul_manager_destroy(SoulManager* manager) {
    if (!manager) {
        return;
    }

    /* Destroy all souls */
    for (size_t i = 0; i < manager->count; i++) {
        soul_destroy(manager->souls[i]);
    }

    free(manager->souls);
    free(manager);
}

bool soul_manager_add(SoulManager* manager, Soul* soul) {
    if (!manager || !soul) {
        return false;
    }

    /* Grow array if needed */
    if (manager->count >= manager->capacity) {
        size_t new_capacity = manager->capacity * GROWTH_FACTOR;
        Soul** new_souls = (Soul**)realloc(manager->souls, new_capacity * sizeof(Soul*));
        if (!new_souls) {
            return false;
        }
        manager->souls = new_souls;
        manager->capacity = new_capacity;
    }

    /* Add soul to array */
    manager->souls[manager->count++] = soul;
    return true;
}

bool soul_manager_remove(SoulManager* manager, uint32_t soul_id) {
    if (!manager) {
        return false;
    }

    /* Find soul by ID */
    for (size_t i = 0; i < manager->count; i++) {
        if (manager->souls[i]->id == soul_id) {
            /* Destroy the soul */
            soul_destroy(manager->souls[i]);

            /* Shift remaining souls down */
            for (size_t j = i; j < manager->count - 1; j++) {
                manager->souls[j] = manager->souls[j + 1];
            }

            manager->count--;
            return true;
        }
    }

    return false;
}

Soul* soul_manager_get(SoulManager* manager, uint32_t soul_id) {
    if (!manager) {
        return NULL;
    }

    /* Linear search for soul by ID */
    for (size_t i = 0; i < manager->count; i++) {
        if (manager->souls[i]->id == soul_id) {
            return manager->souls[i];
        }
    }

    return NULL;
}

Soul** soul_manager_get_filtered(SoulManager* manager, const SoulFilter* filter, size_t* count_out) {
    if (!manager || !count_out) {
        return NULL;
    }

    *count_out = 0;

    /* Defensive: Ensure count doesn't exceed capacity */
    if (manager->count > manager->capacity) {
        /* Data corruption detected */
        return NULL;
    }

    /* If no filter, return all souls */
    if (!filter) {
        if (manager->count == 0) {
            return NULL;
        }

        Soul** result = (Soul**)malloc(manager->count * sizeof(Soul*));
        if (!result) {
            return NULL;
        }

        memcpy(result, manager->souls, manager->count * sizeof(Soul*));
        *count_out = manager->count;
        return result;
    }

    /* Count matching souls first */
    size_t match_count = 0;
    for (size_t i = 0; i < manager->count; i++) {
        /* Defensive bounds check */
        if (i >= manager->capacity) {
            break;
        }

        Soul* soul = manager->souls[i];
        if (!soul) {
            continue;  /* Skip NULL souls */
        }

        bool matches = true;

        /* Check type filter */
        if (filter->type != -1 && soul->type != (SoulType)filter->type) {
            matches = false;
        }

        /* Check quality filter */
        if (soul->quality < filter->quality_min || soul->quality > filter->quality_max) {
            matches = false;
        }

        /* Check bound filter */
        if (filter->bound_filter == 0 && soul->bound) {
            matches = false;
        } else if (filter->bound_filter == 1 && !soul->bound) {
            matches = false;
        }

        if (matches) {
            match_count++;
        }
    }

    if (match_count == 0) {
        return NULL;
    }

    /* Allocate result array */
    Soul** result = (Soul**)malloc(match_count * sizeof(Soul*));
    if (!result) {
        return NULL;
    }

    /* Fill result array with matching souls */
    size_t result_index = 0;
    for (size_t i = 0; i < manager->count; i++) {
        /* Defensive bounds check */
        if (i >= manager->capacity) {
            break;
        }

        Soul* soul = manager->souls[i];
        if (!soul) {
            continue;  /* Skip NULL souls */
        }

        bool matches = true;

        /* Check type filter */
        if (filter->type != -1 && soul->type != (SoulType)filter->type) {
            matches = false;
        }

        /* Check quality filter */
        if (soul->quality < filter->quality_min || soul->quality > filter->quality_max) {
            matches = false;
        }

        /* Check bound filter */
        if (filter->bound_filter == 0 && soul->bound) {
            matches = false;
        } else if (filter->bound_filter == 1 && !soul->bound) {
            matches = false;
        }

        if (matches) {
            result[result_index++] = soul;
        }
    }

    *count_out = match_count;
    return result;
}

void soul_manager_sort(SoulManager* manager, SoulSortCriteria criteria) {
    if (!manager || manager->count <= 1) {
        return;
    }

    /* Select appropriate comparator */
    int (*comparator)(const void*, const void*) = NULL;

    switch (criteria) {
        case SOUL_SORT_ID:
            comparator = compare_by_id;
            break;
        case SOUL_SORT_TYPE:
            comparator = compare_by_type;
            break;
        case SOUL_SORT_QUALITY:
            comparator = compare_by_quality;
            break;
        case SOUL_SORT_QUALITY_DESC:
            comparator = compare_by_quality_desc;
            break;
        case SOUL_SORT_ENERGY:
            comparator = compare_by_energy;
            break;
        case SOUL_SORT_ENERGY_DESC:
            comparator = compare_by_energy_desc;
            break;
        default:
            return;
    }

    /* Sort using qsort */
    qsort(manager->souls, manager->count, sizeof(Soul*), comparator);
}

size_t soul_manager_count(SoulManager* manager) {
    if (!manager) {
        return 0;
    }
    return manager->count;
}

size_t soul_manager_count_by_type(SoulManager* manager, SoulType type) {
    if (!manager) {
        return 0;
    }

    size_t count = 0;
    for (size_t i = 0; i < manager->count; i++) {
        if (manager->souls[i]->type == type) {
            count++;
        }
    }

    return count;
}

uint32_t soul_manager_total_energy(SoulManager* manager) {
    if (!manager) {
        return 0;
    }

    uint32_t total = 0;
    for (size_t i = 0; i < manager->count; i++) {
        total += manager->souls[i]->energy;
    }

    return total;
}

uint32_t soul_manager_total_unbound_energy(SoulManager* manager) {
    if (!manager) {
        return 0;
    }

    uint32_t total = 0;
    for (size_t i = 0; i < manager->count; i++) {
        if (!manager->souls[i]->bound) {
            total += manager->souls[i]->energy;
        }
    }

    return total;
}

void soul_manager_clear(SoulManager* manager) {
    if (!manager) {
        return;
    }

    /* Destroy all souls */
    for (size_t i = 0; i < manager->count; i++) {
        soul_destroy(manager->souls[i]);
    }

    manager->count = 0;
}

SoulFilter soul_filter_default(void) {
    SoulFilter filter;
    filter.type = -1;
    filter.quality_min = 0;
    filter.quality_max = 100;
    filter.bound_filter = -1;
    return filter;
}

SoulFilter soul_filter_by_type(SoulType type) {
    SoulFilter filter;
    filter.type = (int)type;
    filter.quality_min = 0;
    filter.quality_max = 100;
    filter.bound_filter = -1;
    return filter;
}

SoulFilter soul_filter_unbound(void) {
    SoulFilter filter;
    filter.type = -1;
    filter.quality_min = 0;
    filter.quality_max = 100;
    filter.bound_filter = 0;
    return filter;
}

SoulFilter soul_filter_min_quality(SoulQuality min_quality) {
    SoulFilter filter;
    filter.type = -1;
    filter.quality_min = min_quality;
    filter.quality_max = 100;
    filter.bound_filter = -1;
    return filter;
}

/* Comparator functions for sorting */

static int compare_by_id(const void* a, const void* b) {
    const Soul* soul_a = *(const Soul**)a;
    const Soul* soul_b = *(const Soul**)b;

    if (soul_a->id < soul_b->id) return -1;
    if (soul_a->id > soul_b->id) return 1;
    return 0;
}

static int compare_by_type(const void* a, const void* b) {
    const Soul* soul_a = *(const Soul**)a;
    const Soul* soul_b = *(const Soul**)b;

    if (soul_a->type < soul_b->type) return -1;
    if (soul_a->type > soul_b->type) return 1;
    return 0;
}

static int compare_by_quality(const void* a, const void* b) {
    const Soul* soul_a = *(const Soul**)a;
    const Soul* soul_b = *(const Soul**)b;

    if (soul_a->quality < soul_b->quality) return -1;
    if (soul_a->quality > soul_b->quality) return 1;
    return 0;
}

static int compare_by_quality_desc(const void* a, const void* b) {
    return -compare_by_quality(a, b);
}

static int compare_by_energy(const void* a, const void* b) {
    const Soul* soul_a = *(const Soul**)a;
    const Soul* soul_b = *(const Soul**)b;

    if (soul_a->energy < soul_b->energy) return -1;
    if (soul_a->energy > soul_b->energy) return 1;
    return 0;
}

static int compare_by_energy_desc(const void* a, const void* b) {
    return -compare_by_energy(a, b);
}

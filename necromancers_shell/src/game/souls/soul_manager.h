#ifndef SOUL_MANAGER_H
#define SOUL_MANAGER_H

#include "soul.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @file soul_manager.h
 * @brief Soul collection management system
 *
 * Manages a collection of souls with filtering, sorting, and querying capabilities.
 */

/**
 * @brief Opaque soul manager structure
 *
 * Manages a dynamic collection of souls.
 * Implementation details are hidden in the .c file.
 */
typedef struct SoulManager SoulManager;

/**
 * @brief Filter criteria for querying souls
 *
 * All fields are optional. If a field is set to a special value
 * (type = -1, quality_min = 0, quality_max = 100, etc.), it is ignored.
 */
typedef struct {
    int type;                  /**< Soul type filter (-1 = any) */
    SoulQuality quality_min;   /**< Minimum quality (0 = no minimum) */
    SoulQuality quality_max;   /**< Maximum quality (100 = no maximum) */
    int bound_filter;          /**< Bound filter: -1=any, 0=unbound only, 1=bound only */
} SoulFilter;

/**
 * @brief Sort criteria for soul lists
 */
typedef enum {
    SOUL_SORT_ID,           /**< Sort by soul ID (chronological) */
    SOUL_SORT_TYPE,         /**< Sort by soul type */
    SOUL_SORT_QUALITY,      /**< Sort by quality (ascending) */
    SOUL_SORT_QUALITY_DESC, /**< Sort by quality (descending) */
    SOUL_SORT_ENERGY,       /**< Sort by energy (ascending) */
    SOUL_SORT_ENERGY_DESC   /**< Sort by energy (descending) */
} SoulSortCriteria;

/**
 * @brief Create a new soul manager
 *
 * @return Pointer to new soul manager, or NULL on failure
 */
SoulManager* soul_manager_create(void);

/**
 * @brief Destroy soul manager and all contained souls
 *
 * @param manager Pointer to soul manager (can be NULL)
 */
void soul_manager_destroy(SoulManager* manager);

/**
 * @brief Add a soul to the manager
 *
 * The manager takes ownership of the soul.
 * The soul pointer should not be used after this call.
 *
 * @param manager Pointer to soul manager
 * @param soul Pointer to soul to add
 * @return true on success, false on failure
 */
bool soul_manager_add(SoulManager* manager, Soul* soul);

/**
 * @brief Remove a soul from the manager by ID
 *
 * The soul is destroyed after removal.
 *
 * @param manager Pointer to soul manager
 * @param soul_id ID of soul to remove
 * @return true on success, false if soul not found
 */
bool soul_manager_remove(SoulManager* manager, uint32_t soul_id);

/**
 * @brief Get a soul by ID
 *
 * Returns a pointer to the soul in the manager's collection.
 * The pointer remains valid until the soul is removed or manager is destroyed.
 *
 * @param manager Pointer to soul manager
 * @param soul_id ID of soul to get
 * @return Pointer to soul, or NULL if not found
 */
Soul* soul_manager_get(SoulManager* manager, uint32_t soul_id);

/**
 * @brief Get filtered list of souls
 *
 * Returns an array of pointers to souls matching the filter.
 * The caller must free the returned array (but not the souls themselves).
 *
 * @param manager Pointer to soul manager
 * @param filter Pointer to filter criteria (NULL for no filtering)
 * @param count_out Pointer to store count of returned souls
 * @return Array of soul pointers, or NULL on failure. Must be freed by caller.
 */
Soul** soul_manager_get_filtered(SoulManager* manager, const SoulFilter* filter, size_t* count_out);

/**
 * @brief Sort souls in the manager
 *
 * Sorts the internal soul array according to the given criteria.
 *
 * @param manager Pointer to soul manager
 * @param criteria Sort criteria
 */
void soul_manager_sort(SoulManager* manager, SoulSortCriteria criteria);

/**
 * @brief Get total number of souls
 *
 * @param manager Pointer to soul manager
 * @return Total count of souls, or 0 if manager is NULL
 */
size_t soul_manager_count(SoulManager* manager);

/**
 * @brief Get count of souls by type
 *
 * @param manager Pointer to soul manager
 * @param type Soul type to count
 * @return Count of souls of the given type
 */
size_t soul_manager_count_by_type(SoulManager* manager, SoulType type);

/**
 * @brief Calculate total energy of all souls
 *
 * @param manager Pointer to soul manager
 * @return Sum of energy from all souls
 */
uint32_t soul_manager_total_energy(SoulManager* manager);

/**
 * @brief Calculate total energy of unbound souls
 *
 * @param manager Pointer to soul manager
 * @return Sum of energy from unbound souls
 */
uint32_t soul_manager_total_unbound_energy(SoulManager* manager);

/**
 * @brief Clear all souls from the manager
 *
 * Destroys all souls in the collection.
 *
 * @param manager Pointer to soul manager
 */
void soul_manager_clear(SoulManager* manager);

/**
 * @brief Create a default filter (matches all souls)
 *
 * @return Default filter with no restrictions
 */
SoulFilter soul_filter_default(void);

/**
 * @brief Create a filter for a specific soul type
 *
 * @param type Soul type to filter for
 * @return Filter matching only the specified type
 */
SoulFilter soul_filter_by_type(SoulType type);

/**
 * @brief Create a filter for unbound souls only
 *
 * @return Filter matching only unbound souls
 */
SoulFilter soul_filter_unbound(void);

/**
 * @brief Create a filter for souls with minimum quality
 *
 * @param min_quality Minimum quality threshold
 * @return Filter matching souls with quality >= min_quality
 */
SoulFilter soul_filter_min_quality(SoulQuality min_quality);

#endif /* SOUL_MANAGER_H */

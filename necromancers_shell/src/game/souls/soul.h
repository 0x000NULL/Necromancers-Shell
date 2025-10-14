#ifndef SOUL_H
#define SOUL_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

/**
 * @file soul.h
 * @brief Core soul system for Necromancer's Shell
 *
 * Souls are the primary currency and resource in the game.
 * Each soul has a type, quality, energy value, and optional memories.
 */

/**
 * @brief Types of souls available in the game
 *
 * Different soul types provide different amounts of energy and
 * have different applications in necromantic rituals.
 */
typedef enum {
    SOUL_TYPE_COMMON,     /**< Common soul (10-20 energy) */
    SOUL_TYPE_WARRIOR,    /**< Warrior soul (20-40 energy) */
    SOUL_TYPE_MAGE,       /**< Mage soul (30-50 energy) */
    SOUL_TYPE_INNOCENT,   /**< Innocent soul (15-25 energy) */
    SOUL_TYPE_CORRUPTED,  /**< Corrupted soul (25-35 energy) */
    SOUL_TYPE_ANCIENT,    /**< Ancient soul (50-100 energy) */
    SOUL_TYPE_COUNT       /**< Number of soul types */
} SoulType;

/**
 * @brief Soul quality value (0-100)
 *
 * Higher quality souls provide more energy and better results
 * in necromantic operations.
 */
typedef uint8_t SoulQuality;

/**
 * @brief Maximum length for soul memory strings
 */
#define SOUL_MEMORY_MAX_LENGTH 256

/**
 * @brief Core soul structure
 *
 * Represents a single soul harvested from a creature.
 * Souls can be bound to minions or used as energy sources.
 */
typedef struct {
    uint32_t id;                              /**< Unique soul identifier */
    SoulType type;                            /**< Type of soul */
    SoulQuality quality;                      /**< Quality value (0-100) */
    char memories[SOUL_MEMORY_MAX_LENGTH];    /**< Flavor text memories */
    uint32_t energy;                          /**< Calculated energy value */
    bool bound;                               /**< Whether soul is bound to a minion */
    uint32_t bound_minion_id;                 /**< ID of bound minion (0 if unbound) */
    time_t timestamp;                         /**< When soul was harvested */
} Soul;

/**
 * @brief Create a new soul
 *
 * Allocates and initializes a new soul structure.
 * Automatically calculates energy based on type and quality.
 * Generates random memories based on soul type.
 *
 * @param type Soul type
 * @param quality Soul quality (0-100)
 * @return Pointer to newly allocated soul, or NULL on failure
 */
Soul* soul_create(SoulType type, SoulQuality quality);

/**
 * @brief Destroy a soul and free its memory
 *
 * @param soul Pointer to soul to destroy (can be NULL)
 */
void soul_destroy(Soul* soul);

/**
 * @brief Get the name of a soul type
 *
 * @param type Soul type
 * @return String name of the soul type (e.g., "Common", "Warrior")
 */
const char* soul_type_name(SoulType type);

/**
 * @brief Calculate energy value for a soul
 *
 * Energy is based on type (base range) and quality (multiplier).
 * Formula varies by type but generally: base_energy * (quality / 100.0)
 *
 * @param type Soul type
 * @param quality Soul quality (0-100)
 * @return Calculated energy value
 */
uint32_t soul_calculate_energy(SoulType type, SoulQuality quality);

/**
 * @brief Bind a soul to a minion
 *
 * Marks the soul as bound to a specific minion.
 * Bound souls cannot be used for other purposes until unbound.
 *
 * @param soul Pointer to soul
 * @param minion_id ID of minion to bind to
 * @return true on success, false if soul is NULL or already bound
 */
bool soul_bind(Soul* soul, uint32_t minion_id);

/**
 * @brief Unbind a soul from its minion
 *
 * Marks the soul as unbound and available for other uses.
 *
 * @param soul Pointer to soul
 * @return true on success, false if soul is NULL or not bound
 */
bool soul_unbind(Soul* soul);

/**
 * @brief Generate random memories for a soul
 *
 * Creates flavor text based on soul type and quality.
 * Memories are stored in the soul's memories field.
 *
 * @param soul Pointer to soul
 * @param type Soul type (influences memory content)
 * @param quality Soul quality (influences memory clarity)
 */
void soul_generate_memories(Soul* soul, SoulType type, SoulQuality quality);

/**
 * @brief Get a formatted description of the soul
 *
 * Returns a human-readable description including type, quality,
 * energy, and bound status.
 *
 * @param soul Pointer to soul
 * @param buffer Buffer to write description into
 * @param buffer_size Size of buffer
 * @return Number of characters written (excluding null terminator)
 */
int soul_get_description(const Soul* soul, char* buffer, size_t buffer_size);

#endif /* SOUL_H */

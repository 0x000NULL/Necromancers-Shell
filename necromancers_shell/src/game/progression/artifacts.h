/**
 * @file artifacts.h
 * @brief Artifact system for powerful unique items
 *
 * Artifacts are rare, powerful items that provide significant bonuses or
 * unlock special abilities. They can be found through exploration, research,
 * or quest rewards. Each artifact is unique and can only be acquired once.
 *
 * Key concepts:
 * - Unique Items: Each artifact exists only once in the game
 * - Discovery: Artifacts are hidden and must be found
 * - Powerful Effects: Significant stat bonuses or unique abilities
 * - Quest Integration: Some artifacts are rewards from quest lines
 */

#ifndef NECROMANCER_ARTIFACTS_H
#define NECROMANCER_ARTIFACTS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Artifact rarities
 */
typedef enum {
    ARTIFACT_RARITY_UNCOMMON = 0,   /**< Minor artifacts */
    ARTIFACT_RARITY_RARE,           /**< Significant artifacts */
    ARTIFACT_RARITY_EPIC,           /**< Major artifacts */
    ARTIFACT_RARITY_LEGENDARY,      /**< Unique legendary artifacts */
    ARTIFACT_RARITY_COUNT
} ArtifactRarity;

/**
 * @brief Artifact effect types
 */
typedef enum {
    ARTIFACT_EFFECT_STAT_BONUS = 0, /**< Permanent stat increase */
    ARTIFACT_EFFECT_ABILITY,        /**< Unlock new ability */
    ARTIFACT_EFFECT_PASSIVE,        /**< Ongoing passive effect */
    ARTIFACT_EFFECT_UNIQUE,         /**< Special unique effect */
    ARTIFACT_EFFECT_COUNT
} ArtifactEffectType;

/**
 * @brief Individual artifact definition
 */
typedef struct {
    uint32_t id;                    /**< Unique artifact ID */
    char name[64];                  /**< Artifact name */
    char description[256];          /**< Artifact description */
    char lore[512];                 /**< Lore/flavor text */
    ArtifactRarity rarity;          /**< Rarity tier */

    /* Effect */
    ArtifactEffectType effect_type; /**< Type of effect */
    char effect_stat[32];           /**< Stat name (for STAT_BONUS) */
    float effect_value;             /**< Effect magnitude */
    char effect_ability[64];        /**< Ability name (for ABILITY) */

    /* Discovery */
    uint32_t discovery_location_id; /**< Location where found (0 if quest reward) */
    char discovery_method[64];      /**< How to discover (explore, research, quest) */

    /* State */
    bool discovered;                /**< Whether player has found this */
    bool equipped;                  /**< Whether currently equipped/active */
} Artifact;

/**
 * @brief Artifact collection manager (opaque)
 */
typedef struct ArtifactCollection ArtifactCollection;

/**
 * @brief Create a new artifact collection
 *
 * @return Newly allocated ArtifactCollection, or NULL on failure
 */
ArtifactCollection* artifact_collection_create(void);

/**
 * @brief Destroy artifact collection and free all resources
 *
 * @param collection Artifact collection to destroy (can be NULL)
 */
void artifact_collection_destroy(ArtifactCollection* collection);

/**
 * @brief Add an artifact to the collection
 *
 * @param collection Artifact collection
 * @param artifact Artifact to add (copied internally)
 * @return true on success, false on failure
 */
bool artifact_add(ArtifactCollection* collection, const Artifact* artifact);

/**
 * @brief Get an artifact by ID
 *
 * @param collection Artifact collection
 * @param artifact_id Artifact ID
 * @return Pointer to artifact, or NULL if not found
 */
const Artifact* artifact_get(const ArtifactCollection* collection, uint32_t artifact_id);

/**
 * @brief Discover an artifact
 *
 * Marks artifact as discovered. Caller should handle discovery events.
 *
 * @param collection Artifact collection
 * @param artifact_id Artifact to discover
 * @return true on success, false if artifact not found or already discovered
 */
bool artifact_discover(ArtifactCollection* collection, uint32_t artifact_id);

/**
 * @brief Equip an artifact
 *
 * Marks artifact as equipped/active. Some artifacts may require being
 * equipped to grant their effects.
 *
 * @param collection Artifact collection
 * @param artifact_id Artifact to equip
 * @return true on success, false if artifact not found or not discovered
 */
bool artifact_equip(ArtifactCollection* collection, uint32_t artifact_id);

/**
 * @brief Unequip an artifact
 *
 * @param collection Artifact collection
 * @param artifact_id Artifact to unequip
 * @return true on success, false if artifact not found
 */
bool artifact_unequip(ArtifactCollection* collection, uint32_t artifact_id);

/**
 * @brief Get all artifacts by rarity
 *
 * @param collection Artifact collection
 * @param rarity Rarity to filter by
 * @param results Output array of artifact IDs (caller allocates)
 * @param max_results Maximum number of results
 * @return Number of artifacts found
 */
size_t artifact_get_by_rarity(const ArtifactCollection* collection,
                               ArtifactRarity rarity,
                               uint32_t* results,
                               size_t max_results);

/**
 * @brief Get all discovered artifacts
 *
 * @param collection Artifact collection
 * @param results Output array of artifact IDs (caller allocates)
 * @param max_results Maximum number of results
 * @return Number of discovered artifacts
 */
size_t artifact_get_discovered(const ArtifactCollection* collection,
                                uint32_t* results,
                                size_t max_results);

/**
 * @brief Get all equipped artifacts
 *
 * @param collection Artifact collection
 * @param results Output array of artifact IDs (caller allocates)
 * @param max_results Maximum number of results
 * @return Number of equipped artifacts
 */
size_t artifact_get_equipped(const ArtifactCollection* collection,
                              uint32_t* results,
                              size_t max_results);

/**
 * @brief Calculate total stat bonus from equipped artifacts
 *
 * @param collection Artifact collection
 * @param stat_name Stat to calculate
 * @return Total bonus multiplier (e.g., 1.15 for +15%)
 */
float artifact_get_stat_bonus(const ArtifactCollection* collection, const char* stat_name);

/**
 * @brief Check if an ability is unlocked by artifacts
 *
 * @param collection Artifact collection
 * @param ability_name Ability name
 * @return true if unlocked, false otherwise
 */
bool artifact_is_ability_unlocked(const ArtifactCollection* collection, const char* ability_name);

/**
 * @brief Load artifacts from data file
 *
 * @param collection Artifact collection
 * @param filepath Path to artifacts.dat file
 * @return Number of artifacts loaded
 */
size_t artifact_load_from_file(ArtifactCollection* collection, const char* filepath);

/**
 * @brief Get rarity name
 *
 * @param rarity Rarity enum
 * @return Rarity name string
 */
const char* artifact_rarity_name(ArtifactRarity rarity);

/**
 * @brief Get rarity color (for display)
 *
 * @param rarity Rarity enum
 * @return ANSI color code string
 */
const char* artifact_rarity_color(ArtifactRarity rarity);

/**
 * @brief Get statistics for artifact collection
 *
 * @param collection Artifact collection
 * @param total_artifacts Output: total artifacts available
 * @param discovered_artifacts Output: total discovered artifacts
 * @param equipped_artifacts Output: total equipped artifacts
 */
void artifact_get_stats(const ArtifactCollection* collection,
                        size_t* total_artifacts,
                        size_t* discovered_artifacts,
                        size_t* equipped_artifacts);

#endif /* NECROMANCER_ARTIFACTS_H */

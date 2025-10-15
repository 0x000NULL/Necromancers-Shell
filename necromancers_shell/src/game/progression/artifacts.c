/**
 * @file artifacts.c
 * @brief Implementation of artifact system
 */

#include "artifacts.h"
#include "../../data/data_loader.h"
#include "../../utils/logger.h"
#include <stdlib.h>
#include <string.h>

#define MAX_ARTIFACTS 50

/**
 * @brief Artifact collection structure
 */
struct ArtifactCollection {
    Artifact artifacts[MAX_ARTIFACTS];  /**< Artifact array */
    size_t artifact_count;              /**< Number of artifacts */
};

ArtifactCollection* artifact_collection_create(void) {
    ArtifactCollection* collection = calloc(1, sizeof(ArtifactCollection));
    if (!collection) {
        LOG_ERROR("Failed to allocate artifact collection");
        return NULL;
    }

    collection->artifact_count = 0;
    LOG_DEBUG("Artifact collection created");
    return collection;
}

void artifact_collection_destroy(ArtifactCollection* collection) {
    if (!collection) {
        return;
    }

    free(collection);
    LOG_DEBUG("Artifact collection destroyed");
}

bool artifact_add(ArtifactCollection* collection, const Artifact* artifact) {
    if (!collection || !artifact) {
        return false;
    }

    if (collection->artifact_count >= MAX_ARTIFACTS) {
        LOG_WARN("Artifact collection is full (max %d artifacts)", MAX_ARTIFACTS);
        return false;
    }

    /* Check for duplicate ID */
    for (size_t i = 0; i < collection->artifact_count; i++) {
        if (collection->artifacts[i].id == artifact->id) {
            LOG_WARN("Artifact ID %u already exists", artifact->id);
            return false;
        }
    }

    /* Copy artifact */
    collection->artifacts[collection->artifact_count] = *artifact;
    collection->artifact_count++;

    LOG_DEBUG("Added artifact %u: %s", artifact->id, artifact->name);
    return true;
}

const Artifact* artifact_get(const ArtifactCollection* collection, uint32_t artifact_id) {
    if (!collection) {
        return NULL;
    }

    for (size_t i = 0; i < collection->artifact_count; i++) {
        if (collection->artifacts[i].id == artifact_id) {
            return &collection->artifacts[i];
        }
    }

    return NULL;
}

bool artifact_discover(ArtifactCollection* collection, uint32_t artifact_id) {
    if (!collection) {
        return false;
    }

    for (size_t i = 0; i < collection->artifact_count; i++) {
        if (collection->artifacts[i].id == artifact_id) {
            if (collection->artifacts[i].discovered) {
                LOG_DEBUG("Artifact %u already discovered", artifact_id);
                return false;
            }

            collection->artifacts[i].discovered = true;
            LOG_INFO("Discovered artifact: %s", collection->artifacts[i].name);
            return true;
        }
    }

    LOG_WARN("Artifact %u not found", artifact_id);
    return false;
}

bool artifact_equip(ArtifactCollection* collection, uint32_t artifact_id) {
    if (!collection) {
        return false;
    }

    for (size_t i = 0; i < collection->artifact_count; i++) {
        if (collection->artifacts[i].id == artifact_id) {
            if (!collection->artifacts[i].discovered) {
                LOG_DEBUG("Cannot equip undiscovered artifact %u", artifact_id);
                return false;
            }

            collection->artifacts[i].equipped = true;
            LOG_INFO("Equipped artifact: %s", collection->artifacts[i].name);
            return true;
        }
    }

    LOG_WARN("Artifact %u not found", artifact_id);
    return false;
}

bool artifact_unequip(ArtifactCollection* collection, uint32_t artifact_id) {
    if (!collection) {
        return false;
    }

    for (size_t i = 0; i < collection->artifact_count; i++) {
        if (collection->artifacts[i].id == artifact_id) {
            collection->artifacts[i].equipped = false;
            LOG_INFO("Unequipped artifact: %s", collection->artifacts[i].name);
            return true;
        }
    }

    LOG_WARN("Artifact %u not found", artifact_id);
    return false;
}

size_t artifact_get_by_rarity(const ArtifactCollection* collection,
                               ArtifactRarity rarity,
                               uint32_t* results,
                               size_t max_results) {
    if (!collection || !results || max_results == 0) {
        return 0;
    }

    size_t count = 0;
    for (size_t i = 0; i < collection->artifact_count && count < max_results; i++) {
        if (collection->artifacts[i].rarity == rarity) {
            results[count++] = collection->artifacts[i].id;
        }
    }

    return count;
}

size_t artifact_get_discovered(const ArtifactCollection* collection,
                                uint32_t* results,
                                size_t max_results) {
    if (!collection || !results || max_results == 0) {
        return 0;
    }

    size_t count = 0;
    for (size_t i = 0; i < collection->artifact_count && count < max_results; i++) {
        if (collection->artifacts[i].discovered) {
            results[count++] = collection->artifacts[i].id;
        }
    }

    return count;
}

size_t artifact_get_equipped(const ArtifactCollection* collection,
                              uint32_t* results,
                              size_t max_results) {
    if (!collection || !results || max_results == 0) {
        return 0;
    }

    size_t count = 0;
    for (size_t i = 0; i < collection->artifact_count && count < max_results; i++) {
        if (collection->artifacts[i].equipped) {
            results[count++] = collection->artifacts[i].id;
        }
    }

    return count;
}

float artifact_get_stat_bonus(const ArtifactCollection* collection, const char* stat_name) {
    if (!collection || !stat_name) {
        return 1.0f;
    }

    float total_bonus = 1.0f;

    for (size_t i = 0; i < collection->artifact_count; i++) {
        const Artifact* artifact = &collection->artifacts[i];

        if (artifact->equipped &&
            artifact->effect_type == ARTIFACT_EFFECT_STAT_BONUS &&
            strcmp(artifact->effect_stat, stat_name) == 0) {
            total_bonus += artifact->effect_value;
        }
    }

    return total_bonus;
}

bool artifact_is_ability_unlocked(const ArtifactCollection* collection, const char* ability_name) {
    if (!collection || !ability_name) {
        return false;
    }

    for (size_t i = 0; i < collection->artifact_count; i++) {
        const Artifact* artifact = &collection->artifacts[i];

        if (artifact->discovered &&
            artifact->effect_type == ARTIFACT_EFFECT_ABILITY &&
            strcmp(artifact->effect_ability, ability_name) == 0) {
            return true;
        }
    }

    return false;
}

size_t artifact_load_from_file(ArtifactCollection* collection, const char* filepath) {
    if (!collection || !filepath) {
        LOG_ERROR("artifact_load_from_file: NULL parameter");
        return 0;
    }

    DataFile* data = data_file_load(filepath);
    if (!data) {
        LOG_WARN("Failed to load artifact data from %s", filepath);
        return 0;
    }

    size_t loaded = 0;
    size_t section_count;
    const DataSection** sections = data_file_get_sections(data, "ARTIFACT", &section_count);

    for (size_t i = 0; i < section_count; i++) {
        const DataSection* section = sections[i];
        if (!section) continue;

        Artifact artifact = {0};
        artifact.id = (uint32_t)atoi(section->section_id);

        /* Load basic properties */
        const DataValue* val;
        val = data_section_get(section, "name");
        strncpy(artifact.name, data_value_get_string(val, "Unknown"), sizeof(artifact.name) - 1);

        val = data_section_get(section, "description");
        strncpy(artifact.description, data_value_get_string(val, ""), sizeof(artifact.description) - 1);

        val = data_section_get(section, "lore");
        strncpy(artifact.lore, data_value_get_string(val, ""), sizeof(artifact.lore) - 1);

        /* Load rarity */
        char rarity_str[32];
        val = data_section_get(section, "rarity");
        strncpy(rarity_str, data_value_get_string(val, "uncommon"), sizeof(rarity_str) - 1);
        if (strcmp(rarity_str, "rare") == 0) {
            artifact.rarity = ARTIFACT_RARITY_RARE;
        } else if (strcmp(rarity_str, "epic") == 0) {
            artifact.rarity = ARTIFACT_RARITY_EPIC;
        } else if (strcmp(rarity_str, "legendary") == 0) {
            artifact.rarity = ARTIFACT_RARITY_LEGENDARY;
        } else {
            artifact.rarity = ARTIFACT_RARITY_UNCOMMON;
        }

        /* Load effect */
        char effect_type_str[32];
        val = data_section_get(section, "effect_type");
        strncpy(effect_type_str, data_value_get_string(val, "stat_bonus"), sizeof(effect_type_str) - 1);
        if (strcmp(effect_type_str, "ability") == 0) {
            artifact.effect_type = ARTIFACT_EFFECT_ABILITY;
        } else if (strcmp(effect_type_str, "passive") == 0) {
            artifact.effect_type = ARTIFACT_EFFECT_PASSIVE;
        } else if (strcmp(effect_type_str, "unique") == 0) {
            artifact.effect_type = ARTIFACT_EFFECT_UNIQUE;
        } else {
            artifact.effect_type = ARTIFACT_EFFECT_STAT_BONUS;
        }

        val = data_section_get(section, "effect_stat");
        strncpy(artifact.effect_stat, data_value_get_string(val, ""), sizeof(artifact.effect_stat) - 1);

        val = data_section_get(section, "effect_value");
        artifact.effect_value = (float)data_value_get_float(val, 0.0);

        val = data_section_get(section, "effect_ability");
        strncpy(artifact.effect_ability, data_value_get_string(val, ""), sizeof(artifact.effect_ability) - 1);

        /* Load discovery info */
        val = data_section_get(section, "discovery_location_id");
        artifact.discovery_location_id = (uint32_t)data_value_get_int(val, 0);

        val = data_section_get(section, "discovery_method");
        strncpy(artifact.discovery_method, data_value_get_string(val, "explore"), sizeof(artifact.discovery_method) - 1);

        artifact.discovered = false;
        artifact.equipped = false;

        if (artifact_add(collection, &artifact)) {
            loaded++;
        }
    }

    data_file_destroy(data);

    LOG_INFO("Loaded %zu artifacts from %s", loaded, filepath);
    return loaded;
}

const char* artifact_rarity_name(ArtifactRarity rarity) {
    switch (rarity) {
        case ARTIFACT_RARITY_UNCOMMON:   return "Uncommon";
        case ARTIFACT_RARITY_RARE:       return "Rare";
        case ARTIFACT_RARITY_EPIC:       return "Epic";
        case ARTIFACT_RARITY_LEGENDARY:  return "Legendary";
        default:                         return "Unknown";
    }
}

const char* artifact_rarity_color(ArtifactRarity rarity) {
    switch (rarity) {
        case ARTIFACT_RARITY_UNCOMMON:   return "\033[32m";  /* Green */
        case ARTIFACT_RARITY_RARE:       return "\033[34m";  /* Blue */
        case ARTIFACT_RARITY_EPIC:       return "\033[35m";  /* Magenta */
        case ARTIFACT_RARITY_LEGENDARY:  return "\033[33m";  /* Yellow */
        default:                         return "\033[0m";   /* Reset */
    }
}

void artifact_get_stats(const ArtifactCollection* collection,
                        size_t* total_artifacts,
                        size_t* discovered_artifacts,
                        size_t* equipped_artifacts) {
    if (!collection) {
        if (total_artifacts) *total_artifacts = 0;
        if (discovered_artifacts) *discovered_artifacts = 0;
        if (equipped_artifacts) *equipped_artifacts = 0;
        return;
    }

    if (total_artifacts) {
        *total_artifacts = collection->artifact_count;
    }

    if (discovered_artifacts) {
        size_t count = 0;
        for (size_t i = 0; i < collection->artifact_count; i++) {
            if (collection->artifacts[i].discovered) {
                count++;
            }
        }
        *discovered_artifacts = count;
    }

    if (equipped_artifacts) {
        size_t count = 0;
        for (size_t i = 0; i < collection->artifact_count; i++) {
            if (collection->artifacts[i].equipped) {
                count++;
            }
        }
        *equipped_artifacts = count;
    }
}

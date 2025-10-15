/**
 * @file test_artifacts.c
 * @brief Unit tests for artifact system
 */

#include "../src/game/progression/artifacts.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "FAIL: %s\n", message); \
            return; \
        } \
    } while(0)

#define ASSERT_EQ(expected, actual, message) \
    do { \
        if ((expected) != (actual)) { \
            fprintf(stderr, "FAIL: %s (expected %d, got %d)\n", message, (int)(expected), (int)(actual)); \
            return; \
        } \
    } while(0)

static void test_create_destroy(void) {
    printf("Running test_create_destroy...\n");

    ArtifactCollection* collection = artifact_collection_create();
    ASSERT(collection != NULL, "Failed to create artifact collection");

    artifact_collection_destroy(collection);
    artifact_collection_destroy(NULL);  /* Should not crash */
}

static void test_add_artifact(void) {
    printf("Running test_add_artifact...\n");

    ArtifactCollection* collection = artifact_collection_create();
    ASSERT(collection != NULL, "Failed to create artifact collection");

    Artifact artifact = {
        .id = 1,
        .rarity = ARTIFACT_RARITY_RARE,
        .effect_type = ARTIFACT_EFFECT_STAT_BONUS,
        .effect_value = 0.25f,
        .discovery_location_id = 5,
        .discovered = false,
        .equipped = false
    };
    strncpy(artifact.name, "Crown of Bones", sizeof(artifact.name) - 1);
    strncpy(artifact.description, "Increases minion power", sizeof(artifact.description) - 1);
    strncpy(artifact.lore, "Ancient crown worn by the first necromancer king", sizeof(artifact.lore) - 1);
    strncpy(artifact.effect_stat, "minion_damage", sizeof(artifact.effect_stat) - 1);
    strncpy(artifact.discovery_method, "explore", sizeof(artifact.discovery_method) - 1);

    bool success = artifact_add(collection, &artifact);
    ASSERT(success, "Failed to add artifact");

    const Artifact* retrieved = artifact_get(collection, 1);
    ASSERT(retrieved != NULL, "Failed to retrieve artifact");
    ASSERT_EQ(1, retrieved->id, "Artifact ID mismatch");
    ASSERT(strcmp(retrieved->name, "Crown of Bones") == 0, "Artifact name mismatch");

    /* Test duplicate */
    success = artifact_add(collection, &artifact);
    ASSERT(!success, "Should not allow duplicate artifact ID");

    artifact_collection_destroy(collection);
}

static void test_discover_artifact(void) {
    printf("Running test_discover_artifact...\n");

    ArtifactCollection* collection = artifact_collection_create();
    ASSERT(collection != NULL, "Failed to create artifact collection");

    Artifact artifact = {
        .id = 1,
        .discovered = false,
        .equipped = false
    };
    strncpy(artifact.name, "Test Artifact", sizeof(artifact.name) - 1);

    artifact_add(collection, &artifact);

    /* Initially not discovered */
    const Artifact* retrieved = artifact_get(collection, 1);
    ASSERT(!retrieved->discovered, "Artifact should not be discovered initially");

    /* Discover artifact */
    bool success = artifact_discover(collection, 1);
    ASSERT(success, "Failed to discover artifact");

    retrieved = artifact_get(collection, 1);
    ASSERT(retrieved->discovered, "Artifact should be discovered");

    /* Try to discover again (should fail) */
    success = artifact_discover(collection, 1);
    ASSERT(!success, "Should not discover already discovered artifact");

    artifact_collection_destroy(collection);
}

static void test_equip_artifact(void) {
    printf("Running test_equip_artifact...\n");

    ArtifactCollection* collection = artifact_collection_create();
    ASSERT(collection != NULL, "Failed to create artifact collection");

    Artifact artifact = {
        .id = 1,
        .discovered = false,
        .equipped = false
    };

    artifact_add(collection, &artifact);

    /* Try to equip undiscovered artifact (should fail) */
    bool success = artifact_equip(collection, 1);
    ASSERT(!success, "Should not equip undiscovered artifact");

    /* Discover artifact */
    artifact_discover(collection, 1);

    /* Now equip */
    success = artifact_equip(collection, 1);
    ASSERT(success, "Failed to equip artifact");

    const Artifact* retrieved = artifact_get(collection, 1);
    ASSERT(retrieved->equipped, "Artifact should be equipped");

    /* Unequip */
    success = artifact_unequip(collection, 1);
    ASSERT(success, "Failed to unequip artifact");

    retrieved = artifact_get(collection, 1);
    ASSERT(!retrieved->equipped, "Artifact should not be equipped");

    artifact_collection_destroy(collection);
}

static void test_get_by_rarity(void) {
    printf("Running test_get_by_rarity...\n");

    ArtifactCollection* collection = artifact_collection_create();
    ASSERT(collection != NULL, "Failed to create artifact collection");

    /* Add artifacts with different rarities */
    for (int i = 1; i <= 3; i++) {
        Artifact artifact = {
            .id = (uint32_t)i,
            .rarity = ARTIFACT_RARITY_RARE,
            .discovered = false
        };
        snprintf(artifact.name, sizeof(artifact.name), "Rare Artifact %d", i);
        artifact_add(collection, &artifact);
    }

    for (int i = 4; i <= 5; i++) {
        Artifact artifact = {
            .id = (uint32_t)i,
            .rarity = ARTIFACT_RARITY_LEGENDARY,
            .discovered = false
        };
        snprintf(artifact.name, sizeof(artifact.name), "Legendary Artifact %d", i);
        artifact_add(collection, &artifact);
    }

    uint32_t results[10];
    size_t count = artifact_get_by_rarity(collection, ARTIFACT_RARITY_RARE, results, 10);
    ASSERT_EQ(3, count, "Should have 3 rare artifacts");

    count = artifact_get_by_rarity(collection, ARTIFACT_RARITY_LEGENDARY, results, 10);
    ASSERT_EQ(2, count, "Should have 2 legendary artifacts");

    artifact_collection_destroy(collection);
}

static void test_stat_bonus(void) {
    printf("Running test_stat_bonus...\n");

    ArtifactCollection* collection = artifact_collection_create();
    ASSERT(collection != NULL, "Failed to create artifact collection");

    /* Add artifacts with stat bonuses */
    Artifact artifact1 = {
        .id = 1,
        .effect_type = ARTIFACT_EFFECT_STAT_BONUS,
        .effect_value = 0.15f,
        .discovered = true,
        .equipped = false
    };
    strncpy(artifact1.effect_stat, "minion_hp", sizeof(artifact1.effect_stat) - 1);

    Artifact artifact2 = {
        .id = 2,
        .effect_type = ARTIFACT_EFFECT_STAT_BONUS,
        .effect_value = 0.20f,
        .discovered = true,
        .equipped = false
    };
    strncpy(artifact2.effect_stat, "minion_hp", sizeof(artifact2.effect_stat) - 1);

    artifact_add(collection, &artifact1);
    artifact_add(collection, &artifact2);

    /* Initially no bonus (artifacts not equipped) */
    float bonus = artifact_get_stat_bonus(collection, "minion_hp");
    ASSERT(bonus == 1.0f, "Should have no bonus initially");

    /* Equip first artifact */
    artifact_equip(collection, 1);
    bonus = artifact_get_stat_bonus(collection, "minion_hp");
    ASSERT(bonus == 1.15f, "Should have 15% bonus");

    /* Equip second artifact */
    artifact_equip(collection, 2);
    bonus = artifact_get_stat_bonus(collection, "minion_hp");
    ASSERT(bonus == 1.35f, "Should have 35% bonus");

    artifact_collection_destroy(collection);
}

static void test_ability_unlock(void) {
    printf("Running test_ability_unlock...\n");

    ArtifactCollection* collection = artifact_collection_create();
    ASSERT(collection != NULL, "Failed to create artifact collection");

    Artifact artifact = {
        .id = 1,
        .effect_type = ARTIFACT_EFFECT_ABILITY,
        .discovered = false
    };
    strncpy(artifact.effect_ability, "teleport", sizeof(artifact.effect_ability) - 1);

    artifact_add(collection, &artifact);

    /* Initially not unlocked */
    bool unlocked = artifact_is_ability_unlocked(collection, "teleport");
    ASSERT(!unlocked, "Ability should not be unlocked initially");

    /* Discover artifact */
    artifact_discover(collection, 1);
    unlocked = artifact_is_ability_unlocked(collection, "teleport");
    ASSERT(unlocked, "Ability should be unlocked after discovery");

    artifact_collection_destroy(collection);
}

static void test_get_discovered(void) {
    printf("Running test_get_discovered...\n");

    ArtifactCollection* collection = artifact_collection_create();
    ASSERT(collection != NULL, "Failed to create artifact collection");

    /* Add artifacts */
    for (int i = 1; i <= 5; i++) {
        Artifact artifact = {
            .id = (uint32_t)i,
            .discovered = false
        };
        artifact_add(collection, &artifact);
    }

    /* Discover 2 artifacts */
    artifact_discover(collection, 1);
    artifact_discover(collection, 3);

    uint32_t results[10];
    size_t count = artifact_get_discovered(collection, results, 10);
    ASSERT_EQ(2, count, "Should have 2 discovered artifacts");

    artifact_collection_destroy(collection);
}

static void test_get_equipped(void) {
    printf("Running test_get_equipped...\n");

    ArtifactCollection* collection = artifact_collection_create();
    ASSERT(collection != NULL, "Failed to create artifact collection");

    /* Add artifacts */
    for (int i = 1; i <= 5; i++) {
        Artifact artifact = {
            .id = (uint32_t)i,
            .discovered = true,
            .equipped = false
        };
        artifact_add(collection, &artifact);
    }

    /* Equip 3 artifacts */
    artifact_equip(collection, 1);
    artifact_equip(collection, 2);
    artifact_equip(collection, 5);

    uint32_t results[10];
    size_t count = artifact_get_equipped(collection, results, 10);
    ASSERT_EQ(3, count, "Should have 3 equipped artifacts");

    artifact_collection_destroy(collection);
}

static void test_get_stats(void) {
    printf("Running test_get_stats...\n");

    ArtifactCollection* collection = artifact_collection_create();
    ASSERT(collection != NULL, "Failed to create artifact collection");

    /* Add artifacts */
    for (int i = 1; i <= 5; i++) {
        Artifact artifact = {
            .id = (uint32_t)i,
            .discovered = false,
            .equipped = false
        };
        artifact_add(collection, &artifact);
    }

    /* Discover 3, equip 2 */
    artifact_discover(collection, 1);
    artifact_discover(collection, 2);
    artifact_discover(collection, 3);
    artifact_equip(collection, 1);
    artifact_equip(collection, 2);

    size_t total, discovered, equipped;
    artifact_get_stats(collection, &total, &discovered, &equipped);

    ASSERT_EQ(5, total, "Should have 5 total artifacts");
    ASSERT_EQ(3, discovered, "Should have 3 discovered artifacts");
    ASSERT_EQ(2, equipped, "Should have 2 equipped artifacts");

    artifact_collection_destroy(collection);
}

static void test_rarity_names(void) {
    printf("Running test_rarity_names...\n");

    ASSERT(strcmp(artifact_rarity_name(ARTIFACT_RARITY_UNCOMMON), "Uncommon") == 0,
           "Uncommon rarity name mismatch");
    ASSERT(strcmp(artifact_rarity_name(ARTIFACT_RARITY_RARE), "Rare") == 0,
           "Rare rarity name mismatch");
    ASSERT(strcmp(artifact_rarity_name(ARTIFACT_RARITY_EPIC), "Epic") == 0,
           "Epic rarity name mismatch");
    ASSERT(strcmp(artifact_rarity_name(ARTIFACT_RARITY_LEGENDARY), "Legendary") == 0,
           "Legendary rarity name mismatch");

    /* Test color codes (just check they're not NULL or empty) */
    const char* color = artifact_rarity_color(ARTIFACT_RARITY_RARE);
    ASSERT(color != NULL && strlen(color) > 0, "Rarity color should not be empty");
}

int main(void) {
    printf("Running Artifact Tests...\n\n");

    test_create_destroy();
    test_add_artifact();
    test_discover_artifact();
    test_equip_artifact();
    test_get_by_rarity();
    test_stat_bonus();
    test_ability_unlock();
    test_get_discovered();
    test_get_equipped();
    test_get_stats();
    test_rarity_names();

    printf("\n\033[32mAll artifact tests passed!\033[0m\n");
    return 0;
}

/**
 * @file test_death_network.c
 * @brief Unit tests for Death Network system
 */

#include "../src/game/world/death_network.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* ANSI colors */
#define COLOR_GREEN "\033[32m"
#define COLOR_RED   "\033[31m"
#define COLOR_RESET "\033[0m"

/* Test counter */
static int tests_run = 0;
static int tests_passed = 0;

/* Test macros */
#define TEST(name) \
    static void name(void); \
    static void name##_wrapper(void) { \
        printf("Running %s...\n", #name); \
        tests_run++; \
        name(); \
        tests_passed++; \
        printf("%s✓ %s passed%s\n", COLOR_GREEN, #name, COLOR_RESET); \
    } \
    static void name(void)

#define RUN_TEST(name) name##_wrapper()

#define ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf("%s✗ Assertion failed: %s%s\n", COLOR_RED, message, COLOR_RESET); \
            printf("  at %s:%d\n", __FILE__, __LINE__); \
            exit(1); \
        } \
    } while(0)

#define ASSERT_EQ(expected, actual, message) \
    ASSERT((expected) == (actual), message)

#define ASSERT_NE(expected, actual, message) \
    ASSERT((expected) != (actual), message)

#define ASSERT_NULL(ptr, message) \
    ASSERT((ptr) == NULL, message)

#define ASSERT_NOT_NULL(ptr, message) \
    ASSERT((ptr) != NULL, message)

/* ========================================================================
 * Test Cases
 * ======================================================================== */

TEST(test_create_destroy) {
    DeathNetwork* network = death_network_create();
    ASSERT_NOT_NULL(network, "Network creation failed");

    death_network_destroy(network);
    death_network_destroy(NULL);  /* Should not crash */
}

TEST(test_add_location) {
    DeathNetwork* network = death_network_create();
    ASSERT_NOT_NULL(network, "Network creation failed");

    /* Add a graveyard location */
    bool result = death_network_add_location(network, 1, 60, 20, 3);
    ASSERT(result, "Failed to add location");

    /* Verify node exists */
    const DeathNode* node = death_network_get_node(network, 1);
    ASSERT_NOT_NULL(node, "Node not found after adding");
    ASSERT_EQ(1, node->location_id, "Wrong location ID");
    ASSERT_EQ(60, node->signature, "Wrong signature");
    ASSERT_EQ(60, node->base_signature, "Wrong base signature");
    ASSERT_EQ(20, node->max_corpses, "Wrong max corpses");
    ASSERT_EQ(3, node->regen_rate, "Wrong regen rate");
    ASSERT_EQ(10, node->available_corpses, "Should start with 50% corpses");

    death_network_destroy(network);
}

TEST(test_add_duplicate_location) {
    DeathNetwork* network = death_network_create();
    ASSERT_NOT_NULL(network, "Network creation failed");

    death_network_add_location(network, 1, 60, 20, 3);

    /* Try to add duplicate */
    bool result = death_network_add_location(network, 1, 70, 30, 5);
    ASSERT(!result, "Should not allow duplicate locations");

    /* Verify original node unchanged */
    const DeathNode* node = death_network_get_node(network, 1);
    ASSERT_EQ(60, node->signature, "Node should not be modified");

    death_network_destroy(network);
}

TEST(test_get_nonexistent_node) {
    DeathNetwork* network = death_network_create();
    ASSERT_NOT_NULL(network, "Network creation failed");

    const DeathNode* node = death_network_get_node(network, 999);
    ASSERT_NULL(node, "Should return NULL for nonexistent node");

    death_network_destroy(network);
}

TEST(test_trigger_death_event) {
    DeathNetwork* network = death_network_create();
    ASSERT_NOT_NULL(network, "Network creation failed");

    death_network_add_location(network, 1, 40, 50, 2);

    /* Trigger a battle event */
    DeathEvent event = {
        .location_id = 1,
        .type = DEATH_EVENT_BATTLE,
        .death_count = 10,
        .avg_quality = DEATH_QUALITY_GOOD,
        .timestamp_hours = 0
    };

    bool result = death_network_trigger_event(network, &event);
    ASSERT(result, "Failed to trigger event");

    /* Verify effects */
    const DeathNode* node = death_network_get_node(network, 1);
    ASSERT_EQ(45, node->signature, "Signature should increase by 5 (10 deaths / 2)");
    ASSERT_EQ(35, node->available_corpses, "Should have 25 (initial 50% of 50) + 10 (event) corpses");
    ASSERT_EQ(10, node->total_deaths, "Should track total deaths");
    ASSERT_EQ(DEATH_EVENT_BATTLE, node->last_event_type, "Should record event type");

    death_network_destroy(network);
}

TEST(test_trigger_event_unknown_location) {
    DeathNetwork* network = death_network_create();
    ASSERT_NOT_NULL(network, "Network creation failed");

    DeathEvent event = {
        .location_id = 999,
        .type = DEATH_EVENT_NATURAL,
        .death_count = 1,
        .avg_quality = DEATH_QUALITY_POOR,
        .timestamp_hours = 0
    };

    bool result = death_network_trigger_event(network, &event);
    ASSERT(!result, "Should fail for unknown location");

    death_network_destroy(network);
}

TEST(test_harvest_corpses) {
    DeathNetwork* network = death_network_create();
    ASSERT_NOT_NULL(network, "Network creation failed");

    death_network_add_location(network, 1, 50, 50, 2);

    /* Harvest some corpses */
    DeathQuality qualities[5];
    uint32_t harvested = death_network_harvest_corpses(network, 1, 5, qualities);

    ASSERT_EQ(5, harvested, "Should harvest 5 corpses");

    /* Verify qualities are valid */
    for (int i = 0; i < 5; i++) {
        ASSERT(qualities[i] >= DEATH_QUALITY_POOR &&
               qualities[i] <= DEATH_QUALITY_LEGENDARY,
               "Quality should be valid");
    }

    /* Verify corpses removed */
    const DeathNode* node = death_network_get_node(network, 1);
    ASSERT_EQ(20, node->available_corpses, "Should have 25 - 5 = 20 corpses");

    death_network_destroy(network);
}

TEST(test_harvest_more_than_available) {
    DeathNetwork* network = death_network_create();
    ASSERT_NOT_NULL(network, "Network creation failed");

    death_network_add_location(network, 1, 50, 20, 2);

    /* Try to harvest more than available */
    DeathQuality qualities[20];
    uint32_t harvested = death_network_harvest_corpses(network, 1, 20, qualities);

    ASSERT_EQ(10, harvested, "Should only harvest available corpses (50% of 20)");

    const DeathNode* node = death_network_get_node(network, 1);
    ASSERT_EQ(0, node->available_corpses, "Should have 0 corpses remaining");

    death_network_destroy(network);
}

TEST(test_harvest_from_unknown_location) {
    DeathNetwork* network = death_network_create();
    ASSERT_NOT_NULL(network, "Network creation failed");

    uint32_t harvested = death_network_harvest_corpses(network, 999, 5, NULL);
    ASSERT_EQ(0, harvested, "Should harvest 0 from unknown location");

    death_network_destroy(network);
}

TEST(test_corpse_regeneration) {
    DeathNetwork* network = death_network_create();
    ASSERT_NOT_NULL(network, "Network creation failed");

    /* Add location with regen rate of 4 corpses/day */
    death_network_add_location(network, 1, 50, 50, 4);

    /* Harvest all corpses */
    death_network_harvest_corpses(network, 1, 25, NULL);

    const DeathNode* node1 = death_network_get_node(network, 1);
    ASSERT_EQ(0, node1->available_corpses, "Should start with 0 corpses");

    /* Update for 24 hours (1 day) */
    death_network_update(network, 24);

    node1 = death_network_get_node(network, 1);
    ASSERT_EQ(4, node1->available_corpses, "Should regenerate 4 corpses per day");

    /* Update for another 48 hours (2 days) */
    death_network_update(network, 48);

    node1 = death_network_get_node(network, 1);
    /* Should have at least 12 corpses (4+8), but may have more from random events */
    ASSERT(node1->available_corpses >= 12, "Should have at least 12 corpses");

    death_network_destroy(network);
}

TEST(test_corpse_regeneration_cap) {
    DeathNetwork* network = death_network_create();
    ASSERT_NOT_NULL(network, "Network creation failed");

    death_network_add_location(network, 1, 50, 20, 5);

    /* Update for 100 hours (should hit max) */
    death_network_update(network, 100);

    const DeathNode* node = death_network_get_node(network, 1);
    ASSERT_EQ(20, node->available_corpses, "Should cap at max_corpses");

    death_network_destroy(network);
}

TEST(test_signature_decay) {
    DeathNetwork* network = death_network_create();
    ASSERT_NOT_NULL(network, "Network creation failed");

    death_network_add_location(network, 1, 50, 20, 2);

    /* Increase signature with event */
    DeathEvent event = {
        .location_id = 1,
        .type = DEATH_EVENT_PLAGUE,
        .death_count = 20,
        .avg_quality = DEATH_QUALITY_AVERAGE,
        .timestamp_hours = 0
    };
    death_network_trigger_event(network, &event);

    const DeathNode* node1 = death_network_get_node(network, 1);
    uint8_t initial_sig = node1->signature;
    ASSERT(initial_sig > 50, "Signature should be elevated");

    /* Update for 24 hours (should decay by 1) */
    death_network_update(network, 24);

    node1 = death_network_get_node(network, 1);
    ASSERT_EQ(initial_sig - 1, node1->signature, "Signature should decay by 1");

    death_network_destroy(network);
}

TEST(test_quality_distribution) {
    DeathNetwork* network = death_network_create();
    ASSERT_NOT_NULL(network, "Network creation failed");

    death_network_add_location(network, 1, 50, 100, 5);

    /* Set quality distribution: 100% legendary */
    bool result = death_network_set_quality_distribution(network, 1,
                                                         0,   /* poor */
                                                         0,   /* average */
                                                         0,   /* good */
                                                         0,   /* excellent */
                                                         100  /* legendary */);
    ASSERT(result, "Failed to set quality distribution");

    /* Harvest and verify all legendary */
    DeathQuality qualities[10];
    uint32_t harvested = death_network_harvest_corpses(network, 1, 10, qualities);
    ASSERT_EQ(10, harvested, "Should harvest 10 corpses");

    for (int i = 0; i < 10; i++) {
        ASSERT_EQ(DEATH_QUALITY_LEGENDARY, qualities[i],
                  "All qualities should be legendary");
    }

    death_network_destroy(network);
}

TEST(test_quality_distribution_invalid_sum) {
    DeathNetwork* network = death_network_create();
    ASSERT_NOT_NULL(network, "Network creation failed");

    death_network_add_location(network, 1, 50, 100, 5);

    /* Try invalid distribution (sum != 100) */
    bool result = death_network_set_quality_distribution(network, 1,
                                                         50, 50, 0, 0, 0);  /* Sum = 100, valid */
    ASSERT(result, "Valid distribution should succeed");

    result = death_network_set_quality_distribution(network, 1,
                                                    50, 40, 0, 0, 0);  /* Sum = 90, invalid */
    ASSERT(!result, "Invalid distribution should fail");

    death_network_destroy(network);
}

TEST(test_death_quality_to_soul_energy) {
    ASSERT_EQ(10, death_quality_to_soul_energy(DEATH_QUALITY_POOR), "Poor = 10 energy");
    ASSERT_EQ(20, death_quality_to_soul_energy(DEATH_QUALITY_AVERAGE), "Average = 20 energy");
    ASSERT_EQ(35, death_quality_to_soul_energy(DEATH_QUALITY_GOOD), "Good = 35 energy");
    ASSERT_EQ(50, death_quality_to_soul_energy(DEATH_QUALITY_EXCELLENT), "Excellent = 50 energy");
    ASSERT_EQ(100, death_quality_to_soul_energy(DEATH_QUALITY_LEGENDARY), "Legendary = 100 energy");
}

TEST(test_get_hotspots) {
    DeathNetwork* network = death_network_create();
    ASSERT_NOT_NULL(network, "Network creation failed");

    /* Add locations with varying signatures */
    death_network_add_location(network, 1, 90, 50, 5);  /* Overwhelming */
    death_network_add_location(network, 2, 70, 30, 3);  /* Strong */
    death_network_add_location(network, 3, 60, 20, 2);  /* Moderate (>= 60) */
    death_network_add_location(network, 4, 30, 10, 1);  /* Weak */
    death_network_add_location(network, 5, 10, 5, 1);   /* Dormant */

    uint32_t hotspots[10];
    size_t count = death_network_get_hotspots(network, hotspots, 10);

    /* Should return locations with signature >= 60 (SIGNATURE_MODERATE) */
    ASSERT_EQ(3, count, "Should find 3 hotspots");
    ASSERT_EQ(1, hotspots[0], "First hotspot should be location 1 (90)");
    ASSERT_EQ(2, hotspots[1], "Second hotspot should be location 2 (70)");
    ASSERT_EQ(3, hotspots[2], "Third hotspot should be location 3 (60)");

    death_network_destroy(network);
}

TEST(test_get_stats) {
    DeathNetwork* network = death_network_create();
    ASSERT_NOT_NULL(network, "Network creation failed");

    death_network_add_location(network, 1, 60, 50, 3);
    death_network_add_location(network, 2, 40, 30, 2);

    /* Trigger some deaths */
    DeathEvent event = {
        .location_id = 1,
        .type = DEATH_EVENT_BATTLE,
        .death_count = 5,
        .avg_quality = DEATH_QUALITY_AVERAGE,
        .timestamp_hours = 0
    };
    death_network_trigger_event(network, &event);

    size_t total_locations;
    uint32_t total_corpses, total_deaths;
    DeathSignature avg_signature;

    death_network_get_stats(network, &total_locations, &total_corpses,
                           &total_deaths, &avg_signature);

    ASSERT_EQ(2, total_locations, "Should have 2 locations");
    ASSERT_EQ(5, total_deaths, "Should track 5 deaths");
    ASSERT(total_corpses > 0, "Should have corpses");
    ASSERT(avg_signature > 0, "Should have average signature");

    death_network_destroy(network);
}

TEST(test_string_utilities) {
    /* Test event type names */
    ASSERT(strcmp("Natural", death_event_type_name(DEATH_EVENT_NATURAL)) == 0,
           "Natural event name");
    ASSERT(strcmp("Battle", death_event_type_name(DEATH_EVENT_BATTLE)) == 0,
           "Battle event name");
    ASSERT(strcmp("Plague", death_event_type_name(DEATH_EVENT_PLAGUE)) == 0,
           "Plague event name");

    /* Test quality names */
    ASSERT(strcmp("Poor", death_quality_name(DEATH_QUALITY_POOR)) == 0,
           "Poor quality name");
    ASSERT(strcmp("Legendary", death_quality_name(DEATH_QUALITY_LEGENDARY)) == 0,
           "Legendary quality name");

    /* Test signature descriptions */
    ASSERT(strcmp("Dormant", death_signature_description(10)) == 0,
           "Dormant description");
    ASSERT(strcmp("Weak", death_signature_description(30)) == 0,
           "Weak description");
    ASSERT(strcmp("Moderate", death_signature_description(50)) == 0,
           "Moderate description");
    ASSERT(strcmp("Strong", death_signature_description(70)) == 0,
           "Strong description");
    ASSERT(strcmp("Overwhelming", death_signature_description(90)) == 0,
           "Overwhelming description");
}

TEST(test_scan_network) {
    DeathNetwork* network = death_network_create();
    ASSERT_NOT_NULL(network, "Network creation failed");

    /* Add multiple locations */
    death_network_add_location(network, 1, 80, 40, 4);
    death_network_add_location(network, 2, 60, 30, 3);
    death_network_add_location(network, 3, 40, 20, 2);
    death_network_add_location(network, 4, 20, 10, 1);

    /* Scan from location 1 */
    uint32_t results[10];
    size_t count = death_network_scan(network, 1, results, 10);

    ASSERT_EQ(3, count, "Should find 3 other locations");

    /* Results should be sorted by signature (descending) */
    const DeathNode* node0 = death_network_get_node(network, results[0]);
    const DeathNode* node1 = death_network_get_node(network, results[1]);
    const DeathNode* node2 = death_network_get_node(network, results[2]);

    ASSERT(node0->signature >= node1->signature, "First result should have highest signature");
    ASSERT(node1->signature >= node2->signature, "Should be sorted descending");

    death_network_destroy(network);
}

/* ========================================================================
 * Test Runner
 * ======================================================================== */

int main(void) {
    printf("\n");
    printf("===========================================\n");
    printf("   Death Network System Unit Tests\n");
    printf("===========================================\n\n");

    /* Run all tests */
    RUN_TEST(test_create_destroy);
    RUN_TEST(test_add_location);
    RUN_TEST(test_add_duplicate_location);
    RUN_TEST(test_get_nonexistent_node);
    RUN_TEST(test_trigger_death_event);
    RUN_TEST(test_trigger_event_unknown_location);
    RUN_TEST(test_harvest_corpses);
    RUN_TEST(test_harvest_more_than_available);
    RUN_TEST(test_harvest_from_unknown_location);
    RUN_TEST(test_corpse_regeneration);
    RUN_TEST(test_corpse_regeneration_cap);
    RUN_TEST(test_signature_decay);
    RUN_TEST(test_quality_distribution);
    RUN_TEST(test_quality_distribution_invalid_sum);
    RUN_TEST(test_death_quality_to_soul_energy);
    RUN_TEST(test_get_hotspots);
    RUN_TEST(test_get_stats);
    RUN_TEST(test_string_utilities);
    RUN_TEST(test_scan_network);

    /* Print summary */
    printf("\n");
    printf("===========================================\n");
    if (tests_passed == tests_run) {
        printf("%sAll %d tests passed!%s\n", COLOR_GREEN, tests_run, COLOR_RESET);
    } else {
        printf("%s%d/%d tests passed%s\n", COLOR_RED, tests_passed, tests_run, COLOR_RESET);
    }
    printf("===========================================\n\n");

    return (tests_passed == tests_run) ? 0 : 1;
}

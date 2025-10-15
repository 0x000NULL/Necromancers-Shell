/**
 * @file test_encounter.c
 * @brief Unit tests for encounter generation system
 */

#include "../src/game/combat/encounter.h"
#include "../src/game/combat/enemy.h"
#include "../src/game/world/location.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) do { \
    printf("Running test: %s...", name); \
    tests_run++; \
} while(0)

#define PASS() do { \
    printf(" PASS\n"); \
    tests_passed++; \
} while(0)

/* Test: Create encounter from template */
void test_create_from_template(void) {
    TEST("test_create_from_template");

    EncounterTemplate* enc = encounter_create_from_template("lone_priest");
    assert(enc != NULL);
    assert(strcmp(enc->id, "lone_priest") == 0);
    assert(enc->difficulty_rating == 3);
    assert(enc->enemy_type_count > 0);

    encounter_destroy(enc);

    PASS();
}

/* Test: Invalid template */
void test_invalid_template(void) {
    TEST("test_invalid_template");

    EncounterTemplate* enc = encounter_create_from_template("nonexistent");
    assert(enc == NULL);

    PASS();
}

/* Test: Generate encounter by difficulty */
void test_generate_by_difficulty(void) {
    TEST("test_generate_by_difficulty");

    /* Generate easy encounter (difficulty 2) */
    EncounterTemplate* easy = encounter_generate(2, LOCATION_TYPE_VILLAGE);
    assert(easy != NULL);
    assert(easy->difficulty_rating >= 1);
    assert(easy->difficulty_rating <= 4);
    encounter_destroy(easy);

    /* Generate medium encounter (difficulty 5) */
    EncounterTemplate* medium = encounter_generate(5, LOCATION_TYPE_GRAVEYARD);
    assert(medium != NULL);
    assert(medium->difficulty_rating >= 3);
    assert(medium->difficulty_rating <= 7);
    encounter_destroy(medium);

    /* Generate hard encounter (difficulty 9) */
    EncounterTemplate* hard = encounter_generate(9, LOCATION_TYPE_CRYPT);
    assert(hard != NULL);
    assert(hard->difficulty_rating >= 7);
    assert(hard->difficulty_rating <= 10);
    encounter_destroy(hard);

    PASS();
}

/* Test: Get total enemies */
void test_get_total_enemies(void) {
    TEST("test_get_total_enemies");

    /* Lone priest: 1 enemy */
    EncounterTemplate* lone = encounter_create_from_template("lone_priest");
    assert(encounter_get_total_enemies(lone) == 1);
    encounter_destroy(lone);

    /* Guard patrol: 3 enemies */
    EncounterTemplate* patrol = encounter_create_from_template("guard_patrol");
    assert(encounter_get_total_enemies(patrol) == 3);
    encounter_destroy(patrol);

    /* Church battalion: 5 enemies (2+2+1) */
    EncounterTemplate* battalion = encounter_create_from_template("church_battalion");
    assert(encounter_get_total_enemies(battalion) == 5);
    encounter_destroy(battalion);

    PASS();
}

/* Test: Spawn enemies */
void test_spawn_enemies(void) {
    TEST("test_spawn_enemies");

    EncounterTemplate* enc = encounter_create_from_template("church_squad");
    assert(enc != NULL);

    uint8_t count;
    Enemy** enemies = encounter_spawn_enemies(enc, &count);

    assert(enemies != NULL);
    assert(count == 2);  /* 1 Paladin + 1 Priest */

    /* Verify enemies were created */
    for (uint8_t i = 0; i < count; i++) {
        assert(enemies[i] != NULL);
        assert(enemies[i]->health > 0);
        enemy_destroy(enemies[i]);
    }

    free(enemies);
    encounter_destroy(enc);

    PASS();
}

/* Test: Spawn multiple enemy types */
void test_spawn_multiple_types(void) {
    TEST("test_spawn_multiple_types");

    EncounterTemplate* enc = encounter_create_from_template("guard_patrol");
    assert(enc != NULL);

    uint8_t count;
    Enemy** enemies = encounter_spawn_enemies(enc, &count);

    assert(enemies != NULL);
    assert(count == 3);  /* 3 Guards */

    /* All should be guards */
    for (uint8_t i = 0; i < count; i++) {
        assert(enemies[i]->type == ENEMY_TYPE_GUARD);
        enemy_destroy(enemies[i]);
    }

    free(enemies);
    encounter_destroy(enc);

    PASS();
}

/* Test: Calculate threat level */
void test_calculate_threat(void) {
    TEST("test_calculate_threat");

    /* Easy encounter: low threat */
    EncounterTemplate* easy = encounter_create_from_template("patrol_weak");
    uint32_t easy_threat = encounter_calculate_threat(easy);
    assert(easy_threat > 0);

    /* Hard encounter: high threat */
    EncounterTemplate* hard = encounter_create_from_template("church_battalion");
    uint32_t hard_threat = encounter_calculate_threat(hard);
    assert(hard_threat > 0);

    /* Hard should have significantly higher threat */
    assert(hard_threat > easy_threat * 3);

    encounter_destroy(easy);
    encounter_destroy(hard);

    PASS();
}

/* Test: Location-based adjustments */
void test_location_adjustments(void) {
    TEST("test_location_adjustments");

    /* Generate encounters for different location types */
    EncounterTemplate* village = encounter_generate(5, LOCATION_TYPE_VILLAGE);
    assert(village != NULL);

    EncounterTemplate* crypt = encounter_generate(5, LOCATION_TYPE_CRYPT);
    assert(crypt != NULL);

    EncounterTemplate* battlefield = encounter_generate(5, LOCATION_TYPE_BATTLEFIELD);
    assert(battlefield != NULL);

    /* All should be valid */
    assert(village->enemy_type_count > 0);
    assert(crypt->enemy_type_count > 0);
    assert(battlefield->enemy_type_count > 0);

    encounter_destroy(village);
    encounter_destroy(crypt);
    encounter_destroy(battlefield);

    PASS();
}

/* Test: Difficulty range */
void test_difficulty_range(void) {
    TEST("test_difficulty_range");

    /* Generate many encounters at difficulty 5 */
    int total_generated = 0;
    int in_range = 0;

    for (int i = 0; i < 20; i++) {
        EncounterTemplate* enc = encounter_generate(5, LOCATION_TYPE_GRAVEYARD);
        if (enc) {
            total_generated++;
            /* Should be within ±2 of requested difficulty */
            if (enc->difficulty_rating >= 3 && enc->difficulty_rating <= 7) {
                in_range++;
            }
            encounter_destroy(enc);
        }
    }

    assert(total_generated > 0);
    assert(in_range == total_generated);  /* All should be in range */

    PASS();
}

/* Test: All predefined templates */
void test_all_templates(void) {
    TEST("test_all_templates");

    const char* templates[] = {
        "patrol_weak",
        "lone_priest",
        "guard_patrol",
        "church_squad",
        "inquisition_team",
        "rival_necromancer",
        "church_battalion"
    };

    for (size_t i = 0; i < sizeof(templates) / sizeof(templates[0]); i++) {
        EncounterTemplate* enc = encounter_create_from_template(templates[i]);
        assert(enc != NULL);
        assert(strlen(enc->id) > 0);
        assert(strlen(enc->description) > 0);
        assert(enc->enemy_type_count > 0);
        assert(enc->enemy_type_count <= MAX_ENCOUNTER_ENEMIES);
        assert(enc->difficulty_rating >= 1);
        assert(enc->difficulty_rating <= 10);

        /* Verify can spawn enemies */
        uint8_t count;
        Enemy** enemies = encounter_spawn_enemies(enc, &count);
        assert(enemies != NULL);
        assert(count > 0);

        for (uint8_t j = 0; j < count; j++) {
            enemy_destroy(enemies[j]);
        }
        free(enemies);

        encounter_destroy(enc);
    }

    PASS();
}

/* Test: NULL safety */
void test_null_safety(void) {
    TEST("test_null_safety");

    encounter_destroy(NULL);
    assert(encounter_get_total_enemies(NULL) == 0);
    assert(encounter_calculate_threat(NULL) == 0);

    uint8_t count;
    assert(encounter_spawn_enemies(NULL, &count) == NULL);
    assert(encounter_create_from_template(NULL) == NULL);

    PASS();
}

int main(void) {
    printf("=== Encounter System Tests ===\n\n");

    test_create_from_template();
    test_invalid_template();
    test_generate_by_difficulty();
    test_get_total_enemies();
    test_spawn_enemies();
    test_spawn_multiple_types();
    test_calculate_threat();
    test_location_adjustments();
    test_difficulty_range();
    test_all_templates();
    test_null_safety();

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    return (tests_run == tests_passed) ? 0 : 1;
}

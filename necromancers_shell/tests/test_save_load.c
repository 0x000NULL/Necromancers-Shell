/**
 * @file test_save_load.c
 * @brief Tests for save/load system
 */

#include "../src/data/save_load.h"
#include "../src/game/game_state.h"
#include "../src/game/souls/soul.h"
#include "../src/game/souls/soul_manager.h"
#include "../src/game/minions/minion.h"
#include "../src/game/minions/minion_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

/* Test counter */
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(func) \
    printf("TEST: %s\n", #func); \
    if (func()) { \
        tests_passed++; \
        printf("  PASS\n"); \
    } else { \
        tests_failed++; \
        printf("  FAIL\n"); \
    }

/* Helper to create a test GameState with some data */
static GameState* create_test_state(void) {
    GameState* state = calloc(1, sizeof(GameState));
    if (!state) {
        return NULL;
    }

    /* Initialize managers */
    state->souls = soul_manager_create();
    state->minions = minion_manager_create(10);

    /* Add some test data */
    Soul* soul1 = soul_create(SOUL_TYPE_WARRIOR, 75);
    soul1->id = 1;
    soul_manager_add(state->souls, soul1);

    Soul* soul2 = soul_create(SOUL_TYPE_MAGE, 90);
    soul2->id = 2;
    soul_manager_add(state->souls, soul2);

    Minion* minion1 = minion_create(MINION_TYPE_SKELETON, "Bonesy", 0);
    minion1->id = 1;
    minion_manager_add(state->minions, minion1);

    /* Initialize simple structs */
    resources_init(&state->resources);
    state->resources.soul_energy = 500;
    state->resources.mana = 75;
    state->resources.day_count = 42;

    corruption_init(&state->corruption);
    corruption_add(&state->corruption, 25, "Test corruption", 10);

    consciousness_init(&state->consciousness);

    /* Set scalar fields */
    state->current_location_id = 3;
    state->player_level = 5;
    state->player_experience = 1234;
    state->next_soul_id = 3;
    state->next_minion_id = 2;
    state->civilian_kills = 0;
    state->game_completed = false;
    state->ending_achieved = ENDING_NONE;
    state->initialized = true;

    return state;
}

/* Test: Save and load preserves data */
static bool test_save_load_roundtrip(void) {
    const char* test_path = "/tmp/test_save.dat";

    /* Create test state */
    GameState* original = create_test_state();
    if (!original) {
        return false;
    }

    /* Save */
    if (!save_game(original, test_path)) {
        game_state_destroy(original);
        return false;
    }

    /* Load */
    char error[256];
    GameState* loaded = load_game(test_path, error, sizeof(error));
    if (!loaded) {
        printf("  Load error: %s\n", error);
        game_state_destroy(original);
        unlink(test_path);
        return false;
    }

    /* Verify data */
    bool success = true;

    /* Check souls */
    if (soul_manager_count(loaded->souls) != 2) {
        printf("  Soul count mismatch\n");
        success = false;
    }

    Soul* loaded_soul = soul_manager_get(loaded->souls, 1);
    if (!loaded_soul || loaded_soul->type != SOUL_TYPE_WARRIOR || loaded_soul->quality != 75) {
        printf("  Soul data mismatch\n");
        success = false;
    }

    /* Check minions */
    if (minion_manager_count(loaded->minions) != 1) {
        printf("  Minion count mismatch\n");
        success = false;
    }

    Minion* loaded_minion = minion_manager_get(loaded->minions, 1);
    if (!loaded_minion || strcmp(loaded_minion->name, "Bonesy") != 0) {
        printf("  Minion data mismatch\n");
        success = false;
    }

    /* Check resources */
    if (loaded->resources.soul_energy != 500 ||
        loaded->resources.mana != 75 ||
        loaded->resources.day_count != 42) {
        printf("  Resources mismatch\n");
        success = false;
    }

    /* Check corruption */
    if (loaded->corruption.corruption != 25) {
        printf("  Corruption mismatch\n");
        success = false;
    }

    /* Check scalar fields */
    if (loaded->current_location_id != 3 ||
        loaded->player_level != 5 ||
        loaded->player_experience != 1234 ||
        loaded->next_soul_id != 3 ||
        loaded->next_minion_id != 2) {
        printf("  Scalar fields mismatch\n");
        success = false;
    }

    /* Cleanup */
    game_state_destroy(original);
    game_state_destroy(loaded);
    unlink(test_path);

    return success;
}

/* Test: Validate detects corrupted file */
static bool test_validate_corrupted_file(void) {
    const char* test_path = "/tmp/test_corrupted.dat";

    /* Create and save a valid file */
    GameState* state = create_test_state();
    if (!state) {
        return false;
    }

    if (!save_game(state, test_path)) {
        game_state_destroy(state);
        return false;
    }

    game_state_destroy(state);

    /* Corrupt the file */
    FILE* fp = fopen(test_path, "r+b");
    if (!fp) {
        unlink(test_path);
        return false;
    }

    /* Seek to data section and corrupt it */
    fseek(fp, 100, SEEK_SET);
    uint8_t bad_data[10] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    fwrite(bad_data, 1, 10, fp);
    fclose(fp);

    /* Validate should fail */
    bool valid = validate_save_file(test_path);
    unlink(test_path);

    return !valid;  /* Success if validation fails */
}

/* Test: Version compatibility check */
static bool test_version_compatibility(void) {
    bool success = true;

    /* Same version should be compatible */
    if (!is_version_compatible(SAVE_VERSION_MAJOR, SAVE_VERSION_MINOR, SAVE_VERSION_PATCH)) {
        printf("  Same version not compatible\n");
        success = false;
    }

    /* Different major version should be incompatible */
    if (is_version_compatible(SAVE_VERSION_MAJOR + 1, 0, 0)) {
        printf("  Different major version compatible (should not be)\n");
        success = false;
    }

    /* Same major, different minor should be compatible (for now) */
    if (!is_version_compatible(SAVE_VERSION_MAJOR, SAVE_VERSION_MINOR + 1, 0)) {
        printf("  Same major, different minor not compatible\n");
        success = false;
    }

    return success;
}

/* Test: Save file exists check */
static bool test_save_file_exists(void) {
    const char* test_path = "/tmp/test_exists.dat";

    /* File should not exist yet */
    if (save_file_exists(test_path)) {
        return false;
    }

    /* Create state and save */
    GameState* state = create_test_state();
    if (!state) {
        return false;
    }

    if (!save_game(state, test_path)) {
        game_state_destroy(state);
        return false;
    }

    game_state_destroy(state);

    /* File should exist now */
    bool exists = save_file_exists(test_path);
    unlink(test_path);

    return exists;
}

/* Test: Get save file size */
static bool test_get_save_file_size(void) {
    const char* test_path = "/tmp/test_size.dat";

    /* Create and save */
    GameState* state = create_test_state();
    if (!state) {
        return false;
    }

    if (!save_game(state, test_path)) {
        game_state_destroy(state);
        return false;
    }

    game_state_destroy(state);

    /* Get size */
    size_t size = get_save_file_size(test_path);
    unlink(test_path);

    /* Size should be non-zero */
    return (size > 0);
}

/* Test: Backup save file */
static bool test_backup_save_file(void) {
    const char* test_path = "/tmp/test_backup.dat";
    const char* backup_path = "/tmp/test_backup.dat.bak";

    /* Create and save */
    GameState* state = create_test_state();
    if (!state) {
        return false;
    }

    if (!save_game(state, test_path)) {
        game_state_destroy(state);
        return false;
    }

    /* Save again (should create backup) */
    if (!save_game(state, test_path)) {
        game_state_destroy(state);
        unlink(test_path);
        return false;
    }

    game_state_destroy(state);

    /* Check backup exists */
    bool backup_exists = (access(backup_path, F_OK) == 0);

    /* Cleanup */
    unlink(test_path);
    unlink(backup_path);

    return backup_exists;
}

/* Test: Save metadata JSON */
static bool test_save_metadata_json(void) {
    const char* test_path = "/tmp/test_meta.json";

    /* Create state */
    GameState* state = create_test_state();
    if (!state) {
        return false;
    }

    /* Save metadata */
    if (!save_metadata_json(state, test_path)) {
        game_state_destroy(state);
        return false;
    }

    game_state_destroy(state);

    /* Check file exists */
    bool exists = (access(test_path, F_OK) == 0);

    /* Read and check contents */
    if (exists) {
        FILE* fp = fopen(test_path, "r");
        if (fp) {
            char buffer[1024];
            size_t read = fread(buffer, 1, sizeof(buffer) - 1, fp);
            buffer[read] = '\0';
            fclose(fp);

            /* Check for expected fields */
            if (strstr(buffer, "\"version\"") == NULL ||
                strstr(buffer, "\"player_level\": 5") == NULL ||
                strstr(buffer, "\"corruption\": 25") == NULL ||
                strstr(buffer, "\"day_count\": 42") == NULL) {
                exists = false;
            }
        }
    }

    unlink(test_path);
    return exists;
}

/* Test: Load nonexistent file */
static bool test_load_nonexistent(void) {
    char error[256];
    GameState* loaded = load_game("/tmp/nonexistent_save.dat", error, sizeof(error));

    if (loaded) {
        game_state_destroy(loaded);
        return false;  /* Should not have loaded */
    }

    return true;  /* Correct - should fail */
}

/* Test: Empty state save/load */
static bool test_empty_state(void) {
    const char* test_path = "/tmp/test_empty.dat";

    /* Create empty state */
    GameState* state = calloc(1, sizeof(GameState));
    if (!state) {
        return false;
    }

    state->souls = soul_manager_create();
    state->minions = minion_manager_create(1);
    resources_init(&state->resources);
    corruption_init(&state->corruption);
    consciousness_init(&state->consciousness);
    state->initialized = true;

    /* Save */
    if (!save_game(state, test_path)) {
        game_state_destroy(state);
        return false;
    }

    /* Load */
    char error[256];
    GameState* loaded = load_game(test_path, error, sizeof(error));

    bool success = (loaded != NULL);
    if (loaded) {
        /* Verify empty */
        success = success && (soul_manager_count(loaded->souls) == 0);
        success = success && (minion_manager_count(loaded->minions) == 0);
        game_state_destroy(loaded);
    }

    game_state_destroy(state);
    unlink(test_path);

    return success;
}

int main(void) {
    printf("=== Save/Load System Tests ===\n\n");

    TEST(test_save_load_roundtrip);
    TEST(test_validate_corrupted_file);
    TEST(test_version_compatibility);
    TEST(test_save_file_exists);
    TEST(test_get_save_file_size);
    TEST(test_backup_save_file);
    TEST(test_save_metadata_json);
    TEST(test_load_nonexistent);
    TEST(test_empty_state);

    printf("\n=== Test Summary ===\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);

    return (tests_failed == 0) ? 0 : 1;
}

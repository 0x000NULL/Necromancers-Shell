#include "../src/game/minions/minion.h"
#include "../src/game/minions/minion_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Test counter */
static int tests_run = 0;
static int tests_passed = 0;

/* Test helper macros */
#define TEST_START(name) \
    do { \
        printf("Running test: %s\n", name); \
        tests_run++; \
    } while (0)

#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf("  FAIL: %s\n", message); \
            return; \
        } \
    } while (0)

#define TEST_PASS() \
    do { \
        printf("  PASS\n"); \
        tests_passed++; \
    } while (0)

/**
 * Test: minion_create basic functionality
 */
void test_minion_create_basic(void) {
    TEST_START("minion_create_basic");

    Minion* minion = minion_create(MINION_TYPE_ZOMBIE, "Bob", 0);
    TEST_ASSERT(minion != NULL, "minion_create should not return NULL");
    TEST_ASSERT(minion->type == MINION_TYPE_ZOMBIE, "Minion type should be ZOMBIE");
    TEST_ASSERT(strcmp(minion->name, "Bob") == 0, "Minion name should be 'Bob'");
    TEST_ASSERT(minion->bound_soul_id == 0, "Minion should not have soul bound");
    TEST_ASSERT(minion->level == 1, "Minion should start at level 1");
    TEST_ASSERT(minion->experience == 0, "Minion should start with 0 XP");
    TEST_ASSERT(minion->stats.health == minion->stats.health_max, "Minion should start at full health");
    TEST_ASSERT(minion->stats.health_max == 100, "Zombie should have 100 max HP");
    TEST_ASSERT(minion->stats.attack == 15, "Zombie should have 15 attack");

    minion_destroy(minion);
    TEST_PASS();
}

/**
 * Test: minion_create with auto-generated name
 */
void test_minion_create_auto_name(void) {
    TEST_START("minion_create_auto_name");

    Minion* minion = minion_create(MINION_TYPE_SKELETON, NULL, 0);
    TEST_ASSERT(minion != NULL, "minion_create should not return NULL");
    TEST_ASSERT(strlen(minion->name) > 0, "Minion should have auto-generated name");
    TEST_ASSERT(strstr(minion->name, "Skeleton") != NULL, "Name should contain type");

    minion_destroy(minion);
    TEST_PASS();
}

/**
 * Test: minion_create with invalid type
 */
void test_minion_create_invalid_type(void) {
    TEST_START("minion_create_invalid_type");

    Minion* minion = minion_create(MINION_TYPE_COUNT, "Invalid", 0);
    TEST_ASSERT(minion == NULL, "minion_create should return NULL for invalid type");

    TEST_PASS();
}

/**
 * Test: minion_type_name
 */
void test_minion_type_name(void) {
    TEST_START("minion_type_name");

    TEST_ASSERT(strcmp(minion_type_name(MINION_TYPE_ZOMBIE), "Zombie") == 0, "Zombie name");
    TEST_ASSERT(strcmp(minion_type_name(MINION_TYPE_SKELETON), "Skeleton") == 0, "Skeleton name");
    TEST_ASSERT(strcmp(minion_type_name(MINION_TYPE_GHOUL), "Ghoul") == 0, "Ghoul name");
    TEST_ASSERT(strcmp(minion_type_name(MINION_TYPE_WRAITH), "Wraith") == 0, "Wraith name");
    TEST_ASSERT(strcmp(minion_type_name(MINION_TYPE_WIGHT), "Wight") == 0, "Wight name");
    TEST_ASSERT(strcmp(minion_type_name(MINION_TYPE_REVENANT), "Revenant") == 0, "Revenant name");

    TEST_PASS();
}

/**
 * Test: minion_calculate_raise_cost
 */
void test_minion_calculate_raise_cost(void) {
    TEST_START("minion_calculate_raise_cost");

    TEST_ASSERT(minion_calculate_raise_cost(MINION_TYPE_ZOMBIE) == 50, "Zombie cost 50");
    TEST_ASSERT(minion_calculate_raise_cost(MINION_TYPE_SKELETON) == 75, "Skeleton cost 75");
    TEST_ASSERT(minion_calculate_raise_cost(MINION_TYPE_GHOUL) == 100, "Ghoul cost 100");
    TEST_ASSERT(minion_calculate_raise_cost(MINION_TYPE_WRAITH) == 150, "Wraith cost 150");
    TEST_ASSERT(minion_calculate_raise_cost(MINION_TYPE_WIGHT) == 200, "Wight cost 200");
    TEST_ASSERT(minion_calculate_raise_cost(MINION_TYPE_REVENANT) == 300, "Revenant cost 300");

    TEST_PASS();
}

/**
 * Test: minion_bind_soul and minion_unbind_soul
 */
void test_minion_soul_binding(void) {
    TEST_START("minion_soul_binding");

    Minion* minion = minion_create(MINION_TYPE_ZOMBIE, "Test", 0);
    TEST_ASSERT(minion != NULL, "minion_create should succeed");

    /* Bind soul */
    bool result = minion_bind_soul(minion, 123);
    TEST_ASSERT(result == true, "minion_bind_soul should succeed");
    TEST_ASSERT(minion->bound_soul_id == 123, "Soul ID should be 123");

    /* Try to bind again (should fail) */
    result = minion_bind_soul(minion, 456);
    TEST_ASSERT(result == false, "minion_bind_soul should fail when already bound");
    TEST_ASSERT(minion->bound_soul_id == 123, "Soul ID should still be 123");

    /* Unbind soul */
    uint32_t unbound_id = minion_unbind_soul(minion);
    TEST_ASSERT(unbound_id == 123, "Unbound soul ID should be 123");
    TEST_ASSERT(minion->bound_soul_id == 0, "Minion should have no soul bound");

    minion_destroy(minion);
    TEST_PASS();
}

/**
 * Test: minion_take_damage and minion_heal
 */
void test_minion_damage_and_heal(void) {
    TEST_START("minion_damage_and_heal");

    Minion* minion = minion_create(MINION_TYPE_ZOMBIE, "Test", 0);
    TEST_ASSERT(minion != NULL, "minion_create should succeed");
    TEST_ASSERT(minion->stats.health == 100, "Zombie should start with 100 HP");

    /* Take damage */
    bool alive = minion_take_damage(minion, 30);
    TEST_ASSERT(alive == true, "Minion should still be alive");
    TEST_ASSERT(minion->stats.health == 70, "Health should be 70");

    /* Take more damage */
    alive = minion_take_damage(minion, 40);
    TEST_ASSERT(alive == true, "Minion should still be alive");
    TEST_ASSERT(minion->stats.health == 30, "Health should be 30");

    /* Heal */
    minion_heal(minion, 20);
    TEST_ASSERT(minion->stats.health == 50, "Health should be 50");

    /* Heal beyond max */
    minion_heal(minion, 100);
    TEST_ASSERT(minion->stats.health == 100, "Health should be capped at 100");

    /* Take fatal damage */
    alive = minion_take_damage(minion, 150);
    TEST_ASSERT(alive == false, "Minion should be dead");
    TEST_ASSERT(minion->stats.health == 0, "Health should be 0");

    minion_destroy(minion);
    TEST_PASS();
}

/**
 * Test: minion_add_experience
 */
void test_minion_add_experience(void) {
    TEST_START("minion_add_experience");

    Minion* minion = minion_create(MINION_TYPE_ZOMBIE, "Test", 0);
    TEST_ASSERT(minion != NULL, "minion_create should succeed");
    TEST_ASSERT(minion->level == 1, "Should start at level 1");
    TEST_ASSERT(minion->experience == 0, "Should start with 0 XP");

    uint32_t initial_attack = minion->stats.attack;
    uint32_t initial_hp_max = minion->stats.health_max;

    /* Add XP but not enough to level */
    bool leveled = minion_add_experience(minion, 50);
    TEST_ASSERT(leveled == false, "Should not level up");
    TEST_ASSERT(minion->level == 1, "Should still be level 1");
    TEST_ASSERT(minion->experience == 50, "Should have 50 XP");

    /* Add enough to level up (need 100 total for level 2) */
    leveled = minion_add_experience(minion, 50);
    TEST_ASSERT(leveled == true, "Should level up");
    TEST_ASSERT(minion->level == 2, "Should be level 2");
    TEST_ASSERT(minion->experience == 0, "XP should reset to 0");
    TEST_ASSERT(minion->stats.attack > initial_attack, "Attack should increase");
    TEST_ASSERT(minion->stats.health_max > initial_hp_max, "Max HP should increase");
    TEST_ASSERT(minion->stats.health == minion->stats.health_max, "Should be at full HP after level up");

    minion_destroy(minion);
    TEST_PASS();
}

/**
 * Test: minion_manager_create and destroy
 */
void test_minion_manager_create(void) {
    TEST_START("minion_manager_create");

    MinionManager* manager = minion_manager_create(10);
    TEST_ASSERT(manager != NULL, "minion_manager_create should succeed");
    TEST_ASSERT(minion_manager_count(manager) == 0, "Manager should start empty");

    minion_manager_destroy(manager);
    TEST_PASS();
}

/**
 * Test: minion_manager_add and get
 */
void test_minion_manager_add_get(void) {
    TEST_START("minion_manager_add_get");

    MinionManager* manager = minion_manager_create(10);
    TEST_ASSERT(manager != NULL, "Manager creation should succeed");

    Minion* minion1 = minion_create(MINION_TYPE_ZOMBIE, "Minion1", 0);
    minion1->id = 1;
    Minion* minion2 = minion_create(MINION_TYPE_SKELETON, "Minion2", 0);
    minion2->id = 2;

    bool added = minion_manager_add(manager, minion1);
    TEST_ASSERT(added == true, "Adding minion1 should succeed");
    TEST_ASSERT(minion_manager_count(manager) == 1, "Count should be 1");

    added = minion_manager_add(manager, minion2);
    TEST_ASSERT(added == true, "Adding minion2 should succeed");
    TEST_ASSERT(minion_manager_count(manager) == 2, "Count should be 2");

    Minion* retrieved = minion_manager_get(manager, 1);
    TEST_ASSERT(retrieved != NULL, "Should retrieve minion1");
    TEST_ASSERT(retrieved->id == 1, "Retrieved minion should have ID 1");

    retrieved = minion_manager_get(manager, 2);
    TEST_ASSERT(retrieved != NULL, "Should retrieve minion2");
    TEST_ASSERT(retrieved->id == 2, "Retrieved minion should have ID 2");

    retrieved = minion_manager_get(manager, 999);
    TEST_ASSERT(retrieved == NULL, "Should not retrieve non-existent minion");

    minion_manager_destroy(manager);
    TEST_PASS();
}

/**
 * Test: minion_manager_remove
 */
void test_minion_manager_remove(void) {
    TEST_START("minion_manager_remove");

    MinionManager* manager = minion_manager_create(10);

    Minion* minion1 = minion_create(MINION_TYPE_ZOMBIE, "Minion1", 0);
    minion1->id = 1;
    Minion* minion2 = minion_create(MINION_TYPE_SKELETON, "Minion2", 0);
    minion2->id = 2;

    minion_manager_add(manager, minion1);
    minion_manager_add(manager, minion2);
    TEST_ASSERT(minion_manager_count(manager) == 2, "Should have 2 minions");

    Minion* removed = minion_manager_remove(manager, 1);
    TEST_ASSERT(removed != NULL, "Should remove minion1");
    TEST_ASSERT(removed->id == 1, "Removed minion should have ID 1");
    TEST_ASSERT(minion_manager_count(manager) == 1, "Should have 1 minion left");

    /* Caller owns removed minion, must destroy it */
    minion_destroy(removed);

    Minion* retrieved = minion_manager_get(manager, 1);
    TEST_ASSERT(retrieved == NULL, "Minion1 should no longer be in manager");

    retrieved = minion_manager_get(manager, 2);
    TEST_ASSERT(retrieved != NULL, "Minion2 should still be in manager");

    minion_manager_destroy(manager);
    TEST_PASS();
}

/**
 * Test: minion_manager_count_by_type
 */
void test_minion_manager_count_by_type(void) {
    TEST_START("minion_manager_count_by_type");

    MinionManager* manager = minion_manager_create(10);

    for (int i = 0; i < 3; i++) {
        Minion* zombie = minion_create(MINION_TYPE_ZOMBIE, NULL, 0);
        zombie->id = i;
        minion_manager_add(manager, zombie);
    }

    for (int i = 3; i < 5; i++) {
        Minion* skeleton = minion_create(MINION_TYPE_SKELETON, NULL, 0);
        skeleton->id = i;
        minion_manager_add(manager, skeleton);
    }

    TEST_ASSERT(minion_manager_count_by_type(manager, MINION_TYPE_ZOMBIE) == 3, "Should have 3 zombies");
    TEST_ASSERT(minion_manager_count_by_type(manager, MINION_TYPE_SKELETON) == 2, "Should have 2 skeletons");
    TEST_ASSERT(minion_manager_count_by_type(manager, MINION_TYPE_GHOUL) == 0, "Should have 0 ghouls");

    minion_manager_destroy(manager);
    TEST_PASS();
}

/**
 * Test: minion_manager_get_at_location
 */
void test_minion_manager_get_at_location(void) {
    TEST_START("minion_manager_get_at_location");

    MinionManager* manager = minion_manager_create(10);

    for (int i = 0; i < 3; i++) {
        Minion* minion = minion_create(MINION_TYPE_ZOMBIE, NULL, 0);
        minion->id = i;
        minion->location_id = 1;  /* Location 1 */
        minion_manager_add(manager, minion);
    }

    for (int i = 3; i < 5; i++) {
        Minion* minion = minion_create(MINION_TYPE_SKELETON, NULL, 0);
        minion->id = i;
        minion->location_id = 2;  /* Location 2 */
        minion_manager_add(manager, minion);
    }

    size_t count = 0;
    Minion** at_loc_1 = minion_manager_get_at_location(manager, 1, &count);
    TEST_ASSERT(at_loc_1 != NULL, "Should get minions at location 1");
    TEST_ASSERT(count == 3, "Should have 3 minions at location 1");
    free(at_loc_1);

    Minion** at_loc_2 = minion_manager_get_at_location(manager, 2, &count);
    TEST_ASSERT(at_loc_2 != NULL, "Should get minions at location 2");
    TEST_ASSERT(count == 2, "Should have 2 minions at location 2");
    free(at_loc_2);

    Minion** at_loc_3 = minion_manager_get_at_location(manager, 3, &count);
    TEST_ASSERT(at_loc_3 == NULL, "Should get NULL for location 3");
    TEST_ASSERT(count == 0, "Should have 0 minions at location 3");

    minion_manager_destroy(manager);
    TEST_PASS();
}

/**
 * Main test runner
 */
int main(void) {
    printf("\n=== Running Minion Tests ===\n\n");

    test_minion_create_basic();
    test_minion_create_auto_name();
    test_minion_create_invalid_type();
    test_minion_type_name();
    test_minion_calculate_raise_cost();
    test_minion_soul_binding();
    test_minion_damage_and_heal();
    test_minion_add_experience();
    test_minion_manager_create();
    test_minion_manager_add_get();
    test_minion_manager_remove();
    test_minion_manager_count_by_type();
    test_minion_manager_get_at_location();

    printf("\n=== Test Summary ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    return (tests_run == tests_passed) ? 0 : 1;
}

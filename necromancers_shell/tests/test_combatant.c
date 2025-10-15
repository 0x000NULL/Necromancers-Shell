/**
 * @file test_combatant.c
 * @brief Unit tests for combatant wrapper system
 */

#include "../src/game/combat/combatant.h"
#include "../src/game/minions/minion.h"
#include "../src/game/combat/enemy.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Test counter */
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

/* Test: Create combatant from minion */
void test_create_from_minion(void) {
    TEST("test_create_from_minion");

    /* Create a minion */
    Minion* minion = minion_create(MINION_TYPE_ZOMBIE, "TestZombie", 0);
    assert(minion != NULL);

    /* Create combatant from minion */
    Combatant* combatant = combatant_create_from_minion(minion, true);
    assert(combatant != NULL);

    /* Verify stats were copied */
    assert(combatant->type == COMBATANT_TYPE_MINION);
    assert(combatant->is_player_controlled == true);
    assert(combatant->health == minion->stats.health);
    assert(combatant->health_max == minion->stats.health_max);
    assert(combatant->attack == minion->stats.attack);
    assert(combatant->defense == minion->stats.defense);
    assert(combatant->speed == minion->stats.speed);
    assert(strcmp(combatant->name, minion->name) == 0);
    assert(combatant->entity == minion);

    combatant_destroy(combatant);
    minion_destroy(minion);

    PASS();
}

/* Test: Create combatant from enemy */
void test_create_from_enemy(void) {
    TEST("test_create_from_enemy");

    /* Create an enemy */
    Enemy* enemy = enemy_create(ENEMY_TYPE_GUARD, 1);
    assert(enemy != NULL);

    /* Create combatant from enemy */
    Combatant* combatant = combatant_create_from_enemy(enemy, NULL);
    assert(combatant != NULL);

    /* Verify stats were copied */
    assert(combatant->type == COMBATANT_TYPE_ENEMY);
    assert(combatant->is_player_controlled == false);
    assert(combatant->health == enemy->health);
    assert(combatant->health_max == enemy->health_max);
    assert(combatant->attack == enemy->attack);
    assert(combatant->defense == enemy->defense);
    assert(combatant->speed == enemy->speed);
    assert(strcmp(combatant->name, enemy->name) == 0);
    assert(combatant->entity == enemy);

    combatant_destroy(combatant);
    enemy_destroy(enemy);

    PASS();
}

/* Test: Take damage */
void test_take_damage(void) {
    TEST("test_take_damage");

    Minion* minion = minion_create(MINION_TYPE_SKELETON, "Bones", 0);
    Combatant* combatant = combatant_create_from_minion(minion, true);

    uint32_t original_hp = combatant->health;

    /* Apply damage */
    bool alive = combatant_take_damage(combatant, 20);
    assert(alive == true);
    assert(combatant->health == original_hp - 20);

    /* Apply lethal damage */
    alive = combatant_take_damage(combatant, 1000);
    assert(alive == false);
    assert(combatant->health == 0);

    combatant_destroy(combatant);
    minion_destroy(minion);

    PASS();
}

/* Test: Healing */
void test_heal(void) {
    TEST("test_heal");

    Minion* minion = minion_create(MINION_TYPE_ZOMBIE, "Groaner", 0);
    Combatant* combatant = combatant_create_from_minion(minion, true);

    /* Damage the combatant */
    combatant_take_damage(combatant, 50);
    uint32_t damaged_hp = combatant->health;

    /* Heal */
    combatant_heal(combatant, 30);
    assert(combatant->health == damaged_hp + 30);

    /* Overheal (should cap at max) */
    combatant_heal(combatant, 1000);
    assert(combatant->health == combatant->health_max);

    combatant_destroy(combatant);
    minion_destroy(minion);

    PASS();
}

/* Test: Is alive */
void test_is_alive(void) {
    TEST("test_is_alive");

    Minion* minion = minion_create(MINION_TYPE_WRAITH, "Phantom", 0);
    Combatant* combatant = combatant_create_from_minion(minion, true);

    assert(combatant_is_alive(combatant) == true);

    /* Kill the combatant */
    combatant_take_damage(combatant, 1000);
    assert(combatant_is_alive(combatant) == false);

    combatant_destroy(combatant);
    minion_destroy(minion);

    PASS();
}

/* Test: Defending stance */
void test_defending(void) {
    TEST("test_defending");

    Minion* minion = minion_create(MINION_TYPE_ZOMBIE, "Tank", 0);
    Combatant* combatant = combatant_create_from_minion(minion, true);

    uint32_t normal_defense = combatant_get_effective_defense(combatant);

    /* Set defending */
    combatant_set_defending(combatant, true);
    uint32_t defending_defense = combatant_get_effective_defense(combatant);

    /* Should be +50% */
    assert(defending_defense == (normal_defense * 150) / 100);

    /* Clear defending */
    combatant_set_defending(combatant, false);
    assert(combatant_get_effective_defense(combatant) == normal_defense);

    combatant_destroy(combatant);
    minion_destroy(minion);

    PASS();
}

/* Test: Roll initiative */
void test_roll_initiative(void) {
    TEST("test_roll_initiative");

    Minion* minion = minion_create(MINION_TYPE_SKELETON, "Speedy", 0);
    Combatant* combatant = combatant_create_from_minion(minion, true);

    /* Roll initiative multiple times */
    combatant_roll_initiative(combatant);
    uint8_t init1 = combatant->initiative;
    assert(init1 > 0);

    combatant_roll_initiative(combatant);
    uint8_t init2 = combatant->initiative;
    assert(init2 > 0);

    /* Should be in reasonable range (speed + random(0-50)) */
    assert(init1 >= combatant->speed);
    assert(init2 >= combatant->speed);

    combatant_destroy(combatant);
    minion_destroy(minion);

    PASS();
}

/* Test: Reset turn flags */
void test_reset_turn_flags(void) {
    TEST("test_reset_turn_flags");

    Minion* minion = minion_create(MINION_TYPE_ZOMBIE, "Test", 0);
    Combatant* combatant = combatant_create_from_minion(minion, true);

    /* Set flags */
    combatant->has_acted_this_turn = true;
    combatant_set_defending(combatant, true);

    /* Reset */
    combatant_reset_turn_flags(combatant);

    assert(combatant->has_acted_this_turn == false);
    assert(combatant->is_defending == false);

    combatant_destroy(combatant);
    minion_destroy(minion);

    PASS();
}

/* Test: Sync to entity (minion) */
void test_sync_to_minion(void) {
    TEST("test_sync_to_minion");

    Minion* minion = minion_create(MINION_TYPE_GHOUL, "Test", 0);
    Combatant* combatant = combatant_create_from_minion(minion, true);

    /* Modify combatant stats */
    combatant->health = 50;
    combatant->attack = 100;

    /* Sync back to minion */
    combatant_sync_to_entity(combatant);

    assert(minion->stats.health == 50);
    assert(minion->stats.attack == 100);

    combatant_destroy(combatant);
    minion_destroy(minion);

    PASS();
}

/* Test: Sync to entity (enemy) */
void test_sync_to_enemy(void) {
    TEST("test_sync_to_enemy");

    Enemy* enemy = enemy_create(ENEMY_TYPE_PALADIN, 1);
    Combatant* combatant = combatant_create_from_enemy(enemy, NULL);

    /* Modify combatant stats */
    combatant->health = 75;
    combatant->defense = 60;

    /* Sync back to enemy */
    combatant_sync_to_entity(combatant);

    assert(enemy->health == 75);
    assert(enemy->defense == 60);

    combatant_destroy(combatant);
    enemy_destroy(enemy);

    PASS();
}

/* Test: NULL safety */
void test_null_safety(void) {
    TEST("test_null_safety");

    /* All functions should handle NULL gracefully */
    combatant_destroy(NULL);
    assert(combatant_take_damage(NULL, 10) == false);
    combatant_heal(NULL, 10);
    assert(combatant_is_alive(NULL) == false);
    combatant_set_defending(NULL, true);
    assert(combatant_get_effective_defense(NULL) == 0);
    combatant_reset_turn_flags(NULL);
    combatant_roll_initiative(NULL);
    combatant_sync_to_entity(NULL);

    PASS();
}

int main(void) {
    printf("=== Combatant Tests ===\n\n");

    test_create_from_minion();
    test_create_from_enemy();
    test_take_damage();
    test_heal();
    test_is_alive();
    test_defending();
    test_roll_initiative();
    test_reset_turn_flags();
    test_sync_to_minion();
    test_sync_to_enemy();
    test_null_safety();

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    return (tests_run == tests_passed) ? 0 : 1;
}

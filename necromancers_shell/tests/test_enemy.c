/**
 * @file test_enemy.c
 * @brief Unit tests for enemy system
 */

#include "../src/game/combat/enemy.h"
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

/* Test: Create and destroy enemy */
void test_enemy_create_destroy(void) {
    TEST("test_enemy_create_destroy");

    Enemy* enemy = enemy_create(ENEMY_TYPE_GUARD, 1);
    assert(enemy != NULL);
    assert(enemy->id == 1);
    assert(enemy->type == ENEMY_TYPE_GUARD);
    assert(enemy->health > 0);
    assert(enemy->health == enemy->health_max);

    enemy_destroy(enemy);

    PASS();
}

/* Test: Enemy types */
void test_enemy_types(void) {
    TEST("test_enemy_types");

    /* Test each enemy type */
    EnemyType types[] = {
        ENEMY_TYPE_PALADIN,
        ENEMY_TYPE_PRIEST,
        ENEMY_TYPE_INQUISITOR,
        ENEMY_TYPE_VILLAGER,
        ENEMY_TYPE_GUARD,
        ENEMY_TYPE_RIVAL_NECROMANCER
    };

    for (size_t i = 0; i < sizeof(types) / sizeof(types[0]); i++) {
        Enemy* enemy = enemy_create(types[i], i + 1);
        assert(enemy != NULL);
        assert(enemy->type == types[i]);
        assert(enemy->health > 0);
        assert(enemy->attack > 0);
        assert(enemy->speed > 0);

        /* Verify type name */
        const char* name = enemy_type_name(types[i]);
        assert(name != NULL);
        assert(strlen(name) > 0);

        enemy_destroy(enemy);
    }

    PASS();
}

/* Test: Enemy base stats */
void test_enemy_base_stats(void) {
    TEST("test_enemy_base_stats");

    /* Paladin: High defense, moderate attack */
    uint32_t hp, atk, def, spd;
    enemy_get_base_stats(ENEMY_TYPE_PALADIN, &hp, &atk, &def, &spd);
    assert(hp == 120);
    assert(atk == 35);
    assert(def == 40);
    assert(spd == 8);

    /* Inquisitor: High attack, low defense */
    enemy_get_base_stats(ENEMY_TYPE_INQUISITOR, &hp, &atk, &def, &spd);
    assert(hp == 80);
    assert(atk == 50);
    assert(def == 20);
    assert(spd == 12);

    /* Villager: Low everything */
    enemy_get_base_stats(ENEMY_TYPE_VILLAGER, &hp, &atk, &def, &spd);
    assert(hp == 30);
    assert(atk == 5);
    assert(def == 5);
    assert(spd == 6);

    PASS();
}

/* Test: Enemy default AI */
void test_enemy_default_ai(void) {
    TEST("test_enemy_default_ai");

    /* Paladins are defensive */
    assert(enemy_get_default_ai(ENEMY_TYPE_PALADIN) == AI_BEHAVIOR_DEFENSIVE);

    /* Priests are supportive */
    assert(enemy_get_default_ai(ENEMY_TYPE_PRIEST) == AI_BEHAVIOR_SUPPORT);

    /* Inquisitors are aggressive */
    assert(enemy_get_default_ai(ENEMY_TYPE_INQUISITOR) == AI_BEHAVIOR_AGGRESSIVE);

    /* Guards are balanced */
    assert(enemy_get_default_ai(ENEMY_TYPE_GUARD) == AI_BEHAVIOR_BALANCED);

    /* Rival necromancers are tactical */
    assert(enemy_get_default_ai(ENEMY_TYPE_RIVAL_NECROMANCER) == AI_BEHAVIOR_TACTICAL);

    PASS();
}

/* Test: Enemy rewards */
void test_enemy_rewards(void) {
    TEST("test_enemy_rewards");

    /* High-level enemies give more rewards */
    Enemy* inquisitor = enemy_create(ENEMY_TYPE_INQUISITOR, 1);
    assert(enemy_get_experience_reward(inquisitor) == 60);
    assert(enemy_get_soul_energy_reward(inquisitor) == 50);

    /* Low-level enemies give less */
    Enemy* villager = enemy_create(ENEMY_TYPE_VILLAGER, 2);
    assert(enemy_get_experience_reward(villager) == 10);
    assert(enemy_get_soul_energy_reward(villager) == 5);

    enemy_destroy(inquisitor);
    enemy_destroy(villager);

    PASS();
}

/* Test: Enemy take damage */
void test_enemy_take_damage(void) {
    TEST("test_enemy_take_damage");

    Enemy* enemy = enemy_create(ENEMY_TYPE_GUARD, 1);
    uint32_t original_hp = enemy->health;

    /* Apply damage */
    bool alive = enemy_take_damage(enemy, 20);
    assert(alive == true);
    assert(enemy->health == original_hp - 20);

    /* Apply lethal damage */
    alive = enemy_take_damage(enemy, 1000);
    assert(alive == false);
    assert(enemy->health == 0);

    enemy_destroy(enemy);

    PASS();
}

/* Test: Enemy healing */
void test_enemy_heal(void) {
    TEST("test_enemy_heal");

    Enemy* enemy = enemy_create(ENEMY_TYPE_PRIEST, 1);

    /* Damage the enemy */
    enemy_take_damage(enemy, 30);
    uint32_t damaged_hp = enemy->health;

    /* Heal */
    enemy_heal(enemy, 20);
    assert(enemy->health == damaged_hp + 20);

    /* Overheal (should cap at max) */
    enemy_heal(enemy, 1000);
    assert(enemy->health == enemy->health_max);

    enemy_destroy(enemy);

    PASS();
}

/* Test: Enemy is alive */
void test_enemy_is_alive(void) {
    TEST("test_enemy_is_alive");

    Enemy* enemy = enemy_create(ENEMY_TYPE_GUARD, 1);

    assert(enemy_is_alive(enemy) == true);

    /* Kill the enemy */
    enemy_take_damage(enemy, 1000);
    assert(enemy_is_alive(enemy) == false);

    enemy_destroy(enemy);

    PASS();
}

/* Test: AI behavior names */
void test_ai_behavior_names(void) {
    TEST("test_ai_behavior_names");

    assert(strcmp(ai_behavior_name(AI_BEHAVIOR_AGGRESSIVE), "Aggressive") == 0);
    assert(strcmp(ai_behavior_name(AI_BEHAVIOR_DEFENSIVE), "Defensive") == 0);
    assert(strcmp(ai_behavior_name(AI_BEHAVIOR_BALANCED), "Balanced") == 0);
    assert(strcmp(ai_behavior_name(AI_BEHAVIOR_SUPPORT), "Support") == 0);
    assert(strcmp(ai_behavior_name(AI_BEHAVIOR_TACTICAL), "Tactical") == 0);

    PASS();
}

/* Test: Enemy type name */
void test_enemy_type_name(void) {
    TEST("test_enemy_type_name");

    assert(strcmp(enemy_type_name(ENEMY_TYPE_PALADIN), "Paladin") == 0);
    assert(strcmp(enemy_type_name(ENEMY_TYPE_PRIEST), "Priest") == 0);
    assert(strcmp(enemy_type_name(ENEMY_TYPE_INQUISITOR), "Inquisitor") == 0);
    assert(strcmp(enemy_type_name(ENEMY_TYPE_VILLAGER), "Villager") == 0);
    assert(strcmp(enemy_type_name(ENEMY_TYPE_GUARD), "Guard") == 0);
    assert(strcmp(enemy_type_name(ENEMY_TYPE_RIVAL_NECROMANCER), "Rival Necromancer") == 0);

    /* Invalid type */
    assert(strcmp(enemy_type_name(ENEMY_TYPE_COUNT), "Unknown") == 0);

    PASS();
}

/* Test: NULL safety */
void test_null_safety(void) {
    TEST("test_null_safety");

    enemy_destroy(NULL);
    assert(enemy_get_experience_reward(NULL) == 0);
    assert(enemy_get_soul_energy_reward(NULL) == 0);
    assert(enemy_take_damage(NULL, 10) == false);
    enemy_heal(NULL, 10);
    assert(enemy_is_alive(NULL) == false);

    PASS();
}

int main(void) {
    printf("=== Enemy System Tests ===\n\n");

    test_enemy_create_destroy();
    test_enemy_types();
    test_enemy_base_stats();
    test_enemy_default_ai();
    test_enemy_rewards();
    test_enemy_take_damage();
    test_enemy_heal();
    test_enemy_is_alive();
    test_ai_behavior_names();
    test_enemy_type_name();
    test_null_safety();

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    return (tests_run == tests_passed) ? 0 : 1;
}

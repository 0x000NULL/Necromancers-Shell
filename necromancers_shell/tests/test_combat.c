/**
 * @file test_combat.c
 * @brief Unit tests for combat state machine
 */

#include "../src/game/combat/combat.h"
#include "../src/game/combat/combatant.h"
#include "../src/game/minions/minion.h"
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

/* Test: Create and destroy combat state */
void test_combat_create_destroy(void) {
    TEST("test_combat_create_destroy");

    CombatState* combat = combat_state_create();
    assert(combat != NULL);
    assert(combat->phase == COMBAT_PHASE_INIT);
    assert(combat->turn_number == 0);
    assert(combat->player_force_count == 0);
    assert(combat->enemy_force_count == 0);

    combat_state_destroy(combat);

    PASS();
}

/* Test: Add combatants */
void test_add_combatants(void) {
    TEST("test_add_combatants");

    CombatState* combat = combat_state_create();

    /* Add player forces */
    Minion* minion1 = minion_create(MINION_TYPE_ZOMBIE, "Z1", 0);
    Combatant* c1 = combatant_create_from_minion(minion1, true);
    assert(combat_add_player_combatant(combat, c1) == true);
    assert(combat->player_force_count == 1);

    Minion* minion2 = minion_create(MINION_TYPE_SKELETON, "S1", 0);
    Combatant* c2 = combatant_create_from_minion(minion2, true);
    assert(combat_add_player_combatant(combat, c2) == true);
    assert(combat->player_force_count == 2);

    /* Add enemy forces */
    Enemy* enemy1 = enemy_create(ENEMY_TYPE_GUARD, 1);
    Combatant* e1 = combatant_create_from_enemy(enemy1, NULL);
    assert(combat_add_enemy_combatant(combat, e1) == true);
    assert(combat->enemy_force_count == 1);

    combat_state_destroy(combat);
    minion_destroy(minion1);
    minion_destroy(minion2);
    enemy_destroy(enemy1);

    PASS();
}

/* Test: Initialize combat */
void test_combat_initialize(void) {
    TEST("test_combat_initialize");

    CombatState* combat = combat_state_create();

    /* Add combatants */
    Minion* minion = minion_create(MINION_TYPE_ZOMBIE, "Z1", 0);
    Combatant* player = combatant_create_from_minion(minion, true);
    combat_add_player_combatant(combat, player);

    Enemy* enemy = enemy_create(ENEMY_TYPE_GUARD, 1);
    Combatant* e = combatant_create_from_enemy(enemy, NULL);
    combat_add_enemy_combatant(combat, e);

    /* Initialize */
    assert(combat_initialize(combat) == true);

    /* Check state */
    assert(combat->phase == COMBAT_PHASE_PLAYER_TURN);
    assert(combat->turn_number == 1);
    assert(combat->turn_order_count == 2);
    assert(combat->current_turn_index == 0);

    /* Check initiative was rolled */
    assert(player->initiative > 0);
    assert(e->initiative > 0);

    combat_state_destroy(combat);
    minion_destroy(minion);
    enemy_destroy(enemy);

    PASS();
}

/* Test: Turn order calculation */
void test_turn_order(void) {
    TEST("test_turn_order");

    CombatState* combat = combat_state_create();

    /* Create combatants with known speeds */
    Minion* fast = minion_create(MINION_TYPE_WRAITH, "Fast", 0);  /* Speed 20 */
    Minion* slow = minion_create(MINION_TYPE_ZOMBIE, "Slow", 0);  /* Speed 5 */

    Combatant* c_fast = combatant_create_from_minion(fast, true);
    Combatant* c_slow = combatant_create_from_minion(slow, true);

    /* Manually set initiative to ensure order */
    c_fast->initiative = 200;
    c_slow->initiative = 50;

    combat_add_player_combatant(combat, c_fast);
    combat_add_player_combatant(combat, c_slow);

    /* Calculate turn order */
    combat_calculate_turn_order(combat);

    /* Fast should be first */
    assert(combat->turn_order[0] == c_fast);
    assert(combat->turn_order[1] == c_slow);

    combat_state_destroy(combat);
    minion_destroy(fast);
    minion_destroy(slow);

    PASS();
}

/* Test: Check victory */
void test_check_victory(void) {
    TEST("test_check_victory");

    CombatState* combat = combat_state_create();

    /* Add player force */
    Minion* minion = minion_create(MINION_TYPE_ZOMBIE, "Z1", 0);
    Combatant* player = combatant_create_from_minion(minion, true);
    combat_add_player_combatant(combat, player);

    /* Add dead enemy */
    Enemy* enemy = enemy_create(ENEMY_TYPE_GUARD, 1);
    Combatant* e = combatant_create_from_enemy(enemy, NULL);
    combatant_take_damage(e, 1000);  /* Kill it */
    combat_add_enemy_combatant(combat, e);

    /* Should be victory */
    assert(combat_check_victory(combat) == true);
    assert(combat_check_defeat(combat) == false);

    combat_state_destroy(combat);
    minion_destroy(minion);
    enemy_destroy(enemy);

    PASS();
}

/* Test: Check defeat */
void test_check_defeat(void) {
    TEST("test_check_defeat");

    CombatState* combat = combat_state_create();

    /* Add dead player force */
    Minion* minion = minion_create(MINION_TYPE_ZOMBIE, "Z1", 0);
    Combatant* player = combatant_create_from_minion(minion, true);
    combatant_take_damage(player, 1000);  /* Kill it */
    combat_add_player_combatant(combat, player);

    /* Add alive enemy */
    Enemy* enemy = enemy_create(ENEMY_TYPE_GUARD, 1);
    Combatant* e = combatant_create_from_enemy(enemy, NULL);
    combat_add_enemy_combatant(combat, e);

    /* Should be defeat */
    assert(combat_check_victory(combat) == false);
    assert(combat_check_defeat(combat) == true);

    combat_state_destroy(combat);
    minion_destroy(minion);
    enemy_destroy(enemy);

    PASS();
}

/* Test: Combat log */
void test_combat_log(void) {
    TEST("test_combat_log");

    CombatState* combat = combat_state_create();

    /* Add messages */
    combat_log_message(combat, "Message 1");
    combat_log_message(combat, "Message 2");
    combat_log_message(combat, "Message 3");

    assert(combat->log_count == 3);

    /* Retrieve messages */
    const char* messages[3];
    size_t count = combat_get_log_messages(combat, 3, messages);

    assert(count == 3);
    /* Messages should be in reverse order (most recent first) */
    assert(strcmp(messages[0], "Message 3") == 0);
    assert(strcmp(messages[1], "Message 2") == 0);
    assert(strcmp(messages[2], "Message 1") == 0);

    combat_state_destroy(combat);

    PASS();
}

/* Test: Find combatant by ID */
void test_find_combatant(void) {
    TEST("test_find_combatant");

    CombatState* combat = combat_state_create();

    Minion* minion = minion_create(MINION_TYPE_ZOMBIE, "TestZombie", 0);
    minion->id = 42;
    Combatant* player = combatant_create_from_minion(minion, true);
    combat_add_player_combatant(combat, player);

    Enemy* enemy = enemy_create(ENEMY_TYPE_GUARD, 99);
    Combatant* e = combatant_create_from_enemy(enemy, NULL);
    combat_add_enemy_combatant(combat, e);

    /* Find by ID */
    Combatant* found_player = combat_find_combatant(combat, "M42");
    assert(found_player == player);

    Combatant* found_enemy = combat_find_combatant(combat, "E99");
    assert(found_enemy == e);

    Combatant* not_found = combat_find_combatant(combat, "M999");
    assert(not_found == NULL);

    combat_state_destroy(combat);
    minion_destroy(minion);
    enemy_destroy(enemy);

    PASS();
}

/* Test: Count living forces */
void test_count_living(void) {
    TEST("test_count_living");

    CombatState* combat = combat_state_create();

    /* Add 2 player forces */
    Minion* m1 = minion_create(MINION_TYPE_ZOMBIE, "Z1", 0);
    Minion* m2 = minion_create(MINION_TYPE_SKELETON, "S1", 0);
    Combatant* p1 = combatant_create_from_minion(m1, true);
    Combatant* p2 = combatant_create_from_minion(m2, true);
    combat_add_player_combatant(combat, p1);
    combat_add_player_combatant(combat, p2);

    /* Add 3 enemies */
    Enemy* e1 = enemy_create(ENEMY_TYPE_GUARD, 1);
    Enemy* e2 = enemy_create(ENEMY_TYPE_GUARD, 2);
    Enemy* e3 = enemy_create(ENEMY_TYPE_GUARD, 3);
    Combatant* en1 = combatant_create_from_enemy(e1, NULL);
    Combatant* en2 = combatant_create_from_enemy(e2, NULL);
    Combatant* en3 = combatant_create_from_enemy(e3, NULL);
    combat_add_enemy_combatant(combat, en1);
    combat_add_enemy_combatant(combat, en2);
    combat_add_enemy_combatant(combat, en3);

    /* All alive */
    assert(combat_count_living_player_forces(combat) == 2);
    assert(combat_count_living_enemy_forces(combat) == 3);

    /* Kill one player */
    combatant_take_damage(p1, 1000);
    assert(combat_count_living_player_forces(combat) == 1);

    /* Kill two enemies */
    combatant_take_damage(en1, 1000);
    combatant_take_damage(en2, 1000);
    assert(combat_count_living_enemy_forces(combat) == 1);

    combat_state_destroy(combat);
    minion_destroy(m1);
    minion_destroy(m2);
    enemy_destroy(e1);
    enemy_destroy(e2);
    enemy_destroy(e3);

    PASS();
}

/* Test: Advance turn */
void test_advance_turn(void) {
    TEST("test_advance_turn");

    CombatState* combat = combat_state_create();

    /* Add combatants */
    Minion* minion = minion_create(MINION_TYPE_ZOMBIE, "Z1", 0);
    Combatant* player = combatant_create_from_minion(minion, true);
    combat_add_player_combatant(combat, player);

    Enemy* enemy = enemy_create(ENEMY_TYPE_GUARD, 1);
    Combatant* e = combatant_create_from_enemy(enemy, NULL);
    combat_add_enemy_combatant(combat, e);

    combat_initialize(combat);

    /* Advance turn */
    uint8_t initial_index = combat->current_turn_index;
    combat_advance_turn(combat);

    /* Should move to next combatant */
    assert(combat->current_turn_index == initial_index + 1);

    combat_state_destroy(combat);
    minion_destroy(minion);
    enemy_destroy(enemy);

    PASS();
}

/* Test: Combat end */
void test_combat_end(void) {
    TEST("test_combat_end");

    CombatState* combat = combat_state_create();

    combat_end(combat, COMBAT_OUTCOME_VICTORY);

    assert(combat->phase == COMBAT_PHASE_END);
    assert(combat->outcome == COMBAT_OUTCOME_VICTORY);

    combat_state_destroy(combat);

    PASS();
}

/* Test: NULL safety */
void test_null_safety(void) {
    TEST("test_null_safety");

    combat_state_destroy(NULL);
    assert(combat_add_player_combatant(NULL, NULL) == false);
    assert(combat_add_enemy_combatant(NULL, NULL) == false);
    assert(combat_initialize(NULL) == false);
    combat_calculate_turn_order(NULL);
    combat_update(NULL, 0.0);
    combat_advance_turn(NULL);
    assert(combat_check_victory(NULL) == false);
    assert(combat_check_defeat(NULL) == false);
    combat_log_message(NULL, "test");
    assert(combat_get_log_messages(NULL, 1, NULL) == 0);
    assert(combat_find_combatant(NULL, "M1") == NULL);
    assert(combat_get_active_combatant(NULL) == NULL);
    assert(combat_count_living_player_forces(NULL) == 0);
    assert(combat_count_living_enemy_forces(NULL) == 0);
    combat_process_ai_turn(NULL);
    combat_end(NULL, COMBAT_OUTCOME_VICTORY);

    PASS();
}

int main(void) {
    printf("=== Combat State Tests ===\n\n");

    test_combat_create_destroy();
    test_add_combatants();
    test_combat_initialize();
    test_turn_order();
    test_check_victory();
    test_check_defeat();
    test_combat_log();
    test_find_combatant();
    test_count_living();
    test_advance_turn();
    test_combat_end();
    test_null_safety();

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    return (tests_run == tests_passed) ? 0 : 1;
}

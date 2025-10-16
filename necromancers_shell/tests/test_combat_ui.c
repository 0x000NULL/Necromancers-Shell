/**
 * @file test_combat_ui.c
 * @brief Safety tests for combat UI rendering (buffer overflow protection)
 */

#include "../src/game/combat/combat_ui.h"
#include "../src/game/combat/combatant.h"
#include "../src/game/minions/minion.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Test counter */
static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) do { \
    printf("Running %s...\n", name); \
    tests_run++; \
} while(0)

#define PASS() do { \
    printf("  PASSED\n"); \
    tests_passed++; \
} while(0)

/**
 * @brief Test health bar formatting with normal values
 */
void test_health_bar_normal(void) {
    TEST("test_health_bar_normal");

    char buf[256];

    /* Test full health */
    combat_ui_format_health_bar(buf, sizeof(buf), 100, 100, 20);
    assert(strlen(buf) < sizeof(buf));

    /* Test half health */
    combat_ui_format_health_bar(buf, sizeof(buf), 50, 100, 20);
    assert(strlen(buf) < sizeof(buf));

    /* Test low health */
    combat_ui_format_health_bar(buf, sizeof(buf), 10, 100, 20);
    assert(strlen(buf) < sizeof(buf));

    /* Test zero health */
    combat_ui_format_health_bar(buf, sizeof(buf), 0, 100, 20);
    assert(strlen(buf) < sizeof(buf));

    PASS();
}

/**
 * @brief Test health bar with edge case widths
 */
void test_health_bar_edge_widths(void) {
    TEST("test_health_bar_edge_widths");

    char buf[256];

    /* Test minimum width */
    combat_ui_format_health_bar(buf, sizeof(buf), 50, 100, 1);
    assert(strlen(buf) < sizeof(buf));

    /* Test very wide bar (should be clamped) */
    combat_ui_format_health_bar(buf, sizeof(buf), 50, 100, 100);
    assert(strlen(buf) < sizeof(buf));

    /* Test negative width (edge case) */
    combat_ui_format_health_bar(buf, sizeof(buf), 50, 100, -1);
    assert(strlen(buf) < sizeof(buf));

    PASS();
}

/**
 * @brief Test health bar with small buffer (should not overflow)
 */
void test_health_bar_small_buffer(void) {
    TEST("test_health_bar_small_buffer");

    char buf[32];  /* Intentionally small */

    /* This should not overflow the buffer */
    combat_ui_format_health_bar(buf, sizeof(buf), 999999, 999999, 20);
    assert(strlen(buf) < sizeof(buf));

    /* Verify null termination */
    assert(buf[sizeof(buf) - 1] == '\0' || buf[31] == '\0');

    PASS();
}

/**
 * @brief Test health bar with maximum values
 */
void test_health_bar_max_values(void) {
    TEST("test_health_bar_max_values");

    char buf[256];

    /* Test max uint32_t values */
    combat_ui_format_health_bar(buf, sizeof(buf), 4294967295U, 4294967295U, 20);
    assert(strlen(buf) < sizeof(buf));

    /* Test large values */
    combat_ui_format_health_bar(buf, sizeof(buf), 1000000, 2000000, 40);
    assert(strlen(buf) < sizeof(buf));

    PASS();
}

/**
 * @brief Test combatant formatting with normal names
 */
void test_combatant_format_normal(void) {
    TEST("test_combatant_format_normal");

    char buf[256];

    /* Create a minion with normal name */
    Minion* minion = minion_create(MINION_TYPE_ZOMBIE, "Groaner", 0);
    assert(minion != NULL);

    Combatant* combatant = combatant_create_from_minion(minion, true);
    assert(combatant != NULL);

    /* Format combatant */
    combat_ui_format_combatant(buf, sizeof(buf), combatant);
    assert(strlen(buf) < sizeof(buf));

    combatant_destroy(combatant);
    minion_destroy(minion);

    PASS();
}

/**
 * @brief Test combatant formatting with long names (buffer overflow protection)
 */
void test_combatant_format_long_name(void) {
    TEST("test_combatant_format_long_name");

    char buf[256];

    /* Create a minion with very long name */
    const char* long_name = "VeryLongNameThatExceedsNormalLimitsAndCouldCauseBufferOverflow";
    Minion* minion = minion_create(MINION_TYPE_SKELETON, long_name, 0);
    assert(minion != NULL);

    Combatant* combatant = combatant_create_from_minion(minion, true);
    assert(combatant != NULL);

    /* Format combatant - should not overflow */
    combat_ui_format_combatant(buf, sizeof(buf), combatant);
    assert(strlen(buf) < sizeof(buf));

    combatant_destroy(combatant);
    minion_destroy(minion);

    PASS();
}

/**
 * @brief Test combatant formatting with small buffer
 */
void test_combatant_format_small_buffer(void) {
    TEST("test_combatant_format_small_buffer");

    char buf[64];  /* Intentionally small */

    Minion* minion = minion_create(MINION_TYPE_GHOUL, "TestMinion", 0);
    assert(minion != NULL);

    Combatant* combatant = combatant_create_from_minion(minion, true);
    assert(combatant != NULL);

    /* Format combatant - should not overflow small buffer */
    combat_ui_format_combatant(buf, sizeof(buf), combatant);
    assert(strlen(buf) < sizeof(buf));

    combatant_destroy(combatant);
    minion_destroy(minion);

    PASS();
}

/**
 * @brief Test combatant with various statuses (DEAD, DEFENDING, ACTED)
 */
void test_combatant_format_statuses(void) {
    TEST("test_combatant_format_statuses");

    char buf[256];

    Minion* minion = minion_create(MINION_TYPE_WRAITH, "Specter", 0);
    assert(minion != NULL);

    Combatant* combatant = combatant_create_from_minion(minion, true);
    assert(combatant != NULL);

    /* Test DEAD status */
    combatant->health = 0;
    combat_ui_format_combatant(buf, sizeof(buf), combatant);
    assert(strlen(buf) < sizeof(buf));
    assert(strstr(buf, "DEAD") != NULL);

    /* Test DEFENDING status */
    combatant->health = 50;
    combatant->is_defending = true;
    combat_ui_format_combatant(buf, sizeof(buf), combatant);
    assert(strlen(buf) < sizeof(buf));
    assert(strstr(buf, "DEFENDING") != NULL);

    /* Test ACTED status */
    combatant->is_defending = false;
    combatant->has_acted_this_turn = true;
    combat_ui_format_combatant(buf, sizeof(buf), combatant);
    assert(strlen(buf) < sizeof(buf));
    assert(strstr(buf, "ACTED") != NULL);

    combatant_destroy(combatant);
    minion_destroy(minion);

    PASS();
}

/**
 * @brief Test HP color selection
 */
void test_hp_color(void) {
    TEST("test_hp_color");

    const char* color;

    /* Test high health (green) */
    color = combat_ui_get_hp_color(80, 100);
    assert(color != NULL);

    /* Test medium health (yellow) */
    color = combat_ui_get_hp_color(50, 100);
    assert(color != NULL);

    /* Test low health (red) */
    color = combat_ui_get_hp_color(10, 100);
    assert(color != NULL);

    /* Test zero health */
    color = combat_ui_get_hp_color(0, 100);
    assert(color != NULL);

    /* Test full health */
    color = combat_ui_get_hp_color(100, 100);
    assert(color != NULL);

    PASS();
}

/**
 * @brief Test buffer overflow protection with repeated formatting
 */
void test_repeated_formatting(void) {
    TEST("test_repeated_formatting");

    char buf[128];

    /* Repeatedly format to ensure no cumulative buffer issues */
    for (int i = 0; i < 100; i++) {
        combat_ui_format_health_bar(buf, sizeof(buf), i, 100, 20);
        assert(strlen(buf) < sizeof(buf));
    }

    PASS();
}

/**
 * @brief Test null parameter handling
 */
void test_null_parameters(void) {
    TEST("test_null_parameters");

    char buf[128];

    /* Test null combatant (should handle gracefully) */
    combat_ui_format_combatant(buf, sizeof(buf), NULL);
    /* Should not crash */

    /* Test null buffer (should handle gracefully) */
    combat_ui_format_health_bar(NULL, 0, 50, 100, 20);
    /* Should not crash */

    PASS();
}

int main(void) {
    printf("=== Combat UI Safety Tests ===\n\n");

    test_health_bar_normal();
    test_health_bar_edge_widths();
    test_health_bar_small_buffer();
    test_health_bar_max_values();
    test_combatant_format_normal();
    test_combatant_format_long_name();
    test_combatant_format_small_buffer();
    test_combatant_format_statuses();
    test_hp_color();
    test_repeated_formatting();
    test_null_parameters();

    printf("\n=== Test Summary ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    if (tests_run == tests_passed) {
        printf("\nAll safety tests passed!\n");
        return 0;
    } else {
        printf("\nSome tests failed!\n");
        return 1;
    }
}

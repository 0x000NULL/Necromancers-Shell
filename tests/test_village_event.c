#include "../src/game/events/village_event.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

/* Test color codes */
#define COLOR_GREEN "\033[32m"
#define COLOR_RED "\033[31m"
#define COLOR_RESET "\033[0m"

/* Test counter */
static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) \
    do { \
        tests_run++; \
        printf("  Testing %s... ", name); \
        fflush(stdout);

#define PASS() \
        tests_passed++; \
        printf(COLOR_GREEN "✓ PASSED" COLOR_RESET "\n"); \
    } while(0)

/* Test: Create and destroy */
static void test_village_event_create_destroy(void) {
    TEST("village_event_create_destroy");

    VillageEvent* event = village_event_create("ashbrook", "Ashbrook", 147);
    assert(event != NULL);
    assert(strcmp(event->village_id, "ashbrook") == 0);
    assert(strcmp(event->village_name, "Ashbrook") == 0);
    assert(event->total_population == 147);
    assert(event->breakdown_count == 0);
    assert(event->average_soul_quality == 0.0f);
    assert(event->total_soul_energy == 0);
    assert(event->total_corruption_cost == 0);
    assert(event->event_triggered == false);
    assert(event->outcome == OUTCOME_NONE);

    village_event_destroy(event);
    PASS();
}

/* Test: Add population breakdown */
static void test_village_event_add_population(void) {
    TEST("village_event_add_population");

    VillageEvent* event = village_event_create("test", "Test Village", 100);

    /* Add children */
    assert(village_event_add_population(event, POP_CHILDREN, 30, 85.0f, 5) == true);
    assert(event->breakdown_count == 1);
    assert(event->breakdown[0].category == POP_CHILDREN);
    assert(event->breakdown[0].count == 30);
    assert(event->breakdown[0].average_soul_quality == 85.0f);
    assert(event->breakdown[0].corruption_per_soul == 5);

    /* Add warriors */
    assert(village_event_add_population(event, POP_WARRIORS, 20, 75.0f, 1) == true);
    assert(event->breakdown_count == 2);

    /* Add adults */
    assert(village_event_add_population(event, POP_ADULTS, 50, 70.0f, 2) == true);
    assert(event->breakdown_count == 3);

    village_event_destroy(event);
    PASS();
}

/* Test: Calculate totals */
static void test_village_event_calculate_totals(void) {
    TEST("village_event_calculate_totals");

    VillageEvent* event = village_event_create("test", "Test Village", 100);

    /* Add population */
    village_event_add_population(event, POP_CHILDREN, 30, 80.0f, 5);
    village_event_add_population(event, POP_WARRIORS, 20, 75.0f, 1);
    village_event_add_population(event, POP_ADULTS, 50, 70.0f, 2);

    /* Calculate totals */
    assert(village_event_calculate_totals(event) == true);

    /* Verify average quality: (30*80 + 20*75 + 50*70) / 100 = 73.5 */
    assert(event->average_soul_quality >= 73.0f && event->average_soul_quality <= 74.0f);

    /* Verify corruption: 30*5 + 20*1 + 50*2 = 270 */
    assert(event->total_corruption_cost == 270);

    /* Verify energy is calculated */
    assert(event->total_soul_energy > 0);

    /* Verify children corruption tracked separately: 30*5 = 150 */
    assert(event->children_corruption == 150);

    village_event_destroy(event);
    PASS();
}

/* Test: Trigger event */
static void test_village_event_trigger(void) {
    TEST("village_event_trigger");

    VillageEvent* event = village_event_create("test", "Test Village", 100);

    assert(event->event_triggered == false);
    assert(village_event_trigger(event, 47) == true);
    assert(event->event_triggered == true);
    assert(event->trigger_day == 47);

    /* Cannot trigger again */
    assert(village_event_trigger(event, 50) == false);

    village_event_destroy(event);
    PASS();
}

/* Test: Spare outcome */
static void test_village_event_spare(void) {
    TEST("village_event_spare");

    VillageEvent* event = village_event_create("test", "Test Village", 100);
    village_event_trigger(event, 47);

    assert(village_event_spare(event, 50) == true);
    assert(event->outcome == OUTCOME_SPARED);
    assert(event->resolution_day == 50);
    assert(event->triggers_divine_attention == false);
    assert(event->locks_revenant_path == false);
    assert(event->locks_wraith_path == false);

    /* Check moral consequence text */
    const char* consequence = village_event_get_moral_consequence(event);
    assert(consequence != NULL);
    assert(strstr(consequence, "spared") != NULL);

    village_event_destroy(event);
    PASS();
}

/* Test: Partial harvest */
static void test_village_event_partial_harvest(void) {
    TEST("village_event_partial_harvest");

    VillageEvent* event = village_event_create("test", "Test Village", 100);
    village_event_add_population(event, POP_CHILDREN, 30, 85.0f, 5);
    village_event_add_population(event, POP_WARRIORS, 20, 75.0f, 1);
    village_event_add_population(event, POP_CRIMINALS, 10, 60.0f, 0);
    village_event_add_population(event, POP_ADULTS, 40, 70.0f, 2);
    village_event_calculate_totals(event);
    village_event_trigger(event, 47);

    uint32_t souls = 0;
    uint32_t energy = 0;
    uint32_t corruption = 0;

    assert(village_event_partial_harvest(event, 50, &souls, &energy, &corruption) == true);
    assert(event->outcome == OUTCOME_PARTIAL_HARVEST);
    assert(event->resolution_day == 50);

    /* Should harvest only warriors (20) + criminals (10) = 30 */
    assert(souls == 30);
    assert(energy > 0);
    assert(corruption > 0);

    /* Partial harvest doesn't trigger divine attention */
    assert(event->triggers_divine_attention == false);
    assert(event->locks_revenant_path == false);
    assert(event->locks_wraith_path == false);

    village_event_destroy(event);
    PASS();
}

/* Test: Full harvest */
static void test_village_event_full_harvest(void) {
    TEST("village_event_full_harvest");

    VillageEvent* event = village_event_create("ashbrook", "Ashbrook", 147);
    village_event_add_population(event, POP_CHILDREN, 32, 85.0f, 5);
    village_event_add_population(event, POP_WARRIORS, 12, 75.0f, 1);
    village_event_add_population(event, POP_ELDERS, 18, 60.0f, 2);
    village_event_add_population(event, POP_ADULTS, 80, 70.0f, 2);
    village_event_add_population(event, POP_CLERGY, 3, 90.0f, 8);
    village_event_add_population(event, POP_CRIMINALS, 2, 55.0f, 0);
    village_event_calculate_totals(event);
    village_event_trigger(event, 47);

    uint32_t souls = 0;
    uint32_t energy = 0;
    uint32_t corruption = 0;

    assert(village_event_full_harvest(event, 50, &souls, &energy, &corruption) == true);
    assert(event->outcome == OUTCOME_FULL_HARVEST);
    assert(event->resolution_day == 50);

    /* Should harvest entire population */
    assert(souls == 147);
    assert(energy == event->total_soul_energy);
    assert(corruption == event->total_corruption_cost);

    /* Full harvest triggers major consequences */
    assert(event->triggers_divine_attention == true);
    assert(event->locks_revenant_path == true);
    assert(event->locks_wraith_path == true);

    /* Check consequence mentions children */
    const char* consequence = village_event_get_moral_consequence(event);
    assert(strstr(consequence, "children") != NULL || strstr(consequence, "all") != NULL);

    village_event_destroy(event);
    PASS();
}

/* Test: Path lock checking */
static void test_village_event_check_path_locks(void) {
    TEST("village_event_check_path_locks");

    VillageEvent* event = village_event_create("test", "Test Village", 100);
    village_event_add_population(event, POP_CHILDREN, 50, 80.0f, 5);
    village_event_add_population(event, POP_ADULTS, 50, 70.0f, 2);
    village_event_calculate_totals(event);
    village_event_trigger(event, 47);

    uint32_t souls, energy, corruption;
    village_event_full_harvest(event, 50, &souls, &energy, &corruption);

    bool revenant_locked = false;
    bool wraith_locked = false;

    assert(village_event_check_path_locks(event, &revenant_locked, &wraith_locked) == true);
    assert(revenant_locked == true);
    assert(wraith_locked == true);

    village_event_destroy(event);
    PASS();
}

/* Test: Get population count */
static void test_village_event_get_population_count(void) {
    TEST("village_event_get_population_count");

    VillageEvent* event = village_event_create("test", "Test Village", 100);
    village_event_add_population(event, POP_CHILDREN, 30, 85.0f, 5);
    village_event_add_population(event, POP_WARRIORS, 20, 75.0f, 1);
    village_event_add_population(event, POP_ADULTS, 50, 70.0f, 2);

    assert(village_event_get_population_count(event, POP_CHILDREN) == 30);
    assert(village_event_get_population_count(event, POP_WARRIORS) == 20);
    assert(village_event_get_population_count(event, POP_ADULTS) == 50);
    assert(village_event_get_population_count(event, POP_ELDERS) == 0);

    village_event_destroy(event);
    PASS();
}

/* Test: Divine attention check */
static void test_village_event_triggers_divine_attention(void) {
    TEST("village_event_triggers_divine_attention");

    VillageEvent* event = village_event_create("test", "Test Village", 100);
    village_event_add_population(event, POP_ADULTS, 100, 70.0f, 2);
    village_event_calculate_totals(event);
    village_event_trigger(event, 47);

    /* Initially no divine attention */
    assert(village_event_triggers_divine_attention(event) == false);

    /* Spare doesn't trigger */
    village_event_spare(event, 50);
    assert(village_event_triggers_divine_attention(event) == false);

    /* Reset and test full harvest */
    village_event_destroy(event);
    event = village_event_create("test2", "Test Village 2", 100);
    village_event_add_population(event, POP_CHILDREN, 50, 80.0f, 5);
    village_event_add_population(event, POP_ADULTS, 50, 70.0f, 2);
    village_event_calculate_totals(event);
    village_event_trigger(event, 47);

    uint32_t souls, energy, corruption;
    village_event_full_harvest(event, 50, &souls, &energy, &corruption);

    /* Full harvest triggers divine attention */
    assert(village_event_triggers_divine_attention(event) == true);

    village_event_destroy(event);
    PASS();
}

/* Test: Outcome names */
static void test_village_event_outcome_name(void) {
    TEST("village_event_outcome_name");

    assert(strcmp(village_event_outcome_name(OUTCOME_NONE), "None") == 0);
    assert(strcmp(village_event_outcome_name(OUTCOME_SPARED), "Spared") == 0);
    assert(strcmp(village_event_outcome_name(OUTCOME_PARTIAL_HARVEST), "Partial Harvest") == 0);
    assert(strcmp(village_event_outcome_name(OUTCOME_FULL_HARVEST), "Full Harvest") == 0);
    assert(strcmp(village_event_outcome_name(OUTCOME_DEFENDED), "Defended") == 0);

    PASS();
}

/* Test: Category names */
static void test_village_event_category_name(void) {
    TEST("village_event_category_name");

    assert(strcmp(village_event_category_name(POP_CHILDREN), "Children") == 0);
    assert(strcmp(village_event_category_name(POP_WARRIORS), "Warriors") == 0);
    assert(strcmp(village_event_category_name(POP_ELDERS), "Elders") == 0);
    assert(strcmp(village_event_category_name(POP_ADULTS), "Adults") == 0);
    assert(strcmp(village_event_category_name(POP_CLERGY), "Clergy") == 0);
    assert(strcmp(village_event_category_name(POP_CRIMINALS), "Criminals") == 0);

    PASS();
}

/* Test: Calculate children corruption */
static void test_village_event_calculate_children_corruption(void) {
    TEST("village_event_calculate_children_corruption");

    VillageEvent* event = village_event_create("test", "Test Village", 100);
    village_event_add_population(event, POP_CHILDREN, 32, 85.0f, 5);
    village_event_add_population(event, POP_WARRIORS, 20, 75.0f, 1);
    village_event_add_population(event, POP_ADULTS, 48, 70.0f, 2);
    village_event_calculate_totals(event);

    /* Children corruption: 32 * 5 = 160 */
    uint32_t children_corruption = village_event_calculate_children_corruption(event);
    assert(children_corruption == 160);

    village_event_destroy(event);
    PASS();
}

/* Test: Moral consequence text */
static void test_village_event_get_moral_consequence(void) {
    TEST("village_event_get_moral_consequence");

    VillageEvent* event = village_event_create("test", "Test Village", 100);

    /* Before resolution */
    const char* consequence = village_event_get_moral_consequence(event);
    assert(strcmp(consequence, "Event not yet resolved") == 0);

    /* After sparing */
    village_event_trigger(event, 47);
    village_event_spare(event, 50);
    consequence = village_event_get_moral_consequence(event);
    assert(strlen(consequence) > 0);
    assert(strcmp(consequence, "Event not yet resolved") != 0);

    village_event_destroy(event);
    PASS();
}

/* Test: Full Ashbrook scenario */
static void test_ashbrook_full_scenario(void) {
    TEST("ashbrook_full_scenario");

    /* Create Ashbrook event */
    VillageEvent* event = village_event_create("ashbrook", "Ashbrook", 147);

    /* Add population as per data file */
    village_event_add_population(event, POP_CHILDREN, 32, 85.0f, 5);
    village_event_add_population(event, POP_WARRIORS, 12, 75.0f, 1);
    village_event_add_population(event, POP_ELDERS, 18, 60.0f, 2);
    village_event_add_population(event, POP_ADULTS, 80, 70.0f, 2);
    village_event_add_population(event, POP_CLERGY, 3, 90.0f, 8);
    village_event_add_population(event, POP_CRIMINALS, 2, 55.0f, 0);

    /* Calculate totals */
    assert(village_event_calculate_totals(event) == true);

    /* Verify population breakdown */
    assert(event->breakdown_count == 6);
    assert(event->total_population == 147);

    /* Verify children corruption: 32 * 5 = 160 */
    assert(event->children_corruption == 160);

    /* Trigger on Day 47 */
    assert(village_event_trigger(event, 47) == true);
    assert(event->trigger_day == 47);

    /* Simulate full harvest */
    uint32_t souls, energy, corruption;
    assert(village_event_full_harvest(event, 47, &souls, &energy, &corruption) == true);

    /* Verify outcomes */
    assert(souls == 147);
    assert(energy > 1000 && energy < 1100); /* Should be around 1,068 */
    assert(corruption > 300); /* Should be 308 total */
    assert(event->triggers_divine_attention == true);
    assert(event->locks_revenant_path == true);
    assert(event->locks_wraith_path == true);

    village_event_destroy(event);
    PASS();
}

int main(void) {
    printf("Running Village Event system tests...\n\n");

    test_village_event_create_destroy();
    test_village_event_add_population();
    test_village_event_calculate_totals();
    test_village_event_trigger();
    test_village_event_spare();
    test_village_event_partial_harvest();
    test_village_event_full_harvest();
    test_village_event_check_path_locks();
    test_village_event_get_population_count();
    test_village_event_triggers_divine_attention();
    test_village_event_outcome_name();
    test_village_event_category_name();
    test_village_event_calculate_children_corruption();
    test_village_event_get_moral_consequence();
    test_ashbrook_full_scenario();

    /* Summary */
    printf("\n");
    if (tests_passed == tests_run) {
        printf(COLOR_GREEN "All %d tests passed!" COLOR_RESET "\n", tests_passed);
        return 0;
    } else {
        printf(COLOR_RED "%d/%d tests passed" COLOR_RESET "\n", tests_passed, tests_run);
        return 1;
    }
}

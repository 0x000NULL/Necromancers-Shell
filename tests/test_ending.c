#include "../src/game/narrative/endings/ending.h"
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
static void test_ending_manager_create_destroy(void) {
    TEST("ending_manager_create_destroy");

    EndingManager* manager = ending_manager_create();
    assert(manager != NULL);
    assert(manager->current_ending == ENDING_NONE);
    assert(manager->game_over == false);
    assert(manager->ending_day == 0);

    /* Verify all endings initialized */
    for (int i = 0; i < 7; i++) {
        assert(manager->endings[i].type == (EndingType)(i + 1));
        assert(manager->endings[i].achievement == ACHIEVEMENT_AVAILABLE);
        assert(manager->endings[i].path_locked == false);
        assert(manager->endings[i].day_achieved == 0);
    }

    ending_manager_destroy(manager);
    PASS();
}

/* Test: Lock ending path */
static void test_ending_manager_lock_path(void) {
    TEST("ending_manager_lock_path");

    EndingManager* manager = ending_manager_create();

    assert(ending_manager_lock_path(manager, ENDING_REVENANT, "Full harvest occurred") == true);
    assert(manager->endings[0].path_locked == true);
    assert(manager->endings[0].achievement == ACHIEVEMENT_LOCKED);
    assert(strcmp(manager->endings[0].lock_reason, "Full harvest occurred") == 0);

    /* Lock another path */
    assert(ending_manager_lock_path(manager, ENDING_WRAITH, "Too corrupted") == true);
    assert(manager->endings[1].path_locked == true);

    ending_manager_destroy(manager);
    PASS();
}

/* Test: Check availability */
static void test_ending_manager_is_available(void) {
    TEST("ending_manager_is_available");

    EndingManager* manager = ending_manager_create();

    /* All should be available initially */
    assert(ending_manager_is_available(manager, ENDING_REVENANT) == true);
    assert(ending_manager_is_available(manager, ENDING_MORNINGSTAR) == true);

    /* Lock one */
    ending_manager_lock_path(manager, ENDING_REVENANT, "Test");
    assert(ending_manager_is_available(manager, ENDING_REVENANT) == false);
    assert(ending_manager_is_available(manager, ENDING_WRAITH) == true);

    ending_manager_destroy(manager);
    PASS();
}

/* Test: Check Revenant requirements */
static void test_ending_revenant_requirements(void) {
    TEST("ending_revenant_requirements");

    EndingManager* manager = ending_manager_create();

    /* Valid Revenant scenario */
    uint8_t corruption = 20;
    uint8_t consciousness = 80;
    int16_t divine_favor[7] = {50, 50, 50, 20, -10, 30, 40}; /* 3+ gods with favor */
    bool ashbrook_spared = true;
    bool did_full_harvest = false;
    float thessara_trust = 60.0f;
    bool council_verdict = true;
    bool void_battle = false;
    uint8_t trials = 5;
    uint32_t souls = 300;
    uint8_t alliances = 2;

    assert(ending_manager_check_requirements(manager, ENDING_REVENANT,
           corruption, consciousness, divine_favor, ashbrook_spared, did_full_harvest,
           thessara_trust, council_verdict, void_battle, trials, souls, alliances) == true);

    /* Fail if Ashbrook not spared */
    ashbrook_spared = false;
    assert(ending_manager_check_requirements(manager, ENDING_REVENANT,
           corruption, consciousness, divine_favor, ashbrook_spared, did_full_harvest,
           thessara_trust, council_verdict, void_battle, trials, souls, alliances) == false);

    /* Fail if did full harvest */
    ashbrook_spared = true;
    did_full_harvest = true;
    assert(ending_manager_check_requirements(manager, ENDING_REVENANT,
           corruption, consciousness, divine_favor, ashbrook_spared, did_full_harvest,
           thessara_trust, council_verdict, void_battle, trials, souls, alliances) == false);

    /* Fail if corruption too high */
    did_full_harvest = false;
    corruption = 40;
    assert(ending_manager_check_requirements(manager, ENDING_REVENANT,
           corruption, consciousness, divine_favor, ashbrook_spared, did_full_harvest,
           thessara_trust, council_verdict, void_battle, trials, souls, alliances) == false);

    ending_manager_destroy(manager);
    PASS();
}

/* Test: Check Morningstar requirements */
static void test_ending_morningstar_requirements(void) {
    TEST("ending_morningstar_requirements");

    EndingManager* manager = ending_manager_create();

    /* Valid Morningstar scenario - very strict */
    uint8_t corruption = 10;
    uint8_t consciousness = 95;
    int16_t divine_favor[7] = {70, 70, 65, 70, -20, 80, 65}; /* 5+ gods with favor */
    bool ashbrook_spared = true;
    bool did_full_harvest = false;
    float thessara_trust = 90.0f;
    bool council_verdict = true;
    bool void_battle = true;
    uint8_t trials = 7; /* All trials */
    uint32_t souls = 500;
    uint8_t alliances = 4;

    assert(ending_manager_check_requirements(manager, ENDING_MORNINGSTAR,
           corruption, consciousness, divine_favor, ashbrook_spared, did_full_harvest,
           thessara_trust, council_verdict, void_battle, trials, souls, alliances) == true);

    /* Fail if void battle not won */
    void_battle = false;
    assert(ending_manager_check_requirements(manager, ENDING_MORNINGSTAR,
           corruption, consciousness, divine_favor, ashbrook_spared, did_full_harvest,
           thessara_trust, council_verdict, void_battle, trials, souls, alliances) == false);

    /* Fail if not all trials completed */
    void_battle = true;
    trials = 6;
    assert(ending_manager_check_requirements(manager, ENDING_MORNINGSTAR,
           corruption, consciousness, divine_favor, ashbrook_spared, did_full_harvest,
           thessara_trust, council_verdict, void_battle, trials, souls, alliances) == false);

    ending_manager_destroy(manager);
    PASS();
}

/* Test: Check Lich Lord requirements */
static void test_ending_lich_lord_requirements(void) {
    TEST("ending_lich_lord_requirements");

    EndingManager* manager = ending_manager_create();

    /* Valid Lich Lord scenario - high corruption */
    uint8_t corruption = 85;
    uint8_t consciousness = 50;
    int16_t divine_favor[7] = {-80, -90, -70, -100, -60, -85, -75}; /* All negative */
    bool ashbrook_spared = false;
    bool did_full_harvest = true;
    float thessara_trust = 10.0f;
    bool council_verdict = false;
    bool void_battle = false;
    uint8_t trials = 2;
    uint32_t souls = 7000;
    uint8_t alliances = 0;

    assert(ending_manager_check_requirements(manager, ENDING_LICH_LORD,
           corruption, consciousness, divine_favor, ashbrook_spared, did_full_harvest,
           thessara_trust, council_verdict, void_battle, trials, souls, alliances) == true);

    /* Fail if corruption too low */
    corruption = 60;
    assert(ending_manager_check_requirements(manager, ENDING_LICH_LORD,
           corruption, consciousness, divine_favor, ashbrook_spared, did_full_harvest,
           thessara_trust, council_verdict, void_battle, trials, souls, alliances) == false);

    /* Fail if not enough souls harvested */
    corruption = 85;
    souls = 2000;
    assert(ending_manager_check_requirements(manager, ENDING_LICH_LORD,
           corruption, consciousness, divine_favor, ashbrook_spared, did_full_harvest,
           thessara_trust, council_verdict, void_battle, trials, souls, alliances) == false);

    ending_manager_destroy(manager);
    PASS();
}

/* Test: Trigger ending */
static void test_ending_manager_trigger_ending(void) {
    TEST("ending_manager_trigger_ending");

    EndingManager* manager = ending_manager_create();

    assert(manager->game_over == false);
    assert(ending_manager_trigger_ending(manager, ENDING_WRAITH, 250) == true);

    assert(manager->game_over == true);
    assert(manager->current_ending == ENDING_WRAITH);
    assert(manager->ending_day == 250);
    assert(manager->endings[1].achievement == ACHIEVEMENT_UNLOCKED);
    assert(manager->endings[1].day_achieved == 250);

    /* Cannot trigger another ending after game over */
    assert(ending_manager_trigger_ending(manager, ENDING_REVENANT, 260) == false);
    assert(manager->current_ending == ENDING_WRAITH); /* Unchanged */

    ending_manager_destroy(manager);
    PASS();
}

/* Test: Cannot trigger locked ending */
static void test_ending_trigger_locked(void) {
    TEST("ending_trigger_locked");

    EndingManager* manager = ending_manager_create();

    ending_manager_lock_path(manager, ENDING_REVENANT, "Test lock");

    /* Cannot trigger locked ending */
    assert(ending_manager_trigger_ending(manager, ENDING_REVENANT, 200) == false);
    assert(manager->game_over == false);

    ending_manager_destroy(manager);
    PASS();
}

/* Test: Ending type names */
static void test_ending_type_name(void) {
    TEST("ending_type_name");

    assert(strcmp(ending_type_name(ENDING_NONE), "None") == 0);
    assert(strcmp(ending_type_name(ENDING_REVENANT), "The Revenant") == 0);
    assert(strcmp(ending_type_name(ENDING_WRAITH), "The Wraith Administrator") == 0);
    assert(strcmp(ending_type_name(ENDING_MORNINGSTAR), "The Morningstar") == 0);
    assert(strcmp(ending_type_name(ENDING_ARCHON), "The Archon") == 0);
    assert(strcmp(ending_type_name(ENDING_LICH_LORD), "The Lich Lord") == 0);
    assert(strcmp(ending_type_name(ENDING_OBLIVION), "Oblivion") == 0);
    assert(strcmp(ending_type_name(ENDING_DIVINE_DESTRUCTION), "Divine Destruction") == 0);

    PASS();
}

/* Test: Get ending */
static void test_ending_manager_get_ending(void) {
    TEST("ending_manager_get_ending");

    EndingManager* manager = ending_manager_create();

    const Ending* revenant = ending_manager_get_ending(manager, ENDING_REVENANT);
    assert(revenant != NULL);
    assert(revenant->type == ENDING_REVENANT);
    assert(strcmp(revenant->name, "The Revenant") == 0);
    assert(strlen(revenant->description) > 0);
    assert(strlen(revenant->epilogue) > 0);

    const Ending* morningstar = ending_manager_get_ending(manager, ENDING_MORNINGSTAR);
    assert(morningstar != NULL);
    assert(morningstar->type == ENDING_MORNINGSTAR);

    ending_manager_destroy(manager);
    PASS();
}

/* Test: Get available endings */
static void test_ending_manager_get_available_endings(void) {
    TEST("ending_manager_get_available_endings");

    EndingManager* manager = ending_manager_create();

    size_t count = 0;
    const EndingType* available = ending_manager_get_available_endings(manager, &count);
    assert(available != NULL);
    assert(count == 7); /* All initially available */

    /* Lock two endings */
    ending_manager_lock_path(manager, ENDING_REVENANT, "Test");
    ending_manager_lock_path(manager, ENDING_WRAITH, "Test");

    available = ending_manager_get_available_endings(manager, &count);
    assert(count == 5); /* 7 - 2 locked */

    /* Verify the returned types don't include locked ones */
    bool found_revenant = false;
    bool found_wraith = false;
    for (size_t i = 0; i < count; i++) {
        if (available[i] == ENDING_REVENANT) found_revenant = true;
        if (available[i] == ENDING_WRAITH) found_wraith = true;
    }
    assert(found_revenant == false);
    assert(found_wraith == false);

    ending_manager_destroy(manager);
    PASS();
}

/* Test: Check Divine Destruction requirements */
static void test_ending_divine_destruction_requirements(void) {
    TEST("ending_divine_destruction_requirements");

    EndingManager* manager = ending_manager_create();

    /* Divine Destruction scenario - total failure */
    uint8_t corruption = 100;
    uint8_t consciousness = 5;
    int16_t divine_favor[7] = {-60, -100, -85, -95, -80, -100, -90}; /* At least one >= -70 */
    bool ashbrook_spared = false;
    bool did_full_harvest = true;
    float thessara_trust = 0.0f;
    bool council_verdict = false;
    bool void_battle = false;
    uint8_t trials = 0;
    uint32_t souls = 10000;
    uint8_t alliances = 0;

    assert(ending_manager_check_requirements(manager, ENDING_DIVINE_DESTRUCTION,
           corruption, consciousness, divine_favor, ashbrook_spared, did_full_harvest,
           thessara_trust, council_verdict, void_battle, trials, souls, alliances) == true);

    /* Fail if corruption not high enough */
    corruption = 80;
    assert(ending_manager_check_requirements(manager, ENDING_DIVINE_DESTRUCTION,
           corruption, consciousness, divine_favor, ashbrook_spared, did_full_harvest,
           thessara_trust, council_verdict, void_battle, trials, souls, alliances) == false);

    ending_manager_destroy(manager);
    PASS();
}

/* Test: Wraith ending requirements */
static void test_ending_wraith_requirements(void) {
    TEST("ending_wraith_requirements");

    EndingManager* manager = ending_manager_create();

    /* Valid Wraith scenario */
    uint8_t corruption = 35;
    uint8_t consciousness = 85;
    int16_t divine_favor[7] = {30, 25, 15, 40, -10, 35, 20}; /* 2+ with favor */
    bool ashbrook_spared = false; /* Can harvest partially */
    bool did_full_harvest = false; /* But not full harvest */
    float thessara_trust = 80.0f; /* High trust required */
    bool council_verdict = true;
    bool void_battle = false;
    uint8_t trials = 7; /* All trials required */
    uint32_t souls = 800;
    uint8_t alliances = 3;

    assert(ending_manager_check_requirements(manager, ENDING_WRAITH,
           corruption, consciousness, divine_favor, ashbrook_spared, did_full_harvest,
           thessara_trust, council_verdict, void_battle, trials, souls, alliances) == true);

    /* Fail if Thessara trust too low */
    thessara_trust = 60.0f;
    assert(ending_manager_check_requirements(manager, ENDING_WRAITH,
           corruption, consciousness, divine_favor, ashbrook_spared, did_full_harvest,
           thessara_trust, council_verdict, void_battle, trials, souls, alliances) == false);

    /* Fail if not all trials completed */
    thessara_trust = 80.0f;
    trials = 6;
    assert(ending_manager_check_requirements(manager, ENDING_WRAITH,
           corruption, consciousness, divine_favor, ashbrook_spared, did_full_harvest,
           thessara_trust, council_verdict, void_battle, trials, souls, alliances) == false);

    ending_manager_destroy(manager);
    PASS();
}

int main(void) {
    printf("Running Ending system tests...\n\n");

    test_ending_manager_create_destroy();
    test_ending_manager_lock_path();
    test_ending_manager_is_available();
    test_ending_revenant_requirements();
    test_ending_morningstar_requirements();
    test_ending_lich_lord_requirements();
    test_ending_manager_trigger_ending();
    test_ending_trigger_locked();
    test_ending_type_name();
    test_ending_manager_get_ending();
    test_ending_manager_get_available_endings();
    test_ending_divine_destruction_requirements();
    test_ending_wraith_requirements();

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

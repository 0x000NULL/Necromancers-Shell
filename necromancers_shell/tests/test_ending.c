/**
 * @file test_ending.c
 * @brief Unit tests for ending system
 */

#include "../src/game/endings/ending_system.h"
#include "../src/game/game_state.h"
#include "../src/utils/logger.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Mock game state for testing */
static GameState create_mock_state(uint8_t corruption, float consciousness, uint32_t day) {
    GameState state = {0};
    state.corruption.corruption = corruption;
    state.consciousness.stability = consciousness;
    state.resources.day_count = day;
    return state;
}

void test_ending_system_create_destroy(void) {
    printf("Test: ending_system_create_destroy... ");

    EndingSystem* system = ending_system_create();
    assert(system != NULL);
    assert(system->chosen_ending == ENDING_NONE);
    assert(system->game_ended == false);
    assert(system->morningstar_attempted == false);
    assert(system->trials_completed == 0);

    ending_system_destroy(system);

    printf("PASS\n");
}

void test_ending_check_requirements(void) {
    printf("Test: ending_check_requirements... ");

    EndingRequirements req = {
        .min_corruption = 40,
        .max_corruption = 60,
        .min_consciousness = 50.0f,
        .min_day = 1000,
        .requires_archon = true,
        .requires_morningstar = false
    };

    /* Valid case */
    assert(ending_check_requirements(&req, 50, 60.0f, 1500, true) == true);

    /* Corruption too low */
    assert(ending_check_requirements(&req, 30, 60.0f, 1500, true) == false);

    /* Corruption too high */
    assert(ending_check_requirements(&req, 70, 60.0f, 1500, true) == false);

    /* Consciousness too low */
    assert(ending_check_requirements(&req, 50, 40.0f, 1500, true) == false);

    /* Day too early */
    assert(ending_check_requirements(&req, 50, 60.0f, 500, true) == false);

    /* Not Archon */
    assert(ending_check_requirements(&req, 50, 60.0f, 1500, false) == false);

    /* NULL requirements */
    assert(ending_check_requirements(NULL, 50, 60.0f, 1500, true) == false);

    printf("PASS\n");
}

void test_revenant_ending_available(void) {
    printf("Test: revenant_ending_available... ");

    EndingSystem* system = ending_system_create();
    assert(system != NULL);

    /* Low corruption, high consciousness, Archon status, late game */
    GameState state = create_mock_state(25, 80.0f, 2000);
    ending_system_check_availability(system, &state);

    const Ending* ending = ending_system_get_ending(system, ENDING_REVENANT);
    assert(ending != NULL);
    assert(ending->unlocked == true);

    /* Should not unlock Lich Lord (corruption too low) */
    const Ending* lich = ending_system_get_ending(system, ENDING_LICH_LORD);
    assert(lich != NULL);
    assert(lich->unlocked == false);

    ending_system_destroy(system);

    printf("PASS\n");
}

void test_lich_lord_ending_available(void) {
    printf("Test: lich_lord_ending_available... ");

    EndingSystem* system = ending_system_create();
    assert(system != NULL);

    /* High corruption (point of no return) */
    GameState state = create_mock_state(75, 60.0f, 500);
    ending_system_check_availability(system, &state);

    const Ending* ending = ending_system_get_ending(system, ENDING_LICH_LORD);
    assert(ending != NULL);
    assert(ending->unlocked == true);

    /* Should not unlock Revenant (corruption too high) */
    const Ending* revenant = ending_system_get_ending(system, ENDING_REVENANT);
    assert(revenant != NULL);
    assert(revenant->unlocked == false);

    ending_system_destroy(system);

    printf("PASS\n");
}

void test_reaper_ending_available(void) {
    printf("Test: reaper_ending_available... ");

    EndingSystem* system = ending_system_create();
    assert(system != NULL);

    /* Mid corruption, decent consciousness, mid game */
    GameState state = create_mock_state(50, 70.0f, 1500);
    ending_system_check_availability(system, &state);

    const Ending* ending = ending_system_get_ending(system, ENDING_REAPER);
    assert(ending != NULL);
    assert(ending->unlocked == true);

    ending_system_destroy(system);

    printf("PASS\n");
}

void test_wraith_ending_available(void) {
    printf("Test: wraith_ending_available... ");

    EndingSystem* system = ending_system_create();
    assert(system != NULL);

    /* Mid corruption, high consciousness, mid-late game */
    GameState state = create_mock_state(50, 75.0f, 6000);
    ending_system_check_availability(system, &state);

    const Ending* ending = ending_system_get_ending(system, ENDING_WRAITH);
    assert(ending != NULL);
    assert(ending->unlocked == true);

    ending_system_destroy(system);

    printf("PASS\n");
}

void test_morningstar_ending_requires_attempt(void) {
    printf("Test: morningstar_ending_requires_attempt... ");

    EndingSystem* system = ending_system_create();
    assert(system != NULL);

    /* Perfect conditions for Morningstar, but not attempted */
    GameState state = create_mock_state(50, 15.0f, 11000);
    ending_system_check_availability(system, &state);

    const Ending* ending = ending_system_get_ending(system, ENDING_ARCHON_MORNINGSTAR);
    assert(ending != NULL);
    assert(ending->unlocked == false); /* Not unlocked without attempt */

    /* Now start Morningstar attempt */
    bool success = ending_system_start_morningstar(system);
    assert(success == true);
    assert(system->morningstar_attempted == true);

    /* Check availability again */
    ending_system_check_availability(system, &state);
    assert(ending->unlocked == true); /* Now unlocked */

    ending_system_destroy(system);

    printf("PASS\n");
}

void test_error_ending_requires_attempt(void) {
    printf("Test: error_ending_requires_attempt... ");

    EndingSystem* system = ending_system_create();
    assert(system != NULL);

    /* Conditions for Error ending (very low consciousness) */
    GameState state = create_mock_state(50, 5.0f, 11000);

    /* Start Morningstar attempt */
    ending_system_start_morningstar(system);

    ending_system_check_availability(system, &state);

    const Ending* ending = ending_system_get_ending(system, ENDING_ERROR);
    assert(ending != NULL);
    assert(ending->unlocked == true);

    ending_system_destroy(system);

    printf("PASS\n");
}

void test_get_available_endings(void) {
    printf("Test: get_available_endings... ");

    EndingSystem* system = ending_system_create();
    assert(system != NULL);

    /* State that unlocks multiple endings */
    GameState state = create_mock_state(50, 70.0f, 6000);
    ending_system_check_availability(system, &state);

    size_t count = 0;
    const Ending** available = ending_system_get_available(system, &count);
    assert(available != NULL);
    assert(count > 0);

    /* Should have Reaper and Wraith available */
    bool found_reaper = false;
    bool found_wraith = false;
    for (size_t i = 0; i < count; i++) {
        if (available[i]->type == ENDING_REAPER) found_reaper = true;
        if (available[i]->type == ENDING_WRAITH) found_wraith = true;
    }
    assert(found_reaper == true);
    assert(found_wraith == true);

    ending_system_destroy(system);

    printf("PASS\n");
}

void test_trigger_ending(void) {
    printf("Test: trigger_ending... ");

    EndingSystem* system = ending_system_create();
    assert(system != NULL);

    GameState state = create_mock_state(50, 70.0f, 6000);
    ending_system_check_availability(system, &state);

    /* Trigger Reaper ending */
    bool success = ending_system_trigger(system, &state, ENDING_REAPER);
    assert(success == true);
    assert(system->game_ended == true);
    assert(system->chosen_ending == ENDING_REAPER);
    assert(system->ending_day == 6000);

    /* Get the ending and check it's marked as chosen */
    const Ending* ending = ending_system_get_ending(system, ENDING_REAPER);
    assert(ending != NULL);
    assert(ending->chosen == true);

    /* Cannot trigger another ending after game ended */
    success = ending_system_trigger(system, &state, ENDING_WRAITH);
    assert(success == false);

    ending_system_destroy(system);

    printf("PASS\n");
}

void test_trigger_unavailable_ending(void) {
    printf("Test: trigger_unavailable_ending... ");

    EndingSystem* system = ending_system_create();
    assert(system != NULL);

    /* State that doesn't unlock Lich Lord */
    GameState state = create_mock_state(30, 70.0f, 1000);
    ending_system_check_availability(system, &state);

    /* Try to trigger Lich Lord (should fail) */
    bool success = ending_system_trigger(system, &state, ENDING_LICH_LORD);
    assert(success == false);
    assert(system->game_ended == false);

    ending_system_destroy(system);

    printf("PASS\n");
}

void test_morningstar_trials(void) {
    printf("Test: morningstar_trials... ");

    EndingSystem* system = ending_system_create();
    assert(system != NULL);

    /* Start Morningstar attempt */
    bool success = ending_system_start_morningstar(system);
    assert(success == true);

    /* Cannot start again */
    success = ending_system_start_morningstar(system);
    assert(success == false);

    /* Complete trials in order */
    assert(system->trials_completed == 0);

    success = ending_system_complete_trial(system, 0); /* Combat Vorathos */
    assert(success == true);
    assert(system->trials_completed == 1);
    assert(system->trial_combat_vorathos == true);

    success = ending_system_complete_trial(system, 1); /* Kael Fragment */
    assert(success == true);
    assert(system->trials_completed == 2);

    success = ending_system_complete_trial(system, 2); /* Master Magic */
    assert(success == true);
    assert(system->trials_completed == 3);

    success = ending_system_complete_trial(system, 3); /* Perfect Corruption */
    assert(success == true);
    assert(system->trials_completed == 4);

    success = ending_system_complete_trial(system, 4); /* Apotheosis Engine */
    assert(success == true);
    assert(system->trials_completed == 5);

    success = ending_system_complete_trial(system, 5); /* Death & Transcend */
    assert(success == true);
    assert(system->trials_completed == 6);

    success = ending_system_complete_trial(system, 6); /* Claim Domain */
    assert(success == true);
    assert(system->trials_completed == 7);

    /* Get trial progress */
    uint8_t progress = ending_system_get_trial_progress(system);
    assert(progress == 7);

    ending_system_destroy(system);

    printf("PASS\n");
}

void test_trial_without_morningstar(void) {
    printf("Test: trial_without_morningstar... ");

    EndingSystem* system = ending_system_create();
    assert(system != NULL);

    /* Try to complete trial without starting Morningstar */
    bool success = ending_system_complete_trial(system, 0);
    assert(success == false);
    assert(system->trials_completed == 0);

    ending_system_destroy(system);

    printf("PASS\n");
}

void test_has_ended(void) {
    printf("Test: has_ended... ");

    EndingSystem* system = ending_system_create();
    assert(system != NULL);

    /* Initially not ended */
    assert(ending_system_has_ended(system) == false);

    /* Trigger ending */
    GameState state = create_mock_state(75, 60.0f, 1000);
    ending_system_check_availability(system, &state);
    ending_system_trigger(system, &state, ENDING_LICH_LORD);

    /* Now ended */
    assert(ending_system_has_ended(system) == true);

    /* Get chosen ending */
    EndingType chosen = ending_system_get_chosen(system);
    assert(chosen == ENDING_LICH_LORD);

    ending_system_destroy(system);

    printf("PASS\n");
}

void test_ending_names_and_descriptions(void) {
    printf("Test: ending_names_and_descriptions... ");

    EndingSystem* system = ending_system_create();
    assert(system != NULL);

    /* Check Revenant ending */
    const Ending* revenant = ending_system_get_ending(system, ENDING_REVENANT);
    assert(revenant != NULL);
    assert(strcmp(revenant->name, "Revenant Route") == 0);
    assert(strcmp(revenant->achievement, "The Second Life") == 0);
    assert(strlen(revenant->description) > 0);

    /* Check Lich Lord ending */
    const Ending* lich = ending_system_get_ending(system, ENDING_LICH_LORD);
    assert(lich != NULL);
    assert(strcmp(lich->name, "Lich Lord Route") == 0);
    assert(strcmp(lich->achievement, "The Perfect Machine") == 0);

    /* Check Reaper ending */
    const Ending* reaper = ending_system_get_ending(system, ENDING_REAPER);
    assert(reaper != NULL);
    assert(strcmp(reaper->name, "Reaper Route") == 0);
    assert(strcmp(reaper->achievement, "The Eternal Administrator") == 0);

    /* Check Archon/Morningstar ending */
    const Ending* morningstar = ending_system_get_ending(system, ENDING_ARCHON_MORNINGSTAR);
    assert(morningstar != NULL);
    assert(strcmp(morningstar->name, "Archon/Morningstar Route") == 0);
    assert(strcmp(morningstar->achievement, "The Eighth Architect") == 0);

    /* Check Wraith ending */
    const Ending* wraith = ending_system_get_ending(system, ENDING_WRAITH);
    assert(wraith != NULL);
    assert(strcmp(wraith->name, "Wraith Route") == 0);
    assert(strcmp(wraith->achievement, "The Distributed Consciousness") == 0);

    /* Check Error ending */
    const Ending* error = ending_system_get_ending(system, ENDING_ERROR);
    assert(error != NULL);
    assert(strcmp(error->name, "Error Route") == 0);
    assert(strcmp(error->achievement, "The Eternal Error") == 0);

    ending_system_destroy(system);

    printf("PASS\n");
}

int main(void) {
    /* Suppress log output during tests */
    logger_set_level(LOG_LEVEL_FATAL + 1);

    printf("\n=== Ending System Tests ===\n\n");

    test_ending_system_create_destroy();
    test_ending_check_requirements();
    test_revenant_ending_available();
    test_lich_lord_ending_available();
    test_reaper_ending_available();
    test_wraith_ending_available();
    test_morningstar_ending_requires_attempt();
    test_error_ending_requires_attempt();
    test_get_available_endings();
    test_trigger_ending();
    test_trigger_unavailable_ending();
    test_morningstar_trials();
    test_trial_without_morningstar();
    test_has_ended();
    test_ending_names_and_descriptions();

    printf("\n=== All Ending System Tests Passed! ===\n\n");

    return 0;
}

/**
 * @file test_null_space.c
 * @brief Unit tests for null space location system
 */

#include "../src/game/world/null_space.h"
#include "../src/utils/logger.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

void test_null_space_creation(void) {
    printf("Test: null_space_creation... ");

    NullSpaceState* null_space = null_space_create();
    assert(null_space != NULL);

    /* Initial state */
    assert(null_space->discovered == false);
    assert(null_space->can_access == false);
    assert(null_space->currently_inside == false);
    assert(null_space->current_activity == NULL_SPACE_IDLE);
    assert(null_space->subjective_hours == 0);
    assert(null_space->visit_count == 0);
    assert(null_space->total_visits == 0);
    assert(null_space->total_subjective_hours == 0);
    assert(null_space->thessara_meetings == 0);
    assert(null_space->network_debug_count == 0);
    assert(null_space->meditation_count == 0);
    assert(null_space->archon_workspace == false);
    assert(null_space->wraith_observation == false);
    assert(null_space->network_root_access == false);

    null_space_destroy(null_space);

    printf("PASS\n");
}

void test_null_space_discovery(void) {
    printf("Test: null_space_discovery... ");

    NullSpaceState* null_space = null_space_create();
    assert(null_space != NULL);

    /* Discover null space on day 50 */
    bool success = null_space_discover(null_space, 50);
    assert(success == true);
    assert(null_space->discovered == true);
    assert(null_space->discovery_day == 50);

    /* Cannot discover twice */
    success = null_space_discover(null_space, 100);
    assert(success == false);
    assert(null_space->discovery_day == 50); /* Unchanged */

    null_space_destroy(null_space);

    printf("PASS\n");
}

void test_null_space_access_unlock(void) {
    printf("Test: null_space_access_unlock... ");

    NullSpaceState* null_space = null_space_create();
    null_space_discover(null_space, 10);

    /* Initially no access */
    assert(null_space_has_access(null_space) == false);

    /* Unlock access on day 75 */
    bool success = null_space_unlock_access(null_space, 75);
    assert(success == true);
    assert(null_space->can_access == true);
    assert(null_space->access_unlock_day == 75);
    assert(null_space_has_access(null_space) == true);

    /* Cannot unlock twice */
    success = null_space_unlock_access(null_space, 150);
    assert(success == false);
    assert(null_space->access_unlock_day == 75); /* Unchanged */

    null_space_destroy(null_space);

    printf("PASS\n");
}

void test_null_space_enter_exit(void) {
    printf("Test: null_space_enter_exit... ");

    NullSpaceState* null_space = null_space_create();
    null_space_discover(null_space, 10);
    null_space_unlock_access(null_space, 20);

    /* Cannot enter without access */
    NullSpaceState* no_access = null_space_create();
    bool success = null_space_enter(no_access, 50, NULL_SPACE_MEDITATION);
    assert(success == false);
    null_space_destroy(no_access);

    /* Enter null space on day 100 */
    success = null_space_enter(null_space, 100, NULL_SPACE_MEDITATION);
    assert(success == true);
    assert(null_space->currently_inside == true);
    assert(null_space_is_inside(null_space) == true);
    assert(null_space->current_activity == NULL_SPACE_MEDITATION);
    assert(null_space->entry_day == 100);
    assert(null_space->subjective_hours == 0);

    /* Cannot enter twice */
    success = null_space_enter(null_space, 101, NULL_SPACE_IDLE);
    assert(success == false);

    /* Add subjective time */
    success = null_space_add_time(null_space, 5);
    assert(success == true);
    assert(null_space->subjective_hours == 5);

    success = null_space_add_time(null_space, 10);
    assert(success == true);
    assert(null_space->subjective_hours == 15);

    /* Exit null space */
    success = null_space_exit(null_space, "Meditated deeply");
    assert(success == true);
    assert(null_space->currently_inside == false);
    assert(null_space_is_inside(null_space) == false);
    assert(null_space->visit_count == 1);
    assert(null_space->total_visits == 1);
    assert(null_space->total_subjective_hours == 15);
    assert(null_space->meditation_count == 1);

    /* Verify visit record */
    const NullSpaceVisit* visit = null_space_get_visit(null_space, 0);
    assert(visit != NULL);
    assert(visit->visit_day == 100);
    assert(visit->activity == NULL_SPACE_MEDITATION);
    assert(visit->duration_hours == 15);
    assert(strcmp(visit->notes, "Meditated deeply") == 0);

    /* Cannot exit twice */
    success = null_space_exit(null_space, NULL);
    assert(success == false);

    null_space_destroy(null_space);

    printf("PASS\n");
}

void test_null_space_activity_change(void) {
    printf("Test: null_space_activity_change... ");

    NullSpaceState* null_space = null_space_create();
    null_space_discover(null_space, 10);
    null_space_unlock_access(null_space, 20);

    /* Enter with one activity */
    null_space_enter(null_space, 50, NULL_SPACE_IDLE);
    assert(null_space->current_activity == NULL_SPACE_IDLE);

    /* Change to meeting Thessara */
    bool success = null_space_change_activity(null_space, NULL_SPACE_MEETING_THESSARA);
    assert(success == true);
    assert(null_space->current_activity == NULL_SPACE_MEETING_THESSARA);

    /* Change to network debug */
    success = null_space_change_activity(null_space, NULL_SPACE_NETWORK_DEBUG);
    assert(success == true);
    assert(null_space->current_activity == NULL_SPACE_NETWORK_DEBUG);

    null_space_exit(null_space, NULL);
    assert(null_space->network_debug_count == 1); /* Based on exit activity */

    /* Cannot change activity when not inside */
    success = null_space_change_activity(null_space, NULL_SPACE_MEDITATION);
    assert(success == false);

    null_space_destroy(null_space);

    printf("PASS\n");
}

void test_null_space_statistics(void) {
    printf("Test: null_space_statistics... ");

    NullSpaceState* null_space = null_space_create();
    null_space_discover(null_space, 10);
    null_space_unlock_access(null_space, 20);

    /* Multiple visits with different activities */

    /* Visit 1: Meeting Thessara */
    null_space_enter(null_space, 30, NULL_SPACE_MEETING_THESSARA);
    null_space_add_time(null_space, 3);
    null_space_exit(null_space, "Met Thessara");
    assert(null_space->total_visits == 1);
    assert(null_space->total_subjective_hours == 3);
    assert(null_space->thessara_meetings == 1);

    /* Visit 2: Network debugging */
    null_space_enter(null_space, 40, NULL_SPACE_NETWORK_DEBUG);
    null_space_add_time(null_space, 8);
    null_space_exit(null_space, "Debugged network");
    assert(null_space->total_visits == 2);
    assert(null_space->total_subjective_hours == 11);
    assert(null_space->network_debug_count == 1);

    /* Visit 3: Meditation */
    null_space_enter(null_space, 50, NULL_SPACE_MEDITATION);
    null_space_add_time(null_space, 5);
    null_space_exit(null_space, "Meditated");
    assert(null_space->total_visits == 3);
    assert(null_space->total_subjective_hours == 16);
    assert(null_space->meditation_count == 1);

    /* Visit 4: Another Thessara meeting */
    null_space_enter(null_space, 60, NULL_SPACE_MEETING_THESSARA);
    null_space_add_time(null_space, 2);
    null_space_exit(null_space, "Second meeting");
    assert(null_space->total_visits == 4);
    assert(null_space->total_subjective_hours == 18);
    assert(null_space->thessara_meetings == 2);

    /* Verify visit history */
    assert(null_space->visit_count == 4);

    const NullSpaceVisit* visit1 = null_space_get_visit(null_space, 0);
    assert(visit1->activity == NULL_SPACE_MEETING_THESSARA);
    assert(visit1->duration_hours == 3);

    const NullSpaceVisit* visit2 = null_space_get_visit(null_space, 1);
    assert(visit2->activity == NULL_SPACE_NETWORK_DEBUG);
    assert(visit2->duration_hours == 8);

    const NullSpaceVisit* visit3 = null_space_get_visit(null_space, 2);
    assert(visit3->activity == NULL_SPACE_MEDITATION);
    assert(visit3->duration_hours == 5);

    const NullSpaceVisit* visit4 = null_space_get_visit(null_space, 3);
    assert(visit4->activity == NULL_SPACE_MEETING_THESSARA);
    assert(visit4->duration_hours == 2);

    null_space_destroy(null_space);

    printf("PASS\n");
}

void test_null_space_special_unlocks(void) {
    printf("Test: null_space_special_unlocks... ");

    NullSpaceState* null_space = null_space_create();
    assert(null_space != NULL);

    /* Initial state - no unlocks */
    assert(null_space->archon_workspace == false);
    assert(null_space->wraith_observation == false);
    assert(null_space->network_root_access == false);

    /* Unlock Archon workspace */
    bool success = null_space_unlock_archon_workspace(null_space);
    assert(success == true);
    assert(null_space->archon_workspace == true);

    /* Cannot unlock twice */
    success = null_space_unlock_archon_workspace(null_space);
    assert(success == false);

    /* Unlock Wraith observation */
    success = null_space_unlock_wraith_observation(null_space);
    assert(success == true);
    assert(null_space->wraith_observation == true);

    /* Cannot unlock twice */
    success = null_space_unlock_wraith_observation(null_space);
    assert(success == false);

    /* Unlock network root access */
    success = null_space_unlock_network_root_access(null_space);
    assert(success == true);
    assert(null_space->network_root_access == true);

    /* Cannot unlock twice */
    success = null_space_unlock_network_root_access(null_space);
    assert(success == false);

    null_space_destroy(null_space);

    printf("PASS\n");
}

void test_null_space_activity_names(void) {
    printf("Test: null_space_activity_names... ");

    assert(strcmp(null_space_activity_name(NULL_SPACE_IDLE), "Idle") == 0);
    assert(strcmp(null_space_activity_name(NULL_SPACE_MEETING_THESSARA), "Meeting Thessara") == 0);
    assert(strcmp(null_space_activity_name(NULL_SPACE_NETWORK_DEBUG), "Network Debug") == 0);
    assert(strcmp(null_space_activity_name(NULL_SPACE_MEDITATION), "Meditation") == 0);
    assert(strcmp(null_space_activity_name(NULL_SPACE_RESEARCH), "Research") == 0);
    assert(strcmp(null_space_activity_name(NULL_SPACE_SOUL_ANALYSIS), "Soul Analysis") == 0);

    /* Out of range */
    assert(strcmp(null_space_activity_name((NullSpaceActivity)100), "Unknown") == 0);

    printf("PASS\n");
}

void test_null_space_visit_limit(void) {
    printf("Test: null_space_visit_limit... ");

    NullSpaceState* null_space = null_space_create();
    null_space_discover(null_space, 10);
    null_space_unlock_access(null_space, 20);

    /* Fill up visit history */
    for (size_t i = 0; i < MAX_NULL_SPACE_VISITS; i++) {
        null_space_enter(null_space, 100 + i, NULL_SPACE_IDLE);
        null_space_add_time(null_space, 1);
        null_space_exit(null_space, "Test visit");
    }

    assert(null_space->visit_count == MAX_NULL_SPACE_VISITS);
    assert(null_space->total_visits == MAX_NULL_SPACE_VISITS);

    /* One more visit - should still track statistics but not add to array */
    null_space_enter(null_space, 200, NULL_SPACE_IDLE);
    null_space_add_time(null_space, 1);
    null_space_exit(null_space, "Overflow visit");

    assert(null_space->visit_count == MAX_NULL_SPACE_VISITS); /* Array full */
    assert(null_space->total_visits == MAX_NULL_SPACE_VISITS + 1); /* Stats still update */

    /* Verify all visits are accessible */
    for (size_t i = 0; i < MAX_NULL_SPACE_VISITS; i++) {
        const NullSpaceVisit* visit = null_space_get_visit(null_space, i);
        assert(visit != NULL);
    }

    /* Out of range access */
    const NullSpaceVisit* visit = null_space_get_visit(null_space, MAX_NULL_SPACE_VISITS);
    assert(visit == NULL);

    null_space_destroy(null_space);

    printf("PASS\n");
}

void test_null_space_time_tracking(void) {
    printf("Test: null_space_time_tracking... ");

    NullSpaceState* null_space = null_space_create();
    null_space_discover(null_space, 10);
    null_space_unlock_access(null_space, 20);

    /* Enter null space */
    null_space_enter(null_space, 50, NULL_SPACE_MEDITATION);
    assert(null_space->subjective_hours == 0);

    /* Add time incrementally */
    null_space_add_time(null_space, 2);
    assert(null_space->subjective_hours == 2);

    null_space_add_time(null_space, 3);
    assert(null_space->subjective_hours == 5);

    null_space_add_time(null_space, 10);
    assert(null_space->subjective_hours == 15);

    /* Exit and verify total */
    null_space_exit(null_space, NULL);
    assert(null_space->total_subjective_hours == 15);
    assert(null_space->subjective_hours == 0); /* Cleared on exit */

    /* Cannot add time when not inside */
    bool success = null_space_add_time(null_space, 5);
    assert(success == false);
    assert(null_space->subjective_hours == 0);

    null_space_destroy(null_space);

    printf("PASS\n");
}

void test_null_space_null_safety(void) {
    printf("Test: null_space_null_safety... ");

    /* All functions should handle NULL gracefully */
    assert(null_space_discover(NULL, 10) == false);
    assert(null_space_unlock_access(NULL, 10) == false);
    assert(null_space_enter(NULL, 10, NULL_SPACE_IDLE) == false);
    assert(null_space_exit(NULL, NULL) == false);
    assert(null_space_change_activity(NULL, NULL_SPACE_MEDITATION) == false);
    assert(null_space_add_time(NULL, 5) == false);
    assert(null_space_unlock_archon_workspace(NULL) == false);
    assert(null_space_unlock_wraith_observation(NULL) == false);
    assert(null_space_unlock_network_root_access(NULL) == false);
    assert(null_space_is_inside(NULL) == false);
    assert(null_space_has_access(NULL) == false);
    assert(null_space_get_visit(NULL, 0) == NULL);

    /* Destroy should handle NULL */
    null_space_destroy(NULL); /* Should not crash */

    printf("PASS\n");
}

int main(void) {
    /* Suppress log output during tests */
    logger_set_level(LOG_LEVEL_FATAL + 1);

    printf("\n=== Null Space Location System Tests ===\n\n");

    test_null_space_creation();
    test_null_space_discovery();
    test_null_space_access_unlock();
    test_null_space_enter_exit();
    test_null_space_activity_change();
    test_null_space_statistics();
    test_null_space_special_unlocks();
    test_null_space_activity_names();
    test_null_space_visit_limit();
    test_null_space_time_tracking();
    test_null_space_null_safety();

    printf("\n=== All Null Space Tests Passed! ===\n\n");

    return 0;
}

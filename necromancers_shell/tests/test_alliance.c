#include "../src/game/narrative/alliances/alliance.h"
#include "../src/game/narrative/alliances/alliance_manager.h"
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

/* Test: Create and destroy alliance */
static void test_alliance_create_destroy(void) {
    TEST("alliance_create_destroy");

    Alliance* alliance = alliance_create("vorgath", ALLIANCE_NEUTRAL);
    assert(alliance != NULL);
    assert(strcmp(alliance->npc_id, "vorgath") == 0);
    assert(alliance->type == ALLIANCE_NEUTRAL);
    assert(alliance->phylactery_oath == false);
    assert(alliance->knowledge_shared == 0);
    assert(alliance->resources_shared == 0);
    assert(alliance->trust_level == 0.0f);
    assert(alliance->days_since_formed == 0);
    assert(alliance->interactions == 0);

    alliance_destroy(alliance);
    PASS();
}

/* Test: Upgrade alliance */
static void test_alliance_upgrade(void) {
    TEST("alliance_upgrade");

    Alliance* alliance = alliance_create("seraphine", ALLIANCE_NEUTRAL);
    assert(alliance != NULL);

    /* Set trust high enough for upgrades */
    alliance_modify_trust(alliance, 80.0f);

    /* Upgrade from NEUTRAL to NON_AGGRESSION */
    bool result = alliance_upgrade(alliance);
    assert(result == true);
    assert(alliance->type == ALLIANCE_NON_AGGRESSION);

    /* Upgrade to INFO_EXCHANGE */
    result = alliance_upgrade(alliance);
    assert(result == true);
    assert(alliance->type == ALLIANCE_INFO_EXCHANGE);

    /* Upgrade to FULL */
    result = alliance_upgrade(alliance);
    assert(result == true);
    assert(alliance->type == ALLIANCE_FULL);

    /* Cannot upgrade beyond FULL */
    result = alliance_upgrade(alliance);
    assert(result == false);
    assert(alliance->type == ALLIANCE_FULL);

    alliance_destroy(alliance);
    PASS();
}

/* Test: Downgrade alliance */
static void test_alliance_downgrade(void) {
    TEST("alliance_downgrade");

    Alliance* alliance = alliance_create("mordak", ALLIANCE_FULL);
    assert(alliance != NULL);

    /* Downgrade from FULL to INFO_EXCHANGE */
    bool result = alliance_downgrade(alliance);
    assert(result == true);
    assert(alliance->type == ALLIANCE_INFO_EXCHANGE);

    /* Downgrade to NON_AGGRESSION */
    result = alliance_downgrade(alliance);
    assert(result == true);
    assert(alliance->type == ALLIANCE_NON_AGGRESSION);

    /* Downgrade to NEUTRAL */
    result = alliance_downgrade(alliance);
    assert(result == true);
    assert(alliance->type == ALLIANCE_NEUTRAL);

    /* Downgrade to HOSTILE */
    result = alliance_downgrade(alliance);
    assert(result == true);
    assert(alliance->type == ALLIANCE_HOSTILE);

    /* Cannot downgrade beyond HOSTILE */
    result = alliance_downgrade(alliance);
    assert(result == false);
    assert(alliance->type == ALLIANCE_HOSTILE);

    alliance_destroy(alliance);
    PASS();
}

/* Test: Phylactery oath prevents downgrade */
static void test_phylactery_oath(void) {
    TEST("phylactery_oath_prevents_downgrade");

    Alliance* alliance = alliance_create("vorgath", ALLIANCE_FULL);
    assert(alliance != NULL);

    /* Bind with phylactery oath */
    bool result = alliance_bind_phylactery_oath(alliance);
    assert(result == true);
    assert(alliance->phylactery_oath == true);

    /* Cannot bind again */
    result = alliance_bind_phylactery_oath(alliance);
    assert(result == false);

    /* Oath prevents downgrade */
    result = alliance_downgrade(alliance);
    assert(result == false);
    assert(alliance->type == ALLIANCE_FULL);

    /* Oath prevents set_type to lower */
    result = alliance_set_type(alliance, ALLIANCE_NEUTRAL);
    assert(result == false);
    assert(alliance->type == ALLIANCE_FULL);

    /* Can still set to same or higher (though FULL is max) */
    result = alliance_set_type(alliance, ALLIANCE_FULL);
    assert(result == true);

    alliance_destroy(alliance);
    PASS();
}

/* Test: Knowledge sharing increases trust */
static void test_knowledge_sharing(void) {
    TEST("knowledge_sharing_increases_trust");

    Alliance* alliance = alliance_create("seraphine", ALLIANCE_INFO_EXCHANGE);
    assert(alliance != NULL);

    float initial_trust = alliance->trust_level;

    /* Share knowledge */
    bool result = alliance_add_knowledge(alliance, 5);
    assert(result == true);
    assert(alliance->knowledge_shared == 5);
    assert(alliance->trust_level > initial_trust);

    /* Share more knowledge */
    result = alliance_add_knowledge(alliance, 3);
    assert(result == true);
    assert(alliance->knowledge_shared == 8);

    alliance_destroy(alliance);
    PASS();
}

/* Test: Resource sharing increases trust */
static void test_resource_sharing(void) {
    TEST("resource_sharing_increases_trust");

    Alliance* alliance = alliance_create("archivist", ALLIANCE_FULL);
    assert(alliance != NULL);

    float initial_trust = alliance->trust_level;

    /* Share resources */
    bool result = alliance_add_resources(alliance, 1000);
    assert(result == true);
    assert(alliance->resources_shared == 1000);
    assert(alliance->trust_level > initial_trust);

    alliance_destroy(alliance);
    PASS();
}

/* Test: Trust modification and clamping */
static void test_trust_modification(void) {
    TEST("trust_modification_and_clamping");

    Alliance* alliance = alliance_create("vorgath", ALLIANCE_NEUTRAL);
    assert(alliance != NULL);

    /* Increase trust */
    alliance_modify_trust(alliance, 50.0f);
    assert(alliance->trust_level == 50.0f);

    /* Increase beyond 100 - should clamp */
    alliance_modify_trust(alliance, 60.0f);
    assert(alliance->trust_level == 100.0f);

    /* Decrease trust */
    alliance_modify_trust(alliance, -30.0f);
    assert(alliance->trust_level == 70.0f);

    /* Decrease below 0 - should clamp */
    alliance_modify_trust(alliance, -100.0f);
    assert(alliance->trust_level == 0.0f);

    alliance_destroy(alliance);
    PASS();
}

/* Test: Can upgrade checks trust requirements */
static void test_can_upgrade_trust_requirements(void) {
    TEST("can_upgrade_checks_trust_requirements");

    Alliance* alliance = alliance_create("seraphine", ALLIANCE_NEUTRAL);
    assert(alliance != NULL);

    /* Cannot upgrade to NON_AGGRESSION without 20+ trust */
    alliance->trust_level = 10.0f;
    assert(alliance_can_upgrade(alliance) == false);

    /* Can upgrade with 20+ trust */
    alliance->trust_level = 25.0f;
    assert(alliance_can_upgrade(alliance) == true);

    /* Upgrade to NON_AGGRESSION */
    alliance_upgrade(alliance);
    assert(alliance->type == ALLIANCE_NON_AGGRESSION);

    /* Cannot upgrade to INFO_EXCHANGE without 40+ trust */
    alliance->trust_level = 30.0f;
    assert(alliance_can_upgrade(alliance) == false);

    /* Can upgrade with 40+ trust */
    alliance->trust_level = 45.0f;
    assert(alliance_can_upgrade(alliance) == true);

    alliance_destroy(alliance);
    PASS();
}

/* Test: Alliance type names */
static void test_alliance_type_names(void) {
    TEST("alliance_type_names");

    assert(strcmp(alliance_type_name(ALLIANCE_HOSTILE), "Hostile") == 0);
    assert(strcmp(alliance_type_name(ALLIANCE_NEUTRAL), "Neutral") == 0);
    assert(strcmp(alliance_type_name(ALLIANCE_NON_AGGRESSION), "Non-Aggression Pact") == 0);
    assert(strcmp(alliance_type_name(ALLIANCE_INFO_EXCHANGE), "Information Exchange") == 0);
    assert(strcmp(alliance_type_name(ALLIANCE_FULL), "Full Alliance") == 0);

    PASS();
}

/* Test: Alliance supports coordination */
static void test_supports_coordination(void) {
    TEST("alliance_supports_coordination");

    assert(alliance_supports_coordination(ALLIANCE_HOSTILE) == false);
    assert(alliance_supports_coordination(ALLIANCE_NEUTRAL) == false);
    assert(alliance_supports_coordination(ALLIANCE_NON_AGGRESSION) == false);
    assert(alliance_supports_coordination(ALLIANCE_INFO_EXCHANGE) == true);
    assert(alliance_supports_coordination(ALLIANCE_FULL) == true);

    PASS();
}

/* Test: Alliance manager create/destroy */
static void test_alliance_manager_create_destroy(void) {
    TEST("alliance_manager_create_destroy");

    AllianceManager* manager = alliance_manager_create();
    assert(manager != NULL);
    assert(manager->alliance_count == 0);

    alliance_manager_destroy(manager);
    PASS();
}

/* Test: Alliance manager add/remove */
static void test_alliance_manager_add_remove(void) {
    TEST("alliance_manager_add_remove");

    AllianceManager* manager = alliance_manager_create();
    assert(manager != NULL);

    /* Add alliance */
    bool result = alliance_manager_add(manager, "vorgath", ALLIANCE_NEUTRAL);
    assert(result == true);
    assert(manager->alliance_count == 1);

    /* Cannot add duplicate */
    result = alliance_manager_add(manager, "vorgath", ALLIANCE_FULL);
    assert(result == false);
    assert(manager->alliance_count == 1);

    /* Add another */
    result = alliance_manager_add(manager, "seraphine", ALLIANCE_INFO_EXCHANGE);
    assert(result == true);
    assert(manager->alliance_count == 2);

    /* Remove */
    result = alliance_manager_remove(manager, "vorgath");
    assert(result == true);
    assert(manager->alliance_count == 1);

    /* Cannot remove non-existent */
    result = alliance_manager_remove(manager, "vorgath");
    assert(result == false);

    alliance_manager_destroy(manager);
    PASS();
}

/* Test: Alliance manager find */
static void test_alliance_manager_find(void) {
    TEST("alliance_manager_find");

    AllianceManager* manager = alliance_manager_create();
    assert(manager != NULL);

    alliance_manager_add(manager, "vorgath", ALLIANCE_NEUTRAL);
    alliance_manager_add(manager, "seraphine", ALLIANCE_FULL);
    alliance_manager_add(manager, "mordak", ALLIANCE_HOSTILE);

    /* Find existing */
    Alliance* alliance = alliance_manager_find(manager, "seraphine");
    assert(alliance != NULL);
    assert(strcmp(alliance->npc_id, "seraphine") == 0);
    assert(alliance->type == ALLIANCE_FULL);

    /* Find non-existent */
    alliance = alliance_manager_find(manager, "unknown");
    assert(alliance == NULL);

    alliance_manager_destroy(manager);
    PASS();
}

/* Test: Alliance manager count by type */
static void test_alliance_manager_count_by_type(void) {
    TEST("alliance_manager_count_by_type");

    AllianceManager* manager = alliance_manager_create();
    assert(manager != NULL);

    alliance_manager_add(manager, "vorgath", ALLIANCE_FULL);
    alliance_manager_add(manager, "seraphine", ALLIANCE_FULL);
    alliance_manager_add(manager, "mordak", ALLIANCE_HOSTILE);
    alliance_manager_add(manager, "archivist", ALLIANCE_NEUTRAL);

    assert(alliance_manager_count_by_type(manager, ALLIANCE_HOSTILE) == 1);
    assert(alliance_manager_count_by_type(manager, ALLIANCE_NEUTRAL) == 1);
    assert(alliance_manager_count_by_type(manager, ALLIANCE_FULL) == 2);

    alliance_manager_destroy(manager);
    PASS();
}

/* Test: Alliance manager coordination allies */
static void test_alliance_manager_coordination(void) {
    TEST("alliance_manager_coordination_allies");

    AllianceManager* manager = alliance_manager_create();
    assert(manager != NULL);

    alliance_manager_add(manager, "vorgath", ALLIANCE_FULL);
    alliance_manager_add(manager, "seraphine", ALLIANCE_INFO_EXCHANGE);
    alliance_manager_add(manager, "mordak", ALLIANCE_HOSTILE);
    alliance_manager_add(manager, "archivist", ALLIANCE_NEUTRAL);

    /* INFO_EXCHANGE and FULL support coordination */
    assert(alliance_manager_coordination_allies(manager) == 2);
    assert(alliance_manager_has_coordination(manager) == true);

    alliance_manager_destroy(manager);
    PASS();
}

/* Test: Alliance manager average trust */
static void test_alliance_manager_average_trust(void) {
    TEST("alliance_manager_average_trust");

    AllianceManager* manager = alliance_manager_create();
    assert(manager != NULL);

    alliance_manager_add(manager, "vorgath", ALLIANCE_FULL);
    alliance_manager_add(manager, "seraphine", ALLIANCE_INFO_EXCHANGE);

    /* Set trust levels */
    Alliance* a1 = alliance_manager_find(manager, "vorgath");
    Alliance* a2 = alliance_manager_find(manager, "seraphine");
    alliance_modify_trust(a1, 80.0f);
    alliance_modify_trust(a2, 60.0f);

    /* Average should be 70.0 */
    float avg = alliance_manager_average_trust(manager);
    assert(avg == 70.0f);

    alliance_manager_destroy(manager);
    PASS();
}

/* Test: Alliance manager advance time */
static void test_alliance_manager_advance_time(void) {
    TEST("alliance_manager_advance_time");

    AllianceManager* manager = alliance_manager_create();
    assert(manager != NULL);

    alliance_manager_add(manager, "vorgath", ALLIANCE_FULL);
    alliance_manager_add(manager, "seraphine", ALLIANCE_INFO_EXCHANGE);

    /* Advance time */
    alliance_manager_advance_time(manager, 10);

    Alliance* a1 = alliance_manager_find(manager, "vorgath");
    Alliance* a2 = alliance_manager_find(manager, "seraphine");

    assert(a1->days_since_formed == 10);
    assert(a2->days_since_formed == 10);

    alliance_manager_destroy(manager);
    PASS();
}

int main(void) {
    printf("Running alliance system tests...\n\n");

    /* Alliance tests */
    test_alliance_create_destroy();
    test_alliance_upgrade();
    test_alliance_downgrade();
    test_phylactery_oath();
    test_knowledge_sharing();
    test_resource_sharing();
    test_trust_modification();
    test_can_upgrade_trust_requirements();
    test_alliance_type_names();
    test_supports_coordination();

    /* Alliance manager tests */
    test_alliance_manager_create_destroy();
    test_alliance_manager_add_remove();
    test_alliance_manager_find();
    test_alliance_manager_count_by_type();
    test_alliance_manager_coordination();
    test_alliance_manager_average_trust();
    test_alliance_manager_advance_time();

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

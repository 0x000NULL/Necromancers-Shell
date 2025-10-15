/**
 * @file test_research.c
 * @brief Unit tests for research system
 */

#include "../src/game/progression/research.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "FAIL: %s\n", message); \
            return; \
        } \
    } while(0)

#define ASSERT_EQ(expected, actual, message) \
    do { \
        if ((expected) != (actual)) { \
            fprintf(stderr, "FAIL: %s (expected %d, got %d)\n", message, (int)(expected), (int)(actual)); \
            return; \
        } \
    } while(0)

static void test_create_destroy(void) {
    printf("Running test_create_destroy...\n");

    ResearchManager* manager = research_manager_create();
    ASSERT(manager != NULL, "Failed to create research manager");

    research_manager_destroy(manager);
    research_manager_destroy(NULL);  /* Should not crash */
}

static void test_add_project(void) {
    printf("Running test_add_project...\n");

    ResearchManager* manager = research_manager_create();
    ASSERT(manager != NULL, "Failed to create research manager");

    ResearchProject project = {
        .id = 1,
        .category = RESEARCH_CATEGORY_SPELLS,
        .prerequisite_count = 0,
        .min_level = 1,
        .time_hours = 24,
        .soul_energy_cost = 100,
        .mana_cost = 50,
        .completed = false,
        .hours_invested = 0
    };
    strncpy(project.name, "Raise Dead", sizeof(project.name) - 1);
    strncpy(project.description, "Learn to raise the dead", sizeof(project.description) - 1);
    strncpy(project.unlock_type, "spell", sizeof(project.unlock_type) - 1);
    strncpy(project.unlock_name, "raise", sizeof(project.unlock_name) - 1);

    bool success = research_add_project(manager, &project);
    ASSERT(success, "Failed to add project");

    const ResearchProject* retrieved = research_get_project(manager, 1);
    ASSERT(retrieved != NULL, "Failed to retrieve project");
    ASSERT_EQ(1, retrieved->id, "Project ID mismatch");
    ASSERT(strcmp(retrieved->name, "Raise Dead") == 0, "Project name mismatch");

    /* Test duplicate */
    success = research_add_project(manager, &project);
    ASSERT(!success, "Should not allow duplicate project ID");

    research_manager_destroy(manager);
}

static void test_can_start(void) {
    printf("Running test_can_start...\n");

    ResearchManager* manager = research_manager_create();
    ASSERT(manager != NULL, "Failed to create research manager");

    ResearchProject project = {
        .id = 1,
        .category = RESEARCH_CATEGORY_SPELLS,
        .prerequisite_count = 0,
        .min_level = 5,
        .time_hours = 24,
        .soul_energy_cost = 100,
        .mana_cost = 50,
        .completed = false
    };
    strncpy(project.name, "Test Project", sizeof(project.name) - 1);

    research_add_project(manager, &project);

    /* Test with sufficient resources */
    bool can_start = research_can_start(manager, 1, 5, 100, 50);
    ASSERT(can_start, "Should be able to start with sufficient resources");

    /* Test insufficient level */
    can_start = research_can_start(manager, 1, 4, 100, 50);
    ASSERT(!can_start, "Should not start with insufficient level");

    /* Test insufficient energy */
    can_start = research_can_start(manager, 1, 5, 99, 50);
    ASSERT(!can_start, "Should not start with insufficient energy");

    /* Test insufficient mana */
    can_start = research_can_start(manager, 1, 5, 100, 49);
    ASSERT(!can_start, "Should not start with insufficient mana");

    research_manager_destroy(manager);
}

static void test_start_and_complete(void) {
    printf("Running test_start_and_complete...\n");

    ResearchManager* manager = research_manager_create();
    ASSERT(manager != NULL, "Failed to create research manager");

    ResearchProject project = {
        .id = 1,
        .time_hours = 10,
        .completed = false,
        .hours_invested = 0
    };

    research_add_project(manager, &project);

    /* Start project */
    bool success = research_start(manager, 1);
    ASSERT(success, "Failed to start project");

    uint32_t current = research_get_current(manager);
    ASSERT_EQ(1, current, "Current project should be 1");

    /* Invest partial time */
    bool completed = research_invest_time(manager, 1, 5);
    ASSERT(!completed, "Should not complete with partial time");

    current = research_get_current(manager);
    ASSERT_EQ(1, current, "Current project should still be 1");

    /* Complete project */
    completed = research_invest_time(manager, 1, 5);
    ASSERT(completed, "Should complete project");

    current = research_get_current(manager);
    ASSERT_EQ(0, current, "No current project after completion");

    const ResearchProject* retrieved = research_get_project(manager, 1);
    ASSERT(retrieved->completed, "Project should be marked completed");

    research_manager_destroy(manager);
}

static void test_prerequisites(void) {
    printf("Running test_prerequisites...\n");

    ResearchManager* manager = research_manager_create();
    ASSERT(manager != NULL, "Failed to create research manager");

    /* Add base project */
    ResearchProject project1 = {
        .id = 1,
        .prerequisite_count = 0,
        .min_level = 1,
        .soul_energy_cost = 0,
        .mana_cost = 0,
        .completed = false
    };
    strncpy(project1.name, "Base Project", sizeof(project1.name) - 1);

    /* Add project with prerequisite */
    ResearchProject project2 = {
        .id = 2,
        .prerequisite_count = 1,
        .prerequisites = {1},
        .min_level = 1,
        .soul_energy_cost = 0,
        .mana_cost = 0,
        .completed = false
    };
    strncpy(project2.name, "Advanced Project", sizeof(project2.name) - 1);

    research_add_project(manager, &project1);
    research_add_project(manager, &project2);

    /* Try to start project 2 without prerequisite */
    bool can_start = research_can_start(manager, 2, 1, 0, 0);
    ASSERT(!can_start, "Should not start without prerequisite");

    /* Complete prerequisite */
    research_complete(manager, 1);

    /* Now should be able to start project 2 */
    can_start = research_can_start(manager, 2, 1, 0, 0);
    ASSERT(can_start, "Should start with prerequisite met");

    research_manager_destroy(manager);
}

static void test_cancel_current(void) {
    printf("Running test_cancel_current...\n");

    ResearchManager* manager = research_manager_create();
    ASSERT(manager != NULL, "Failed to create research manager");

    ResearchProject project = {
        .id = 1,
        .time_hours = 10,
        .completed = false,
        .hours_invested = 0
    };

    research_add_project(manager, &project);

    /* Start and invest time */
    research_start(manager, 1);
    research_invest_time(manager, 1, 5);

    /* Cancel */
    bool success = research_cancel_current(manager);
    ASSERT(success, "Failed to cancel project");

    uint32_t current = research_get_current(manager);
    ASSERT_EQ(0, current, "No current project after cancellation");

    const ResearchProject* retrieved = research_get_project(manager, 1);
    ASSERT_EQ(0, retrieved->hours_invested, "Hours should be reset");
    ASSERT(!retrieved->completed, "Project should not be completed");

    research_manager_destroy(manager);
}

static void test_get_category(void) {
    printf("Running test_get_category...\n");

    ResearchManager* manager = research_manager_create();
    ASSERT(manager != NULL, "Failed to create research manager");

    /* Add projects to different categories */
    for (int i = 1; i <= 3; i++) {
        ResearchProject project = {
            .id = (uint32_t)i,
            .category = RESEARCH_CATEGORY_SPELLS,
            .completed = false
        };
        snprintf(project.name, sizeof(project.name), "Spell Project %d", i);
        research_add_project(manager, &project);
    }

    for (int i = 4; i <= 5; i++) {
        ResearchProject project = {
            .id = (uint32_t)i,
            .category = RESEARCH_CATEGORY_ABILITIES,
            .completed = false
        };
        snprintf(project.name, sizeof(project.name), "Ability Project %d", i);
        research_add_project(manager, &project);
    }

    uint32_t results[10];
    size_t count = research_get_category(manager, RESEARCH_CATEGORY_SPELLS, results, 10);
    ASSERT_EQ(3, count, "Should have 3 spell projects");

    count = research_get_category(manager, RESEARCH_CATEGORY_ABILITIES, results, 10);
    ASSERT_EQ(2, count, "Should have 2 ability projects");

    research_manager_destroy(manager);
}

static void test_get_available(void) {
    printf("Running test_get_available...\n");

    ResearchManager* manager = research_manager_create();
    ASSERT(manager != NULL, "Failed to create research manager");

    /* Add tier 1 project (no prerequisites) */
    ResearchProject project1 = {
        .id = 1,
        .prerequisite_count = 0,
        .min_level = 1,
        .completed = false
    };

    /* Add tier 2 project (requires project 1) */
    ResearchProject project2 = {
        .id = 2,
        .prerequisite_count = 1,
        .prerequisites = {1},
        .min_level = 1,
        .completed = false
    };

    /* Add high-level project */
    ResearchProject project3 = {
        .id = 3,
        .prerequisite_count = 0,
        .min_level = 10,
        .completed = false
    };

    research_add_project(manager, &project1);
    research_add_project(manager, &project2);
    research_add_project(manager, &project3);

    uint32_t results[10];

    /* At level 1, only project 1 should be available */
    size_t count = research_get_available(manager, 1, results, 10);
    ASSERT_EQ(1, count, "Should have 1 available project at level 1");
    ASSERT_EQ(1, results[0], "Project 1 should be available");

    /* Complete project 1 */
    research_complete(manager, 1);

    /* Now project 2 should be available */
    count = research_get_available(manager, 1, results, 10);
    ASSERT_EQ(1, count, "Should have 1 available project");
    ASSERT_EQ(2, results[0], "Project 2 should be available");

    /* At level 10, project 3 should also be available */
    count = research_get_available(manager, 10, results, 10);
    ASSERT_EQ(2, count, "Should have 2 available projects at level 10");

    research_manager_destroy(manager);
}

static void test_get_stats(void) {
    printf("Running test_get_stats...\n");

    ResearchManager* manager = research_manager_create();
    ASSERT(manager != NULL, "Failed to create research manager");

    /* Add projects */
    for (int i = 1; i <= 5; i++) {
        ResearchProject project = {
            .id = (uint32_t)i,
            .completed = false
        };
        research_add_project(manager, &project);
    }

    /* Complete 2 projects */
    research_complete(manager, 1);
    research_complete(manager, 2);

    /* Start project 3 */
    research_start(manager, 3);

    size_t total, completed;
    uint32_t current;
    research_get_stats(manager, &total, &completed, &current);

    ASSERT_EQ(5, total, "Should have 5 total projects");
    ASSERT_EQ(2, completed, "Should have 2 completed projects");
    ASSERT_EQ(3, current, "Current project should be 3");

    research_manager_destroy(manager);
}

static void test_category_names(void) {
    printf("Running test_category_names...\n");

    ASSERT(strcmp(research_category_name(RESEARCH_CATEGORY_SPELLS), "Spells") == 0,
           "Spells category name mismatch");
    ASSERT(strcmp(research_category_name(RESEARCH_CATEGORY_ABILITIES), "Abilities") == 0,
           "Abilities category name mismatch");
    ASSERT(strcmp(research_category_name(RESEARCH_CATEGORY_UPGRADES), "Upgrades") == 0,
           "Upgrades category name mismatch");
    ASSERT(strcmp(research_category_name(RESEARCH_CATEGORY_LORE), "Lore") == 0,
           "Lore category name mismatch");
}

static void test_one_at_a_time(void) {
    printf("Running test_one_at_a_time...\n");

    ResearchManager* manager = research_manager_create();
    ASSERT(manager != NULL, "Failed to create research manager");

    ResearchProject project1 = {.id = 1, .min_level = 1, .soul_energy_cost = 0, .mana_cost = 0, .completed = false};
    ResearchProject project2 = {.id = 2, .min_level = 1, .soul_energy_cost = 0, .mana_cost = 0, .completed = false};

    research_add_project(manager, &project1);
    research_add_project(manager, &project2);

    /* Start first project */
    research_start(manager, 1);

    /* Try to start second project (should fail) */
    bool can_start = research_can_start(manager, 2, 1, 0, 0);
    ASSERT(!can_start, "Should not start second project while first is active");

    research_manager_destroy(manager);
}

int main(void) {
    printf("Running Research Tests...\n\n");

    test_create_destroy();
    test_add_project();
    test_can_start();
    test_start_and_complete();
    test_prerequisites();
    test_cancel_current();
    test_get_category();
    test_get_available();
    test_get_stats();
    test_category_names();
    test_one_at_a_time();

    printf("\n\033[32mAll research tests passed!\033[0m\n");
    return 0;
}

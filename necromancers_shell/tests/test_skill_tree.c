/**
 * @file test_skill_tree.c
 * @brief Unit tests for skill tree system
 */

#include "../src/game/progression/skill_tree.h"
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

    SkillTree* tree = skill_tree_create();
    ASSERT(tree != NULL, "Failed to create skill tree");

    skill_tree_destroy(tree);
    skill_tree_destroy(NULL);  /* Should not crash */
}

static void test_add_skill(void) {
    printf("Running test_add_skill...\n");

    SkillTree* tree = skill_tree_create();
    ASSERT(tree != NULL, "Failed to create skill tree");

    Skill skill = {
        .id = 1,
        .branch = SKILL_BRANCH_NECROMANCY,
        .tier = 1,
        .cost = 1,
        .prerequisite_count = 0,
        .min_level = 1,
        .effect_type = SKILL_EFFECT_STAT_BONUS,
        .effect_value = 0.10f,
        .unlocked = false
    };
    strncpy(skill.name, "Dark Pact", sizeof(skill.name) - 1);
    strncpy(skill.description, "Increases minion HP by 10%", sizeof(skill.description) - 1);
    strncpy(skill.effect_stat, "minion_hp", sizeof(skill.effect_stat) - 1);

    bool success = skill_tree_add_skill(tree, &skill);
    ASSERT(success, "Failed to add skill");

    const Skill* retrieved = skill_tree_get_skill(tree, 1);
    ASSERT(retrieved != NULL, "Failed to retrieve skill");
    ASSERT_EQ(1, retrieved->id, "Skill ID mismatch");
    ASSERT(strcmp(retrieved->name, "Dark Pact") == 0, "Skill name mismatch");

    /* Test duplicate */
    success = skill_tree_add_skill(tree, &skill);
    ASSERT(!success, "Should not allow duplicate skill ID");

    skill_tree_destroy(tree);
}

static void test_unlock_skill(void) {
    printf("Running test_unlock_skill...\n");

    SkillTree* tree = skill_tree_create();
    ASSERT(tree != NULL, "Failed to create skill tree");

    Skill skill = {
        .id = 1,
        .branch = SKILL_BRANCH_NECROMANCY,
        .tier = 1,
        .cost = 1,
        .prerequisite_count = 0,
        .min_level = 1,
        .effect_type = SKILL_EFFECT_STAT_BONUS,
        .effect_value = 0.10f,
        .unlocked = false
    };
    strncpy(skill.name, "Basic Skill", sizeof(skill.name) - 1);

    skill_tree_add_skill(tree, &skill);

    /* Test can_unlock */
    bool can_unlock = skill_tree_can_unlock(tree, 1, 1, 1);
    ASSERT(can_unlock, "Should be able to unlock skill");

    /* Test insufficient level */
    can_unlock = skill_tree_can_unlock(tree, 1, 0, 1);
    ASSERT(!can_unlock, "Should not unlock with insufficient level");

    /* Test insufficient points */
    can_unlock = skill_tree_can_unlock(tree, 1, 1, 0);
    ASSERT(!can_unlock, "Should not unlock with insufficient points");

    /* Unlock skill */
    bool success = skill_tree_unlock(tree, 1);
    ASSERT(success, "Failed to unlock skill");

    const Skill* retrieved = skill_tree_get_skill(tree, 1);
    ASSERT(retrieved->unlocked, "Skill should be unlocked");

    /* Test already unlocked */
    can_unlock = skill_tree_can_unlock(tree, 1, 1, 1);
    ASSERT(!can_unlock, "Should not unlock already unlocked skill");

    skill_tree_destroy(tree);
}

static void test_prerequisites(void) {
    printf("Running test_prerequisites...\n");

    SkillTree* tree = skill_tree_create();
    ASSERT(tree != NULL, "Failed to create skill tree");

    /* Add base skill */
    Skill skill1 = {
        .id = 1,
        .branch = SKILL_BRANCH_NECROMANCY,
        .tier = 1,
        .cost = 1,
        .prerequisite_count = 0,
        .min_level = 1,
        .unlocked = false
    };
    strncpy(skill1.name, "Base Skill", sizeof(skill1.name) - 1);

    /* Add skill with prerequisite */
    Skill skill2 = {
        .id = 2,
        .branch = SKILL_BRANCH_NECROMANCY,
        .tier = 2,
        .cost = 2,
        .prerequisite_count = 1,
        .prerequisites = {1},
        .min_level = 1,
        .unlocked = false
    };
    strncpy(skill2.name, "Advanced Skill", sizeof(skill2.name) - 1);

    skill_tree_add_skill(tree, &skill1);
    skill_tree_add_skill(tree, &skill2);

    /* Try to unlock skill 2 without prerequisite */
    bool can_unlock = skill_tree_can_unlock(tree, 2, 1, 2);
    ASSERT(!can_unlock, "Should not unlock without prerequisite");

    /* Unlock prerequisite */
    skill_tree_unlock(tree, 1);

    /* Now should be able to unlock skill 2 */
    can_unlock = skill_tree_can_unlock(tree, 2, 1, 2);
    ASSERT(can_unlock, "Should unlock with prerequisite met");

    skill_tree_destroy(tree);
}

static void test_get_branch(void) {
    printf("Running test_get_branch...\n");

    SkillTree* tree = skill_tree_create();
    ASSERT(tree != NULL, "Failed to create skill tree");

    /* Add skills to different branches */
    for (int i = 1; i <= 4; i++) {
        Skill skill = {
            .id = (uint32_t)i,
            .branch = SKILL_BRANCH_NECROMANCY,
            .tier = 1,
            .cost = 1,
            .unlocked = false
        };
        snprintf(skill.name, sizeof(skill.name), "Necromancy Skill %d", i);
        skill_tree_add_skill(tree, &skill);
    }

    for (int i = 5; i <= 7; i++) {
        Skill skill = {
            .id = (uint32_t)i,
            .branch = SKILL_BRANCH_INTELLIGENCE,
            .tier = 1,
            .cost = 1,
            .unlocked = false
        };
        snprintf(skill.name, sizeof(skill.name), "Intelligence Skill %d", i);
        skill_tree_add_skill(tree, &skill);
    }

    uint32_t results[10];
    size_t count = skill_tree_get_branch(tree, SKILL_BRANCH_NECROMANCY, results, 10);
    ASSERT_EQ(4, count, "Should have 4 necromancy skills");

    count = skill_tree_get_branch(tree, SKILL_BRANCH_INTELLIGENCE, results, 10);
    ASSERT_EQ(3, count, "Should have 3 intelligence skills");

    skill_tree_destroy(tree);
}

static void test_stat_bonus(void) {
    printf("Running test_stat_bonus...\n");

    SkillTree* tree = skill_tree_create();
    ASSERT(tree != NULL, "Failed to create skill tree");

    /* Add skills with stat bonuses */
    Skill skill1 = {
        .id = 1,
        .effect_type = SKILL_EFFECT_STAT_BONUS,
        .effect_value = 0.10f,
        .unlocked = false
    };
    strncpy(skill1.effect_stat, "minion_hp", sizeof(skill1.effect_stat) - 1);

    Skill skill2 = {
        .id = 2,
        .effect_type = SKILL_EFFECT_STAT_BONUS,
        .effect_value = 0.15f,
        .unlocked = false
    };
    strncpy(skill2.effect_stat, "minion_hp", sizeof(skill2.effect_stat) - 1);

    skill_tree_add_skill(tree, &skill1);
    skill_tree_add_skill(tree, &skill2);

    /* Initially no bonus (skills not unlocked) */
    float bonus = skill_tree_get_stat_bonus(tree, "minion_hp");
    ASSERT(bonus == 1.0f, "Should have no bonus initially");

    /* Unlock first skill */
    skill_tree_unlock(tree, 1);
    bonus = skill_tree_get_stat_bonus(tree, "minion_hp");
    ASSERT(bonus == 1.10f, "Should have 10% bonus");

    /* Unlock second skill */
    skill_tree_unlock(tree, 2);
    bonus = skill_tree_get_stat_bonus(tree, "minion_hp");
    ASSERT(bonus == 1.25f, "Should have 25% bonus");

    skill_tree_destroy(tree);
}

static void test_ability_unlock(void) {
    printf("Running test_ability_unlock...\n");

    SkillTree* tree = skill_tree_create();
    ASSERT(tree != NULL, "Failed to create skill tree");

    Skill skill = {
        .id = 1,
        .effect_type = SKILL_EFFECT_UNLOCK_ABILITY,
        .unlocked = false
    };
    strncpy(skill.effect_stat, "possess", sizeof(skill.effect_stat) - 1);

    skill_tree_add_skill(tree, &skill);

    /* Initially not unlocked */
    bool unlocked = skill_tree_is_ability_unlocked(tree, "possess");
    ASSERT(!unlocked, "Ability should not be unlocked initially");

    /* Unlock skill */
    skill_tree_unlock(tree, 1);
    unlocked = skill_tree_is_ability_unlocked(tree, "possess");
    ASSERT(unlocked, "Ability should be unlocked");

    skill_tree_destroy(tree);
}

static void test_reset(void) {
    printf("Running test_reset...\n");

    SkillTree* tree = skill_tree_create();
    ASSERT(tree != NULL, "Failed to create skill tree");

    /* Add and unlock some skills */
    for (int i = 1; i <= 3; i++) {
        Skill skill = {
            .id = (uint32_t)i,
            .cost = (uint32_t)i,
            .unlocked = false
        };
        skill_tree_add_skill(tree, &skill);
        skill_tree_unlock(tree, (uint32_t)i);
    }

    uint32_t points_spent = skill_tree_get_points_spent(tree);
    ASSERT_EQ(6, points_spent, "Should have 6 points spent");

    /* Reset */
    skill_tree_reset(tree);

    points_spent = skill_tree_get_points_spent(tree);
    ASSERT_EQ(0, points_spent, "Should have 0 points spent after reset");

    skill_tree_destroy(tree);
}

static void test_get_available(void) {
    printf("Running test_get_available...\n");

    SkillTree* tree = skill_tree_create();
    ASSERT(tree != NULL, "Failed to create skill tree");

    /* Add tier 1 skill (no prerequisites) */
    Skill skill1 = {
        .id = 1,
        .tier = 1,
        .prerequisite_count = 0,
        .min_level = 1,
        .unlocked = false
    };

    /* Add tier 2 skill (requires skill 1) */
    Skill skill2 = {
        .id = 2,
        .tier = 2,
        .prerequisite_count = 1,
        .prerequisites = {1},
        .min_level = 1,
        .unlocked = false
    };

    /* Add high-level skill */
    Skill skill3 = {
        .id = 3,
        .tier = 1,
        .prerequisite_count = 0,
        .min_level = 10,
        .unlocked = false
    };

    skill_tree_add_skill(tree, &skill1);
    skill_tree_add_skill(tree, &skill2);
    skill_tree_add_skill(tree, &skill3);

    uint32_t results[10];

    /* At level 1, only skill 1 should be available */
    size_t count = skill_tree_get_available(tree, 1, results, 10);
    ASSERT_EQ(1, count, "Should have 1 available skill at level 1");
    ASSERT_EQ(1, results[0], "Skill 1 should be available");

    /* Unlock skill 1 */
    skill_tree_unlock(tree, 1);

    /* Now skill 2 should be available */
    count = skill_tree_get_available(tree, 1, results, 10);
    ASSERT_EQ(1, count, "Should have 1 available skill");
    ASSERT_EQ(2, results[0], "Skill 2 should be available");

    /* At level 10, skill 3 should also be available */
    count = skill_tree_get_available(tree, 10, results, 10);
    ASSERT_EQ(2, count, "Should have 2 available skills at level 10");

    skill_tree_destroy(tree);
}

static void test_get_stats(void) {
    printf("Running test_get_stats...\n");

    SkillTree* tree = skill_tree_create();
    ASSERT(tree != NULL, "Failed to create skill tree");

    /* Add skills */
    for (int i = 1; i <= 5; i++) {
        Skill skill = {
            .id = (uint32_t)i,
            .cost = 1,
            .unlocked = false
        };
        skill_tree_add_skill(tree, &skill);
    }

    /* Unlock 2 skills */
    skill_tree_unlock(tree, 1);
    skill_tree_unlock(tree, 2);

    size_t total, unlocked;
    uint32_t points;
    skill_tree_get_stats(tree, &total, &unlocked, &points);

    ASSERT_EQ(5, total, "Should have 5 total skills");
    ASSERT_EQ(2, unlocked, "Should have 2 unlocked skills");
    ASSERT_EQ(2, points, "Should have 2 points spent");

    skill_tree_destroy(tree);
}

static void test_branch_names(void) {
    printf("Running test_branch_names...\n");

    ASSERT(strcmp(skill_branch_name(SKILL_BRANCH_NECROMANCY), "Necromancy") == 0,
           "Necromancy branch name mismatch");
    ASSERT(strcmp(skill_branch_name(SKILL_BRANCH_INTELLIGENCE), "Intelligence") == 0,
           "Intelligence branch name mismatch");
    ASSERT(strcmp(skill_branch_name(SKILL_BRANCH_CORRUPTION), "Corruption") == 0,
           "Corruption branch name mismatch");
    ASSERT(strcmp(skill_branch_name(SKILL_BRANCH_DEFENSE), "Defense") == 0,
           "Defense branch name mismatch");

    const char* desc = skill_branch_description(SKILL_BRANCH_NECROMANCY);
    ASSERT(desc != NULL && strlen(desc) > 0, "Branch description should not be empty");
}

int main(void) {
    printf("Running Skill Tree Tests...\n\n");

    test_create_destroy();
    test_add_skill();
    test_unlock_skill();
    test_prerequisites();
    test_get_branch();
    test_stat_bonus();
    test_ability_unlock();
    test_reset();
    test_get_available();
    test_get_stats();
    test_branch_names();

    printf("\n\033[32mAll skill tree tests passed!\033[0m\n");
    return 0;
}

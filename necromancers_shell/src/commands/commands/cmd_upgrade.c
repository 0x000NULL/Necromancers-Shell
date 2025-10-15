/**
 * @file cmd_upgrade.c
 * @brief Upgrade command - Manage skill tree
 */

#include "commands.h"
#include "../../game/game_state.h"
#include "../../game/progression/skill_tree.h"
#include "../../terminal/ui_feedback.h"
#include "../../utils/logger.h"
#include "../parser.h"
#include <stdio.h>
#include <string.h>

CommandResult cmd_upgrade(ParsedCommand* cmd) {
    (void)cmd; /* Suppress unused parameter warning */

    GameState* state = game_state_get_instance();
    if (!state) {
        return command_result_error(EXEC_ERROR_INTERNAL, "Game state not initialized");
    }

    /* TODO: Add skill tree to GameState */
    return command_result_error(EXEC_ERROR_INTERNAL,
                               "Skill tree system not yet integrated into GameState");

    #if 0
    /* Get subcommand */
    const char* subcommand = parsed_command_get_arg(cmd, 0);

    /* Calculate available skill points (placeholder - actual calculation TBD) */
    uint32_t total_points = state->player_level; // 1 point per level
    uint32_t spent_points = skill_tree_get_points_spent(state->skill_tree);
    uint32_t available_points = (total_points > spent_points) ? (total_points - spent_points) : 0;

    /* No args - show overview */
    if (!subcommand) {
        size_t total_skills, unlocked_skills;
        uint32_t points_spent;
        skill_tree_get_stats(state->skill_tree, &total_skills, &unlocked_skills, &points_spent);

        ui_print_success("Skill Tree Overview:");
        printf("\n");
        printf("Total Skills: %zu\n", total_skills);
        printf("Unlocked Skills: %zu\n", unlocked_skills);
        printf("Skill Points: %u spent / %u available\n", points_spent, available_points);
        printf("\n");

        /* Show available skills */
        uint32_t results[50];
        size_t count = skill_tree_get_available(state->skill_tree, state->player_level, results, 50);

        if (count > 0) {
            printf("Available to Unlock (%zu):\n", count);
            for (size_t i = 0; i < count; i++) {
                const Skill* skill = skill_tree_get_skill(state->skill_tree, results[i]);
                if (skill) {
                    display_skill(skill, false);
                }
            }
        }

        printf("\nUse 'upgrade info <id>' for details\n");
        printf("Use 'upgrade unlock <id>' to unlock a skill\n");
        printf("Use 'upgrade branch <name>' to view a skill branch\n");

        return command_result_success("Displayed skill tree overview");
    }

    /* upgrade info <id> */
    if (strcmp(subcommand, "info") == 0) {
        const char* id_str = parsed_command_get_arg(cmd, 1);
        if (!id_str) {
            return command_result_error("Usage: upgrade info <skill_id>");
        }

        uint32_t skill_id = (uint32_t)atoi(id_str);
        const Skill* skill = skill_tree_get_skill(state->skill_tree, skill_id);

        if (!skill) {
            return command_result_error("Skill not found");
        }

        display_skill(skill, true);
        return command_result_success("Displayed skill info");
    }

    /* upgrade unlock <id> */
    if (strcmp(subcommand, "unlock") == 0) {
        const char* id_str = parsed_command_get_arg(cmd, 1);
        if (!id_str) {
            return command_result_error("Usage: upgrade unlock <skill_id>");
        }

        uint32_t skill_id = (uint32_t)atoi(id_str);
        const Skill* skill = skill_tree_get_skill(state->skill_tree, skill_id);

        if (!skill) {
            return command_result_error("Skill not found");
        }

        /* Check if can unlock */
        if (!skill_tree_can_unlock(state->skill_tree, skill_id, state->player_level, available_points)) {
            ui_print_error("Cannot unlock skill - check level, points, and prerequisites");
            return command_result_error("Requirements not met");
        }

        /* Unlock skill */
        if (!skill_tree_unlock(state->skill_tree, skill_id)) {
            return command_result_error("Failed to unlock skill");
        }

        ui_print_success("Skill unlocked: %s", skill->name);
        printf("Remaining points: %u\n", available_points - skill->cost);

        return command_result_success("Unlocked skill");
    }

    /* upgrade branch <name> */
    if (strcmp(subcommand, "branch") == 0) {
        const char* branch_name = parsed_command_get_arg(cmd, 1);
        if (!branch_name) {
            /* List all branches */
            ui_print_success("Skill Branches:");
            printf("\n");
            for (int i = 0; i < SKILL_BRANCH_COUNT; i++) {
                SkillBranch branch = (SkillBranch)i;
                printf("  %s: %s\n", skill_branch_name(branch), skill_branch_description(branch));
            }
            printf("\nUse 'upgrade branch <name>' to view skills in a branch\n");
            return command_result_success("Listed branches");
        }

        /* Find branch by name */
        SkillBranch branch = SKILL_BRANCH_NECROMANCY;
        bool found = false;

        for (int i = 0; i < SKILL_BRANCH_COUNT; i++) {
            if (strcasecmp(branch_name, skill_branch_name((SkillBranch)i)) == 0) {
                branch = (SkillBranch)i;
                found = true;
                break;
            }
        }

        if (!found) {
            return command_result_error("Invalid branch name");
        }

        /* Display skills in branch */
        uint32_t results[50];
        size_t count = skill_tree_get_branch(state->skill_tree, branch, results, 50);

        ui_print_success("%s Skills:", skill_branch_name(branch));
        printf("%s\n\n", skill_branch_description(branch));

        if (count == 0) {
            ui_print_info("No skills in this branch");
            return command_result_success("No skills in branch");
        }

        for (size_t i = 0; i < count; i++) {
            const Skill* skill = skill_tree_get_skill(state->skill_tree, results[i]);
            if (skill) {
                display_skill(skill, false);
            }
        }

        return command_result_success("Displayed branch skills");
    }

    /* upgrade unlocked */
    if (strcmp(subcommand, "unlocked") == 0) {
        uint32_t results[50];
        size_t count = skill_tree_get_unlocked(state->skill_tree, results, 50);

        if (count == 0) {
            ui_print_info("No skills unlocked yet");
            return command_result_success("No unlocked skills");
        }

        ui_print_success("Unlocked Skills:");
        printf("\n");

        for (size_t i = 0; i < count; i++) {
            const Skill* skill = skill_tree_get_skill(state->skill_tree, results[i]);
            if (skill) {
                display_skill(skill, false);
            }
        }

        return command_result_success("Listed unlocked skills");
    }

    /* upgrade reset */
    if (strcmp(subcommand, "reset") == 0) {
        ui_print_warning("Are you sure you want to reset all skills? (This action cannot be undone)");
        ui_print_warning("All skill points will be refunded, but this is a debug command.");

        /* In production, add confirmation prompt */
        skill_tree_reset(state->skill_tree);

        ui_print_success("Skill tree reset - all points refunded");
        return command_result_success("Reset skill tree");
    }

    return command_result_error("Unknown subcommand. Use: info, unlock, branch, unlocked, reset");
    #endif
}

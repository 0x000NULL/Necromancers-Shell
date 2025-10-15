/**
 * @file cmd_skills.c
 * @brief Skills command - View unlocked skills and active bonuses
 */

#include "commands.h"
#include "../../game/game_state.h"
#include "../../game/progression/skill_tree.h"
#include "../../terminal/ui_feedback.h"
#include "../../utils/logger.h"
#include "../parser.h"
#include <stdio.h>
#include <string.h>

CommandResult cmd_skills(ParsedCommand* cmd) {
    (void)cmd; /* Suppress unused parameter warning */

    GameState* state = game_state_get_instance();
    if (!state) {
        return command_result_error(EXEC_ERROR_INTERNAL, "Game state not initialized");
    }

    /* TODO: Add skill tree to GameState */
    return command_result_error(EXEC_ERROR_INTERNAL,
                               "Skill tree system not yet integrated into GameState");

    #if 0
    /* Get optional filter */
    const char* filter = parsed_command_get_arg(cmd, 0);

    /* No args - show all unlocked skills and bonuses */
    if (!filter) {
        uint32_t results[50];
        size_t count = skill_tree_get_unlocked(state->skill_tree, results, 50);

        if (count == 0) {
            ui_print_info("No skills unlocked yet");
            printf("\nUse 'upgrade' command to unlock skills\n");
            return command_result_success("No unlocked skills");
        }

        ui_print_success("Active Skills (%zu):", count);
        printf("\n");

        /* Group by branch */
        for (int branch_idx = 0; branch_idx < SKILL_BRANCH_COUNT; branch_idx++) {
            SkillBranch branch = (SkillBranch)branch_idx;
            bool has_skills = false;

            /* Check if any skills in this branch */
            for (size_t i = 0; i < count; i++) {
                const Skill* skill = skill_tree_get_skill(state->skill_tree, results[i]);
                if (skill && skill->branch == branch) {
                    if (!has_skills) {
                        printf("\033[1m%s:\033[0m\n", skill_branch_name(branch));
                        has_skills = true;
                    }

                    printf("  • %s", skill->name);

                    /* Show effect */
                    switch (skill->effect_type) {
                        case SKILL_EFFECT_STAT_BONUS:
                            printf(" (+%.0f%% %s)", skill->effect_value * 100, skill->effect_stat);
                            break;
                        case SKILL_EFFECT_UNLOCK_ABILITY:
                            printf(" (Unlocked: %s)", skill->effect_stat);
                            break;
                        case SKILL_EFFECT_REDUCE_COST:
                            printf(" (-%.0f%% %s)", skill->effect_value * 100, skill->effect_stat);
                            break;
                        case SKILL_EFFECT_PASSIVE_EFFECT:
                            printf(" (Passive)");
                            break;
                        default:
                            break;
                    }
                    printf("\n");
                }
            }

            if (has_skills) {
                printf("\n");
            }
        }

        /* Show total bonuses */
        printf("\033[1mActive Bonuses:\033[0m\n");

        bool has_bonuses = false;
        for (int i = 0; COMMON_STATS[i] != NULL; i++) {
            const char* stat_name = COMMON_STATS[i];
            float bonus = skill_tree_get_stat_bonus(state->skill_tree, stat_name);

            if (bonus != 1.0f) {
                has_bonuses = true;
                printf("  • %s: %.0f%%\n", stat_name, (bonus - 1.0f) * 100);
            }
        }

        if (!has_bonuses) {
            printf("  (None)\n");
        }

        printf("\n");
        printf("Use 'skills bonuses' to see all stat bonuses\n");
        printf("Use 'skills abilities' to see unlocked abilities\n");

        return command_result_success("Displayed active skills");
    }

    /* skills bonuses - detailed bonus view */
    if (strcmp(filter, "bonuses") == 0) {
        ui_print_success("All Stat Bonuses:");
        printf("\n");

        bool has_bonuses = false;
        for (int i = 0; COMMON_STATS[i] != NULL; i++) {
            const char* stat_name = COMMON_STATS[i];
            float bonus = skill_tree_get_stat_bonus(state->skill_tree, stat_name);

            if (bonus != 1.0f) {
                has_bonuses = true;
                float percent = (bonus - 1.0f) * 100;
                if (percent > 0) {
                    printf("  \033[32m+%.0f%%\033[0m %s (%.2fx multiplier)\n",
                           percent, stat_name, bonus);
                } else {
                    printf("  \033[31m%.0f%%\033[0m %s (%.2fx multiplier)\n",
                           percent, stat_name, bonus);
                }
            }
        }

        if (!has_bonuses) {
            ui_print_info("No stat bonuses active");
        }

        return command_result_success("Displayed stat bonuses");
    }

    /* skills abilities - show unlocked abilities */
    if (strcmp(filter, "abilities") == 0) {
        /* Common abilities to check */
        const char* abilities[] = {
            "possess", "scry", "soulforge", "necromantic_sight",
            "death_pact", "soul_transfer", "mass_raise", NULL
        };

        ui_print_success("Unlocked Abilities:");
        printf("\n");

        bool has_abilities = false;
        for (int i = 0; abilities[i] != NULL; i++) {
            if (skill_tree_is_ability_unlocked(state->skill_tree, abilities[i])) {
                has_abilities = true;
                printf("  \033[32m✓\033[0m %s\n", abilities[i]);
            }
        }

        if (!has_abilities) {
            ui_print_info("No special abilities unlocked yet");
        }

        return command_result_success("Displayed unlocked abilities");
    }

    /* skills branch <name> - filter by branch */
    if (strcmp(filter, "branch") == 0) {
        const char* branch_name = parsed_command_get_arg(cmd, 1);
        if (!branch_name) {
            return command_result_error("Usage: skills branch <branch_name>");
        }

        /* Find branch */
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

        /* Get unlocked skills in branch */
        uint32_t all_unlocked[50];
        size_t total_count = skill_tree_get_unlocked(state->skill_tree, all_unlocked, 50);

        ui_print_success("Active %s Skills:", skill_branch_name(branch));
        printf("\n");

        bool has_skills = false;
        for (size_t i = 0; i < total_count; i++) {
            const Skill* skill = skill_tree_get_skill(state->skill_tree, all_unlocked[i]);
            if (skill && skill->branch == branch) {
                has_skills = true;
                printf("  • %s - %s\n", skill->name, skill->description);
            }
        }

        if (!has_skills) {
            ui_print_info("No %s skills unlocked", skill_branch_name(branch));
        }

        return command_result_success("Displayed branch skills");
    }

    return command_result_error("Unknown filter. Use: bonuses, abilities, or branch <name>");
    #endif
}

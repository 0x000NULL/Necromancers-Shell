/**
 * @file skill_tree.c
 * @brief Implementation of skill tree system
 */

#include "skill_tree.h"
#include "../../data/data_loader.h"
#include "../../utils/logger.h"
#include <stdlib.h>
#include <string.h>

#define MAX_SKILLS 100

/**
 * @brief Skill tree structure
 */
struct SkillTree {
    Skill skills[MAX_SKILLS];       /**< Skill array */
    size_t skill_count;             /**< Number of skills */
};

SkillTree* skill_tree_create(void) {
    SkillTree* tree = calloc(1, sizeof(SkillTree));
    if (!tree) {
        LOG_ERROR("Failed to allocate skill tree");
        return NULL;
    }

    tree->skill_count = 0;
    LOG_DEBUG("Skill tree created");
    return tree;
}

void skill_tree_destroy(SkillTree* tree) {
    if (!tree) {
        return;
    }

    free(tree);
    LOG_DEBUG("Skill tree destroyed");
}

bool skill_tree_add_skill(SkillTree* tree, const Skill* skill) {
    if (!tree || !skill) {
        return false;
    }

    if (tree->skill_count >= MAX_SKILLS) {
        LOG_WARN("Skill tree is full (max %d skills)", MAX_SKILLS);
        return false;
    }

    /* Check for duplicate ID */
    for (size_t i = 0; i < tree->skill_count; i++) {
        if (tree->skills[i].id == skill->id) {
            LOG_WARN("Skill ID %u already exists", skill->id);
            return false;
        }
    }

    /* Copy skill */
    tree->skills[tree->skill_count] = *skill;
    tree->skill_count++;

    LOG_DEBUG("Added skill %u: %s", skill->id, skill->name);
    return true;
}

const Skill* skill_tree_get_skill(const SkillTree* tree, uint32_t skill_id) {
    if (!tree) {
        return NULL;
    }

    for (size_t i = 0; i < tree->skill_count; i++) {
        if (tree->skills[i].id == skill_id) {
            return &tree->skills[i];
        }
    }

    return NULL;
}

bool skill_tree_can_unlock(const SkillTree* tree,
                            uint32_t skill_id,
                            uint32_t player_level,
                            uint32_t available_points) {
    if (!tree) {
        return false;
    }

    const Skill* skill = skill_tree_get_skill(tree, skill_id);
    if (!skill) {
        LOG_WARN("Skill %u not found", skill_id);
        return false;
    }

    /* Already unlocked */
    if (skill->unlocked) {
        LOG_DEBUG("Skill %u already unlocked", skill_id);
        return false;
    }

    /* Check level requirement */
    if (player_level < skill->min_level) {
        LOG_DEBUG("Player level %u < required %u", player_level, skill->min_level);
        return false;
    }

    /* Check skill points */
    if (available_points < skill->cost) {
        LOG_DEBUG("Available points %u < cost %u", available_points, skill->cost);
        return false;
    }

    /* Check prerequisites */
    for (uint32_t i = 0; i < skill->prerequisite_count; i++) {
        const Skill* prereq = skill_tree_get_skill(tree, skill->prerequisites[i]);
        if (!prereq || !prereq->unlocked) {
            LOG_DEBUG("Prerequisite skill %u not unlocked", skill->prerequisites[i]);
            return false;
        }
    }

    return true;
}

bool skill_tree_unlock(SkillTree* tree, uint32_t skill_id) {
    if (!tree) {
        return false;
    }

    for (size_t i = 0; i < tree->skill_count; i++) {
        if (tree->skills[i].id == skill_id) {
            tree->skills[i].unlocked = true;
            LOG_INFO("Unlocked skill: %s", tree->skills[i].name);
            return true;
        }
    }

    LOG_WARN("Skill %u not found", skill_id);
    return false;
}

size_t skill_tree_get_branch(const SkillTree* tree,
                              SkillBranch branch,
                              uint32_t* results,
                              size_t max_results) {
    if (!tree || !results || max_results == 0) {
        return 0;
    }

    size_t count = 0;
    for (size_t i = 0; i < tree->skill_count && count < max_results; i++) {
        if (tree->skills[i].branch == branch) {
            results[count++] = tree->skills[i].id;
        }
    }

    return count;
}

size_t skill_tree_get_unlocked(const SkillTree* tree,
                                uint32_t* results,
                                size_t max_results) {
    if (!tree || !results || max_results == 0) {
        return 0;
    }

    size_t count = 0;
    for (size_t i = 0; i < tree->skill_count && count < max_results; i++) {
        if (tree->skills[i].unlocked) {
            results[count++] = tree->skills[i].id;
        }
    }

    return count;
}

size_t skill_tree_get_available(const SkillTree* tree,
                                 uint32_t player_level,
                                 uint32_t* results,
                                 size_t max_results) {
    if (!tree || !results || max_results == 0) {
        return 0;
    }

    size_t count = 0;
    for (size_t i = 0; i < tree->skill_count && count < max_results; i++) {
        const Skill* skill = &tree->skills[i];

        /* Skip if already unlocked */
        if (skill->unlocked) {
            continue;
        }

        /* Check level requirement */
        if (player_level < skill->min_level) {
            continue;
        }

        /* Check prerequisites */
        bool prereqs_met = true;
        for (uint32_t j = 0; j < skill->prerequisite_count; j++) {
            const Skill* prereq = skill_tree_get_skill(tree, skill->prerequisites[j]);
            if (!prereq || !prereq->unlocked) {
                prereqs_met = false;
                break;
            }
        }

        if (prereqs_met) {
            results[count++] = skill->id;
        }
    }

    return count;
}

float skill_tree_get_stat_bonus(const SkillTree* tree, const char* stat_name) {
    if (!tree || !stat_name) {
        return 1.0f;
    }

    float total_bonus = 1.0f;

    for (size_t i = 0; i < tree->skill_count; i++) {
        const Skill* skill = &tree->skills[i];

        if (skill->unlocked &&
            skill->effect_type == SKILL_EFFECT_STAT_BONUS &&
            strcmp(skill->effect_stat, stat_name) == 0) {
            total_bonus += skill->effect_value;
        }
    }

    return total_bonus;
}

bool skill_tree_is_ability_unlocked(const SkillTree* tree, const char* ability_name) {
    if (!tree || !ability_name) {
        return false;
    }

    for (size_t i = 0; i < tree->skill_count; i++) {
        const Skill* skill = &tree->skills[i];

        if (skill->unlocked &&
            skill->effect_type == SKILL_EFFECT_UNLOCK_ABILITY &&
            strcmp(skill->effect_stat, ability_name) == 0) {
            return true;
        }
    }

    return false;
}

uint32_t skill_tree_get_points_spent(const SkillTree* tree) {
    if (!tree) {
        return 0;
    }

    uint32_t total = 0;
    for (size_t i = 0; i < tree->skill_count; i++) {
        if (tree->skills[i].unlocked) {
            total += tree->skills[i].cost;
        }
    }

    return total;
}

void skill_tree_reset(SkillTree* tree) {
    if (!tree) {
        return;
    }

    for (size_t i = 0; i < tree->skill_count; i++) {
        tree->skills[i].unlocked = false;
    }

    LOG_INFO("Skill tree reset (all points refunded)");
}

size_t skill_tree_load_from_file(SkillTree* tree, const char* filepath) {
    if (!tree || !filepath) {
        LOG_ERROR("skill_tree_load_from_file: NULL parameter");
        return 0;
    }

    DataFile* data = data_file_load(filepath);
    if (!data) {
        LOG_WARN("Failed to load skill data from %s", filepath);
        return 0;
    }

    size_t loaded = 0;
    size_t section_count;
    const DataSection** sections = data_file_get_sections(data, "SKILL", &section_count);

    for (size_t i = 0; i < section_count; i++) {
        const DataSection* section = sections[i];
        if (!section) continue;

        Skill skill = {0};
        skill.id = (uint32_t)atoi(section->section_id);

        /* Load basic properties */
        const DataValue* val;
        val = data_section_get(section, "name");
        strncpy(skill.name, data_value_get_string(val, "Unknown"), sizeof(skill.name) - 1);

        val = data_section_get(section, "description");
        strncpy(skill.description, data_value_get_string(val, ""), sizeof(skill.description) - 1);

        /* Load branch */
        char branch_str[32];
        val = data_section_get(section, "branch");
        strncpy(branch_str, data_value_get_string(val, "necromancy"), sizeof(branch_str) - 1);
        if (strcmp(branch_str, "intelligence") == 0) {
            skill.branch = SKILL_BRANCH_INTELLIGENCE;
        } else if (strcmp(branch_str, "corruption") == 0) {
            skill.branch = SKILL_BRANCH_CORRUPTION;
        } else if (strcmp(branch_str, "defense") == 0) {
            skill.branch = SKILL_BRANCH_DEFENSE;
        } else {
            skill.branch = SKILL_BRANCH_NECROMANCY;
        }

        val = data_section_get(section, "tier");
        skill.tier = (uint32_t)data_value_get_int(val, 1);

        val = data_section_get(section, "cost");
        skill.cost = (uint32_t)data_value_get_int(val, 1);

        val = data_section_get(section, "min_level");
        skill.min_level = (uint32_t)data_value_get_int(val, 1);

        /* Load prerequisites */
        val = data_section_get(section, "prerequisites");
        const char* prereq_str = data_value_get_string(val, "");
        if (strlen(prereq_str) > 0) {
            char prereq_copy[256];
            strncpy(prereq_copy, prereq_str, sizeof(prereq_copy) - 1);
            prereq_copy[sizeof(prereq_copy) - 1] = '\0';

            char* token = strtok(prereq_copy, ",");
            skill.prerequisite_count = 0;
            while (token && skill.prerequisite_count < 4) {
                skill.prerequisites[skill.prerequisite_count++] = (uint32_t)atoi(token);
                token = strtok(NULL, ",");
            }
        }

        /* Load effect */
        char effect_type_str[32];
        val = data_section_get(section, "effect_type");
        strncpy(effect_type_str, data_value_get_string(val, "stat_bonus"), sizeof(effect_type_str) - 1);
        if (strcmp(effect_type_str, "unlock_ability") == 0) {
            skill.effect_type = SKILL_EFFECT_UNLOCK_ABILITY;
        } else if (strcmp(effect_type_str, "reduce_cost") == 0) {
            skill.effect_type = SKILL_EFFECT_REDUCE_COST;
        } else if (strcmp(effect_type_str, "passive_effect") == 0) {
            skill.effect_type = SKILL_EFFECT_PASSIVE_EFFECT;
        } else {
            skill.effect_type = SKILL_EFFECT_STAT_BONUS;
        }

        val = data_section_get(section, "effect_stat");
        strncpy(skill.effect_stat, data_value_get_string(val, ""), sizeof(skill.effect_stat) - 1);

        val = data_section_get(section, "effect_value");
        skill.effect_value = (float)data_value_get_float(val, 0.0);

        skill.unlocked = false;

        if (skill_tree_add_skill(tree, &skill)) {
            loaded++;
        }
    }

    data_file_destroy(data);

    LOG_INFO("Loaded %zu skills from %s", loaded, filepath);
    return loaded;
}

const char* skill_branch_name(SkillBranch branch) {
    switch (branch) {
        case SKILL_BRANCH_NECROMANCY:    return "Necromancy";
        case SKILL_BRANCH_INTELLIGENCE:  return "Intelligence";
        case SKILL_BRANCH_CORRUPTION:    return "Corruption";
        case SKILL_BRANCH_DEFENSE:       return "Defense";
        default:                         return "Unknown";
    }
}

const char* skill_branch_description(SkillBranch branch) {
    switch (branch) {
        case SKILL_BRANCH_NECROMANCY:
            return "Raising, binding, and controlling the undead";
        case SKILL_BRANCH_INTELLIGENCE:
            return "Scrying, possession, and gathering intelligence";
        case SKILL_BRANCH_CORRUPTION:
            return "Dark rituals and forbidden magic";
        case SKILL_BRANCH_DEFENSE:
            return "Wards, protection, and defensive magic";
        default:
            return "Unknown branch";
    }
}

void skill_tree_get_stats(const SkillTree* tree,
                          size_t* total_skills,
                          size_t* unlocked_skills,
                          uint32_t* points_spent) {
    if (!tree) {
        if (total_skills) *total_skills = 0;
        if (unlocked_skills) *unlocked_skills = 0;
        if (points_spent) *points_spent = 0;
        return;
    }

    if (total_skills) {
        *total_skills = tree->skill_count;
    }

    if (unlocked_skills) {
        size_t count = 0;
        for (size_t i = 0; i < tree->skill_count; i++) {
            if (tree->skills[i].unlocked) {
                count++;
            }
        }
        *unlocked_skills = count;
    }

    if (points_spent) {
        *points_spent = skill_tree_get_points_spent(tree);
    }
}

/**
 * @file skill_tree.h
 * @brief Skill tree system for player progression
 *
 * The skill tree provides passive upgrades and new abilities as the player
 * progresses. Skills are organized into branches (necromancy, intelligence,
 * corruption, defense) and have prerequisites.
 *
 * Key concepts:
 * - Skill Points: Earned from leveling up, spent to unlock skills
 * - Branches: Four main skill paths (Necromancy, Intelligence, Corruption, Defense)
 * - Prerequisites: Some skills require other skills first
 * - Passive Bonuses: Most skills provide stat increases
 * - Active Abilities: Some skills unlock new commands/abilities
 */

#ifndef NECROMANCER_SKILL_TREE_H
#define NECROMANCER_SKILL_TREE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Skill branches
 */
typedef enum {
    SKILL_BRANCH_NECROMANCY = 0,    /**< Raising, binding, minion control */
    SKILL_BRANCH_INTELLIGENCE,      /**< Scrying, possession, memory reading */
    SKILL_BRANCH_CORRUPTION,        /**< Dark rituals, forbidden magic */
    SKILL_BRANCH_DEFENSE,           /**< Wards, protection, healing */
    SKILL_BRANCH_COUNT
} SkillBranch;

/**
 * @brief Skill effect types
 */
typedef enum {
    SKILL_EFFECT_STAT_BONUS = 0,    /**< Increase a stat (e.g., +10% minion HP) */
    SKILL_EFFECT_UNLOCK_ABILITY,    /**< Unlock new command/ability */
    SKILL_EFFECT_REDUCE_COST,       /**< Reduce resource costs */
    SKILL_EFFECT_PASSIVE_EFFECT,    /**< Ongoing passive effect */
    SKILL_EFFECT_COUNT
} SkillEffectType;

/**
 * @brief Individual skill definition
 */
typedef struct {
    uint32_t id;                    /**< Unique skill ID */
    char name[64];                  /**< Skill name */
    char description[256];          /**< Skill description */
    SkillBranch branch;             /**< Which branch this belongs to */
    uint32_t tier;                  /**< Tier (1-5, higher = more powerful) */
    uint32_t cost;                  /**< Skill points required to unlock */

    /* Prerequisites */
    uint32_t prerequisite_count;    /**< Number of prerequisite skills */
    uint32_t prerequisites[4];      /**< Required skill IDs (up to 4) */
    uint32_t min_level;             /**< Minimum player level required */

    /* Effects */
    SkillEffectType effect_type;    /**< Type of effect */
    char effect_stat[32];           /**< Stat name (for STAT_BONUS) */
    float effect_value;             /**< Effect magnitude */

    /* State */
    bool unlocked;                  /**< Whether player has unlocked this */
} Skill;

/**
 * @brief Skill tree manager (opaque)
 */
typedef struct SkillTree SkillTree;

/**
 * @brief Create a new skill tree
 *
 * @return Newly allocated SkillTree, or NULL on failure
 */
SkillTree* skill_tree_create(void);

/**
 * @brief Destroy skill tree and free all resources
 *
 * @param tree Skill tree to destroy (can be NULL)
 */
void skill_tree_destroy(SkillTree* tree);

/**
 * @brief Add a skill to the tree
 *
 * @param tree Skill tree
 * @param skill Skill to add (copied internally)
 * @return true on success, false on failure
 */
bool skill_tree_add_skill(SkillTree* tree, const Skill* skill);

/**
 * @brief Get a skill by ID
 *
 * @param tree Skill tree
 * @param skill_id Skill ID
 * @return Pointer to skill, or NULL if not found
 */
const Skill* skill_tree_get_skill(const SkillTree* tree, uint32_t skill_id);

/**
 * @brief Check if a skill can be unlocked
 *
 * Validates prerequisites, player level, and skill point availability.
 *
 * @param tree Skill tree
 * @param skill_id Skill to check
 * @param player_level Current player level
 * @param available_points Available skill points
 * @return true if can unlock, false otherwise
 */
bool skill_tree_can_unlock(const SkillTree* tree,
                            uint32_t skill_id,
                            uint32_t player_level,
                            uint32_t available_points);

/**
 * @brief Unlock a skill
 *
 * Marks skill as unlocked. Caller should verify with skill_tree_can_unlock first.
 *
 * @param tree Skill tree
 * @param skill_id Skill to unlock
 * @return true on success, false if skill not found
 */
bool skill_tree_unlock(SkillTree* tree, uint32_t skill_id);

/**
 * @brief Get all skills in a branch
 *
 * @param tree Skill tree
 * @param branch Branch to query
 * @param results Output array of skill IDs (caller allocates)
 * @param max_results Maximum number of results
 * @return Number of skills found
 */
size_t skill_tree_get_branch(const SkillTree* tree,
                              SkillBranch branch,
                              uint32_t* results,
                              size_t max_results);

/**
 * @brief Get all unlocked skills
 *
 * @param tree Skill tree
 * @param results Output array of skill IDs (caller allocates)
 * @param max_results Maximum number of results
 * @return Number of unlocked skills
 */
size_t skill_tree_get_unlocked(const SkillTree* tree,
                                uint32_t* results,
                                size_t max_results);

/**
 * @brief Get all skills available to unlock
 *
 * Returns skills whose prerequisites are met but not yet unlocked.
 *
 * @param tree Skill tree
 * @param player_level Current player level
 * @param results Output array of skill IDs (caller allocates)
 * @param max_results Maximum number of results
 * @return Number of available skills
 */
size_t skill_tree_get_available(const SkillTree* tree,
                                 uint32_t player_level,
                                 uint32_t* results,
                                 size_t max_results);

/**
 * @brief Calculate total stat bonus from unlocked skills
 *
 * Sums all STAT_BONUS effects for a given stat name.
 *
 * @param tree Skill tree
 * @param stat_name Stat to calculate (e.g., "minion_hp", "soul_energy_bonus")
 * @return Total bonus multiplier (e.g., 1.25 for +25%)
 */
float skill_tree_get_stat_bonus(const SkillTree* tree, const char* stat_name);

/**
 * @brief Check if an ability is unlocked
 *
 * Checks if any UNLOCK_ABILITY skill with matching effect_stat is unlocked.
 *
 * @param tree Skill tree
 * @param ability_name Ability name (e.g., "possess", "soulforge")
 * @return true if unlocked, false otherwise
 */
bool skill_tree_is_ability_unlocked(const SkillTree* tree, const char* ability_name);

/**
 * @brief Get total skill points spent
 *
 * @param tree Skill tree
 * @return Total points spent on unlocked skills
 */
uint32_t skill_tree_get_points_spent(const SkillTree* tree);

/**
 * @brief Reset all skills (refund all points)
 *
 * @param tree Skill tree
 */
void skill_tree_reset(SkillTree* tree);

/**
 * @brief Load skills from data file
 *
 * @param tree Skill tree
 * @param filepath Path to skills.dat file
 * @return Number of skills loaded
 */
size_t skill_tree_load_from_file(SkillTree* tree, const char* filepath);

/**
 * @brief Get branch name
 *
 * @param branch Branch enum
 * @return Branch name string
 */
const char* skill_branch_name(SkillBranch branch);

/**
 * @brief Get branch description
 *
 * @param branch Branch enum
 * @return Branch description string
 */
const char* skill_branch_description(SkillBranch branch);

/**
 * @brief Get statistics for skill tree
 *
 * @param tree Skill tree
 * @param total_skills Output: total skills in tree
 * @param unlocked_skills Output: total unlocked skills
 * @param points_spent Output: total points spent
 */
void skill_tree_get_stats(const SkillTree* tree,
                          size_t* total_skills,
                          size_t* unlocked_skills,
                          uint32_t* points_spent);

#endif /* NECROMANCER_SKILL_TREE_H */

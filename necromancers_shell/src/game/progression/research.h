/**
 * @file research.h
 * @brief Research system for unlocking new spells and abilities
 *
 * The research system allows players to invest time and resources into
 * discovering new spells, rituals, and forbidden knowledge. Research
 * projects have time requirements and may have prerequisites.
 *
 * Key concepts:
 * - Research Projects: Individual research tasks
 * - Time Investment: Projects take game hours to complete
 * - Resource Costs: Projects may require soul energy, mana, or materials
 * - Prerequisites: Some projects require other research first
 * - Unlockables: Completing research unlocks spells, abilities, or upgrades
 */

#ifndef NECROMANCER_RESEARCH_H
#define NECROMANCER_RESEARCH_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Research categories
 */
typedef enum {
    RESEARCH_CATEGORY_SPELLS = 0,       /**< New spells and rituals */
    RESEARCH_CATEGORY_ABILITIES,        /**< New abilities (possess, scry) */
    RESEARCH_CATEGORY_UPGRADES,         /**< System upgrades (minion capacity, etc.) */
    RESEARCH_CATEGORY_LORE,             /**< Lore and story unlocks */
    RESEARCH_CATEGORY_COUNT
} ResearchCategory;

/**
 * @brief Research project definition
 */
typedef struct {
    uint32_t id;                    /**< Unique project ID */
    char name[64];                  /**< Project name */
    char description[256];          /**< Project description */
    ResearchCategory category;      /**< Category */

    /* Requirements */
    uint32_t prerequisite_count;    /**< Number of prerequisite projects */
    uint32_t prerequisites[4];      /**< Required project IDs (up to 4) */
    uint32_t min_level;             /**< Minimum player level required */

    /* Costs */
    uint32_t time_hours;            /**< Time required (game hours) */
    uint32_t soul_energy_cost;      /**< Soul energy cost */
    uint32_t mana_cost;             /**< Mana cost */

    /* Unlockables */
    char unlock_type[32];           /**< Type of unlock (spell, ability, upgrade) */
    char unlock_name[64];           /**< Name of what's unlocked */
    char unlock_description[256];   /**< Description of unlock */

    /* State */
    bool completed;                 /**< Whether project is completed */
    uint32_t hours_invested;        /**< Hours invested so far (for in-progress) */
} ResearchProject;

/**
 * @brief Research manager (opaque)
 */
typedef struct ResearchManager ResearchManager;

/**
 * @brief Create a new research manager
 *
 * @return Newly allocated ResearchManager, or NULL on failure
 */
ResearchManager* research_manager_create(void);

/**
 * @brief Destroy research manager and free all resources
 *
 * @param manager Research manager to destroy (can be NULL)
 */
void research_manager_destroy(ResearchManager* manager);

/**
 * @brief Add a research project
 *
 * @param manager Research manager
 * @param project Project to add (copied internally)
 * @return true on success, false on failure
 */
bool research_add_project(ResearchManager* manager, const ResearchProject* project);

/**
 * @brief Get a research project by ID
 *
 * @param manager Research manager
 * @param project_id Project ID
 * @return Pointer to project, or NULL if not found
 */
const ResearchProject* research_get_project(const ResearchManager* manager,
                                             uint32_t project_id);

/**
 * @brief Check if a project can be started
 *
 * Validates prerequisites, player level, and resource availability.
 *
 * @param manager Research manager
 * @param project_id Project to check
 * @param player_level Current player level
 * @param available_energy Available soul energy
 * @param available_mana Available mana
 * @return true if can start, false otherwise
 */
bool research_can_start(const ResearchManager* manager,
                        uint32_t project_id,
                        uint32_t player_level,
                        uint32_t available_energy,
                        uint32_t available_mana);

/**
 * @brief Start a research project
 *
 * Marks project as in-progress. Caller should deduct resources.
 *
 * @param manager Research manager
 * @param project_id Project to start
 * @return true on success, false if project not found
 */
bool research_start(ResearchManager* manager, uint32_t project_id);

/**
 * @brief Invest time in current research project
 *
 * Advances progress on the project. If time_hours >= remaining time,
 * the project completes automatically.
 *
 * @param manager Research manager
 * @param project_id Project to advance
 * @param time_hours Hours to invest
 * @return true if project completed, false if still in progress
 */
bool research_invest_time(ResearchManager* manager,
                          uint32_t project_id,
                          uint32_t time_hours);

/**
 * @brief Complete a research project
 *
 * Marks project as completed. Caller should apply unlocks.
 *
 * @param manager Research manager
 * @param project_id Project to complete
 * @return true on success, false if project not found
 */
bool research_complete(ResearchManager* manager, uint32_t project_id);

/**
 * @brief Get all projects in a category
 *
 * @param manager Research manager
 * @param category Category to query
 * @param results Output array of project IDs (caller allocates)
 * @param max_results Maximum number of results
 * @return Number of projects found
 */
size_t research_get_category(const ResearchManager* manager,
                              ResearchCategory category,
                              uint32_t* results,
                              size_t max_results);

/**
 * @brief Get all completed projects
 *
 * @param manager Research manager
 * @param results Output array of project IDs (caller allocates)
 * @param max_results Maximum number of results
 * @return Number of completed projects
 */
size_t research_get_completed(const ResearchManager* manager,
                               uint32_t* results,
                               size_t max_results);

/**
 * @brief Get all available projects
 *
 * Returns projects whose prerequisites are met but not yet completed.
 *
 * @param manager Research manager
 * @param player_level Current player level
 * @param results Output array of project IDs (caller allocates)
 * @param max_results Maximum number of results
 * @return Number of available projects
 */
size_t research_get_available(const ResearchManager* manager,
                               uint32_t player_level,
                               uint32_t* results,
                               size_t max_results);

/**
 * @brief Get current in-progress project
 *
 * @param manager Research manager
 * @return Project ID of in-progress project, or 0 if none
 */
uint32_t research_get_current(const ResearchManager* manager);

/**
 * @brief Cancel current research project
 *
 * Resets progress but does not refund resources.
 *
 * @param manager Research manager
 * @return true if project cancelled, false if no project in progress
 */
bool research_cancel_current(ResearchManager* manager);

/**
 * @brief Load research projects from data file
 *
 * @param manager Research manager
 * @param filepath Path to research.dat file
 * @return Number of projects loaded
 */
size_t research_load_from_file(ResearchManager* manager, const char* filepath);

/**
 * @brief Get category name
 *
 * @param category Category enum
 * @return Category name string
 */
const char* research_category_name(ResearchCategory category);

/**
 * @brief Get statistics for research
 *
 * @param manager Research manager
 * @param total_projects Output: total projects available
 * @param completed_projects Output: total completed projects
 * @param current_project Output: current in-progress project ID (0 if none)
 */
void research_get_stats(const ResearchManager* manager,
                        size_t* total_projects,
                        size_t* completed_projects,
                        uint32_t* current_project);

#endif /* NECROMANCER_RESEARCH_H */

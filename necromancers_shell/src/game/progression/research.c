/**
 * @file research.c
 * @brief Implementation of research system
 */

#include "research.h"
#include "../../data/data_loader.h"
#include "../../utils/logger.h"
#include <stdlib.h>
#include <string.h>

#define MAX_PROJECTS 50

/**
 * @brief Research manager structure
 */
struct ResearchManager {
    ResearchProject projects[MAX_PROJECTS];    /**< Project array */
    size_t project_count;                      /**< Number of projects */
    uint32_t current_project_id;               /**< Current in-progress project (0 if none) */
};

ResearchManager* research_manager_create(void) {
    ResearchManager* manager = calloc(1, sizeof(ResearchManager));
    if (!manager) {
        LOG_ERROR("Failed to allocate research manager");
        return NULL;
    }

    manager->project_count = 0;
    manager->current_project_id = 0;
    LOG_DEBUG("Research manager created");
    return manager;
}

void research_manager_destroy(ResearchManager* manager) {
    if (!manager) {
        return;
    }

    free(manager);
    LOG_DEBUG("Research manager destroyed");
}

bool research_add_project(ResearchManager* manager, const ResearchProject* project) {
    if (!manager || !project) {
        return false;
    }

    if (manager->project_count >= MAX_PROJECTS) {
        LOG_WARN("Research manager is full (max %d projects)", MAX_PROJECTS);
        return false;
    }

    /* Check for duplicate ID */
    for (size_t i = 0; i < manager->project_count; i++) {
        if (manager->projects[i].id == project->id) {
            LOG_WARN("Research project ID %u already exists", project->id);
            return false;
        }
    }

    /* Copy project */
    manager->projects[manager->project_count] = *project;
    manager->project_count++;

    LOG_DEBUG("Added research project %u: %s", project->id, project->name);
    return true;
}

const ResearchProject* research_get_project(const ResearchManager* manager,
                                             uint32_t project_id) {
    if (!manager) {
        return NULL;
    }

    for (size_t i = 0; i < manager->project_count; i++) {
        if (manager->projects[i].id == project_id) {
            return &manager->projects[i];
        }
    }

    return NULL;
}

bool research_can_start(const ResearchManager* manager,
                        uint32_t project_id,
                        uint32_t player_level,
                        uint32_t available_energy,
                        uint32_t available_mana) {
    if (!manager) {
        return false;
    }

    /* Can't start if already working on something */
    if (manager->current_project_id != 0) {
        LOG_DEBUG("Already working on project %u", manager->current_project_id);
        return false;
    }

    const ResearchProject* project = research_get_project(manager, project_id);
    if (!project) {
        LOG_WARN("Research project %u not found", project_id);
        return false;
    }

    /* Already completed */
    if (project->completed) {
        LOG_DEBUG("Research project %u already completed", project_id);
        return false;
    }

    /* Check level requirement */
    if (player_level < project->min_level) {
        LOG_DEBUG("Player level %u < required %u", player_level, project->min_level);
        return false;
    }

    /* Check resources */
    if (available_energy < project->soul_energy_cost) {
        LOG_DEBUG("Available energy %u < cost %u", available_energy, project->soul_energy_cost);
        return false;
    }

    if (available_mana < project->mana_cost) {
        LOG_DEBUG("Available mana %u < cost %u", available_mana, project->mana_cost);
        return false;
    }

    /* Check prerequisites */
    for (uint32_t i = 0; i < project->prerequisite_count; i++) {
        const ResearchProject* prereq = research_get_project(manager, project->prerequisites[i]);
        if (!prereq || !prereq->completed) {
            LOG_DEBUG("Prerequisite project %u not completed", project->prerequisites[i]);
            return false;
        }
    }

    return true;
}

bool research_start(ResearchManager* manager, uint32_t project_id) {
    if (!manager) {
        return false;
    }

    for (size_t i = 0; i < manager->project_count; i++) {
        if (manager->projects[i].id == project_id) {
            manager->current_project_id = project_id;
            manager->projects[i].hours_invested = 0;
            LOG_INFO("Started research: %s", manager->projects[i].name);
            return true;
        }
    }

    LOG_WARN("Research project %u not found", project_id);
    return false;
}

bool research_invest_time(ResearchManager* manager,
                          uint32_t project_id,
                          uint32_t time_hours) {
    if (!manager) {
        return false;
    }

    for (size_t i = 0; i < manager->project_count; i++) {
        if (manager->projects[i].id == project_id) {
            manager->projects[i].hours_invested += time_hours;

            if (manager->projects[i].hours_invested >= manager->projects[i].time_hours) {
                /* Project completed */
                manager->projects[i].completed = true;
                manager->current_project_id = 0;
                LOG_INFO("Research completed: %s", manager->projects[i].name);
                return true;
            }

            LOG_DEBUG("Research progress: %u/%u hours",
                      manager->projects[i].hours_invested,
                      manager->projects[i].time_hours);
            return false;
        }
    }

    LOG_WARN("Research project %u not found", project_id);
    return false;
}

bool research_complete(ResearchManager* manager, uint32_t project_id) {
    if (!manager) {
        return false;
    }

    for (size_t i = 0; i < manager->project_count; i++) {
        if (manager->projects[i].id == project_id) {
            manager->projects[i].completed = true;
            if (manager->current_project_id == project_id) {
                manager->current_project_id = 0;
            }
            LOG_INFO("Research completed: %s", manager->projects[i].name);
            return true;
        }
    }

    LOG_WARN("Research project %u not found", project_id);
    return false;
}

size_t research_get_category(const ResearchManager* manager,
                              ResearchCategory category,
                              uint32_t* results,
                              size_t max_results) {
    if (!manager || !results || max_results == 0) {
        return 0;
    }

    size_t count = 0;
    for (size_t i = 0; i < manager->project_count && count < max_results; i++) {
        if (manager->projects[i].category == category) {
            results[count++] = manager->projects[i].id;
        }
    }

    return count;
}

size_t research_get_completed(const ResearchManager* manager,
                               uint32_t* results,
                               size_t max_results) {
    if (!manager || !results || max_results == 0) {
        return 0;
    }

    size_t count = 0;
    for (size_t i = 0; i < manager->project_count && count < max_results; i++) {
        if (manager->projects[i].completed) {
            results[count++] = manager->projects[i].id;
        }
    }

    return count;
}

size_t research_get_available(const ResearchManager* manager,
                               uint32_t player_level,
                               uint32_t* results,
                               size_t max_results) {
    if (!manager || !results || max_results == 0) {
        return 0;
    }

    size_t count = 0;
    for (size_t i = 0; i < manager->project_count && count < max_results; i++) {
        const ResearchProject* project = &manager->projects[i];

        /* Skip if already completed */
        if (project->completed) {
            continue;
        }

        /* Check level requirement */
        if (player_level < project->min_level) {
            continue;
        }

        /* Check prerequisites */
        bool prereqs_met = true;
        for (uint32_t j = 0; j < project->prerequisite_count; j++) {
            const ResearchProject* prereq = research_get_project(manager, project->prerequisites[j]);
            if (!prereq || !prereq->completed) {
                prereqs_met = false;
                break;
            }
        }

        if (prereqs_met) {
            results[count++] = project->id;
        }
    }

    return count;
}

uint32_t research_get_current(const ResearchManager* manager) {
    if (!manager) {
        return 0;
    }

    return manager->current_project_id;
}

bool research_cancel_current(ResearchManager* manager) {
    if (!manager || manager->current_project_id == 0) {
        return false;
    }

    for (size_t i = 0; i < manager->project_count; i++) {
        if (manager->projects[i].id == manager->current_project_id) {
            manager->projects[i].hours_invested = 0;
            manager->current_project_id = 0;
            LOG_INFO("Research cancelled: %s", manager->projects[i].name);
            return true;
        }
    }

    return false;
}

size_t research_load_from_file(ResearchManager* manager, const char* filepath) {
    if (!manager || !filepath) {
        LOG_ERROR("research_load_from_file: NULL parameter");
        return 0;
    }

    DataFile* data = data_file_load(filepath);
    if (!data) {
        LOG_WARN("Failed to load research data from %s", filepath);
        return 0;
    }

    size_t loaded = 0;
    size_t section_count;
    const DataSection** sections = data_file_get_sections(data, "RESEARCH", &section_count);

    for (size_t i = 0; i < section_count; i++) {
        const DataSection* section = sections[i];
        if (!section) continue;

        ResearchProject project = {0};
        project.id = (uint32_t)atoi(section->section_id);

        /* Load basic properties */
        const DataValue* val;
        val = data_section_get(section, "name");
        strncpy(project.name, data_value_get_string(val, "Unknown"), sizeof(project.name) - 1);

        val = data_section_get(section, "description");
        strncpy(project.description, data_value_get_string(val, ""), sizeof(project.description) - 1);

        /* Load category */
        char category_str[32];
        val = data_section_get(section, "category");
        strncpy(category_str, data_value_get_string(val, "spells"), sizeof(category_str) - 1);
        if (strcmp(category_str, "abilities") == 0) {
            project.category = RESEARCH_CATEGORY_ABILITIES;
        } else if (strcmp(category_str, "upgrades") == 0) {
            project.category = RESEARCH_CATEGORY_UPGRADES;
        } else if (strcmp(category_str, "lore") == 0) {
            project.category = RESEARCH_CATEGORY_LORE;
        } else {
            project.category = RESEARCH_CATEGORY_SPELLS;
        }

        val = data_section_get(section, "min_level");
        project.min_level = (uint32_t)data_value_get_int(val, 1);

        /* Load costs */
        val = data_section_get(section, "time_hours");
        project.time_hours = (uint32_t)data_value_get_int(val, 1);

        val = data_section_get(section, "soul_energy_cost");
        project.soul_energy_cost = (uint32_t)data_value_get_int(val, 0);

        val = data_section_get(section, "mana_cost");
        project.mana_cost = (uint32_t)data_value_get_int(val, 0);

        /* Load prerequisites */
        val = data_section_get(section, "prerequisites");
        const char* prereq_str = data_value_get_string(val, "");
        if (strlen(prereq_str) > 0) {
            char prereq_copy[256];
            strncpy(prereq_copy, prereq_str, sizeof(prereq_copy) - 1);
            prereq_copy[sizeof(prereq_copy) - 1] = '\0';

            char* token = strtok(prereq_copy, ",");
            project.prerequisite_count = 0;
            while (token && project.prerequisite_count < 4) {
                project.prerequisites[project.prerequisite_count++] = (uint32_t)atoi(token);
                token = strtok(NULL, ",");
            }
        }

        /* Load unlockables */
        val = data_section_get(section, "unlock_type");
        strncpy(project.unlock_type, data_value_get_string(val, ""), sizeof(project.unlock_type) - 1);

        val = data_section_get(section, "unlock_name");
        strncpy(project.unlock_name, data_value_get_string(val, ""), sizeof(project.unlock_name) - 1);

        val = data_section_get(section, "unlock_description");
        strncpy(project.unlock_description, data_value_get_string(val, ""), sizeof(project.unlock_description) - 1);

        project.completed = false;
        project.hours_invested = 0;

        if (research_add_project(manager, &project)) {
            loaded++;
        }
    }

    data_file_destroy(data);

    LOG_INFO("Loaded %zu research projects from %s", loaded, filepath);
    return loaded;
}

const char* research_category_name(ResearchCategory category) {
    switch (category) {
        case RESEARCH_CATEGORY_SPELLS:      return "Spells";
        case RESEARCH_CATEGORY_ABILITIES:   return "Abilities";
        case RESEARCH_CATEGORY_UPGRADES:    return "Upgrades";
        case RESEARCH_CATEGORY_LORE:        return "Lore";
        default:                            return "Unknown";
    }
}

void research_get_stats(const ResearchManager* manager,
                        size_t* total_projects,
                        size_t* completed_projects,
                        uint32_t* current_project) {
    if (!manager) {
        if (total_projects) *total_projects = 0;
        if (completed_projects) *completed_projects = 0;
        if (current_project) *current_project = 0;
        return;
    }

    if (total_projects) {
        *total_projects = manager->project_count;
    }

    if (completed_projects) {
        size_t count = 0;
        for (size_t i = 0; i < manager->project_count; i++) {
            if (manager->projects[i].completed) {
                count++;
            }
        }
        *completed_projects = count;
    }

    if (current_project) {
        *current_project = manager->current_project_id;
    }
}

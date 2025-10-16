#include "archon_trial.h"
#include "../../../data/data_loader.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * @brief Helper function to parse trial type from string
 */
static TrialType parse_trial_type(const char* type_str) {
    if (strcmp(type_str, "combat") == 0) return TRIAL_TYPE_COMBAT;
    if (strcmp(type_str, "puzzle") == 0) return TRIAL_TYPE_PUZZLE;
    if (strcmp(type_str, "moral") == 0) return TRIAL_TYPE_MORAL;
    if (strcmp(type_str, "technical") == 0) return TRIAL_TYPE_TECHNICAL;
    if (strcmp(type_str, "sacrifice") == 0) return TRIAL_TYPE_SACRIFICE;
    if (strcmp(type_str, "leadership") == 0) return TRIAL_TYPE_LEADERSHIP;
    if (strcmp(type_str, "resolve") == 0) return TRIAL_TYPE_RESOLVE;
    return TRIAL_TYPE_COMBAT; /* Default */
}

/**
 * @brief Helper function to get trial by ID (mutable)
 */
static ArchonTrial* get_trial_by_id_mutable(ArchonTrialManager* manager,
                                             uint32_t trial_id) {
    if (!manager) {
        return NULL;
    }

    for (size_t i = 0; i < manager->trial_count; i++) {
        if (manager->trials[i].id == trial_id) {
            return &manager->trials[i];
        }
    }

    return NULL;
}

ArchonTrialManager* archon_trial_manager_create(void) {
    ArchonTrialManager* manager = calloc(1, sizeof(ArchonTrialManager));
    if (!manager) {
        fprintf(stderr, "Failed to allocate trial manager\n");
        return NULL;
    }

    /* Initialize state */
    manager->trial_count = 0;
    manager->current_trial_id = 0;
    manager->path_active = false;
    manager->path_completed = false;
    manager->path_failed = false;

    /* Initialize all trials to locked */
    for (size_t i = 0; i < MAX_ARCHON_TRIALS; i++) {
        manager->trials[i].id = 0;
        manager->trials[i].trial_number = 0;
        manager->trials[i].status = TRIAL_STATUS_LOCKED;
        manager->trials[i].attempts_made = 0;
        manager->trials[i].max_attempts = MAX_TRIAL_ATTEMPTS;
        manager->trials[i].best_score = 0.0f;
        manager->trials[i].score_matters = false;
        manager->trials[i].prerequisite_trial_id = 0;
    }

    return manager;
}

void archon_trial_manager_destroy(ArchonTrialManager* manager) {
    if (!manager) {
        return;
    }

    free(manager);
}

bool archon_trial_load_from_file(ArchonTrialManager* manager, const char* filepath) {
    if (!manager || !filepath) {
        return false;
    }

    DataFile* file = data_file_load(filepath);
    if (!file) {
        fprintf(stderr, "Failed to load trial data from %s\n", filepath);
        return false;
    }

    /* Get all TRIAL sections */
    size_t section_count = 0;
    const DataSection** sections = data_file_get_sections(file, "TRIAL", &section_count);
    if (!sections || section_count == 0) {
        fprintf(stderr, "No TRIAL sections found in %s\n", filepath);
        data_file_destroy(file);
        return false;
    }

    /* Limit to MAX_ARCHON_TRIALS */
    if (section_count > MAX_ARCHON_TRIALS) {
        section_count = MAX_ARCHON_TRIALS;
    }

    /* Parse each trial section */
    for (size_t i = 0; i < section_count; i++) {
        const DataSection* section = sections[i];
        ArchonTrial* trial = &manager->trials[i];

        /* Parse trial data */
        const DataValue* id_val = data_section_get(section, "id");
        const DataValue* number_val = data_section_get(section, "number");
        const DataValue* name_val = data_section_get(section, "name");
        const DataValue* desc_val = data_section_get(section, "description");
        const DataValue* type_val = data_section_get(section, "type");
        const DataValue* completion_val = data_section_get(section, "completion_text");

        /* Unlock conditions */
        const DataValue* corrupt_min_val = data_section_get(section, "unlock_corruption_min");
        const DataValue* corrupt_max_val = data_section_get(section, "unlock_corruption_max");
        const DataValue* conscious_min_val = data_section_get(section, "unlock_consciousness_min");
        const DataValue* prereq_val = data_section_get(section, "prerequisite_trial_id");

        /* Optional fields */
        const DataValue* max_attempts_val = data_section_get(section, "max_attempts");
        const DataValue* score_matters_val = data_section_get(section, "score_matters");

        /* Set trial ID and number */
        trial->id = (uint32_t)data_value_get_int(id_val, i + 1);
        trial->trial_number = (uint32_t)data_value_get_int(number_val, i + 1);

        /* Copy strings safely */
        const char* name = data_value_get_string(name_val, "Unknown Trial");
        strncpy(trial->name, name, sizeof(trial->name) - 1);
        trial->name[sizeof(trial->name) - 1] = '\0';

        const char* desc = data_value_get_string(desc_val, "");
        strncpy(trial->description, desc, sizeof(trial->description) - 1);
        trial->description[sizeof(trial->description) - 1] = '\0';

        const char* completion = data_value_get_string(completion_val, "Trial completed.");
        strncpy(trial->completion_text, completion, sizeof(trial->completion_text) - 1);
        trial->completion_text[sizeof(trial->completion_text) - 1] = '\0';

        /* Parse trial type */
        const char* type_str = data_value_get_string(type_val, "combat");
        trial->type = parse_trial_type(type_str);

        /* Set unlock conditions */
        trial->unlock_corruption_min = (uint8_t)data_value_get_int(corrupt_min_val, 30);
        trial->unlock_corruption_max = (uint8_t)data_value_get_int(corrupt_max_val, 60);
        trial->unlock_consciousness_min = (uint8_t)data_value_get_int(conscious_min_val, 50);
        trial->prerequisite_trial_id = (uint32_t)data_value_get_int(prereq_val, 0);

        /* Optional settings */
        trial->max_attempts = (uint8_t)data_value_get_int(max_attempts_val, MAX_TRIAL_ATTEMPTS);
        trial->score_matters = data_value_get_bool(score_matters_val, false);

        /* Initialize runtime state */
        trial->status = TRIAL_STATUS_LOCKED;
        trial->attempts_made = 0;
        trial->best_score = 0.0f;
    }

    manager->trial_count = section_count;
    data_file_destroy(file);
    return true;
}

bool archon_trial_can_unlock(const ArchonTrialManager* manager,
                              uint32_t trial_id,
                              uint8_t corruption,
                              float consciousness) {
    if (!manager) {
        return false;
    }

    const ArchonTrial* trial = archon_trial_get(manager, trial_id);
    if (!trial) {
        return false;
    }

    /* Already unlocked or passed */
    if (trial->status != TRIAL_STATUS_LOCKED) {
        return false;
    }

    /* Check corruption range */
    if (corruption < trial->unlock_corruption_min ||
        corruption > trial->unlock_corruption_max) {
        return false;
    }

    /* Check consciousness minimum */
    if (consciousness < trial->unlock_consciousness_min) {
        return false;
    }

    /* Check prerequisite */
    if (trial->prerequisite_trial_id != 0) {
        const ArchonTrial* prereq = archon_trial_get(manager, trial->prerequisite_trial_id);
        if (!prereq || prereq->status != TRIAL_STATUS_PASSED) {
            return false;
        }
    }

    return true;
}

bool archon_trial_unlock(ArchonTrialManager* manager,
                         uint32_t trial_id,
                         uint8_t corruption,
                         float consciousness) {
    if (!manager) {
        return false;
    }

    if (!archon_trial_can_unlock(manager, trial_id, corruption, consciousness)) {
        return false;
    }

    ArchonTrial* trial = get_trial_by_id_mutable(manager, trial_id);
    if (!trial) {
        return false;
    }

    trial->status = TRIAL_STATUS_AVAILABLE;
    return true;
}

bool archon_trial_start(ArchonTrialManager* manager, uint32_t trial_id) {
    if (!manager) {
        return false;
    }

    ArchonTrial* trial = get_trial_by_id_mutable(manager, trial_id);
    if (!trial) {
        return false;
    }

    /* Must be available to start */
    if (trial->status != TRIAL_STATUS_AVAILABLE) {
        return false;
    }

    /* Check if max attempts exceeded */
    if (trial->max_attempts > 0 && trial->attempts_made >= trial->max_attempts) {
        trial->status = TRIAL_STATUS_FAILED;
        return false;
    }

    trial->status = TRIAL_STATUS_IN_PROGRESS;
    manager->current_trial_id = trial_id;
    return true;
}

bool archon_trial_complete(ArchonTrialManager* manager,
                           uint32_t trial_id,
                           float score) {
    if (!manager) {
        return false;
    }

    ArchonTrial* trial = get_trial_by_id_mutable(manager, trial_id);
    if (!trial) {
        return false;
    }

    /* Must be in progress to complete */
    if (trial->status != TRIAL_STATUS_IN_PROGRESS) {
        return false;
    }

    /* Record score if it matters */
    if (trial->score_matters && score > trial->best_score) {
        trial->best_score = score;
    }

    trial->status = TRIAL_STATUS_PASSED;
    manager->current_trial_id = 0;

    /* Check if all trials completed */
    if (archon_trial_all_completed(manager)) {
        manager->path_completed = true;
    }

    return true;
}

bool archon_trial_fail(ArchonTrialManager* manager, uint32_t trial_id) {
    if (!manager) {
        return false;
    }

    ArchonTrial* trial = get_trial_by_id_mutable(manager, trial_id);
    if (!trial) {
        return false;
    }

    /* Must be in progress to fail */
    if (trial->status != TRIAL_STATUS_IN_PROGRESS) {
        return false;
    }

    trial->attempts_made++;
    manager->current_trial_id = 0;

    /* Check if attempts exhausted */
    if (trial->max_attempts > 0 && trial->attempts_made >= trial->max_attempts) {
        trial->status = TRIAL_STATUS_FAILED;
        manager->path_failed = true;
        return false; /* Cannot retry */
    }

    /* Allow retry */
    trial->status = TRIAL_STATUS_AVAILABLE;
    return true;
}

const ArchonTrial* archon_trial_get(const ArchonTrialManager* manager,
                                    uint32_t trial_id) {
    if (!manager) {
        return NULL;
    }

    for (size_t i = 0; i < manager->trial_count; i++) {
        if (manager->trials[i].id == trial_id) {
            return &manager->trials[i];
        }
    }

    return NULL;
}

const ArchonTrial* archon_trial_get_by_number(const ArchonTrialManager* manager,
                                               uint32_t trial_number) {
    if (!manager) {
        return NULL;
    }

    for (size_t i = 0; i < manager->trial_count; i++) {
        if (manager->trials[i].trial_number == trial_number) {
            return &manager->trials[i];
        }
    }

    return NULL;
}

const ArchonTrial* archon_trial_get_current(const ArchonTrialManager* manager) {
    if (!manager || manager->current_trial_id == 0) {
        return NULL;
    }

    return archon_trial_get(manager, manager->current_trial_id);
}

bool archon_trial_all_completed(const ArchonTrialManager* manager) {
    if (!manager) {
        return false;
    }

    size_t passed = archon_trial_count_passed(manager);
    return (passed == manager->trial_count && manager->trial_count == MAX_ARCHON_TRIALS);
}

size_t archon_trial_count_passed(const ArchonTrialManager* manager) {
    if (!manager) {
        return 0;
    }

    size_t count = 0;
    for (size_t i = 0; i < manager->trial_count; i++) {
        if (manager->trials[i].status == TRIAL_STATUS_PASSED) {
            count++;
        }
    }

    return count;
}

size_t archon_trial_count_failed(const ArchonTrialManager* manager) {
    if (!manager) {
        return 0;
    }

    size_t count = 0;
    for (size_t i = 0; i < manager->trial_count; i++) {
        if (manager->trials[i].status == TRIAL_STATUS_FAILED) {
            count++;
        }
    }

    return count;
}

bool archon_trial_path_available(const ArchonTrialManager* manager,
                                  uint8_t corruption) {
    if (!manager) {
        return false;
    }

    /* Path failed if any trial permanently failed */
    if (manager->path_failed) {
        return false;
    }

    /* Path failed if corruption outside 30-60% after activation */
    if (manager->path_active) {
        if (corruption < 30 || corruption > 60) {
            return false;
        }
    }

    return true;
}

bool archon_trial_activate_path(ArchonTrialManager* manager,
                                 uint8_t corruption,
                                 float consciousness) {
    if (!manager) {
        return false;
    }

    /* Check corruption range for Archon path (30-60%) */
    if (corruption < 30 || corruption > 60) {
        return false;
    }

    /* Minimum consciousness requirement */
    if (consciousness < 50.0f) {
        return false;
    }

    manager->path_active = true;

    /* Unlock Trial 1 if conditions met */
    if (manager->trial_count > 0) {
        archon_trial_unlock(manager, manager->trials[0].id, corruption, consciousness);
    }

    return true;
}

const char* archon_trial_status_name(TrialStatus status) {
    switch (status) {
        case TRIAL_STATUS_LOCKED:      return "Locked";
        case TRIAL_STATUS_AVAILABLE:   return "Available";
        case TRIAL_STATUS_IN_PROGRESS: return "In Progress";
        case TRIAL_STATUS_PASSED:      return "Passed";
        case TRIAL_STATUS_FAILED:      return "Failed";
        default:                       return "Unknown";
    }
}

const char* archon_trial_type_name(TrialType type) {
    switch (type) {
        case TRIAL_TYPE_COMBAT:      return "Combat";
        case TRIAL_TYPE_PUZZLE:      return "Puzzle";
        case TRIAL_TYPE_MORAL:       return "Moral";
        case TRIAL_TYPE_TECHNICAL:   return "Technical";
        case TRIAL_TYPE_SACRIFICE:   return "Sacrifice";
        case TRIAL_TYPE_LEADERSHIP:  return "Leadership";
        case TRIAL_TYPE_RESOLVE:     return "Resolve";
        default:                     return "Unknown";
    }
}

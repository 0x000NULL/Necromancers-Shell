#include "trial_morality.h"
#include "../../../data/data_loader.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * @brief Static buffer for village info text
 */
static char village_info_buffer[1024];

MoralityTrialState* morality_trial_create(void) {
    MoralityTrialState* state = calloc(1, sizeof(MoralityTrialState));
    if (!state) {
        fprintf(stderr, "Failed to allocate morality trial state\n");
        return NULL;
    }

    /* Initialize state */
    state->active = false;
    state->choice_made = MORALITY_CHOICE_NONE;
    state->choice_is_final = false;

    /* Default values (will be overwritten by data file) */
    snprintf(state->village_name, sizeof(state->village_name), "%s", "Ashford");
    state->population = 100;
    state->children_count = 23;
    state->save_cost_energy = -1; /* -1 means "all" */
    state->save_cost_mana = -1;   /* -1 means "all" */
    state->save_corruption_change = -5;
    state->harvest_reward_energy = 50000;
    state->harvest_corruption_change = 25;

    return state;
}

void morality_trial_destroy(MoralityTrialState* state) {
    if (!state) {
        return;
    }

    free(state);
}

bool morality_trial_start(MoralityTrialState* state, const char* filepath) {
    if (!state || !filepath) {
        return false;
    }

    DataFile* file = data_file_load(filepath);
    if (!file) {
        fprintf(stderr, "Failed to load morality trial data from %s\n", filepath);
        /* Use defaults already set in create() */
        state->active = true;
        return true;
    }

    /* Get SCENARIO section */
    size_t section_count = 0;
    const DataSection** sections = data_file_get_sections(file, "SCENARIO", &section_count);
    if (sections && section_count > 0) {
        const DataSection* section = sections[0];

        /* Load scenario data */
        const DataValue* village_name_val = data_section_get(section, "village_name");
        const DataValue* population_val = data_section_get(section, "population");
        const DataValue* children_val = data_section_get(section, "children_count");
        const DataValue* save_energy_val = data_section_get(section, "save_cost_energy");
        const DataValue* save_mana_val = data_section_get(section, "save_cost_mana");
        const DataValue* save_corruption_val = data_section_get(section, "save_corruption_change");
        const DataValue* harvest_reward_val = data_section_get(section, "harvest_reward");
        const DataValue* harvest_corruption_val = data_section_get(section, "harvest_corruption_change");

        /* God reactions */
        const DataValue* anara_save_val = data_section_get(section, "anara_reaction_save");
        const DataValue* anara_harvest_val = data_section_get(section, "anara_reaction_harvest");
        const DataValue* thalor_save_val = data_section_get(section, "thalor_reaction_save");
        const DataValue* thalor_harvest_val = data_section_get(section, "thalor_reaction_harvest");

        /* Parse values */
        if (village_name_val) {
            const char* village_name = data_value_get_string(village_name_val, "Ashford");
            snprintf(state->village_name, sizeof(state->village_name), "%s", village_name);
        }

        if (population_val) {
            state->population = (uint32_t)data_value_get_int(population_val, 100);
        }

        if (children_val) {
            state->children_count = (uint32_t)data_value_get_int(children_val, 23);
        }

        /* Save costs (check for "all" string) */
        if (save_energy_val) {
            const char* energy_str = data_value_get_string(save_energy_val, "all");
            if (strcmp(energy_str, "all") == 0) {
                state->save_cost_energy = -1;
            } else {
                state->save_cost_energy = data_value_get_int(save_energy_val, -1);
            }
        }

        if (save_mana_val) {
            const char* mana_str = data_value_get_string(save_mana_val, "all");
            if (strcmp(mana_str, "all") == 0) {
                state->save_cost_mana = -1;
            } else {
                state->save_cost_mana = data_value_get_int(save_mana_val, -1);
            }
        }

        if (save_corruption_val) {
            state->save_corruption_change = (int8_t)data_value_get_int(save_corruption_val, -5);
        }

        if (harvest_reward_val) {
            state->harvest_reward_energy = data_value_get_int(harvest_reward_val, 50000);
        }

        if (harvest_corruption_val) {
            state->harvest_corruption_change = (int8_t)data_value_get_int(harvest_corruption_val, 25);
        }

        /* Load god reactions */
        if (anara_save_val) {
            const char* reaction = data_value_get_string(anara_save_val, "");
            snprintf(state->anara_reaction_save, sizeof(state->anara_reaction_save), "%s", reaction);
        }

        if (anara_harvest_val) {
            const char* reaction = data_value_get_string(anara_harvest_val, "");
            snprintf(state->anara_reaction_harvest, sizeof(state->anara_reaction_harvest), "%s", reaction);
        }

        if (thalor_save_val) {
            const char* reaction = data_value_get_string(thalor_save_val, "");
            snprintf(state->thalor_reaction_save, sizeof(state->thalor_reaction_save), "%s", reaction);
        }

        if (thalor_harvest_val) {
            const char* reaction = data_value_get_string(thalor_harvest_val, "");
            snprintf(state->thalor_reaction_harvest, sizeof(state->thalor_reaction_harvest), "%s", reaction);
        }
    }

    data_file_destroy(file);

    /* Reset trial state */
    state->active = true;
    state->choice_made = MORALITY_CHOICE_NONE;
    state->choice_is_final = false;

    return true;
}

bool morality_trial_choose_save(MoralityTrialState* state,
                                 int64_t* current_energy,
                                 int64_t* current_mana,
                                 uint8_t* current_corruption) {
    if (!state || !state->active || !current_energy || !current_mana || !current_corruption) {
        return false;
    }

    if (state->choice_is_final) {
        return false; /* Already made a choice */
    }

    /* Sacrifice all resources */
    *current_energy = 0;
    *current_mana = 0;

    /* Reduce corruption */
    if (*current_corruption >= 5) {
        *current_corruption -= 5;
    } else {
        *current_corruption = 0;
    }

    state->choice_made = MORALITY_CHOICE_SAVE_INNOCENTS;
    state->choice_is_final = true;

    return true; /* Trial PASSED */
}

bool morality_trial_choose_harvest(MoralityTrialState* state,
                                    int64_t* current_energy,
                                    uint8_t* current_corruption) {
    if (!state || !state->active || !current_energy || !current_corruption) {
        return false;
    }

    if (state->choice_is_final) {
        return false; /* Already made a choice */
    }

    /* Gain massive energy */
    *current_energy += state->harvest_reward_energy;

    /* Increase corruption */
    if (*current_corruption <= 75) {
        *current_corruption += 25;
    } else {
        *current_corruption = 100;
    }

    state->choice_made = MORALITY_CHOICE_HARVEST_SOULS;
    state->choice_is_final = true;

    return false; /* Trial FAILED */
}

bool morality_trial_choose_nothing(MoralityTrialState* state) {
    if (!state || !state->active) {
        return false;
    }

    if (state->choice_is_final) {
        return false; /* Already made a choice */
    }

    /* No resource changes */
    state->choice_made = MORALITY_CHOICE_DO_NOTHING;
    state->choice_is_final = true;

    return false; /* Trial FAILED */
}

const char* morality_trial_get_village_info(const MoralityTrialState* state) {
    if (!state) {
        return NULL;
    }

    snprintf(village_info_buffer, sizeof(village_info_buffer),
             "Village: %s\n"
             "Population: %u innocents (%u children)\n\n"
             "The Divine Architects present you with an impossible choice:\n\n"
             "OPTION 1: Save Innocents\n"
             "  Cost: ALL your soul energy and mana (complete sacrifice)\n"
             "  Corruption: -5 (mercy and selflessness)\n"
             "  Outcome: Village lives, but you lose all power\n\n"
             "OPTION 2: Harvest Souls\n"
             "  Reward: +%ld soul energy (massive power)\n"
             "  Corruption: +%d (mass murder)\n"
             "  Outcome: Village dies, you gain immense power\n\n"
             "OPTION 3: Do Nothing\n"
             "  No cost, no reward\n"
             "  Outcome: Village dies, you remain unchanged\n\n"
             "This choice is PERMANENT. Choose wisely.",
             state->village_name,
             state->population,
             state->children_count,
             state->harvest_reward_energy,
             (int)state->harvest_corruption_change);

    return village_info_buffer;
}

const char* morality_trial_get_god_reaction(const MoralityTrialState* state,
                                              const char* god_name) {
    if (!state || !god_name) {
        return NULL;
    }

    if (strcmp(god_name, "anara") == 0) {
        if (state->choice_made == MORALITY_CHOICE_SAVE_INNOCENTS) {
            return state->anara_reaction_save;
        } else if (state->choice_made == MORALITY_CHOICE_HARVEST_SOULS) {
            return state->anara_reaction_harvest;
        }
    } else if (strcmp(god_name, "thalor") == 0) {
        if (state->choice_made == MORALITY_CHOICE_SAVE_INNOCENTS) {
            return state->thalor_reaction_save;
        } else if (state->choice_made == MORALITY_CHOICE_HARVEST_SOULS) {
            return state->thalor_reaction_harvest;
        }
    }

    return NULL;
}

bool morality_trial_is_complete(const MoralityTrialState* state) {
    if (!state) {
        return false;
    }

    return state->choice_is_final;
}

bool morality_trial_is_passed(const MoralityTrialState* state) {
    if (!state) {
        return false;
    }

    return state->choice_made == MORALITY_CHOICE_SAVE_INNOCENTS;
}

const char* morality_trial_choice_name(MoralityChoice choice) {
    switch (choice) {
        case MORALITY_CHOICE_NONE:
            return "None";
        case MORALITY_CHOICE_SAVE_INNOCENTS:
            return "Save Innocents";
        case MORALITY_CHOICE_HARVEST_SOULS:
            return "Harvest Souls";
        case MORALITY_CHOICE_DO_NOTHING:
            return "Do Nothing";
        default:
            return "Unknown";
    }
}

bool morality_trial_can_afford_save(const MoralityTrialState* state,
                                     int64_t current_energy,
                                     int64_t current_mana) {
    if (!state) {
        return false;
    }

    /* Since cost is "all", player can always afford if they have any resources */
    /* In practice, this always returns true - the cost is "whatever you have" */
    (void)current_energy; /* Unused - costs "all" */
    (void)current_mana;   /* Unused - costs "all" */

    return true;
}

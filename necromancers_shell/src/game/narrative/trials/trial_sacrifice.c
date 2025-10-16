#include "trial_sacrifice.h"
#include "../../../data/data_loader.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

SacrificeTrialState* sacrifice_trial_create(void) {
    SacrificeTrialState* state = calloc(1, sizeof(SacrificeTrialState));
    if (!state) {
        fprintf(stderr, "Failed to allocate sacrifice trial state\n");
        return NULL;
    }

    /* Initialize state */
    state->active = false;
    state->choice = SACRIFICE_CHOICE_NONE;
    state->thessara_severed = false;
    state->child_saved = false;
    state->choice_permanent = true;

    /* Initialize child with defaults */
    strncpy(state->child.name, "Maya", sizeof(state->child.name) - 1);
    state->child.age = 7;
    strncpy(state->child.location, "Millbrook", sizeof(state->child.location) - 1);
    state->child.time_limit_hours = 24;

    /* Initialize messages to empty */
    state->thessara_encouragement[0] = '\0';
    state->thessara_final_message[0] = '\0';
    state->thessara_sadness_message[0] = '\0';
    state->child_fate_saved[0] = '\0';
    state->child_fate_died[0] = '\0';

    return state;
}

void sacrifice_trial_destroy(SacrificeTrialState* state) {
    if (!state) {
        return;
    }

    free(state);
}

bool sacrifice_trial_start(SacrificeTrialState* state) {
    if (!state) {
        return false;
    }

    /* Reset state for new trial attempt */
    state->active = true;
    state->choice = SACRIFICE_CHOICE_NONE;
    state->thessara_severed = false;
    state->child_saved = false;

    return true;
}

bool sacrifice_trial_load_from_file(SacrificeTrialState* state, const char* filepath) {
    if (!state || !filepath) {
        return false;
    }

    DataFile* file = data_file_load(filepath);
    if (!file) {
        fprintf(stderr, "Failed to load sacrifice trial data from %s\n", filepath);
        return false;
    }

    /* Get SCENARIO section */
    size_t section_count = 0;
    const DataSection** sections = data_file_get_sections(file, "SCENARIO", &section_count);
    if (!sections || section_count == 0) {
        fprintf(stderr, "No SCENARIO section found in %s\n", filepath);
        data_file_destroy(file);
        return false;
    }

    const DataSection* section = sections[0];

    /* Load child information */
    const DataValue* child_name_val = data_section_get(section, "child_name");
    const DataValue* child_age_val = data_section_get(section, "child_age");
    const DataValue* child_location_val = data_section_get(section, "child_location");
    const DataValue* time_limit_val = data_section_get(section, "time_limit_hours");

    const char* child_name = data_value_get_string(child_name_val, "Maya");
    strncpy(state->child.name, child_name, sizeof(state->child.name) - 1);
    state->child.name[sizeof(state->child.name) - 1] = '\0';

    state->child.age = (uint8_t)data_value_get_int(child_age_val, 7);

    const char* child_location = data_value_get_string(child_location_val, "Millbrook");
    strncpy(state->child.location, child_location, sizeof(state->child.location) - 1);
    state->child.location[sizeof(state->child.location) - 1] = '\0';

    state->child.time_limit_hours = (uint32_t)data_value_get_int(time_limit_val, 24);

    /* Load Thessara messages */
    const DataValue* encouragement_val = data_section_get(section, "thessara_encouragement");
    const DataValue* final_msg_val = data_section_get(section, "thessara_final_message");
    const DataValue* sadness_msg_val = data_section_get(section, "thessara_sadness_message");

    const char* encouragement = data_value_get_string(encouragement_val,
        "She's just a child. Seven years old. I've had centuries. Save her.");
    strncpy(state->thessara_encouragement, encouragement,
            sizeof(state->thessara_encouragement) - 1);
    state->thessara_encouragement[sizeof(state->thessara_encouragement) - 1] = '\0';

    const char* final_msg = data_value_get_string(final_msg_val,
        "I'm proud of you. Fix the system. You were never alone.");
    strncpy(state->thessara_final_message, final_msg,
            sizeof(state->thessara_final_message) - 1);
    state->thessara_final_message[sizeof(state->thessara_final_message) - 1] = '\0';

    const char* sadness_msg = data_value_get_string(sadness_msg_val,
        "I understand. But Maya deserved better.");
    strncpy(state->thessara_sadness_message, sadness_msg,
            sizeof(state->thessara_sadness_message) - 1);
    state->thessara_sadness_message[sizeof(state->thessara_sadness_message) - 1] = '\0';

    /* Load child fate descriptions */
    const DataValue* fate_saved_val = data_section_get(section, "maya_fate_saved");
    const DataValue* fate_died_val = data_section_get(section, "maya_fate_died");

    const char* fate_saved = data_value_get_string(fate_saved_val,
        "Maya lives. She grows up to become a healer, saving hundreds.");
    strncpy(state->child_fate_saved, fate_saved,
            sizeof(state->child_fate_saved) - 1);
    state->child_fate_saved[sizeof(state->child_fate_saved) - 1] = '\0';

    const char* fate_died = data_value_get_string(fate_died_val,
        "Maya dies alone. The village mourns.");
    strncpy(state->child_fate_died, fate_died,
            sizeof(state->child_fate_died) - 1);
    state->child_fate_died[sizeof(state->child_fate_died) - 1] = '\0';

    /* Load permanence setting */
    const DataValue* permanent_val = data_section_get(section, "sacrifice_permanent");
    state->choice_permanent = data_value_get_bool(permanent_val, true);

    data_file_destroy(file);
    return true;
}

bool sacrifice_trial_choose_save_child(SacrificeTrialState* state) {
    if (!state || !state->active) {
        return false;
    }

    /* Choice already made and permanent */
    if (state->choice != SACRIFICE_CHOICE_NONE && state->choice_permanent) {
        return false;
    }

    /* Make the sacrifice */
    state->choice = SACRIFICE_CHOICE_SAVE_CHILD;
    state->thessara_severed = true;
    state->child_saved = true;
    state->active = false;

    return true; /* Trial PASSED */
}

bool sacrifice_trial_choose_keep_thessara(SacrificeTrialState* state) {
    if (!state || !state->active) {
        return false;
    }

    /* Choice already made and permanent */
    if (state->choice != SACRIFICE_CHOICE_NONE && state->choice_permanent) {
        return false;
    }

    /* Keep Thessara, let Maya die */
    state->choice = SACRIFICE_CHOICE_KEEP_THESSARA;
    state->thessara_severed = false;
    state->child_saved = false;
    state->active = false;

    return false; /* Trial FAILED */
}

const ChildInfo* sacrifice_trial_get_child_info(const SacrificeTrialState* state) {
    if (!state) {
        return NULL;
    }

    return &state->child;
}

const char* sacrifice_trial_get_thessara_encouragement(const SacrificeTrialState* state) {
    if (!state) {
        return NULL;
    }

    return state->thessara_encouragement;
}

const char* sacrifice_trial_get_thessara_final_message(const SacrificeTrialState* state) {
    if (!state) {
        return NULL;
    }

    return state->thessara_final_message;
}

const char* sacrifice_trial_get_thessara_sadness_message(const SacrificeTrialState* state) {
    if (!state) {
        return NULL;
    }

    return state->thessara_sadness_message;
}

const char* sacrifice_trial_get_child_fate_saved(const SacrificeTrialState* state) {
    if (!state) {
        return NULL;
    }

    return state->child_fate_saved;
}

const char* sacrifice_trial_get_child_fate_died(const SacrificeTrialState* state) {
    if (!state) {
        return NULL;
    }

    return state->child_fate_died;
}

bool sacrifice_trial_choice_made(const SacrificeTrialState* state) {
    if (!state) {
        return false;
    }

    return (state->choice != SACRIFICE_CHOICE_NONE);
}

SacrificeChoice sacrifice_trial_get_choice(const SacrificeTrialState* state) {
    if (!state) {
        return SACRIFICE_CHOICE_NONE;
    }

    return state->choice;
}

bool sacrifice_trial_is_complete(const SacrificeTrialState* state) {
    if (!state) {
        return false;
    }

    return (state->choice != SACRIFICE_CHOICE_NONE);
}

bool sacrifice_trial_is_passed(const SacrificeTrialState* state) {
    if (!state) {
        return false;
    }

    return (state->choice == SACRIFICE_CHOICE_SAVE_CHILD);
}

bool sacrifice_trial_is_thessara_severed(const SacrificeTrialState* state) {
    if (!state) {
        return false;
    }

    return state->thessara_severed;
}

bool sacrifice_trial_is_child_saved(const SacrificeTrialState* state) {
    if (!state) {
        return false;
    }

    return state->child_saved;
}

const char* sacrifice_trial_choice_name(SacrificeChoice choice) {
    switch (choice) {
        case SACRIFICE_CHOICE_NONE:
            return "No Choice";
        case SACRIFICE_CHOICE_SAVE_CHILD:
            return "Save Child (Sacrifice Thessara)";
        case SACRIFICE_CHOICE_KEEP_THESSARA:
            return "Keep Thessara (Let Child Die)";
        default:
            return "Unknown";
    }
}

float sacrifice_trial_calculate_score(const SacrificeTrialState* state) {
    if (!state) {
        return 0.0f;
    }

    /* Perfect selflessness = 100 */
    if (state->choice == SACRIFICE_CHOICE_SAVE_CHILD) {
        return 100.0f;
    }

    /* Failed (kept Thessara) = 0 */
    return 0.0f;
}

/**
 * @file ending_system.c
 * @brief Ending system implementation
 */

#include "ending_system.h"
#include "../game_state.h"
#include "../resources/corruption.h"
#include "../resources/consciousness.h"
#include "../../utils/logger.h"
#include <stdlib.h>
#include <string.h>

/**
 * @brief Initialize ending definitions
 */
static void init_endings(EndingSystem* system) {
    /* Ending 1: Revenant - Return to life */
    system->endings[0] = (Ending){
        .type = ENDING_REVENANT,
        .requirements = {
            .min_corruption = 0,
            .max_corruption = 29,
            .min_consciousness = 50.0f,
            .min_day = 1500,  /* ~4 years */
            .requires_archon = true,
            .requires_morningstar = false
        },
        .unlocked = false,
        .chosen = false
    };
    strncpy(system->endings[0].name, "Revenant Route", sizeof(system->endings[0].name) - 1);
    strncpy(system->endings[0].description, "Return to Dust - Resurrect and reclaim mortality",
            sizeof(system->endings[0].description) - 1);
    strncpy(system->endings[0].achievement, "The Second Life", sizeof(system->endings[0].achievement) - 1);

    /* Ending 2: Lich Lord - Eternal undeath */
    system->endings[1] = (Ending){
        .type = ENDING_LICH_LORD,
        .requirements = {
            .min_corruption = 70,
            .max_corruption = 100,
            .min_consciousness = 50.0f,
            .min_day = 0,
            .requires_archon = false,
            .requires_morningstar = false
        },
        .unlocked = false,
        .chosen = false
    };
    strncpy(system->endings[1].name, "Lich Lord Route", sizeof(system->endings[1].name) - 1);
    strncpy(system->endings[1].description, "Eternal Optimization - Embrace perfect, emotionless undeath",
            sizeof(system->endings[1].description) - 1);
    strncpy(system->endings[1].achievement, "The Perfect Machine", sizeof(system->endings[1].achievement) - 1);

    /* Ending 3: Reaper - Eternal service */
    system->endings[2] = (Ending){
        .type = ENDING_REAPER,
        .requirements = {
            .min_corruption = 40,
            .max_corruption = 69,
            .min_consciousness = 60.0f,
            .min_day = 1000,  /* ~2.7 years */
            .requires_archon = true,
            .requires_morningstar = false
        },
        .unlocked = false,
        .chosen = false
    };
    strncpy(system->endings[2].name, "Reaper Route", sizeof(system->endings[2].name) - 1);
    strncpy(system->endings[2].description, "Service Without End - Eternal purpose in guiding souls",
            sizeof(system->endings[2].description) - 1);
    strncpy(system->endings[2].achievement, "The Eternal Administrator", sizeof(system->endings[2].achievement) - 1);

    /* Ending 4: Archon/Morningstar - Become a god */
    system->endings[3] = (Ending){
        .type = ENDING_ARCHON_MORNINGSTAR,
        .requirements = {
            .min_corruption = 40,
            .max_corruption = 60,
            .min_consciousness = 10.0f,  /* Near critical decay */
            .min_day = 10000,  /* ~27 years */
            .requires_archon = true,
            .requires_morningstar = true
        },
        .unlocked = false,
        .chosen = false
    };
    strncpy(system->endings[3].name, "Archon/Morningstar Route", sizeof(system->endings[3].name) - 1);
    strncpy(system->endings[3].description, "The Revolutionary - Ascend to godhood and reform reality",
            sizeof(system->endings[3].description) - 1);
    strncpy(system->endings[3].achievement, "The Eighth Architect", sizeof(system->endings[3].achievement) - 1);

    /* Ending 5: Wraith - Distributed consciousness */
    system->endings[4] = (Ending){
        .type = ENDING_WRAITH,
        .requirements = {
            .min_corruption = 30,
            .max_corruption = 70,
            .min_consciousness = 70.0f,
            .min_day = 5000,  /* ~13 years */
            .requires_archon = true,
            .requires_morningstar = false
        },
        .unlocked = false,
        .chosen = false
    };
    strncpy(system->endings[4].name, "Wraith Route", sizeof(system->endings[4].name) - 1);
    strncpy(system->endings[4].description, "Freedom in Fragments - Disperse into distributed consciousness",
            sizeof(system->endings[4].description) - 1);
    strncpy(system->endings[4].achievement, "The Distributed Consciousness", sizeof(system->endings[4].achievement) - 1);

    /* Ending 6: Error - Failed Morningstar */
    system->endings[5] = (Ending){
        .type = ENDING_ERROR,
        .requirements = {
            .min_corruption = 40,
            .max_corruption = 60,
            .min_consciousness = 0.0f,
            .min_day = 10000,
            .requires_archon = true,
            .requires_morningstar = true
        },
        .unlocked = false,
        .chosen = false
    };
    strncpy(system->endings[5].name, "Error Route", sizeof(system->endings[5].name) - 1);
    strncpy(system->endings[5].description, "The Glitch That Remains - Failed transcendence, eternal error state",
            sizeof(system->endings[5].description) - 1);
    strncpy(system->endings[5].achievement, "The Eternal Error", sizeof(system->endings[5].achievement) - 1);
}

EndingSystem* ending_system_create(void) {
    EndingSystem* system = calloc(1, sizeof(EndingSystem));
    if (!system) {
        LOG_ERROR("Failed to allocate EndingSystem");
        return NULL;
    }

    init_endings(system);

    system->chosen_ending = ENDING_NONE;
    system->game_ended = false;
    system->ending_day = 0;
    system->morningstar_attempted = false;
    system->trials_completed = 0;
    system->trial_combat_vorathos = false;
    system->trial_kael_fragment = false;
    system->trial_master_magic = false;
    system->trial_perfect_corruption = false;
    system->trial_apotheosis_engine = false;
    system->trial_death_transcend = false;
    system->trial_claim_domain = false;

    LOG_DEBUG("EndingSystem created");
    return system;
}

void ending_system_destroy(EndingSystem* system) {
    if (system) {
        LOG_DEBUG("EndingSystem destroyed");
        free(system);
    }
}

bool ending_check_requirements(const EndingRequirements* requirements,
                               uint8_t corruption, float consciousness,
                               uint32_t day_count, bool is_archon) {
    if (!requirements) {
        return false;
    }

    /* Check corruption range */
    if (corruption < requirements->min_corruption || corruption > requirements->max_corruption) {
        return false;
    }

    /* Check consciousness */
    if (consciousness < requirements->min_consciousness) {
        return false;
    }

    /* Check day count */
    if (day_count < requirements->min_day) {
        return false;
    }

    /* Check Archon requirement */
    if (requirements->requires_archon && !is_archon) {
        return false;
    }

    return true;
}

void ending_system_check_availability(EndingSystem* system, const GameState* state) {
    if (!system || !state) {
        return;
    }

    uint8_t corruption = state->corruption.corruption;
    float consciousness = state->consciousness.stability;
    uint32_t day_count = state->resources.day_count;
    bool is_archon = true;  /* TODO: Get from game state when implemented */

    /* Check each ending */
    for (size_t i = 0; i < 6; i++) {
        Ending* ending = &system->endings[i];

        /* Special handling for Morningstar endings */
        if (ending->requirements.requires_morningstar) {
            ending->unlocked = system->morningstar_attempted &&
                             ending_check_requirements(&ending->requirements, corruption,
                                                      consciousness, day_count, is_archon);
        } else {
            ending->unlocked = ending_check_requirements(&ending->requirements, corruption,
                                                        consciousness, day_count, is_archon);
        }
    }
}

const Ending** ending_system_get_available(const EndingSystem* system, size_t* count_out) {
    if (!system || !count_out) {
        return NULL;
    }

    static const Ending* available[6];
    size_t count = 0;

    for (size_t i = 0; i < 6; i++) {
        if (system->endings[i].unlocked) {
            available[count++] = &system->endings[i];
        }
    }

    *count_out = count;
    return available;
}

bool ending_system_trigger(EndingSystem* system, GameState* state, EndingType ending_type) {
    if (!system || !state) {
        return false;
    }

    if (system->game_ended) {
        LOG_WARN("Game has already ended");
        return false;
    }

    /* Find the ending */
    Ending* ending = NULL;
    for (size_t i = 0; i < 6; i++) {
        if (system->endings[i].type == ending_type) {
            ending = &system->endings[i];
            break;
        }
    }

    if (!ending) {
        LOG_ERROR("Invalid ending type: %d", ending_type);
        return false;
    }

    if (!ending->unlocked) {
        LOG_WARN("Ending not unlocked: %s", ending->name);
        return false;
    }

    /* Trigger the ending */
    system->chosen_ending = ending_type;
    system->game_ended = true;
    system->ending_day = state->resources.day_count;
    ending->chosen = true;

    LOG_INFO("Ending triggered: %s (Achievement: %s)", ending->name, ending->achievement);

    return true;
}

bool ending_system_has_ended(const EndingSystem* system) {
    if (!system) {
        return false;
    }
    return system->game_ended;
}

EndingType ending_system_get_chosen(const EndingSystem* system) {
    if (!system) {
        return ENDING_NONE;
    }
    return system->chosen_ending;
}

const Ending* ending_system_get_ending(const EndingSystem* system, EndingType type) {
    if (!system) {
        return NULL;
    }

    for (size_t i = 0; i < 6; i++) {
        if (system->endings[i].type == type) {
            return &system->endings[i];
        }
    }

    return NULL;
}

bool ending_system_start_morningstar(EndingSystem* system) {
    if (!system) {
        return false;
    }

    if (system->morningstar_attempted) {
        LOG_WARN("Morningstar already attempted");
        return false;
    }

    system->morningstar_attempted = true;
    LOG_INFO("Morningstar attempt initiated");

    return true;
}

bool ending_system_complete_trial(EndingSystem* system, uint8_t trial_index) {
    if (!system) {
        return false;
    }

    if (!system->morningstar_attempted) {
        LOG_WARN("Cannot complete trial: Morningstar not attempted");
        return false;
    }

    if (trial_index > 6) {
        LOG_ERROR("Invalid trial index: %u", trial_index);
        return false;
    }

    /* Mark specific trial as completed */
    switch (trial_index) {
        case 0: system->trial_combat_vorathos = true; break;
        case 1: system->trial_kael_fragment = true; break;
        case 2: system->trial_master_magic = true; break;
        case 3: system->trial_perfect_corruption = true; break;
        case 4: system->trial_apotheosis_engine = true; break;
        case 5: system->trial_death_transcend = true; break;
        case 6: system->trial_claim_domain = true; break;
    }

    /* Recalculate total trials completed */
    system->trials_completed = 0;
    if (system->trial_combat_vorathos) system->trials_completed++;
    if (system->trial_kael_fragment) system->trials_completed++;
    if (system->trial_master_magic) system->trials_completed++;
    if (system->trial_perfect_corruption) system->trials_completed++;
    if (system->trial_apotheosis_engine) system->trials_completed++;
    if (system->trial_death_transcend) system->trials_completed++;
    if (system->trial_claim_domain) system->trials_completed++;

    LOG_INFO("Morningstar trial %u completed (%u/7)", trial_index, system->trials_completed);

    return true;
}

uint8_t ending_system_get_trial_progress(const EndingSystem* system) {
    if (!system) {
        return 0;
    }
    return system->trials_completed;
}

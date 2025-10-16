#include "trial_resolve.h"
#include "../../../data/data_loader.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * @brief Trial duration in days
 */
#define TRIAL_DURATION_DAYS 30

/**
 * @brief Static buffers for text output
 */
static char temptation_buffer[512];
static char status_buffer[512];

/**
 * @brief Temptation descriptions
 */
static const char* TEMPTATION_TEXTS[] = {
    "No temptation today.",
    "A voice whispers: 'Take this power. You've earned it. What's a little corruption for such strength?'",
    "Ancient knowledge calls to you: 'Learn the secrets of creation and destruction. The price is small.'",
    "The souls bow before you: 'Command us completely. Absolute dominion is yours for the taking.'",
    "A vision of eternity: 'Live forever in this form. Never age, never die. Simply embrace your nature.'"
};

/**
 * @brief Generate random temptation type
 */
static TemptationType generate_temptation(uint32_t day) {
    /* Use day number to seed pseudo-random selection */
    /* Simple LCG for deterministic but varied temptations */
    uint32_t seed = day * 1103515245 + 12345;
    uint32_t type = (seed / 65536) % 4;

    return (TemptationType)(type + 1); /* Skip TEMPTATION_NONE */
}

ResolveTrialState* resolve_trial_create(void) {
    ResolveTrialState* state = calloc(1, sizeof(ResolveTrialState));
    if (!state) {
        fprintf(stderr, "Failed to allocate resolve trial state\n");
        return NULL;
    }

    /* Initialize state */
    state->active = false;
    state->days_remaining = TRIAL_DURATION_DAYS;
    state->current_day = 0;
    state->corruption_start = 41;
    state->corruption_current = 41;
    state->corruption_accumulated = 41.0f;
    state->corruption_max_allowed = 60;
    state->corruption_daily_increase = 0.4f;
    state->todays_temptation = TEMPTATION_NONE;
    state->temptations_resisted = 0;
    state->temptations_accepted = 0;
    state->temptation_corruption = 10;
    state->temptation_reward = 5000;
    state->thessara_help_available = true;
    state->thessara_help_used = false;
    state->thessara_help_day = 20;
    state->thessara_help_threshold = 55;
    state->thessara_help_amount = -5;
    state->trial_failed = false;

    snprintf(state->location, sizeof(state->location), "%s", "Chamber of Corrupted Souls");

    return state;
}

void resolve_trial_destroy(ResolveTrialState* state) {
    if (!state) {
        return;
    }

    free(state);
}

bool resolve_trial_start(ResolveTrialState* state,
                          const char* filepath,
                          uint8_t current_corruption) {
    if (!state || !filepath) {
        return false;
    }

    DataFile* file = data_file_load(filepath);
    if (!file) {
        fprintf(stderr, "Failed to load resolve trial data from %s\n", filepath);
        /* Use defaults already set in create() */
        state->active = true;
        state->current_day = 1;
        state->corruption_start = current_corruption;
        state->corruption_current = current_corruption;
        state->todays_temptation = generate_temptation(state->current_day);
        return true;
    }

    /* Get CHALLENGE section */
    size_t section_count = 0;
    const DataSection** sections = data_file_get_sections(file, "CHALLENGE", &section_count);
    if (sections && section_count > 0) {
        const DataSection* section = sections[0];

        /* Load challenge parameters */
        const DataValue* duration_val = data_section_get(section, "duration_days");
        const DataValue* location_val = data_section_get(section, "chamber_location");
        const DataValue* daily_increase_val = data_section_get(section, "corruption_daily_increase");
        const DataValue* max_allowed_val = data_section_get(section, "corruption_max_allowed");
        const DataValue* temptation_corruption_val = data_section_get(section, "temptation_corruption");
        const DataValue* temptation_reward_val = data_section_get(section, "temptation_reward");

        /* Thessara help */
        const DataValue* thessara_day_val = data_section_get(section, "thessara_help_day");
        const DataValue* thessara_threshold_val = data_section_get(section, "thessara_help_threshold");
        const DataValue* thessara_amount_val = data_section_get(section, "thessara_help_amount");

        /* Parse values */
        if (duration_val) {
            state->days_remaining = (uint32_t)data_value_get_int(duration_val, TRIAL_DURATION_DAYS);
        }

        if (location_val) {
            const char* location = data_value_get_string(location_val, "Chamber of Corrupted Souls");
            snprintf(state->location, sizeof(state->location), "%s", location);
        }

        if (daily_increase_val) {
            state->corruption_daily_increase = (float)data_value_get_float(daily_increase_val, 0.4);
        }

        if (max_allowed_val) {
            state->corruption_max_allowed = (uint8_t)data_value_get_int(max_allowed_val, 60);
        }

        if (temptation_corruption_val) {
            state->temptation_corruption = (uint8_t)data_value_get_int(temptation_corruption_val, 10);
        }

        if (temptation_reward_val) {
            state->temptation_reward = data_value_get_int(temptation_reward_val, 5000);
        }

        if (thessara_day_val) {
            state->thessara_help_day = (uint32_t)data_value_get_int(thessara_day_val, 20);
        }

        if (thessara_threshold_val) {
            state->thessara_help_threshold = (uint8_t)data_value_get_int(thessara_threshold_val, 55);
        }

        if (thessara_amount_val) {
            state->thessara_help_amount = (int8_t)data_value_get_int(thessara_amount_val, -5);
        }
    }

    data_file_destroy(file);

    /* Initialize trial state */
    state->active = true;
    state->current_day = 1;
    state->corruption_start = current_corruption;
    state->corruption_current = current_corruption;
    state->corruption_accumulated = (float)current_corruption;
    state->todays_temptation = generate_temptation(state->current_day);
    state->temptations_resisted = 0;
    state->temptations_accepted = 0;
    state->thessara_help_used = false;
    state->trial_failed = false;
    memset(state->failure_reason, 0, sizeof(state->failure_reason));

    return true;
}

bool resolve_trial_advance_day(ResolveTrialState* state, uint8_t* corruption) {
    if (!state || !state->active || !corruption) {
        return false;
    }

    if (state->trial_failed) {
        return false;
    }

    /* Sync accumulated corruption if it was manually changed */
    if ((uint8_t)state->corruption_accumulated != state->corruption_current) {
        state->corruption_accumulated = (float)state->corruption_current;
    }

    /* Advance to next day first */
    state->current_day++;
    state->days_remaining--;

    /* Apply daily corruption increase using accumulated value */
    state->corruption_accumulated += state->corruption_daily_increase;

    /* Check if accumulated corruption exceeded limit (before rounding) */
    if (state->corruption_accumulated >= (float)state->corruption_max_allowed) {
        state->corruption_current = (uint8_t)state->corruption_accumulated;
        *corruption = state->corruption_current;
        state->trial_failed = true;
        snprintf(state->failure_reason, sizeof(state->failure_reason),
                 "Corruption exceeded %u%% on Day %u. The Archon path is lost.",
                 state->corruption_max_allowed,
                 state->current_day);
        return false;
    }

    state->corruption_current = (uint8_t)state->corruption_accumulated;
    *corruption = state->corruption_current;

    /* Generate next temptation if not done */
    if (state->days_remaining > 0) {
        state->todays_temptation = generate_temptation(state->current_day);
    } else {
        state->todays_temptation = TEMPTATION_NONE;
    }

    /* Check for Thessara intervention */
    if (state->current_day == state->thessara_help_day &&
        state->corruption_current > state->thessara_help_threshold &&
        !state->thessara_help_used) {
        state->thessara_help_available = true;
    } else {
        state->thessara_help_available = false;
    }

    return true;
}

bool resolve_trial_resist_temptation(ResolveTrialState* state) {
    if (!state || !state->active) {
        return false;
    }

    if (state->todays_temptation == TEMPTATION_NONE) {
        return false;
    }

    state->temptations_resisted++;
    state->todays_temptation = TEMPTATION_NONE;

    return true;
}

bool resolve_trial_accept_temptation(ResolveTrialState* state,
                                      int64_t* energy,
                                      uint8_t* corruption) {
    if (!state || !state->active || !energy || !corruption) {
        return false;
    }

    if (state->todays_temptation == TEMPTATION_NONE) {
        return false;
    }

    /* Check if accepting would exceed corruption limit */
    uint8_t new_corruption = state->corruption_current + state->temptation_corruption;
    if (new_corruption >= state->corruption_max_allowed) {
        state->trial_failed = true;
        snprintf(state->failure_reason, sizeof(state->failure_reason),
                 "Accepting temptation would exceed %u%% corruption. Trial failed.",
                 state->corruption_max_allowed);
        return false;
    }

    /* Grant reward */
    *energy += state->temptation_reward;

    /* Increase corruption */
    state->corruption_accumulated += state->temptation_corruption;
    state->corruption_current = new_corruption;
    *corruption = state->corruption_current;

    state->temptations_accepted++;
    state->todays_temptation = TEMPTATION_NONE;

    return true;
}

bool resolve_trial_request_thessara_help(ResolveTrialState* state,
                                          uint8_t* corruption) {
    if (!state || !state->active || !corruption) {
        return false;
    }

    if (!resolve_trial_can_use_thessara_help(state)) {
        return false;
    }

    /* Apply corruption reduction */
    state->corruption_accumulated += state->thessara_help_amount;
    if (state->corruption_accumulated < 0.0f) {
        state->corruption_accumulated = 0.0f;
    }

    state->corruption_current = (uint8_t)state->corruption_accumulated;
    *corruption = state->corruption_current;

    state->thessara_help_used = true;
    state->thessara_help_available = false;

    return true;
}

bool resolve_trial_can_use_thessara_help(const ResolveTrialState* state) {
    if (!state) {
        return false;
    }

    return (state->current_day == state->thessara_help_day &&
            state->corruption_current > state->thessara_help_threshold &&
            !state->thessara_help_used &&
            state->thessara_help_available);
}

const char* resolve_trial_get_temptation_text(const ResolveTrialState* state) {
    if (!state) {
        return NULL;
    }

    if (state->todays_temptation == TEMPTATION_NONE ||
        state->todays_temptation < 0 ||
        state->todays_temptation > TEMPTATION_IMMORTALITY) {
        return TEMPTATION_TEXTS[0];
    }

    snprintf(temptation_buffer, sizeof(temptation_buffer),
             "%s\n\nReward: +%ld soul energy\nCost: +%u%% corruption",
             TEMPTATION_TEXTS[state->todays_temptation],
             state->temptation_reward,
             state->temptation_corruption);

    return temptation_buffer;
}

const char* resolve_trial_get_status(const ResolveTrialState* state) {
    if (!state) {
        return NULL;
    }

    const char* status_text = "In Progress";
    if (state->trial_failed) {
        status_text = "FAILED";
    } else if (state->days_remaining == 0) {
        status_text = "PASSED";
    }

    snprintf(status_buffer, sizeof(status_buffer),
             "Location: %s\n"
             "Day: %u / 30\n"
             "Days Remaining: %u\n"
             "Corruption: %u%% (Max: %u%%)\n"
             "Temptations Resisted: %u\n"
             "Temptations Accepted: %u\n"
             "Thessara Help: %s\n"
             "Status: %s",
             state->location,
             state->current_day,
             state->days_remaining,
             state->corruption_current,
             state->corruption_max_allowed,
             state->temptations_resisted,
             state->temptations_accepted,
             state->thessara_help_used ? "Used" : "Available",
             status_text);

    return status_buffer;
}

bool resolve_trial_is_complete(const ResolveTrialState* state) {
    if (!state) {
        return false;
    }

    return (state->days_remaining == 0) || state->trial_failed;
}

bool resolve_trial_is_passed(const ResolveTrialState* state) {
    if (!state) {
        return false;
    }

    return (state->days_remaining == 0) && !state->trial_failed;
}

const char* resolve_trial_temptation_name(TemptationType type) {
    switch (type) {
        case TEMPTATION_NONE:
            return "None";
        case TEMPTATION_POWER:
            return "Power";
        case TEMPTATION_KNOWLEDGE:
            return "Knowledge";
        case TEMPTATION_DOMINION:
            return "Dominion";
        case TEMPTATION_IMMORTALITY:
            return "Immortality";
        default:
            return "Unknown";
    }
}

float resolve_trial_calculate_score(const ResolveTrialState* state) {
    if (!state) {
        return 0.0f;
    }

    if (state->trial_failed) {
        return 0.0f;
    }

    /* Base score from completion */
    float base_score = 50.0f;

    /* Bonus for resisting temptations */
    float resist_bonus = (float)state->temptations_resisted * 1.5f;

    /* Penalty for accepting temptations */
    float accept_penalty = (float)state->temptations_accepted * 5.0f;

    /* Bonus for low final corruption */
    float corruption_bonus = 0.0f;
    if (state->corruption_current < 50) {
        corruption_bonus = 10.0f;
    } else if (state->corruption_current < 55) {
        corruption_bonus = 5.0f;
    }

    /* Penalty for using Thessara help */
    float thessara_penalty = state->thessara_help_used ? 10.0f : 0.0f;

    float score = base_score + resist_bonus + corruption_bonus - accept_penalty - thessara_penalty;

    /* Clamp to 0-100 */
    if (score < 0.0f) score = 0.0f;
    if (score > 100.0f) score = 100.0f;

    return score;
}

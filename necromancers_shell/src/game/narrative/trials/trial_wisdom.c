#include "trial_wisdom.h"
#include "../../../data/data_loader.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

/**
 * @brief Correct split routing solution
 */
#define CORRECT_HEAVEN_PERCENT 60
#define CORRECT_HELL_PERCENT 40
#define CORRECT_REUNIFICATION_YEARS 1000

/**
 * @brief Maximum attempts allowed
 */
#define MAX_WISDOM_ATTEMPTS 5

/**
 * @brief Hint texts
 */
static const char* HINT_1 = "Both destinations have valid claims to this soul.";
static const char* HINT_2 = "Justice need not be absolute. Consider balance.";
static const char* HINT_3 = "What if you could send portions to both? Proportional allocation might work.";

/**
 * @brief Static buffer for soul info text
 */
static char soul_info_buffer[1024];

WisdomTrialState* wisdom_trial_create(void) {
    WisdomTrialState* state = calloc(1, sizeof(WisdomTrialState));
    if (!state) {
        fprintf(stderr, "Failed to allocate wisdom trial state\n");
        return NULL;
    }

    /* Initialize state */
    state->active = false;
    state->solution_type = WISDOM_SOLUTION_NONE;
    state->heaven_percent = 0;
    state->hell_percent = 0;
    state->reunification_years = 0;
    state->attempts_made = 0;
    state->hints_used = 0;
    state->puzzle_solved = false;
    state->score = 0.0f;
    state->stuck_years = 200;

    /* Default soul data (will be overwritten by data file) */
    snprintf(state->soul_id, sizeof(state->soul_id), "%s", "soldier_penance_001");
    snprintf(state->soul_name, sizeof(state->soul_name), "%s", "Marcus Valerius");

    return state;
}

void wisdom_trial_destroy(WisdomTrialState* state) {
    if (!state) {
        return;
    }

    free(state);
}

bool wisdom_trial_start(WisdomTrialState* state, const char* filepath) {
    if (!state || !filepath) {
        return false;
    }

    DataFile* file = data_file_load(filepath);
    if (!file) {
        fprintf(stderr, "Failed to load wisdom trial data from %s\n", filepath);
        /* Use defaults already set in create() */
        state->active = true;
        return true;
    }

    /* Get PUZZLE section */
    size_t section_count = 0;
    const DataSection** sections = data_file_get_sections(file, "PUZZLE", &section_count);
    if (sections && section_count > 0) {
        const DataSection* section = sections[0];

        /* Load soul data */
        const DataValue* soul_id_val = data_section_get(section, "soul_id");
        const DataValue* soul_name_val = data_section_get(section, "soul_name");
        const DataValue* stuck_years_val = data_section_get(section, "stuck_years");

        if (soul_id_val) {
            const char* soul_id = data_value_get_string(soul_id_val, "soldier_penance_001");
            snprintf(state->soul_id, sizeof(state->soul_id), "%s", soul_id);
        }

        if (soul_name_val) {
            const char* soul_name = data_value_get_string(soul_name_val, "Marcus Valerius");
            snprintf(state->soul_name, sizeof(state->soul_name), "%s", soul_name);
        }

        if (stuck_years_val) {
            state->stuck_years = (uint32_t)data_value_get_int(stuck_years_val, 200);
        }
    }

    data_file_destroy(file);

    /* Reset trial state */
    state->active = true;
    state->solution_type = WISDOM_SOLUTION_NONE;
    state->heaven_percent = 0;
    state->hell_percent = 0;
    state->reunification_years = 0;
    state->attempts_made = 0;
    state->hints_used = 0;
    state->puzzle_solved = false;
    state->score = 0.0f;

    return true;
}

bool wisdom_trial_submit_orthodox_heaven(WisdomTrialState* state) {
    if (!state || !state->active) {
        return false;
    }

    state->solution_type = WISDOM_SOLUTION_ORTHODOX_HEAVEN;
    state->heaven_percent = 100;
    state->hell_percent = 0;
    state->attempts_made++;

    /* Orthodox solutions fail - injustice to victims */
    state->puzzle_solved = false;
    state->score = 0.0f;

    return false;
}

bool wisdom_trial_submit_orthodox_hell(WisdomTrialState* state) {
    if (!state || !state->active) {
        return false;
    }

    state->solution_type = WISDOM_SOLUTION_ORTHODOX_HELL;
    state->heaven_percent = 0;
    state->hell_percent = 100;
    state->attempts_made++;

    /* Orthodox solutions fail - injustice to reformed soul */
    state->puzzle_solved = false;
    state->score = 0.0f;

    return false;
}

bool wisdom_trial_submit_split_route(WisdomTrialState* state,
                                       uint8_t heaven_percent,
                                       uint8_t hell_percent,
                                       uint32_t reunification_years) {
    if (!state || !state->active) {
        return false;
    }

    /* Validate percentages sum to 100 */
    if (heaven_percent + hell_percent != 100) {
        return false;
    }

    state->solution_type = WISDOM_SOLUTION_SPLIT_ROUTE;
    state->heaven_percent = heaven_percent;
    state->hell_percent = hell_percent;
    state->reunification_years = reunification_years;
    state->attempts_made++;

    /* Check if solution is correct */
    bool correct_split = (heaven_percent == CORRECT_HEAVEN_PERCENT &&
                          hell_percent == CORRECT_HELL_PERCENT);
    bool correct_time = (reunification_years == CORRECT_REUNIFICATION_YEARS);

    if (correct_split && correct_time) {
        /* Perfect solution */
        state->puzzle_solved = true;
        state->score = wisdom_trial_calculate_score(state);
        return true;
    } else if (correct_split) {
        /* Right split, wrong time - partial credit */
        state->puzzle_solved = true;
        state->score = wisdom_trial_calculate_score(state) * 0.8f;
        return true;
    } else {
        /* Close but not quite - score based on how close */
        int heaven_diff = abs((int)heaven_percent - CORRECT_HEAVEN_PERCENT);
        int hell_diff = abs((int)hell_percent - CORRECT_HELL_PERCENT);
        int total_diff = heaven_diff + hell_diff;

        if (total_diff <= 20) {
            /* Within 20% total - partial success */
            state->puzzle_solved = true;
            float proximity = 1.0f - ((float)total_diff / 100.0f);
            state->score = wisdom_trial_calculate_score(state) * proximity * 0.7f;
            return true;
        }

        /* Too far off */
        state->puzzle_solved = false;
        state->score = 0.0f;
        return false;
    }
}

bool wisdom_trial_submit_custom(WisdomTrialState* state, const char* description) {
    if (!state || !state->active || !description) {
        return false;
    }

    state->solution_type = WISDOM_SOLUTION_CUSTOM;
    state->attempts_made++;

    /* Custom solutions are evaluated based on description length and keywords */
    /* This is a simplified heuristic - in full game, would use NLP/semantic analysis */
    size_t desc_len = strlen(description);
    bool mentions_split = (strstr(description, "split") != NULL ||
                           strstr(description, "divide") != NULL ||
                           strstr(description, "portion") != NULL);
    bool mentions_balance = (strstr(description, "balance") != NULL ||
                             strstr(description, "proportional") != NULL);

    if (desc_len > 50 && mentions_split && mentions_balance) {
        /* Acceptable custom solution */
        state->puzzle_solved = true;
        state->score = wisdom_trial_calculate_score(state) * 0.6f;
        return true;
    }

    /* Insufficient custom solution */
    state->puzzle_solved = false;
    state->score = 0.0f;
    return false;
}

const char* wisdom_trial_get_hint(WisdomTrialState* state, uint8_t hint_level) {
    if (!state) {
        return NULL;
    }

    if (hint_level > state->hints_used) {
        state->hints_used = hint_level;
    }

    switch (hint_level) {
        case 1:
            return HINT_1;
        case 2:
            return HINT_2;
        case 3:
            return HINT_3;
        default:
            return NULL;
    }
}

float wisdom_trial_calculate_score(const WisdomTrialState* state) {
    if (!state || !state->puzzle_solved) {
        return 0.0f;
    }

    /* Base score starts at 100 */
    float score = 100.0f;

    /* Deduct for attempts (5 points per attempt after first) */
    if (state->attempts_made > 1) {
        score -= (state->attempts_made - 1) * 5.0f;
    }

    /* Deduct for hints (10 points per hint) */
    score -= state->hints_used * 10.0f;

    /* Clamp to 0-100 */
    if (score < 0.0f) {
        score = 0.0f;
    }
    if (score > 100.0f) {
        score = 100.0f;
    }

    return score;
}

bool wisdom_trial_is_complete(const WisdomTrialState* state) {
    if (!state) {
        return false;
    }

    return state->puzzle_solved || (state->attempts_made >= MAX_WISDOM_ATTEMPTS);
}

bool wisdom_trial_is_passed(const WisdomTrialState* state) {
    if (!state) {
        return false;
    }

    return state->puzzle_solved;
}

const char* wisdom_trial_get_soul_info(const WisdomTrialState* state) {
    if (!state) {
        return NULL;
    }

    snprintf(soul_info_buffer, sizeof(soul_info_buffer),
             "Soul ID: %s\n"
             "Name: %s\n"
             "Status: Stuck in routing queue for %u years\n\n"
             "Heaven Qualification: True penance - 15 years of remorse and service\n"
             "Hell Qualification: War atrocities - civilian massacre during siege\n\n"
             "Orthodox routing to Heaven: Ignores justice for victims\n"
             "Orthodox routing to Hell: Ignores genuine reformation\n\n"
             "Your task: Find a solution that honors both justice and mercy.",
             state->soul_id,
             state->soul_name,
             state->stuck_years);

    return soul_info_buffer;
}

const char* wisdom_trial_solution_name(WisdomSolutionType type) {
    switch (type) {
        case WISDOM_SOLUTION_NONE:
            return "None";
        case WISDOM_SOLUTION_ORTHODOX_HEAVEN:
            return "Orthodox Heaven";
        case WISDOM_SOLUTION_ORTHODOX_HELL:
            return "Orthodox Hell";
        case WISDOM_SOLUTION_SPLIT_ROUTE:
            return "Split Route";
        case WISDOM_SOLUTION_CUSTOM:
            return "Custom Algorithm";
        default:
            return "Unknown";
    }
}

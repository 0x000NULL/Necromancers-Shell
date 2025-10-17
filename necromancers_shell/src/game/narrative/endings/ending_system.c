/**
 * @file ending_system.c
 * @brief Implementation of ending determination logic
 */

#define _POSIX_C_SOURCE 200809L

#include "ending_system.h"
#include "../../game_state.h"
#include "../trials/archon_trial.h"
#include "../divine_judgment.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

/* Ending metadata */
typedef struct {
    EndingType type;
    const char* name;
    const char* description;
    const char* category;
    int difficulty;
    const char* success_rate;
} EndingMetadata;

static const EndingMetadata ENDING_INFO[] = {
    {
        .type = ENDING_REVENANT,
        .name = "Revenant Route - Redemption",
        .description = "Resurrect yourself and restore your humanity",
        .category = "Redemption",
        .difficulty = 3,
        .success_rate = "12%"
    },
    {
        .type = ENDING_LICH_LORD,
        .name = "Lich Lord Route - Apotheosis",
        .description = "Embrace undeath as an immortal tyrant",
        .category = "Power",
        .difficulty = 2,
        .success_rate = "34%"
    },
    {
        .type = ENDING_REAPER,
        .name = "Reaper Route - Service",
        .description = "Become death's administrator",
        .category = "Service",
        .difficulty = 3,
        .success_rate = "18%"
    },
    {
        .type = ENDING_ARCHON,
        .name = "Archon Route - Revolution",
        .description = "Rewrite the Death Network protocols",
        .category = "Revolution",
        .difficulty = 4,
        .success_rate = "8%"
    },
    {
        .type = ENDING_WRAITH,
        .name = "Wraith Route - Freedom",
        .description = "Escape as distributed consciousness",
        .category = "Freedom",
        .difficulty = 3,
        .success_rate = "15%"
    },
    {
        .type = ENDING_MORNINGSTAR,
        .name = "Morningstar Route - Transcendence",
        .description = "Become the eighth god",
        .category = "Transcendence",
        .difficulty = 5,
        .success_rate = "0.3%"
    }
};

/* Helper: Get ending metadata */
static const EndingMetadata* get_ending_metadata(EndingType ending) {
    for (size_t i = 0; i < sizeof(ENDING_INFO) / sizeof(ENDING_INFO[0]); i++) {
        if (ENDING_INFO[i].type == ending) {
            return &ENDING_INFO[i];
        }
    }
    return NULL;
}

/* Helper: Calculate average trial score */
static float calculate_avg_trial_score(const GameState* state) {
    if (!state->archon_trials) return 0.0f;

    float total = 0.0f;
    int count = 0;

    for (int i = 0; i < 7; i++) {
        if (state->archon_trials->trials[i].status == TRIAL_STATUS_PASSED) {
            total += state->archon_trials->trials[i].best_score;
            count++;
        }
    }

    return count > 0 ? (total / count) : 0.0f;
}

/* Helper: Count trials passed (score >= 70) */
static int count_trials_passed(const GameState* state) {
    if (!state->archon_trials) return 0;

    int passed = 0;
    for (int i = 0; i < 7; i++) {
        if (state->archon_trials->trials[i].status == TRIAL_STATUS_PASSED &&
            state->archon_trials->trials[i].best_score >= 70.0f) {
            passed++;
        }
    }

    return passed;
}

/* Helper: Check if Divine Council approved */
static bool has_divine_approval(const GameState* state) {
    if (!state->divine_judgment) return false;

    /* Check if judgment was completed and amnesty granted */
    return divine_judgment_is_amnesty_granted(state->divine_judgment);
}

/* Helper: Check Maya choice in Trial 6 */
static bool saved_maya(const GameState* state) {
    if (!state->thessara) return false;
    return state->thessara->severed;  /* Maya saved = Thessara severed */
}

/* Check Revenant ending requirements */
static bool check_revenant_requirements(const GameState* state, EndingRequirement* req) {
    if (req) {
        req->corruption_req = 0.0f;
        req->corruption_req_max = 30.0f;
        req->divine_approval_req = false;
        req->trial_score_req = 0;
        req->trials_passed_req = 0;
    }

    float corruption = (float)state->corruption.corruption;
    uint32_t civilian_kills = state->civilian_kills;
    bool maya_saved = saved_maya(state);

    /* Requirements: corruption <30%, civilians <10, saved Maya */
    if (corruption >= 30.0f) {
        if (req) snprintf(req->reason, sizeof(req->reason),
            "Corruption too high (%.1f%%, need <30%%)", corruption);
        return false;
    }

    if (civilian_kills >= 10) {
        if (req) snprintf(req->reason, sizeof(req->reason),
            "Too many civilian kills (%u, need <10)", civilian_kills);
        return false;
    }

    if (!maya_saved) {
        if (req) snprintf(req->reason, sizeof(req->reason),
            "Must save Maya in Trial 6 (Sacrifice)");
        return false;
    }

    if (req) {
        req->qualified = true;
        snprintf(req->reason, sizeof(req->reason), "All requirements met");
    }
    return true;
}

/* Check Lich Lord ending requirements */
static bool check_lich_lord_requirements(const GameState* state, EndingRequirement* req) {
    if (req) {
        req->corruption_req = 100.0f;
        req->corruption_req_max = -1.0f;
        req->divine_approval_req = false;
        req->trial_score_req = 0;
        req->trials_passed_req = 0;
    }

    float corruption = (float)state->corruption.corruption;

    /* Requirement: corruption = 100% */
    if (corruption < 100.0f) {
        if (req) snprintf(req->reason, sizeof(req->reason),
            "Corruption not maxed (%.1f%%, need 100%%)", corruption);
        return false;
    }

    if (req) {
        req->qualified = true;
        snprintf(req->reason, sizeof(req->reason), "Corruption maxed at 100%%");
    }
    return true;
}

/* Check Reaper ending requirements */
static bool check_reaper_requirements(const GameState* state, EndingRequirement* req) {
    if (req) {
        req->corruption_req = 40.0f;
        req->corruption_req_max = 69.0f;
        req->divine_approval_req = true;
        req->trial_score_req = 0;
        req->trials_passed_req = 0;
    }

    float corruption = (float)state->corruption.corruption;
    bool approved = has_divine_approval(state);

    /* Requirements: corruption 40-69%, divine approval */
    if (corruption < 40.0f || corruption > 69.0f) {
        if (req) snprintf(req->reason, sizeof(req->reason),
            "Corruption out of range (%.1f%%, need 40-69%%)", corruption);
        return false;
    }

    if (!approved) {
        if (req) snprintf(req->reason, sizeof(req->reason),
            "Divine Council approval required");
        return false;
    }

    if (req) {
        req->qualified = true;
        snprintf(req->reason, sizeof(req->reason), "All requirements met");
    }
    return true;
}

/* Check Archon ending requirements */
static bool check_archon_requirements(const GameState* state, EndingRequirement* req) {
    if (req) {
        req->corruption_req = 30.0f;
        req->corruption_req_max = 60.0f;
        req->divine_approval_req = true;
        req->trial_score_req = 70;
        req->trials_passed_req = 6;
    }

    float corruption = (float)state->corruption.corruption;
    bool approved = has_divine_approval(state);
    int trials_passed = count_trials_passed(state);
    float avg_score = calculate_avg_trial_score(state);

    /* Requirements: corruption 30-60%, divine approval, 6+ trials, avg 70+ */
    if (corruption < 30.0f || corruption > 60.0f) {
        if (req) snprintf(req->reason, sizeof(req->reason),
            "Corruption out of range (%.1f%%, need 30-60%%)", corruption);
        return false;
    }

    if (!approved) {
        if (req) snprintf(req->reason, sizeof(req->reason),
            "Divine Council approval required");
        return false;
    }

    if (trials_passed < 6) {
        if (req) snprintf(req->reason, sizeof(req->reason),
            "Not enough trials passed (%d, need 6+)", trials_passed);
        return false;
    }

    if (avg_score < 70.0f) {
        if (req) snprintf(req->reason, sizeof(req->reason),
            "Average trial score too low (%.1f, need 70+)", avg_score);
        return false;
    }

    if (req) {
        req->qualified = true;
        snprintf(req->reason, sizeof(req->reason), "All requirements met");
    }
    return true;
}

/* Check Wraith ending requirements */
static bool check_wraith_requirements(const GameState* state, EndingRequirement* req) {
    if (req) {
        req->corruption_req = 0.0f;
        req->corruption_req_max = 40.0f;
        req->divine_approval_req = false;
        req->trial_score_req = 0;
        req->trials_passed_req = 5;
    }

    float corruption = (float)state->corruption.corruption;
    int trials_passed = count_trials_passed(state);

    /* Requirements: corruption <40%, 5+ trials passed */
    if (corruption >= 40.0f) {
        if (req) snprintf(req->reason, sizeof(req->reason),
            "Corruption too high (%.1f%%, need <40%%)", corruption);
        return false;
    }

    if (trials_passed < 5) {
        if (req) snprintf(req->reason, sizeof(req->reason),
            "Not enough trials passed (%d, need 5+)", trials_passed);
        return false;
    }

    if (req) {
        req->qualified = true;
        snprintf(req->reason, sizeof(req->reason), "All requirements met");
    }
    return true;
}

/* Check Morningstar ending requirements */
static bool check_morningstar_requirements(const GameState* state, EndingRequirement* req) {
    if (req) {
        req->corruption_req = 50.0f;
        req->corruption_req_max = -1.0f;  /* Exactly 50% */
        req->divine_approval_req = true;
        req->trial_score_req = 80;
        req->trials_passed_req = 7;
    }

    float corruption = (float)state->corruption.corruption;
    bool approved = has_divine_approval(state);
    int trials_passed = count_trials_passed(state);
    float avg_score = calculate_avg_trial_score(state);

    /* Requirements: corruption EXACTLY 50% (±0.5%), divine approval, all 7 trials, avg 80+ */
    if (fabs(corruption - 50.0f) > 0.5f) {
        if (req) snprintf(req->reason, sizeof(req->reason),
            "Corruption not exactly 50%% (%.1f%%, need 50.0±0.5%%)", corruption);
        return false;
    }

    if (!approved) {
        if (req) snprintf(req->reason, sizeof(req->reason),
            "Divine Council approval required");
        return false;
    }

    if (trials_passed < 7) {
        if (req) snprintf(req->reason, sizeof(req->reason),
            "Must pass all 7 trials (%d passed)", trials_passed);
        return false;
    }

    if (avg_score < 80.0f) {
        if (req) snprintf(req->reason, sizeof(req->reason),
            "Average trial score too low (%.1f, need 80+)", avg_score);
        return false;
    }

    if (req) {
        req->qualified = true;
        snprintf(req->reason, sizeof(req->reason), "Hardest ending achieved!");
    }
    return true;
}

/* Public API implementations */

bool check_ending_requirements(
    const GameState* state,
    EndingType ending,
    EndingRequirement* req_out
) {
    if (!state) return false;

    /* Initialize requirement structure */
    if (req_out) {
        memset(req_out, 0, sizeof(EndingRequirement));
        req_out->corruption_req_max = -1.0f;
    }

    switch (ending) {
        case ENDING_REVENANT:
            return check_revenant_requirements(state, req_out);
        case ENDING_LICH_LORD:
            return check_lich_lord_requirements(state, req_out);
        case ENDING_REAPER:
            return check_reaper_requirements(state, req_out);
        case ENDING_ARCHON:
            return check_archon_requirements(state, req_out);
        case ENDING_WRAITH:
            return check_wraith_requirements(state, req_out);
        case ENDING_MORNINGSTAR:
            return check_morningstar_requirements(state, req_out);
        default:
            if (req_out) snprintf(req_out->reason, sizeof(req_out->reason),
                "Invalid ending type");
            return false;
    }
}

EndingType determine_ending(const GameState* state) {
    if (!state || !is_game_complete(state)) {
        return ENDING_NONE;
    }

    /* Check in priority order (hardest to easiest) */

    /* 1. Morningstar - Hardest, most specific */
    if (check_ending_requirements(state, ENDING_MORNINGSTAR, NULL)) {
        return ENDING_MORNINGSTAR;
    }

    /* 2. Archon - Requires divine approval + excellent trial performance */
    if (check_ending_requirements(state, ENDING_ARCHON, NULL)) {
        return ENDING_ARCHON;
    }

    /* 3. Revenant - Redemption path (low corruption + Maya saved) */
    if (check_ending_requirements(state, ENDING_REVENANT, NULL)) {
        return ENDING_REVENANT;
    }

    /* 4. Wraith - Freedom path (low corruption + decent trials) */
    if (check_ending_requirements(state, ENDING_WRAITH, NULL)) {
        return ENDING_WRAITH;
    }

    /* 5. Reaper - Service path (medium corruption + divine approval) */
    if (check_ending_requirements(state, ENDING_REAPER, NULL)) {
        return ENDING_REAPER;
    }

    /* 6. Lich Lord - Fallback for high corruption */
    if (check_ending_requirements(state, ENDING_LICH_LORD, NULL)) {
        return ENDING_LICH_LORD;
    }

    /* Should not reach here if game is properly complete */
    return ENDING_NONE;
}

int get_qualified_endings(
    const GameState* state,
    EndingType endings_out[6]
) {
    if (!state || !endings_out) return 0;

    int count = 0;

    /* Check all six endings */
    const EndingType all_endings[] = {
        ENDING_MORNINGSTAR,
        ENDING_ARCHON,
        ENDING_REVENANT,
        ENDING_WRAITH,
        ENDING_REAPER,
        ENDING_LICH_LORD
    };

    for (size_t i = 0; i < 6; i++) {
        if (check_ending_requirements(state, all_endings[i], NULL)) {
            endings_out[count++] = all_endings[i];
        }
    }

    return count;
}

bool is_game_complete(const GameState* state) {
    if (!state) return false;

    /* Requirements for game completion:
     * 1. Divine Judgment completed (day >= 155)
     * 2. All 7 trials attempted
     */

    /* Check day requirement */
    if (state->resources.day_count < 155) {
        return false;
    }

    /* Check trials attempted */
    if (!state->archon_trials) return false;

    for (int i = 0; i < 7; i++) {
        if (state->archon_trials->trials[i].status == TRIAL_STATUS_LOCKED ||
            state->archon_trials->trials[i].status == TRIAL_STATUS_AVAILABLE) {
            return false;
        }
    }

    return true;
}

void calculate_ending_achievement(
    const GameState* state,
    EndingAchievement* achievement
) {
    if (!state || !achievement) return;

    memset(achievement, 0, sizeof(EndingAchievement));

    achievement->ending = determine_ending(state);
    achievement->day_achieved = state->resources.day_count;
    achievement->final_corruption = (float)state->corruption.corruption;
    achievement->trials_passed = count_trials_passed(state);
    achievement->avg_trial_score = calculate_avg_trial_score(state);
    achievement->divine_approval = has_divine_approval(state);
    achievement->civilian_kills = state->civilian_kills;
    achievement->saved_maya = saved_maya(state);
    achievement->achieved_goals = (achievement->ending != ENDING_NONE);
}

const char* ending_get_name(EndingType ending) {
    const EndingMetadata* meta = get_ending_metadata(ending);
    return meta ? meta->name : "Unknown Ending";
}

const char* ending_get_description(EndingType ending) {
    const EndingMetadata* meta = get_ending_metadata(ending);
    return meta ? meta->description : "Unknown ending path";
}

const char* ending_get_category(EndingType ending) {
    const EndingMetadata* meta = get_ending_metadata(ending);
    return meta ? meta->category : "Unknown";
}

int ending_get_difficulty(EndingType ending) {
    const EndingMetadata* meta = get_ending_metadata(ending);
    return meta ? meta->difficulty : 0;
}

const char* ending_get_success_rate(EndingType ending) {
    const EndingMetadata* meta = get_ending_metadata(ending);
    return meta ? meta->success_rate : "Unknown";
}

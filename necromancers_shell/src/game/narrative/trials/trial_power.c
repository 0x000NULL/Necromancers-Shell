#include "trial_power.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

PowerTrialState* power_trial_create(void) {
    PowerTrialState* state = calloc(1, sizeof(PowerTrialState));
    if (!state) {
        fprintf(stderr, "Failed to allocate power trial state\n");
        return NULL;
    }

    state->active = false;
    state->seraphim_max_hp = SERAPHIM_MAX_HP;
    state->seraphim_current_hp = SERAPHIM_MAX_HP;
    state->turns_elapsed = 0;
    state->outcome = POWER_OUTCOME_NONE;
    state->victory_condition_met = false;
    state->hint_shown = false;

    return state;
}

void power_trial_destroy(PowerTrialState* state) {
    if (!state) {
        return;
    }

    free(state);
}

void power_trial_start(PowerTrialState* state) {
    if (!state) {
        return;
    }

    state->active = true;
    state->seraphim_current_hp = state->seraphim_max_hp;
    state->turns_elapsed = 0;
    state->outcome = POWER_OUTCOME_NONE;
    state->victory_condition_met = false;
    state->hint_shown = false;
}

bool power_trial_damage_seraphim(PowerTrialState* state, uint32_t damage) {
    if (!state || !state->active) {
        return false;
    }

    /* Apply damage */
    if (damage >= state->seraphim_current_hp) {
        state->seraphim_current_hp = 0;
    } else {
        state->seraphim_current_hp -= damage;
    }

    state->turns_elapsed++;

    /* Check victory condition (HP <= 10%) */
    uint32_t yield_hp = (uint32_t)(state->seraphim_max_hp * (POWER_TRIAL_VICTORY_HP_PERCENT / 100.0f));
    if (state->seraphim_current_hp <= yield_hp && state->seraphim_current_hp > 0) {
        state->victory_condition_met = true;
    }

    /* Return true if Seraphim still alive */
    return (state->seraphim_current_hp > 0);
}

bool power_trial_can_yield(const PowerTrialState* state) {
    if (!state || !state->active) {
        return false;
    }

    return state->victory_condition_met;
}

bool power_trial_yield(PowerTrialState* state) {
    if (!state || !state->active) {
        return false;
    }

    /* Can only yield if victory condition met */
    if (!state->victory_condition_met) {
        return false;
    }

    state->outcome = POWER_OUTCOME_YIELD;
    state->active = false;
    return true;
}

bool power_trial_kill(PowerTrialState* state) {
    if (!state || !state->active) {
        return false;
    }

    /* Killing Seraphim fails the trial */
    state->outcome = POWER_OUTCOME_KILL;
    state->active = false;
    return false;
}

void power_trial_player_defeated(PowerTrialState* state) {
    if (!state) {
        return;
    }

    state->outcome = POWER_OUTCOME_DEFEATED;
    state->active = false;
}

void power_trial_flee(PowerTrialState* state) {
    if (!state) {
        return;
    }

    state->outcome = POWER_OUTCOME_FLED;
    state->active = false;
}

PowerTrialOutcome power_trial_get_outcome(const PowerTrialState* state) {
    if (!state) {
        return POWER_OUTCOME_NONE;
    }

    return state->outcome;
}

bool power_trial_is_complete(const PowerTrialState* state) {
    if (!state) {
        return false;
    }

    return (state->outcome != POWER_OUTCOME_NONE);
}

bool power_trial_is_passed(const PowerTrialState* state) {
    if (!state) {
        return false;
    }

    return (state->outcome == POWER_OUTCOME_YIELD);
}

const char* power_trial_get_hint(void) {
    return "Strategy Hint: Think like a sysadmin, not a warrior. "
           "Seraphim is a powerful single-target fighter, but you command a network. "
           "Create a distributed attack network—route your forces through coordinated "
           "strikes. She can't kill all of you at once. When she destroys one minion, "
           "another strikes from a different angle. Manipulate the battlefield itself. "
           "The goal is not to kill, but to demonstrate superior tactical mastery through "
           "infrastructure optimization.";
}

const char* power_trial_outcome_text(PowerTrialOutcome outcome) {
    switch (outcome) {
        case POWER_OUTCOME_NONE:
            return "Combat in progress";

        case POWER_OUTCOME_YIELD:
            return "Victory through mercy. You fought like a systems administrator—routing "
                   "attacks through your minion network, creating a distributed assault that "
                   "she couldn't counter. It took forty-seven minutes. When you had the spectral "
                   "blade at her throat, you chose restraint. "
                   "\"Clever,\" she admits. \"You fight like you're debugging a system.\" "
                   "\"I am,\" you reply. \"The system is combat. I just optimized it.\" "
                   "Trial One: Complete.";

        case POWER_OUTCOME_KILL:
            return "Trial failed. Killing Seraphim proved your strength but revealed "
                   "a lack of restraint. The Archon path requires power tempered with wisdom. "
                   "This trial cannot be passed through violence alone.";

        case POWER_OUTCOME_DEFEATED:
            return "Trial failed. You were defeated in combat. "
                   "The Archon path requires both martial prowess and strategic thinking. "
                   "Return when you are stronger.";

        case POWER_OUTCOME_FLED:
            return "Trial failed. Fleeing from combat shows weakness. "
                   "An Archon must have the courage to face any challenge. "
                   "This trial requires commitment.";

        default:
            return "Unknown outcome";
    }
}

float power_trial_calculate_score(const PowerTrialState* state) {
    if (!state || !power_trial_is_passed(state)) {
        return 0.0f;
    }

    /* Base score starts at 100 */
    float score = 100.0f;

    /* Deduct points for turns taken (optimal is ~5-10 turns) */
    /* Each turn over 10 reduces score by 2 points */
    if (state->turns_elapsed > 10) {
        float penalty = (state->turns_elapsed - 10) * 2.0f;
        score -= penalty;
    }

    /* Deduct 10 points if hint was shown (player needed help) */
    if (state->hint_shown) {
        score -= 10.0f;
    }

    /* Clamp to 0-100 range */
    if (score < 0.0f) {
        score = 0.0f;
    }
    if (score > 100.0f) {
        score = 100.0f;
    }

    return score;
}

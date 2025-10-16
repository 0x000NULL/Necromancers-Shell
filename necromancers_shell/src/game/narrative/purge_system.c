/**
 * purge_system.c - Fourth Purge System Implementation
 */

#include "purge_system.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Days in a year (game calendar) */
#define DAYS_PER_YEAR 365

/* Original purge timer (5 years) */
#define ORIGINAL_TIMER_DAYS (5 * DAYS_PER_YEAR)

/* Accelerated timer (1.5 years) */
#define ACCELERATED_TIMER_DAYS ((DAYS_PER_YEAR * 3) / 2)

/* Reformation target */
#define REFORMATION_TARGET 147

/* Estimated casualties */
#define BASE_CASUALTIES 200
#define ARCHON_CASUALTIES 50

/* Enforcer roster */
static const struct {
    EnforcerType type;
    int count;
    int hp;
    int attack;
    int defense;
    const char* specialty;
} ENFORCER_ROSTER[] = {
    {ENFORCER_SERAPHIM, 10, 500, 75, 50, "holy_damage"},
    {ENFORCER_HELLKNIGHT, 8, 600, 80, 60, "fire_damage"},
    {ENFORCER_INEVITABLE, 12, 450, 70, 55, "fate_strike"}
};

#define ENFORCER_ROSTER_SIZE 3

PurgeState* purge_system_create(void) {
    PurgeState* state = calloc(1, sizeof(PurgeState));
    if (!state) {
        return NULL;
    }

    state->purge_number = 4;
    state->reformation_target = REFORMATION_TARGET;
    state->estimated_casualties_base = BASE_CASUALTIES;
    state->estimated_casualties_with_archon = ARCHON_CASUALTIES;

    return state;
}

void purge_system_destroy(PurgeState* state) {
    free(state);
}

bool purge_system_initialize(PurgeState* state) {
    if (!state) {
        return false;
    }

    /* Set original 5-year timer */
    state->days_until_purge = ORIGINAL_TIMER_DAYS;
    state->accelerated = false;

    /* Initialize enforcer roster */
    state->enforcer_count = 0;

    for (int i = 0; i < ENFORCER_ROSTER_SIZE; i++) {
        for (int j = 0; j < ENFORCER_ROSTER[i].count; j++) {
            if (state->enforcer_count >= MAX_ENFORCERS) {
                break;
            }

            DivineEnforcer* enforcer = &state->enforcers[state->enforcer_count++];
            enforcer->type = ENFORCER_ROSTER[i].type;
            enforcer->hp = ENFORCER_ROSTER[i].hp;
            enforcer->attack = ENFORCER_ROSTER[i].attack;
            enforcer->defense = ENFORCER_ROSTER[i].defense;
            snprintf(enforcer->specialty, sizeof(enforcer->specialty), "%s", ENFORCER_ROSTER[i].specialty);
            enforcer->deployed = false;
        }
    }

    return true;
}

void purge_system_accelerate(PurgeState* state) {
    if (!state || state->accelerated) {
        return;
    }

    /* Reduce timer to 1.5 years */
    state->days_until_purge = ACCELERATED_TIMER_DAYS;
    state->accelerated = true;

    /* Increase enforcer deployment rate by marking some as deployed */
    int early_deployments = state->enforcer_count / 4; /* 25% deployed early */
    for (size_t i = 0; i < state->enforcer_count && early_deployments > 0; i++) {
        if (!state->enforcers[i].deployed) {
            state->enforcers[i].deployed = true;
            state->enforcers_deployed++;
            early_deployments--;
        }
    }
}

bool purge_system_advance_day(PurgeState* state) {
    if (!state) {
        return false;
    }

    if (state->days_until_purge <= 0) {
        return true; /* Purge has begun */
    }

    state->days_until_purge--;

    /* Deploy enforcers periodically as purge approaches */
    /* Every 30 days, deploy 1-2 enforcers */
    if (state->days_until_purge % 30 == 0) {
        purge_system_deploy_enforcers(state);
    }

    /* Purge begins when timer reaches 0 */
    return (state->days_until_purge == 0);
}

int purge_system_deploy_enforcers(PurgeState* state) {
    if (!state) {
        return 0;
    }

    int deployed = 0;
    int to_deploy = (state->accelerated ? 2 : 1);

    for (size_t i = 0; i < state->enforcer_count && deployed < to_deploy; i++) {
        if (!state->enforcers[i].deployed) {
            state->enforcers[i].deployed = true;
            state->enforcers_deployed++;
            deployed++;
        }
    }

    return deployed;
}

bool purge_system_reform_necromancer(PurgeState* state, int npc_id) {
    if (!state || !state->archon_intervention_active) {
        return false;
    }

    /* Prevent duplicate reforms (in real implementation, track NPC IDs) */
    (void)npc_id; /* Suppress unused warning - used for tracking in full implementation */
    if (state->necromancers_reformed >= state->reformation_target) {
        return false; /* Target already reached */
    }

    state->necromancers_reformed++;

    /* Check if target reached */
    if (state->necromancers_reformed >= state->reformation_target) {
        state->reformation_successful = true;
    }

    return true;
}

bool purge_system_is_reformation_complete(const PurgeState* state) {
    if (!state) {
        return false;
    }

    return state->reformation_successful;
}

int purge_system_calculate_casualties(const PurgeState* state) {
    if (!state) {
        return 0;
    }

    if (!state->archon_intervention_active) {
        /* Without Archon: 200 deaths */
        return state->estimated_casualties_base;
    }

    if (state->reformation_successful) {
        /* With successful reformation: only 50 deaths */
        return state->estimated_casualties_with_archon;
    }

    /* Partial reformation: interpolate between 50 and 200 */
    int progress = state->necromancers_reformed;
    int target = state->reformation_target;
    int max_casualties = state->estimated_casualties_base;
    int min_casualties = state->estimated_casualties_with_archon;

    /* Linear interpolation */
    int saved = (progress * (max_casualties - min_casualties)) / target;
    return max_casualties - saved;
}

void purge_system_enable_archon_intervention(PurgeState* state) {
    if (!state) {
        return;
    }

    state->archon_intervention_active = true;
}

const DivineEnforcer* purge_system_get_enforcer(const PurgeState* state,
                                               size_t index) {
    if (!state || index >= state->enforcer_count) {
        return NULL;
    }

    return &state->enforcers[index];
}

const char* purge_system_get_enforcer_type_name(EnforcerType type) {
    switch (type) {
        case ENFORCER_SERAPHIM:
            return "Seraphim";
        case ENFORCER_HELLKNIGHT:
            return "Hellknight";
        case ENFORCER_INEVITABLE:
            return "Inevitable";
        default:
            return "Unknown";
    }
}

int purge_system_get_days_remaining(const PurgeState* state) {
    if (!state) {
        return 0;
    }

    return state->days_until_purge;
}

double purge_system_get_years_remaining(const PurgeState* state) {
    if (!state) {
        return 0.0;
    }

    return (double)state->days_until_purge / DAYS_PER_YEAR;
}

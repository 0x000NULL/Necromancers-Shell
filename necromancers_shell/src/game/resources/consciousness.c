#include "consciousness.h"
#include <string.h>
#include <math.h>

/* Consciousness level descriptions */
static const char* CONSCIOUSNESS_DESCRIPTIONS[] = {
    "Your sense of self is fragmenting into void",            /* 0-10% */
    "You struggle to remember who you were",                   /* 11-30% */
    "Your identity feels unstable and shifting",               /* 31-50% */
    "You retain most of your sense of self",                   /* 51-70% */
    "Your consciousness remains clear and focused",            /* 71-90% */
    "You are fully aware and in control of your existence"     /* 91-100% */
};

void consciousness_init(ConsciousnessState* state) {
    if (!state) {
        return;
    }

    state->stability = 100.0f;
    state->decay_rate = -0.1f;  /* -0.1% per month */
    state->months_until_critical = 0;
    state->fragmentation_level = 0.0f;
    state->approaching_wraith = false;
    state->last_decay_month = 0;

    /* Calculate initial months until critical */
    state->months_until_critical = consciousness_months_until_critical(state);
}

void consciousness_apply_decay(ConsciousnessState* state, uint32_t current_month) {
    if (!state) {
        return;
    }

    /* Only apply decay once per month */
    if (state->last_decay_month == current_month) {
        return;
    }

    state->last_decay_month = current_month;

    /* Apply decay (decay_rate is negative, so this reduces stability) */
    state->stability += state->decay_rate;

    /* Clamp to valid range */
    if (state->stability < 0.0f) {
        state->stability = 0.0f;
    } else if (state->stability > 100.0f) {
        state->stability = 100.0f;
    }

    /* Update fragmentation warning flag */
    state->approaching_wraith = consciousness_is_fragmenting(state);

    /* Recalculate months until critical */
    state->months_until_critical = consciousness_months_until_critical(state);
}

bool consciousness_restore(ConsciousnessState* state, float amount) {
    if (!state || amount < 0.0f) {
        return false;
    }

    state->stability += amount;

    /* Clamp to 100% */
    if (state->stability > 100.0f) {
        state->stability = 100.0f;
    }

    /* Update warnings */
    state->approaching_wraith = consciousness_is_fragmenting(state);
    state->months_until_critical = consciousness_months_until_critical(state);

    return true;
}

bool consciousness_reduce(ConsciousnessState* state, float amount) {
    if (!state || amount < 0.0f) {
        return false;
    }

    state->stability -= amount;

    /* Clamp to 0% */
    if (state->stability < 0.0f) {
        state->stability = 0.0f;
    }

    /* Update warnings */
    state->approaching_wraith = consciousness_is_fragmenting(state);
    state->months_until_critical = consciousness_months_until_critical(state);

    return true;
}

void consciousness_set_decay_rate(ConsciousnessState* state, float rate) {
    if (!state) {
        return;
    }

    state->decay_rate = rate;

    /* Recalculate months until critical with new rate */
    state->months_until_critical = consciousness_months_until_critical(state);
}

void consciousness_add_fragmentation(ConsciousnessState* state, float amount) {
    if (!state || amount < 0.0f) {
        return;
    }

    state->fragmentation_level += amount;

    /* Clamp to 100% */
    if (state->fragmentation_level > 100.0f) {
        state->fragmentation_level = 100.0f;
    }

    /* Update warning flag */
    state->approaching_wraith = consciousness_is_fragmenting(state);
}

bool consciousness_is_critical(const ConsciousnessState* state) {
    if (!state) {
        return false;
    }

    return state->stability < 10.0f;
}

bool consciousness_is_stable(const ConsciousnessState* state) {
    if (!state) {
        return false;
    }

    return state->stability >= 80.0f;
}

uint32_t consciousness_months_until_critical(const ConsciousnessState* state) {
    if (!state) {
        return 0;
    }

    /* If already critical, return 0 */
    if (consciousness_is_critical(state)) {
        return 0;
    }

    /* If decay rate is non-negative (not decaying), return large number */
    if (state->decay_rate >= 0.0f) {
        return 9999;
    }

    /* Calculate: (current - 10) / abs(decay_rate) */
    float percent_above_critical = state->stability - 10.0f;
    float months = percent_above_critical / fabsf(state->decay_rate);

    return (uint32_t)ceilf(months);
}

const char* consciousness_get_description(const ConsciousnessState* state) {
    if (!state) {
        return "Unknown consciousness state";
    }

    if (state->stability >= 91.0f) {
        return CONSCIOUSNESS_DESCRIPTIONS[5];
    } else if (state->stability >= 71.0f) {
        return CONSCIOUSNESS_DESCRIPTIONS[4];
    } else if (state->stability >= 51.0f) {
        return CONSCIOUSNESS_DESCRIPTIONS[3];
    } else if (state->stability >= 31.0f) {
        return CONSCIOUSNESS_DESCRIPTIONS[2];
    } else if (state->stability >= 11.0f) {
        return CONSCIOUSNESS_DESCRIPTIONS[1];
    } else {
        return CONSCIOUSNESS_DESCRIPTIONS[0];
    }
}

bool consciousness_is_fragmenting(const ConsciousnessState* state) {
    if (!state) {
        return false;
    }

    return state->fragmentation_level >= 50.0f;
}

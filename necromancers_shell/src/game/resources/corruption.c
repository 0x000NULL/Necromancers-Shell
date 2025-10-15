#include "corruption.h"
#include <string.h>

/* Corruption tier name strings (11 tiers: 0-10) */
static const char* CORRUPTION_TIER_NAMES[] = {
    "Pristine",       /* Tier 0: 0-10% */
    "Tainted",        /* Tier 1: 11-20% */
    "Corrupted",      /* Tier 2: 21-30% */
    "Tainted",        /* Tier 3: 31-40% */
    "Vile",           /* Tier 4: 41-50% */
    "Vile",           /* Tier 5: 51-60% */
    "Abyssal",        /* Tier 6: 61-69% */
    "IRREVERSIBLE",   /* Tier 7: 70% */
    "Damned",         /* Tier 8: 71-89% */
    "Void-Touched",   /* Tier 9: 90-99% */
    "Lich Lord"       /* Tier 10: 100% */
};

/* Corruption tier descriptions (psychological effects per tier) */
static const char* CORRUPTION_TIER_DESCRIPTIONS[] = {
    "You still think of corpses as people who lived and died",                    /* 0-10% */
    "The guilt is fading. You rationalize your actions more easily",              /* 11-20% */
    "You're starting to see souls as resources, means to an end",                 /* 21-30% */
    "Your humanity is cracking. The suffering of others feels distant",           /* 31-40% */
    "You are barely human. Empathy is a fading memory",                           /* 41-50% */
    "Darkness fills your essence, though you can still feel some emotions",       /* 51-60% */
    "You stand 9 points from the point of no return. The abyss calls",           /* 61-69% */
    "THRESHOLD CROSSED: Your soul is unrouteable. Redemption is impossible",      /* 70% */
    "You are damned. Only the paths of Lich and Reaper remain",                  /* 71-89% */
    "Near total loss. You are more void than person",                             /* 90-99% */
    "Transformation complete. You are no longer what you were"                    /* 100% */
};

void corruption_init(CorruptionState* state) {
    if (!state) {
        return;
    }

    state->corruption = 0;
    state->event_count = 0;
    memset(state->events, 0, sizeof(state->events));
}

bool corruption_add(CorruptionState* state, uint8_t amount, const char* description, uint32_t day) {
    if (!state) {
        return false;
    }

    /* Calculate new corruption value (capped at 100) */
    uint32_t new_corruption = state->corruption + amount;
    if (new_corruption > 100) {
        new_corruption = 100;
    }

    /* Record the change */
    int8_t actual_change = (int8_t)(new_corruption - state->corruption);
    state->corruption = (uint8_t)new_corruption;

    /* Record event if there's space */
    if (state->event_count < MAX_CORRUPTION_EVENTS) {
        CorruptionEvent* event = &state->events[state->event_count];
        if (description) {
            strncpy(event->description, description, sizeof(event->description) - 1);
            event->description[sizeof(event->description) - 1] = '\0';
        } else {
            strncpy(event->description, "Unknown corruption event", sizeof(event->description) - 1);
            event->description[sizeof(event->description) - 1] = '\0';
        }
        event->change = actual_change;
        event->day = day;
        state->event_count++;
    }

    return true;
}

bool corruption_reduce(CorruptionState* state, uint8_t amount, const char* description, uint32_t day) {
    if (!state) {
        return false;
    }

    /* Calculate new corruption value (minimum 0) */
    int32_t new_corruption = state->corruption - amount;
    if (new_corruption < 0) {
        new_corruption = 0;
    }

    /* Record the change */
    int8_t actual_change = (int8_t)(new_corruption - state->corruption);
    state->corruption = (uint8_t)new_corruption;

    /* Record event if there's space */
    if (state->event_count < MAX_CORRUPTION_EVENTS) {
        CorruptionEvent* event = &state->events[state->event_count];
        if (description) {
            strncpy(event->description, description, sizeof(event->description) - 1);
            event->description[sizeof(event->description) - 1] = '\0';
        } else {
            strncpy(event->description, "Unknown redemption event", sizeof(event->description) - 1);
            event->description[sizeof(event->description) - 1] = '\0';
        }
        event->change = actual_change;
        event->day = day;
        state->event_count++;
    }

    return true;
}

CorruptionTier corruption_get_tier(const CorruptionState* state) {
    if (!state) {
        return CORRUPTION_TIER_0;
    }

    uint8_t c = state->corruption;

    if (c == 100) {
        return CORRUPTION_TIER_10;
    } else if (c >= 90) {
        return CORRUPTION_TIER_9;
    } else if (c >= 71) {
        return CORRUPTION_TIER_8;
    } else if (c == 70) {
        return CORRUPTION_TIER_7;
    } else if (c >= 61) {
        return CORRUPTION_TIER_6;
    } else if (c >= 51) {
        return CORRUPTION_TIER_5;
    } else if (c >= 41) {
        return CORRUPTION_TIER_4;
    } else if (c >= 31) {
        return CORRUPTION_TIER_3;
    } else if (c >= 21) {
        return CORRUPTION_TIER_2;
    } else if (c >= 11) {
        return CORRUPTION_TIER_1;
    } else {
        return CORRUPTION_TIER_0;
    }
}

const char* corruption_tier_name(CorruptionTier tier) {
    /* Map tier enum value to index (0-10) */
    if (tier == 0) return CORRUPTION_TIER_NAMES[0];
    else if (tier == 11) return CORRUPTION_TIER_NAMES[1];
    else if (tier == 21) return CORRUPTION_TIER_NAMES[2];
    else if (tier == 31) return CORRUPTION_TIER_NAMES[3];
    else if (tier == 41) return CORRUPTION_TIER_NAMES[4];
    else if (tier == 51) return CORRUPTION_TIER_NAMES[5];
    else if (tier == 61) return CORRUPTION_TIER_NAMES[6];
    else if (tier == 70) return CORRUPTION_TIER_NAMES[7];
    else if (tier == 71) return CORRUPTION_TIER_NAMES[8];
    else if (tier == 90) return CORRUPTION_TIER_NAMES[9];
    else if (tier == 100) return CORRUPTION_TIER_NAMES[10];
    else return "Unknown";
}

const char* corruption_get_description(const CorruptionState* state) {
    if (!state) {
        return "Unknown corruption state";
    }

    CorruptionTier tier = corruption_get_tier(state);

    /* Map tier enum to description index */
    if (tier == 0) return CORRUPTION_TIER_DESCRIPTIONS[0];
    else if (tier == 11) return CORRUPTION_TIER_DESCRIPTIONS[1];
    else if (tier == 21) return CORRUPTION_TIER_DESCRIPTIONS[2];
    else if (tier == 31) return CORRUPTION_TIER_DESCRIPTIONS[3];
    else if (tier == 41) return CORRUPTION_TIER_DESCRIPTIONS[4];
    else if (tier == 51) return CORRUPTION_TIER_DESCRIPTIONS[5];
    else if (tier == 61) return CORRUPTION_TIER_DESCRIPTIONS[6];
    else if (tier == 70) return CORRUPTION_TIER_DESCRIPTIONS[7];
    else if (tier == 71) return CORRUPTION_TIER_DESCRIPTIONS[8];
    else if (tier == 90) return CORRUPTION_TIER_DESCRIPTIONS[9];
    else if (tier == 100) return CORRUPTION_TIER_DESCRIPTIONS[10];
    else return "Unknown corruption state";
}

float corruption_calculate_penalty(const CorruptionState* state) {
    if (!state) {
        return 0.0f;
    }

    /* Penalty ranges from 0% (at 0 corruption) to 50% (at 100 corruption) */
    return state->corruption / 200.0f;
}

bool corruption_is_irreversible(const CorruptionState* state) {
    if (!state) {
        return false;
    }

    return state->corruption >= 70;
}

bool corruption_is_damned(const CorruptionState* state) {
    if (!state) {
        return false;
    }

    return state->corruption >= 90;
}

bool corruption_is_pure(const CorruptionState* state) {
    if (!state) {
        return true;
    }

    return state->corruption < 11;
}

bool corruption_revenant_available(const CorruptionState* state) {
    if (!state) {
        return false;
    }

    return state->corruption < 30;
}

bool corruption_wraith_available(const CorruptionState* state) {
    if (!state) {
        return false;
    }

    return state->corruption < 40;
}

bool corruption_archon_available(const CorruptionState* state) {
    if (!state) {
        return false;
    }

    return state->corruption >= 30 && state->corruption <= 60;
}

const CorruptionEvent* corruption_get_latest_event(const CorruptionState* state) {
    if (!state || state->event_count == 0) {
        return NULL;
    }

    return &state->events[state->event_count - 1];
}

const CorruptionEvent* corruption_get_event(const CorruptionState* state, size_t index) {
    if (!state || index >= state->event_count) {
        return NULL;
    }

    return &state->events[index];
}

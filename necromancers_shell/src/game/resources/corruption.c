#include "corruption.h"
#include <string.h>

/* Corruption level name strings */
static const char* CORRUPTION_LEVEL_NAMES[] = {
    "Pure",
    "Tainted",
    "Compromised",
    "Corrupted",
    "Damned"
};

/* Corruption level descriptions */
static const char* CORRUPTION_DESCRIPTIONS[] = {
    "Your soul remains untainted by the darkness",
    "The shadows begin to whisper in your mind",
    "Your morality wavers in the face of power",
    "Darkness seeps into your very essence",
    "You are lost to the abyss, beyond redemption"
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

CorruptionLevel corruption_get_level(const CorruptionState* state) {
    if (!state) {
        return CORRUPTION_PURE;
    }

    if (state->corruption >= CORRUPTION_DAMNED) {
        return CORRUPTION_DAMNED;
    } else if (state->corruption >= CORRUPTION_CORRUPTED) {
        return CORRUPTION_CORRUPTED;
    } else if (state->corruption >= CORRUPTION_COMPROMISED) {
        return CORRUPTION_COMPROMISED;
    } else if (state->corruption >= CORRUPTION_TAINTED) {
        return CORRUPTION_TAINTED;
    } else {
        return CORRUPTION_PURE;
    }
}

const char* corruption_level_name(CorruptionLevel level) {
    switch (level) {
        case CORRUPTION_PURE:
            return CORRUPTION_LEVEL_NAMES[0];
        case CORRUPTION_TAINTED:
            return CORRUPTION_LEVEL_NAMES[1];
        case CORRUPTION_COMPROMISED:
            return CORRUPTION_LEVEL_NAMES[2];
        case CORRUPTION_CORRUPTED:
            return CORRUPTION_LEVEL_NAMES[3];
        case CORRUPTION_DAMNED:
            return CORRUPTION_LEVEL_NAMES[4];
        default:
            return "Unknown";
    }
}

const char* corruption_get_description(const CorruptionState* state) {
    if (!state) {
        return "Unknown corruption state";
    }

    CorruptionLevel level = corruption_get_level(state);

    switch (level) {
        case CORRUPTION_PURE:
            return CORRUPTION_DESCRIPTIONS[0];
        case CORRUPTION_TAINTED:
            return CORRUPTION_DESCRIPTIONS[1];
        case CORRUPTION_COMPROMISED:
            return CORRUPTION_DESCRIPTIONS[2];
        case CORRUPTION_CORRUPTED:
            return CORRUPTION_DESCRIPTIONS[3];
        case CORRUPTION_DAMNED:
            return CORRUPTION_DESCRIPTIONS[4];
        default:
            return "Unknown corruption state";
    }
}

float corruption_calculate_penalty(const CorruptionState* state) {
    if (!state) {
        return 0.0f;
    }

    /* Penalty ranges from 0% (at 0 corruption) to 50% (at 100 corruption) */
    return state->corruption / 200.0f;
}

bool corruption_is_damned(const CorruptionState* state) {
    if (!state) {
        return false;
    }

    return state->corruption >= CORRUPTION_DAMNED;
}

bool corruption_is_pure(const CorruptionState* state) {
    if (!state) {
        return true;
    }

    return state->corruption < CORRUPTION_TAINTED;
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

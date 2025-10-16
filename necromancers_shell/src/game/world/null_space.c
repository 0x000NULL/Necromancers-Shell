/**
 * @file null_space.c
 * @brief Implementation of null space system
 */

#include "null_space.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Activity names */
static const char* ACTIVITY_NAMES[] = {
    "Idle",
    "Meeting Thessara",
    "Network Debug",
    "Meditation",
    "Research",
    "Soul Analysis"
};

NullSpaceState* null_space_create(void) {
    NullSpaceState* null_space = malloc(sizeof(NullSpaceState));
    if (!null_space) {
        return NULL;
    }

    /* Initialize discovery */
    null_space->discovered = false;
    null_space->discovery_day = 0;

    /* Initialize access */
    null_space->can_access = false;
    null_space->access_unlock_day = 0;

    /* Initialize current state */
    null_space->currently_inside = false;
    null_space->current_activity = NULL_SPACE_IDLE;
    null_space->entry_day = 0;
    null_space->subjective_hours = 0;

    /* Initialize visit history */
    null_space->visit_count = 0;

    /* Initialize statistics */
    null_space->total_visits = 0;
    null_space->total_subjective_hours = 0;
    null_space->thessara_meetings = 0;
    null_space->network_debug_count = 0;
    null_space->meditation_count = 0;

    /* Initialize special unlocks */
    null_space->archon_workspace = false;
    null_space->wraith_observation = false;
    null_space->network_root_access = false;

    return null_space;
}

void null_space_destroy(NullSpaceState* null_space) {
    if (null_space) {
        free(null_space);
    }
}

bool null_space_discover(NullSpaceState* null_space, uint32_t day) {
    if (!null_space) {
        return false;
    }

    if (null_space->discovered) {
        return false; /* Already discovered */
    }

    null_space->discovered = true;
    null_space->discovery_day = day;

    return true;
}

bool null_space_unlock_access(NullSpaceState* null_space, uint32_t day) {
    if (!null_space) {
        return false;
    }

    if (null_space->can_access) {
        return false; /* Already unlocked */
    }

    null_space->can_access = true;
    null_space->access_unlock_day = day;

    return true;
}

bool null_space_enter(NullSpaceState* null_space, uint32_t day, NullSpaceActivity activity) {
    if (!null_space) {
        return false;
    }

    if (!null_space->can_access) {
        return false; /* No access */
    }

    if (null_space->currently_inside) {
        return false; /* Already inside */
    }

    null_space->currently_inside = true;
    null_space->current_activity = activity;
    null_space->entry_day = day;
    null_space->subjective_hours = 0;

    return true;
}

bool null_space_exit(NullSpaceState* null_space, const char* notes) {
    if (!null_space) {
        return false;
    }

    if (!null_space->currently_inside) {
        return false; /* Not inside */
    }

    /* Record visit if there's room */
    if (null_space->visit_count < MAX_NULL_SPACE_VISITS) {
        NullSpaceVisit* visit = &null_space->visits[null_space->visit_count];
        visit->visit_day = null_space->entry_day;
        visit->activity = null_space->current_activity;
        visit->duration_hours = null_space->subjective_hours;

        if (notes) {
            snprintf(visit->notes, sizeof(visit->notes), "%s", notes);
        } else {
            visit->notes[0] = '\0';
        }

        null_space->visit_count++;
    }

    /* Update statistics */
    null_space->total_visits++;
    null_space->total_subjective_hours += null_space->subjective_hours;

    /* Update activity-specific counters */
    switch (null_space->current_activity) {
        case NULL_SPACE_MEETING_THESSARA:
            null_space->thessara_meetings++;
            break;
        case NULL_SPACE_NETWORK_DEBUG:
            null_space->network_debug_count++;
            break;
        case NULL_SPACE_MEDITATION:
            null_space->meditation_count++;
            break;
        default:
            break;
    }

    /* Clear current state */
    null_space->currently_inside = false;
    null_space->current_activity = NULL_SPACE_IDLE;
    null_space->entry_day = 0;
    null_space->subjective_hours = 0;

    return true;
}

bool null_space_change_activity(NullSpaceState* null_space, NullSpaceActivity activity) {
    if (!null_space) {
        return false;
    }

    if (!null_space->currently_inside) {
        return false; /* Not inside */
    }

    null_space->current_activity = activity;
    return true;
}

bool null_space_add_time(NullSpaceState* null_space, uint32_t hours) {
    if (!null_space) {
        return false;
    }

    if (!null_space->currently_inside) {
        return false; /* Not inside */
    }

    null_space->subjective_hours += hours;
    return true;
}

bool null_space_unlock_archon_workspace(NullSpaceState* null_space) {
    if (!null_space) {
        return false;
    }

    if (null_space->archon_workspace) {
        return false; /* Already unlocked */
    }

    null_space->archon_workspace = true;
    return true;
}

bool null_space_unlock_wraith_observation(NullSpaceState* null_space) {
    if (!null_space) {
        return false;
    }

    if (null_space->wraith_observation) {
        return false; /* Already unlocked */
    }

    null_space->wraith_observation = true;
    return true;
}

bool null_space_unlock_network_root_access(NullSpaceState* null_space) {
    if (!null_space) {
        return false;
    }

    if (null_space->network_root_access) {
        return false; /* Already unlocked */
    }

    null_space->network_root_access = true;
    return true;
}

bool null_space_is_inside(const NullSpaceState* null_space) {
    if (!null_space) {
        return false;
    }
    return null_space->currently_inside;
}

bool null_space_has_access(const NullSpaceState* null_space) {
    if (!null_space) {
        return false;
    }
    return null_space->can_access;
}

const NullSpaceVisit* null_space_get_visit(const NullSpaceState* null_space, size_t index) {
    if (!null_space || index >= null_space->visit_count) {
        return NULL;
    }
    return &null_space->visits[index];
}

const char* null_space_activity_name(NullSpaceActivity activity) {
    if (activity < 0 || activity >= 6) {
        return "Unknown";
    }
    return ACTIVITY_NAMES[activity];
}

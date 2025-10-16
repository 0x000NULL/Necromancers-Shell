/**
 * archon_state.c - Archon Transformation System Implementation
 */

#include "archon_state.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Reforms required per administrative level */
#define REFORMS_PER_LEVEL 15

/* Privilege definitions */
static const struct {
    ArchonPrivilege privilege;
    int level_required;
    const char* name;
    const char* description;
} PRIVILEGE_DATA[PRIVILEGE_COUNT] = {
    {PRIVILEGE_NETWORK_PATCHING, 1, "Network Patching",
     "Deploy fixes to Death Network bugs"},
    {PRIVILEGE_SPLIT_ROUTING, 2, "Split-Routing",
     "Route souls to multiple afterlives simultaneously"},
    {PRIVILEGE_ENFORCER_COMMAND, 5, "Enforcer Command",
     "Direct divine enforcers during purges"},
    {PRIVILEGE_DIVINE_SUMMONING, 8, "Divine Summoning",
     "Call upon gods for counsel"},
    {PRIVILEGE_REALITY_EDITING, 10, "Reality Editing",
     "Reshape aspects of the Death Network itself"}
};

ArchonState* archon_state_create(void) {
    ArchonState* state = calloc(1, sizeof(ArchonState));
    if (!state) {
        return NULL;
    }

    state->transformed = false;
    state->administrative_level = 0;
    state->reforms_per_level = REFORMS_PER_LEVEL;

    /* Initialize privileges */
    for (int i = 0; i < PRIVILEGE_COUNT; i++) {
        state->privileges[i].privilege = PRIVILEGE_DATA[i].privilege;
        state->privileges[i].level_required = PRIVILEGE_DATA[i].level_required;
        state->privileges[i].unlocked = false;
        strncpy(state->privileges[i].name, PRIVILEGE_DATA[i].name,
               sizeof(state->privileges[i].name) - 1);
        strncpy(state->privileges[i].description, PRIVILEGE_DATA[i].description,
               sizeof(state->privileges[i].description) - 1);
    }

    return state;
}

void archon_state_destroy(ArchonState* state) {
    free(state);
}

bool archon_transform(ArchonState* state, bool all_trials_passed,
                     bool amnesty_granted, int corruption,
                     const char** restrictions, size_t restriction_count,
                     int game_day, bool thessara_alive) {
    if (!state || state->transformed) {
        return false;
    }

    /* Validate requirements */
    if (!all_trials_passed || !amnesty_granted) {
        return false;
    }

    /* Perform transformation */
    state->transformed = true;
    state->transformation_day = game_day;
    state->administrative_level = MIN_ADMIN_LEVEL; /* Start at level 3 */
    state->corruption_locked = corruption;

    /* Load Code of Conduct */
    state->rule_count = 0;
    if (restrictions && restriction_count > 0) {
        size_t count = (restriction_count < MAX_CODE_OF_CONDUCT_RULES) ?
                       restriction_count : MAX_CODE_OF_CONDUCT_RULES;
        for (size_t i = 0; i < count; i++) {
            strncpy(state->code_of_conduct[i], restrictions[i], 255);
            state->code_of_conduct[i][255] = '\0';
            state->rule_count++;
        }
    }

    /* Grant initial privileges based on starting level */
    for (int i = 0; i < PRIVILEGE_COUNT; i++) {
        if (state->privileges[i].level_required <= state->administrative_level) {
            state->privileges[i].unlocked = true;
        }
    }

    /* Generate transformation cutscene */
    char* buf = state->transformation_cutscene;
    size_t remaining = MAX_CUTSCENE_LENGTH;
    int written = 0;

    written = snprintf(buf, remaining,
        "ARCHON TRANSFORMATION CEREMONY\n\n"
        "The Divine Council chamber trembles with power. Seven pillars of light "
        "descend from the infinite void above, each bearing the presence of a god.\n\n"
        "Thalor speaks first, his voice like the tolling of judgment bells:\n"
        "\"By the authority vested in us as Architects of Death, we declare this "
        "transformation lawful. Step forward.\"\n\n"
        "You move to the center of the chamber. The corruption locked within you—"
        "%d%%—begins to crystallize, forming permanent boundaries you can feel "
        "but not see.\n\n", corruption);
    buf += written;
    remaining -= written;

    if (thessara_alive) {
        written = snprintf(buf, remaining,
            "Thessara stands at the chamber's edge, her eyes filled with pride and "
            "something deeper. She nods once. You are not alone in this.\n\n");
        buf += written;
        remaining -= written;
    }

    written = snprintf(buf, remaining,
        "Anara raises her hand. Light flows from her palm, warm and gentle:\n"
        "\"Let compassion guide your judgments. Remember that every soul has a story.\"\n\n"
        "Zymira's presence fills your mind with schematics, protocols, access codes:\n"
        "\"The Death Network's secrets are now yours. Use this knowledge wisely.\"\n\n"
        "Kaelthas gestures, and perfect equilibrium settles into your bones:\n"
        "\"You are the fulcrum upon which justice and mercy balance. Maintain this.\"\n\n"
        "Vorath's power crashes over you like a thunderstorm, terrifying and exhilarating:\n"
        "\"Strength without cruelty. Punishment without sadism. Can you do this?\"\n\n"
        "Nyxara's eyes see through you, past you, into futures you cannot comprehend:\n"
        "\"Your thread burns bright, Archon. But even bright threads can be cut.\"\n\n"
        "Eryndor's warmth is the last to touch you, filling empty spaces with hope:\n"
        "\"Build. Create. Reform. Show us that necromancers can be more than destroyers.\"\n\n"
        "The seven lights converge. Your body burns with divine fire. When the light "
        "fades, you are changed—fundamentally, irrevocably.\n\n"
        "You are an ARCHON now. Administrative Level %d.\n\n"
        "The Code of Conduct is seared into your consciousness:\n",
        state->administrative_level);
    buf += written;
    remaining -= written;

    for (size_t i = 0; i < state->rule_count; i++) {
        written = snprintf(buf, remaining, "%zu. %s\n", i + 1,
                          state->code_of_conduct[i]);
        buf += written;
        remaining -= written;
    }

    written = snprintf(buf, remaining,
        "\nThe gods fade. You remain. The Death Network awaits your command.\n");
    buf += written;
    remaining -= written;

    return true;
}

bool archon_grant_privilege(ArchonState* state, ArchonPrivilege privilege) {
    if (!state || !state->transformed) {
        return false;
    }

    if (privilege < 0 || privilege >= PRIVILEGE_COUNT) {
        return false;
    }

    PrivilegeInfo* priv = &state->privileges[privilege];

    /* Check level requirement */
    if (state->administrative_level < priv->level_required) {
        return false;
    }

    priv->unlocked = true;
    return true;
}

bool archon_advance_level(ArchonState* state) {
    if (!state || !state->transformed) {
        return false;
    }

    /* Check if at max level */
    if (state->administrative_level >= MAX_ADMIN_LEVEL) {
        return false;
    }

    /* Check if enough reforms completed */
    int current_level = state->administrative_level;
    int reforms_for_next_level = (current_level - MIN_ADMIN_LEVEL + 1) * REFORMS_PER_LEVEL;

    if (state->necromancers_reformed < reforms_for_next_level) {
        return false; /* Not enough reforms yet */
    }

    /* Advance level */
    state->administrative_level++;

    /* Grant newly unlocked privileges */
    for (int i = 0; i < PRIVILEGE_COUNT; i++) {
        if (state->privileges[i].level_required == state->administrative_level) {
            state->privileges[i].unlocked = true;
        }
    }

    return true;
}

bool archon_reform_necromancer(ArchonState* state, int npc_id) {
    if (!state || !state->transformed) {
        return false;
    }

    /* In real implementation, would validate NPC and track IDs */
    (void)npc_id; /* Suppress unused warning - used for validation in full implementation */
    state->necromancers_reformed++;

    /* Check if we can advance level */
    archon_advance_level(state);

    return true;
}

bool archon_check_code_violation(const ArchonState* state,
                                 const char* action_description,
                                 int corruption_increase) {
    if (!state || !state->transformed || !action_description) {
        return false;
    }

    /* Check corruption boundaries */
    int new_corruption = state->corruption_locked + corruption_increase;

    /* Check if any rule mentions corruption limits */
    for (size_t i = 0; i < state->rule_count; i++) {
        const char* rule = state->code_of_conduct[i];

        /* Check for "must not exceed" corruption rules */
        if (strstr(rule, "must not exceed") && strstr(rule, "%")) {
            /* Extract percentage (simplified) */
            if (strstr(rule, "65%")) {
                if (new_corruption > 65) {
                    return true; /* Violation */
                }
            }
        }

        /* Check for "must maintain at least" corruption rules */
        if (strstr(rule, "must maintain at least") && strstr(rule, "%")) {
            if (strstr(rule, "25%")) {
                if (new_corruption < 25) {
                    return true; /* Violation */
                }
            }
        }

        /* Check for massacre prohibition */
        if (strstr(rule, "civilian massacres")) {
            if (strstr(action_description, "massacre") ||
                strstr(action_description, "slaughter")) {
                return true; /* Violation */
            }
        }
    }

    return false; /* No violation detected */
}

const char* archon_get_transformation_cutscene(const ArchonState* state) {
    if (!state) {
        return "";
    }
    return state->transformation_cutscene;
}

bool archon_is_privilege_unlocked(const ArchonState* state,
                                  ArchonPrivilege privilege) {
    if (!state || privilege < 0 || privilege >= PRIVILEGE_COUNT) {
        return false;
    }

    return state->privileges[privilege].unlocked;
}

const PrivilegeInfo* archon_get_privilege_info(const ArchonState* state,
                                               ArchonPrivilege privilege) {
    if (!state || privilege < 0 || privilege >= PRIVILEGE_COUNT) {
        return NULL;
    }

    return &state->privileges[privilege];
}

int archon_get_reforms_needed_for_next_level(const ArchonState* state) {
    if (!state || !state->transformed) {
        return 0;
    }

    if (state->administrative_level >= MAX_ADMIN_LEVEL) {
        return 0; /* At max level */
    }

    int current_level = state->administrative_level;
    int reforms_for_next_level = (current_level - MIN_ADMIN_LEVEL + 1) * REFORMS_PER_LEVEL;

    int needed = reforms_for_next_level - state->necromancers_reformed;
    return (needed > 0) ? needed : 0;
}

const char* archon_get_privilege_name(ArchonPrivilege privilege) {
    if (privilege < 0 || privilege >= PRIVILEGE_COUNT) {
        return "Unknown";
    }

    return PRIVILEGE_DATA[privilege].name;
}

/**
 * @file npc.c
 * @brief NPC implementation
 */

#include "npc.h"
#include "../../../utils/logger.h"
#include <stdlib.h>
#include <string.h>

NPC* npc_create(const char* id, const char* name, NPCArchetype archetype) {
    if (!id || !name) {
        LOG_ERROR("npc_create: NULL parameters");
        return NULL;
    }

    NPC* npc = malloc(sizeof(NPC));
    if (!npc) {
        LOG_ERROR("npc_create: malloc failed");
        return NULL;
    }

    /* Initialize structure */
    memset(npc, 0, sizeof(NPC));

    /* Copy identity */
    strncpy(npc->id, id, sizeof(npc->id) - 1);
    strncpy(npc->name, name, sizeof(npc->name) - 1);
    npc->archetype = archetype;

    /* Initialize defaults */
    npc->location_type = NPC_LOCATION_UNKNOWN;
    npc->available = true;
    npc->discovered = false;
    npc->first_met_time = 0;
    npc->dialogue_state_count = 0;
    npc->active_quest_count = 0;
    npc->completed_quest_count = 0;
    npc->memory_count = 0;
    npc->interaction_count = 0;
    npc->last_interaction_time = 0;
    npc->is_hostile = false;
    npc->is_dead = false;
    npc->is_hidden = false;

    /* Set default dialogue state */
    strncpy(npc->current_dialogue_state, "initial", sizeof(npc->current_dialogue_state) - 1);

    return npc;
}

void npc_destroy(NPC* npc) {
    if (!npc) return;
    free(npc);
}

void npc_discover(NPC* npc, const char* location) {
    if (!npc) return;

    if (!npc->discovered) {
        npc->discovered = true;
        npc->first_met_time = time(NULL);

        if (location) {
            strncpy(npc->current_location, location, sizeof(npc->current_location) - 1);
            /* If no home location set, use discovery location as home */
            if (npc->home_location[0] == '\0') {
                strncpy(npc->home_location, location, sizeof(npc->home_location) - 1);
            }
        }

        LOG_INFO("NPC discovered: %s at %s", npc->name, location ? location : "unknown");
    }
}

void npc_interact(NPC* npc) {
    if (!npc) return;

    npc->interaction_count++;
    npc->last_interaction_time = time(NULL);

    LOG_DEBUG("NPC interaction: %s (total: %d)", npc->name, npc->interaction_count);
}

void npc_set_dialogue_state(NPC* npc, const char* state) {
    if (!npc || !state) return;

    strncpy(npc->current_dialogue_state, state, sizeof(npc->current_dialogue_state) - 1);
    LOG_DEBUG("NPC %s dialogue state changed to: %s", npc->name, state);
}

void npc_add_dialogue_state(NPC* npc, const char* state) {
    if (!npc || !state) return;

    if (npc->dialogue_state_count >= MAX_NPC_DIALOGUE_STATES) {
        LOG_WARN("NPC %s: max dialogue states reached", npc->id);
        return;
    }

    /* Check for duplicates */
    for (size_t i = 0; i < npc->dialogue_state_count; i++) {
        if (strcmp(npc->dialogue_states[i], state) == 0) {
            return; /* Already exists */
        }
    }

    strncpy(npc->dialogue_states[npc->dialogue_state_count], state, 63);
    npc->dialogue_states[npc->dialogue_state_count][63] = '\0';
    npc->dialogue_state_count++;
}

bool npc_has_dialogue_state(const NPC* npc, const char* state) {
    if (!npc || !state) return false;

    for (size_t i = 0; i < npc->dialogue_state_count; i++) {
        if (strcmp(npc->dialogue_states[i], state) == 0) {
            return true;
        }
    }

    return false;
}

void npc_add_active_quest(NPC* npc, const char* quest_id) {
    if (!npc || !quest_id) return;

    if (npc->active_quest_count >= MAX_NPC_QUESTS) {
        LOG_WARN("NPC %s: max active quests reached", npc->id);
        return;
    }

    /* Check for duplicates */
    for (size_t i = 0; i < npc->active_quest_count; i++) {
        if (strcmp(npc->active_quests[i], quest_id) == 0) {
            return; /* Already active */
        }
    }

    strncpy(npc->active_quests[npc->active_quest_count], quest_id, 63);
    npc->active_quests[npc->active_quest_count][63] = '\0';
    npc->active_quest_count++;

    LOG_INFO("NPC %s: Quest '%s' added", npc->name, quest_id);
}

bool npc_complete_quest(NPC* npc, const char* quest_id) {
    if (!npc || !quest_id) return false;

    /* Find and remove from active quests */
    bool found = false;
    for (size_t i = 0; i < npc->active_quest_count; i++) {
        if (strcmp(npc->active_quests[i], quest_id) == 0) {
            /* Shift remaining quests down */
            for (size_t j = i; j < npc->active_quest_count - 1; j++) {
                strncpy(npc->active_quests[j], npc->active_quests[j + 1], 63);
                npc->active_quests[j][63] = '\0';
            }
            npc->active_quest_count--;
            found = true;
            break;
        }
    }

    if (!found) {
        return false;
    }

    /* Add to completed quests */
    if (npc->completed_quest_count < MAX_NPC_QUESTS) {
        strncpy(npc->completed_quests[npc->completed_quest_count], quest_id, 63);
        npc->completed_quests[npc->completed_quest_count][63] = '\0';
        npc->completed_quest_count++;
    }

    LOG_INFO("NPC %s: Quest '%s' completed", npc->name, quest_id);
    return true;
}

bool npc_has_active_quest(const NPC* npc, const char* quest_id) {
    if (!npc || !quest_id) return false;

    for (size_t i = 0; i < npc->active_quest_count; i++) {
        if (strcmp(npc->active_quests[i], quest_id) == 0) {
            return true;
        }
    }

    return false;
}

bool npc_has_completed_quest(const NPC* npc, const char* quest_id) {
    if (!npc || !quest_id) return false;

    for (size_t i = 0; i < npc->completed_quest_count; i++) {
        if (strcmp(npc->completed_quests[i], quest_id) == 0) {
            return true;
        }
    }

    return false;
}

void npc_add_unlockable_memory(NPC* npc, const char* memory_id) {
    if (!npc || !memory_id) return;

    if (npc->memory_count >= MAX_NPC_MEMORIES) {
        LOG_WARN("NPC %s: max unlockable memories reached", npc->id);
        return;
    }

    strncpy(npc->unlockable_memories[npc->memory_count], memory_id, 63);
    npc->unlockable_memories[npc->memory_count][63] = '\0';
    npc->memory_count++;
}

void npc_move_to_location(NPC* npc, const char* location) {
    if (!npc || !location) return;

    strncpy(npc->current_location, location, sizeof(npc->current_location) - 1);
    LOG_DEBUG("NPC %s moved to: %s", npc->name, location);
}

bool npc_is_available(const NPC* npc) {
    if (!npc) return false;

    return npc->available && !npc->is_dead && !npc->is_hidden;
}

const char* npc_archetype_to_string(NPCArchetype archetype) {
    switch (archetype) {
        case NPC_ARCHETYPE_MENTOR:
            return "Mentor";
        case NPC_ARCHETYPE_RIVAL:
            return "Rival";
        case NPC_ARCHETYPE_ALLY:
            return "Ally";
        case NPC_ARCHETYPE_ANTAGONIST:
            return "Antagonist";
        case NPC_ARCHETYPE_NEUTRAL:
            return "Neutral";
        case NPC_ARCHETYPE_MYSTERIOUS:
            return "Mysterious";
        default:
            return "Unknown";
    }
}

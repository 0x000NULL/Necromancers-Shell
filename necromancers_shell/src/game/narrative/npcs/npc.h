/**
 * @file npc.h
 * @brief NPC character structure and operations
 */

#ifndef NECROMANCERS_NPC_H
#define NECROMANCERS_NPC_H

#include <stdbool.h>
#include <stddef.h>
#include <time.h>

/* Maximum values for NPC data */
#define MAX_NPC_DIALOGUE_STATES 16
#define MAX_NPC_QUESTS 8
#define MAX_NPC_MEMORIES 8

/* NPC archetype defines personality and role */
typedef enum {
    NPC_ARCHETYPE_MENTOR,      /* Wise guide, teaches player */
    NPC_ARCHETYPE_RIVAL,       /* Competitive, challenges player */
    NPC_ARCHETYPE_ALLY,        /* Friendly, supports player */
    NPC_ARCHETYPE_ANTAGONIST,  /* Opposes player's goals */
    NPC_ARCHETYPE_NEUTRAL,     /* Indifferent, transactional */
    NPC_ARCHETYPE_MYSTERIOUS   /* Cryptic, reveals information slowly */
} NPCArchetype;

/* NPC location state */
typedef enum {
    NPC_LOCATION_UNKNOWN,      /* Location not yet discovered */
    NPC_LOCATION_FIXED,        /* NPC stays at one location */
    NPC_LOCATION_MOBILE,       /* NPC moves between locations */
    NPC_LOCATION_QUEST_BASED   /* Location changes based on quest state */
} NPCLocationType;

/**
 * @struct NPC
 * @brief Represents a non-player character with dialogue, quests, and relationships
 */
typedef struct {
    /* Identity */
    char id[64];
    char name[128];
    char title[128];
    char description[512];

    /* Character data */
    NPCArchetype archetype;
    char faction[64];

    /* Location */
    NPCLocationType location_type;
    char current_location[64];
    char home_location[64];

    /* Availability */
    bool available;
    bool discovered;
    time_t first_met_time;

    /* Dialogue state */
    char current_dialogue_state[64];
    char dialogue_states[MAX_NPC_DIALOGUE_STATES][64];
    size_t dialogue_state_count;

    /* Quest tracking */
    char active_quests[MAX_NPC_QUESTS][64];
    size_t active_quest_count;
    char completed_quests[MAX_NPC_QUESTS][64];
    size_t completed_quest_count;

    /* Memory fragments this NPC can reveal */
    char unlockable_memories[MAX_NPC_MEMORIES][64];
    size_t memory_count;

    /* Interaction tracking */
    int interaction_count;
    time_t last_interaction_time;

    /* Flags for special states */
    bool is_hostile;
    bool is_dead;
    bool is_hidden;
} NPC;

/**
 * @brief Create a new NPC
 * @param id Unique identifier for the NPC
 * @param name Display name
 * @param archetype NPC personality type
 * @return Newly allocated NPC or NULL on failure
 */
NPC* npc_create(const char* id, const char* name, NPCArchetype archetype);

/**
 * @brief Destroy an NPC and free memory
 * @param npc NPC to destroy (can be NULL)
 */
void npc_destroy(NPC* npc);

/**
 * @brief Mark NPC as discovered by player
 * @param npc NPC to discover
 * @param location Location where NPC was discovered
 */
void npc_discover(NPC* npc, const char* location);

/**
 * @brief Record an interaction with the NPC
 * @param npc NPC being interacted with
 */
void npc_interact(NPC* npc);

/**
 * @brief Change NPC's dialogue state
 * @param npc NPC to update
 * @param state New dialogue state ID
 */
void npc_set_dialogue_state(NPC* npc, const char* state);

/**
 * @brief Add a dialogue state to NPC's available states
 * @param npc NPC to update
 * @param state Dialogue state ID to add
 */
void npc_add_dialogue_state(NPC* npc, const char* state);

/**
 * @brief Check if NPC has a specific dialogue state available
 * @param npc NPC to check
 * @param state Dialogue state ID to check
 * @return true if state is available, false otherwise
 */
bool npc_has_dialogue_state(const NPC* npc, const char* state);

/**
 * @brief Add an active quest to the NPC
 * @param npc NPC to update
 * @param quest_id Quest identifier
 */
void npc_add_active_quest(NPC* npc, const char* quest_id);

/**
 * @brief Complete a quest for the NPC
 * @param npc NPC to update
 * @param quest_id Quest identifier
 * @return true if quest was active and completed, false otherwise
 */
bool npc_complete_quest(NPC* npc, const char* quest_id);

/**
 * @brief Check if NPC has an active quest
 * @param npc NPC to check
 * @param quest_id Quest identifier
 * @return true if quest is active, false otherwise
 */
bool npc_has_active_quest(const NPC* npc, const char* quest_id);

/**
 * @brief Check if NPC has completed a quest
 * @param npc NPC to check
 * @param quest_id Quest identifier
 * @return true if quest is completed, false otherwise
 */
bool npc_has_completed_quest(const NPC* npc, const char* quest_id);

/**
 * @brief Add a memory fragment that this NPC can unlock
 * @param npc NPC to update
 * @param memory_id Memory fragment identifier
 */
void npc_add_unlockable_memory(NPC* npc, const char* memory_id);

/**
 * @brief Move NPC to a new location
 * @param npc NPC to move
 * @param location New location ID
 */
void npc_move_to_location(NPC* npc, const char* location);

/**
 * @brief Check if NPC is available for interaction
 * @param npc NPC to check
 * @return true if available, false otherwise
 */
bool npc_is_available(const NPC* npc);

/**
 * @brief Get archetype name as string
 * @param archetype Archetype enum value
 * @return Human-readable archetype name
 */
const char* npc_archetype_to_string(NPCArchetype archetype);

#endif /* NECROMANCERS_NPC_H */

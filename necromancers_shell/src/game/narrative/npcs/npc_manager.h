/**
 * @file npc_manager.h
 * @brief NPC collection manager
 */

#ifndef NECROMANCERS_NPC_MANAGER_H
#define NECROMANCERS_NPC_MANAGER_H

#include "npc.h"
#include <stdbool.h>
#include <stddef.h>

/**
 * @struct NPCManager
 * @brief Manages collection of NPCs in the game
 */
typedef struct {
    NPC** npcs;
    size_t npc_count;
    size_t npc_capacity;
} NPCManager;

/**
 * @brief Create a new NPC manager
 * @return Newly allocated NPCManager or NULL on failure
 */
NPCManager* npc_manager_create(void);

/**
 * @brief Destroy NPC manager and all NPCs
 * @param manager NPCManager to destroy (can be NULL)
 */
void npc_manager_destroy(NPCManager* manager);

/**
 * @brief Add an NPC to the manager
 * @param manager NPCManager to update
 * @param npc NPC to add (manager takes ownership)
 */
void npc_manager_add_npc(NPCManager* manager, NPC* npc);

/**
 * @brief Get an NPC by ID
 * @param manager NPCManager to search
 * @param npc_id NPC identifier
 * @return Pointer to NPC or NULL if not found
 */
NPC* npc_manager_get_npc(const NPCManager* manager, const char* npc_id);

/**
 * @brief Get all discovered NPCs
 * @param manager NPCManager to search
 * @param count_out Output parameter for number of NPCs found
 * @return Array of NPC pointers (caller must free) or NULL if none found
 */
NPC** npc_manager_get_discovered(const NPCManager* manager, size_t* count_out);

/**
 * @brief Get all NPCs at a specific location
 * @param manager NPCManager to search
 * @param location Location ID to filter by
 * @param count_out Output parameter for number of NPCs found
 * @return Array of NPC pointers (caller must free) or NULL if none found
 */
NPC** npc_manager_get_at_location(const NPCManager* manager, const char* location, size_t* count_out);

/**
 * @brief Get all available NPCs (not dead, not hidden)
 * @param manager NPCManager to search
 * @param count_out Output parameter for number of NPCs found
 * @return Array of NPC pointers (caller must free) or NULL if none found
 */
NPC** npc_manager_get_available(const NPCManager* manager, size_t* count_out);

/**
 * @brief Get NPCs by archetype
 * @param manager NPCManager to search
 * @param archetype Archetype to filter by
 * @param count_out Output parameter for number of NPCs found
 * @return Array of NPC pointers (caller must free) or NULL if none found
 */
NPC** npc_manager_get_by_archetype(const NPCManager* manager, NPCArchetype archetype, size_t* count_out);

/**
 * @brief Get NPCs by faction
 * @param manager NPCManager to search
 * @param faction Faction name to filter by
 * @param count_out Output parameter for number of NPCs found
 * @return Array of NPC pointers (caller must free) or NULL if none found
 */
NPC** npc_manager_get_by_faction(const NPCManager* manager, const char* faction, size_t* count_out);

/**
 * @brief Get NPCs with active quests
 * @param manager NPCManager to search
 * @param count_out Output parameter for number of NPCs found
 * @return Array of NPC pointers (caller must free) or NULL if none found
 */
NPC** npc_manager_get_with_active_quests(const NPCManager* manager, size_t* count_out);

/**
 * @brief Discover an NPC by ID
 * @param manager NPCManager to update
 * @param npc_id NPC identifier
 * @param location Location where discovered
 */
void npc_manager_discover_npc(NPCManager* manager, const char* npc_id, const char* location);

/**
 * @brief Load NPCs from a data file
 * @param manager NPCManager to update
 * @param filepath Path to data file
 * @return true on success, false on failure
 */
bool npc_manager_load_from_file(NPCManager* manager, const char* filepath);

#endif /* NECROMANCERS_NPC_MANAGER_H */

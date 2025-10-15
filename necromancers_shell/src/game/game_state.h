/**
 * @file game_state.h
 * @brief Central game state manager
 *
 * Manages all game systems: souls, minions, territory, resources, corruption.
 * This is the single source of truth for the game's runtime state.
 */

#ifndef NECROMANCER_GAME_STATE_H
#define NECROMANCER_GAME_STATE_H

#include "souls/soul_manager.h"
#include "world/territory.h"
#include "world/location_graph.h"
#include "world/world_map.h"
#include "world/territory_status.h"
#include "world/death_network.h"
#include "resources/resources.h"
#include "resources/corruption.h"
#include "resources/consciousness.h"
#include "narrative/memory/memory_manager.h"
#include "narrative/npcs/npc_manager.h"
#include "narrative/relationships/relationship_manager.h"
#include "narrative/quests/quest_manager.h"
#include "narrative/dialogue/dialogue_manager.h"
#include <stdint.h>
#include <stdbool.h>

/* Forward declarations */
typedef struct MinionManager MinionManager;
typedef struct CombatState CombatState;

/**
 * @brief Central game state structure
 */
typedef struct {
    SoulManager* souls;             /**< Soul inventory manager */
    MinionManager* minions;         /**< Minion army manager */
    TerritoryManager* territory;    /**< World locations and territory */
    LocationGraph* location_graph;  /**< Location connectivity and pathfinding */
    WorldMap* world_map;            /**< World map visualization */
    TerritoryStatusManager* territory_status; /**< Territory control and alerts */
    DeathNetwork* death_network;    /**< Death Network for corpse generation */
    CombatState* combat;            /**< Active combat state (NULL if not in combat) */
    Resources resources;            /**< Resources (energy, mana, time) */
    CorruptionState corruption;     /**< Corruption tracking */
    ConsciousnessState consciousness; /**< Consciousness decay tracking */
    MemoryManager* memories;        /**< Memory fragment collection */
    NPCManager* npcs;               /**< NPC collection manager */
    RelationshipManager* relationships; /**< Player-NPC relationships */
    QuestManager* quests;           /**< Quest collection manager */
    DialogueManager* dialogues;     /**< Dialogue collection manager */
    uint32_t current_location_id;   /**< ID of current location */
    uint32_t player_level;          /**< Player level */
    uint64_t player_experience;     /**< Player XP */
    uint32_t next_soul_id;          /**< Next available soul ID */
    uint32_t next_minion_id;        /**< Next available minion ID */
    bool initialized;               /**< Whether game state is ready */
} GameState;

/**
 * @brief Create and initialize game state
 *
 * Initializes all subsystems and loads default data.
 *
 * @return Newly allocated GameState, or NULL on failure
 */
GameState* game_state_create(void);

/**
 * @brief Destroy game state and all subsystems
 *
 * @param state Game state to destroy (can be NULL)
 */
void game_state_destroy(GameState* state);

/**
 * @brief Get global game state instance
 *
 * @return Pointer to global game state, or NULL if not initialized
 */
GameState* game_state_get_instance(void);

/**
 * @brief Get next available soul ID and increment counter
 *
 * @param state Game state
 * @return Next soul ID
 */
uint32_t game_state_next_soul_id(GameState* state);

/**
 * @brief Get next available minion ID and increment counter
 *
 * @param state Game state
 * @return Next minion ID
 */
uint32_t game_state_next_minion_id(GameState* state);

/**
 * @brief Get current location
 *
 * @param state Game state
 * @return Pointer to current location, or NULL if not found
 */
Location* game_state_get_current_location(const GameState* state);

/**
 * @brief Move player to a new location
 *
 * Validates connection and updates current_location_id.
 *
 * @param state Game state
 * @param location_id ID of location to move to
 * @return true on success, false if location not found or not connected
 */
bool game_state_move_to_location(GameState* state, uint32_t location_id);

/**
 * @brief Update game state for elapsed time
 *
 * Advances time, regenerates mana, etc.
 *
 * @param state Game state
 * @param hours Hours to advance
 */
void game_state_advance_time(GameState* state, uint32_t hours);

#endif /* NECROMANCER_GAME_STATE_H */

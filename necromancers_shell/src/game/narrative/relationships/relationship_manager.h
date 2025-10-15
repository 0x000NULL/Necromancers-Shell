/**
 * @file relationship_manager.h
 * @brief Relationship collection manager
 */

#ifndef NECROMANCERS_RELATIONSHIP_MANAGER_H
#define NECROMANCERS_RELATIONSHIP_MANAGER_H

#include "relationship.h"
#include <stdbool.h>
#include <stddef.h>

/**
 * @struct RelationshipManager
 * @brief Manages collection of NPC relationships
 */
typedef struct {
    Relationship** relationships;
    size_t relationship_count;
    size_t relationship_capacity;
} RelationshipManager;

/**
 * @brief Create a new relationship manager
 * @return Newly allocated RelationshipManager or NULL on failure
 */
RelationshipManager* relationship_manager_create(void);

/**
 * @brief Destroy relationship manager and all relationships
 * @param manager RelationshipManager to destroy (can be NULL)
 */
void relationship_manager_destroy(RelationshipManager* manager);

/**
 * @brief Add a relationship to the manager
 * @param manager RelationshipManager to update
 * @param relationship Relationship to add (manager takes ownership)
 */
void relationship_manager_add_relationship(RelationshipManager* manager, Relationship* relationship);

/**
 * @brief Get relationship with an NPC (creates if doesn't exist)
 * @param manager RelationshipManager to query
 * @param npc_id NPC identifier
 * @return Pointer to Relationship or NULL on failure
 */
Relationship* relationship_manager_get_or_create(RelationshipManager* manager, const char* npc_id);

/**
 * @brief Get relationship with an NPC (returns NULL if doesn't exist)
 * @param manager RelationshipManager to query
 * @param npc_id NPC identifier
 * @return Pointer to Relationship or NULL if not found
 */
Relationship* relationship_manager_get(const RelationshipManager* manager, const char* npc_id);

/**
 * @brief Get all relationships
 * @param manager RelationshipManager to query
 * @param count_out Output parameter for number of relationships
 * @return Array of Relationship pointers (caller must free) or NULL if none
 */
Relationship** relationship_manager_get_all(const RelationshipManager* manager, size_t* count_out);

/**
 * @brief Get relationships by status
 * @param manager RelationshipManager to query
 * @param status Status to filter by
 * @param count_out Output parameter for number of relationships
 * @return Array of Relationship pointers (caller must free) or NULL if none
 */
Relationship** relationship_manager_get_by_status(const RelationshipManager* manager,
                                                   RelationshipStatus status,
                                                   size_t* count_out);

/**
 * @brief Get relationships that meet minimum status
 * @param manager RelationshipManager to query
 * @param min_status Minimum status required
 * @param count_out Output parameter for number of relationships
 * @return Array of Relationship pointers (caller must free) or NULL if none
 */
Relationship** relationship_manager_get_meeting_status(const RelationshipManager* manager,
                                                         RelationshipStatus min_status,
                                                         size_t* count_out);

/**
 * @brief Get relationships with high trust (>= 75)
 * @param manager RelationshipManager to query
 * @param count_out Output parameter for number of relationships
 * @return Array of Relationship pointers (caller must free) or NULL if none
 */
Relationship** relationship_manager_get_high_trust(const RelationshipManager* manager, size_t* count_out);

/**
 * @brief Get relationships with high fear (>= 75)
 * @param manager RelationshipManager to query
 * @param count_out Output parameter for number of relationships
 * @return Array of Relationship pointers (caller must free) or NULL if none
 */
Relationship** relationship_manager_get_high_fear(const RelationshipManager* manager, size_t* count_out);

/**
 * @brief Get active romances
 * @param manager RelationshipManager to query
 * @param count_out Output parameter for number of relationships
 * @return Array of Relationship pointers (caller must free) or NULL if none
 */
Relationship** relationship_manager_get_romances(const RelationshipManager* manager, size_t* count_out);

/**
 * @brief Get rival relationships
 * @param manager RelationshipManager to query
 * @param count_out Output parameter for number of relationships
 * @return Array of Relationship pointers (caller must free) or NULL if none
 */
Relationship** relationship_manager_get_rivals(const RelationshipManager* manager, size_t* count_out);

/**
 * @brief Modify trust with an NPC
 * @param manager RelationshipManager to update
 * @param npc_id NPC identifier
 * @param delta Change in trust
 */
void relationship_manager_modify_trust(RelationshipManager* manager, const char* npc_id, int delta);

/**
 * @brief Modify respect with an NPC
 * @param manager RelationshipManager to update
 * @param npc_id NPC identifier
 * @param delta Change in respect
 */
void relationship_manager_modify_respect(RelationshipManager* manager, const char* npc_id, int delta);

/**
 * @brief Modify fear with an NPC
 * @param manager RelationshipManager to update
 * @param npc_id NPC identifier
 * @param delta Change in fear
 */
void relationship_manager_modify_fear(RelationshipManager* manager, const char* npc_id, int delta);

/**
 * @brief Add an event to relationship with an NPC
 * @param manager RelationshipManager to update
 * @param npc_id NPC identifier
 * @param type Event type
 * @param trust_delta Change in trust
 * @param respect_delta Change in respect
 * @param fear_delta Change in fear
 * @param description Event description
 */
void relationship_manager_add_event(RelationshipManager* manager,
                                   const char* npc_id,
                                   RelationshipEventType type,
                                   int trust_delta,
                                   int respect_delta,
                                   int fear_delta,
                                   const char* description);

#endif /* NECROMANCERS_RELATIONSHIP_MANAGER_H */

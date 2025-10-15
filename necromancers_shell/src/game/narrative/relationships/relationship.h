/**
 * @file relationship.h
 * @brief NPC relationship tracking system
 */

#ifndef NECROMANCERS_RELATIONSHIP_H
#define NECROMANCERS_RELATIONSHIP_H

#include <stdbool.h>
#include <stddef.h>
#include <time.h>

/* Maximum events to track per relationship */
#define MAX_RELATIONSHIP_EVENTS 16

/* Relationship status thresholds */
#define RELATIONSHIP_HOSTILE_THRESHOLD -50
#define RELATIONSHIP_UNFRIENDLY_THRESHOLD -20
#define RELATIONSHIP_NEUTRAL_THRESHOLD 20
#define RELATIONSHIP_FRIENDLY_THRESHOLD 50
#define RELATIONSHIP_ALLIED_THRESHOLD 80

/* Relationship status levels */
typedef enum {
    RELATIONSHIP_STATUS_HOSTILE,    /* -100 to -50 */
    RELATIONSHIP_STATUS_UNFRIENDLY, /* -49 to -20 */
    RELATIONSHIP_STATUS_NEUTRAL,    /* -19 to 20 */
    RELATIONSHIP_STATUS_FRIENDLY,   /* 21 to 50 */
    RELATIONSHIP_STATUS_ALLIED      /* 51 to 100 */
} RelationshipStatus;

/* Event types that affect relationships */
typedef enum {
    RELATIONSHIP_EVENT_QUEST_COMPLETED,
    RELATIONSHIP_EVENT_QUEST_FAILED,
    RELATIONSHIP_EVENT_GIFT_GIVEN,
    RELATIONSHIP_EVENT_DIALOGUE_CHOICE,
    RELATIONSHIP_EVENT_BETRAYAL,
    RELATIONSHIP_EVENT_HELP_PROVIDED,
    RELATIONSHIP_EVENT_HARM_CAUSED,
    RELATIONSHIP_EVENT_SECRET_REVEALED,
    RELATIONSHIP_EVENT_PROMISE_KEPT,
    RELATIONSHIP_EVENT_PROMISE_BROKEN
} RelationshipEventType;

/**
 * @struct RelationshipEvent
 * @brief Records an event that affected a relationship
 */
typedef struct {
    RelationshipEventType type;
    time_t timestamp;
    int trust_delta;
    int respect_delta;
    int fear_delta;
    char description[256];
} RelationshipEvent;

/**
 * @struct Relationship
 * @brief Tracks player's relationship with an NPC
 */
typedef struct {
    char npc_id[64];

    /* Relationship metrics (0-100 scale) */
    int trust;      /* How much NPC trusts the player */
    int respect;    /* How much NPC respects the player */
    int fear;       /* How much NPC fears the player */

    /* Overall relationship score (-100 to 100) */
    int overall_score;
    RelationshipStatus status;

    /* Interaction tracking */
    int total_interactions;
    time_t first_met;
    time_t last_interaction;

    /* Event history */
    RelationshipEvent events[MAX_RELATIONSHIP_EVENTS];
    size_t event_count;

    /* Flags */
    bool is_romanceable;
    bool is_romance_active;
    bool is_rival;
    bool is_locked;  /* Prevents further changes */
} Relationship;

/**
 * @brief Create a new relationship
 * @param npc_id NPC identifier
 * @return Newly allocated Relationship or NULL on failure
 */
Relationship* relationship_create(const char* npc_id);

/**
 * @brief Destroy a relationship and free memory
 * @param relationship Relationship to destroy (can be NULL)
 */
void relationship_destroy(Relationship* relationship);

/**
 * @brief Modify trust value
 * @param relationship Relationship to update
 * @param delta Change in trust (-100 to 100)
 */
void relationship_modify_trust(Relationship* relationship, int delta);

/**
 * @brief Modify respect value
 * @param relationship Relationship to update
 * @param delta Change in respect (-100 to 100)
 */
void relationship_modify_respect(Relationship* relationship, int delta);

/**
 * @brief Modify fear value
 * @param relationship Relationship to update
 * @param delta Change in fear (-100 to 100)
 */
void relationship_modify_fear(Relationship* relationship, int delta);

/**
 * @brief Record an event that affected the relationship
 * @param relationship Relationship to update
 * @param type Event type
 * @param trust_delta Change in trust
 * @param respect_delta Change in respect
 * @param fear_delta Change in fear
 * @param description Event description
 */
void relationship_add_event(Relationship* relationship,
                           RelationshipEventType type,
                           int trust_delta,
                           int respect_delta,
                           int fear_delta,
                           const char* description);

/**
 * @brief Update overall relationship score and status
 * @param relationship Relationship to update
 *
 * Score is calculated as: (trust * 0.5) + (respect * 0.3) - (fear * 0.2)
 * Clamped to -100 to 100 range
 */
void relationship_update_score(Relationship* relationship);

/**
 * @brief Get relationship status from score
 * @param score Overall relationship score
 * @return RelationshipStatus enum value
 */
RelationshipStatus relationship_get_status_from_score(int score);

/**
 * @brief Get relationship status as string
 * @param status Status enum value
 * @return Human-readable status name
 */
const char* relationship_status_to_string(RelationshipStatus status);

/**
 * @brief Get event type as string
 * @param type Event type enum value
 * @return Human-readable event type name
 */
const char* relationship_event_type_to_string(RelationshipEventType type);

/**
 * @brief Check if relationship is at minimum for a given status
 * @param relationship Relationship to check
 * @param required_status Minimum status required
 * @return true if relationship meets or exceeds required status
 */
bool relationship_meets_status(const Relationship* relationship, RelationshipStatus required_status);

/**
 * @brief Get the most recent event
 * @param relationship Relationship to query
 * @return Pointer to most recent event or NULL if no events
 */
const RelationshipEvent* relationship_get_latest_event(const Relationship* relationship);

/**
 * @brief Lock relationship (prevent further changes)
 * @param relationship Relationship to lock
 */
void relationship_lock(Relationship* relationship);

/**
 * @brief Unlock relationship (allow changes)
 * @param relationship Relationship to unlock
 */
void relationship_unlock(Relationship* relationship);

#endif /* NECROMANCERS_RELATIONSHIP_H */

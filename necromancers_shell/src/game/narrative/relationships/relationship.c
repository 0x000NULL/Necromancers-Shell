/**
 * @file relationship.c
 * @brief Relationship implementation
 */

#include "relationship.h"
#include "../../../utils/logger.h"
#include <stdlib.h>
#include <string.h>

/* Clamp value to range */
static int clamp(int value, int min, int max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

Relationship* relationship_create(const char* npc_id) {
    if (!npc_id) {
        LOG_ERROR("relationship_create: NULL npc_id");
        return NULL;
    }

    Relationship* relationship = malloc(sizeof(Relationship));
    if (!relationship) {
        LOG_ERROR("relationship_create: malloc failed");
        return NULL;
    }

    /* Initialize structure */
    memset(relationship, 0, sizeof(Relationship));

    /* Copy NPC ID */
    strncpy(relationship->npc_id, npc_id, sizeof(relationship->npc_id) - 1);

    /* Initialize metrics to neutral */
    relationship->trust = 50;
    relationship->respect = 50;
    relationship->fear = 0;

    /* Calculate initial score */
    relationship_update_score(relationship);

    /* Initialize tracking */
    relationship->total_interactions = 0;
    relationship->first_met = time(NULL);
    relationship->last_interaction = 0;
    relationship->event_count = 0;

    /* Initialize flags */
    relationship->is_romanceable = false;
    relationship->is_romance_active = false;
    relationship->is_rival = false;
    relationship->is_locked = false;

    return relationship;
}

void relationship_destroy(Relationship* relationship) {
    if (!relationship) return;
    free(relationship);
}

void relationship_modify_trust(Relationship* relationship, int delta) {
    if (!relationship || relationship->is_locked) return;

    relationship->trust = clamp(relationship->trust + delta, 0, 100);
    relationship_update_score(relationship);

    LOG_DEBUG("Relationship %s: trust %+d (now %d)", relationship->npc_id, delta, relationship->trust);
}

void relationship_modify_respect(Relationship* relationship, int delta) {
    if (!relationship || relationship->is_locked) return;

    relationship->respect = clamp(relationship->respect + delta, 0, 100);
    relationship_update_score(relationship);

    LOG_DEBUG("Relationship %s: respect %+d (now %d)", relationship->npc_id, delta, relationship->respect);
}

void relationship_modify_fear(Relationship* relationship, int delta) {
    if (!relationship || relationship->is_locked) return;

    relationship->fear = clamp(relationship->fear + delta, 0, 100);
    relationship_update_score(relationship);

    LOG_DEBUG("Relationship %s: fear %+d (now %d)", relationship->npc_id, delta, relationship->fear);
}

void relationship_add_event(Relationship* relationship,
                           RelationshipEventType type,
                           int trust_delta,
                           int respect_delta,
                           int fear_delta,
                           const char* description) {
    if (!relationship) return;

    /* Apply changes */
    relationship_modify_trust(relationship, trust_delta);
    relationship_modify_respect(relationship, respect_delta);
    relationship_modify_fear(relationship, fear_delta);

    /* Record event if space available */
    if (relationship->event_count < MAX_RELATIONSHIP_EVENTS) {
        RelationshipEvent* event = &relationship->events[relationship->event_count];
        event->type = type;
        event->timestamp = time(NULL);
        event->trust_delta = trust_delta;
        event->respect_delta = respect_delta;
        event->fear_delta = fear_delta;

        if (description) {
            strncpy(event->description, description, sizeof(event->description) - 1);
        } else {
            event->description[0] = '\0';
        }

        relationship->event_count++;
    } else {
        /* Shift events and add new one at end */
        for (size_t i = 0; i < MAX_RELATIONSHIP_EVENTS - 1; i++) {
            relationship->events[i] = relationship->events[i + 1];
        }

        RelationshipEvent* event = &relationship->events[MAX_RELATIONSHIP_EVENTS - 1];
        event->type = type;
        event->timestamp = time(NULL);
        event->trust_delta = trust_delta;
        event->respect_delta = respect_delta;
        event->fear_delta = fear_delta;

        if (description) {
            strncpy(event->description, description, sizeof(event->description) - 1);
        } else {
            event->description[0] = '\0';
        }
    }

    relationship->last_interaction = time(NULL);
    relationship->total_interactions++;

    LOG_INFO("Relationship event: %s with %s",
             relationship_event_type_to_string(type),
             relationship->npc_id);
}

void relationship_update_score(Relationship* relationship) {
    if (!relationship) return;

    /* Calculate weighted score: trust (50%), respect (30%), fear (-20%) */
    double score = (relationship->trust * 0.5) +
                   (relationship->respect * 0.3) -
                   (relationship->fear * 0.2);

    /* Convert from 0-100 scale to -100 to 100 scale */
    /* At neutral (50, 50, 0): score = 25 + 15 = 40 */
    /* Adjust to make 40 map to 0 (neutral) */
    score = (score - 40) * 2.5;

    relationship->overall_score = clamp((int)score, -100, 100);
    relationship->status = relationship_get_status_from_score(relationship->overall_score);
}

RelationshipStatus relationship_get_status_from_score(int score) {
    if (score <= RELATIONSHIP_HOSTILE_THRESHOLD) {
        return RELATIONSHIP_STATUS_HOSTILE;
    } else if (score <= RELATIONSHIP_UNFRIENDLY_THRESHOLD) {
        return RELATIONSHIP_STATUS_UNFRIENDLY;
    } else if (score <= RELATIONSHIP_NEUTRAL_THRESHOLD) {
        return RELATIONSHIP_STATUS_NEUTRAL;
    } else if (score <= RELATIONSHIP_FRIENDLY_THRESHOLD) {
        return RELATIONSHIP_STATUS_FRIENDLY;
    } else {
        return RELATIONSHIP_STATUS_ALLIED;
    }
}

const char* relationship_status_to_string(RelationshipStatus status) {
    switch (status) {
        case RELATIONSHIP_STATUS_HOSTILE:
            return "Hostile";
        case RELATIONSHIP_STATUS_UNFRIENDLY:
            return "Unfriendly";
        case RELATIONSHIP_STATUS_NEUTRAL:
            return "Neutral";
        case RELATIONSHIP_STATUS_FRIENDLY:
            return "Friendly";
        case RELATIONSHIP_STATUS_ALLIED:
            return "Allied";
        default:
            return "Unknown";
    }
}

const char* relationship_event_type_to_string(RelationshipEventType type) {
    switch (type) {
        case RELATIONSHIP_EVENT_QUEST_COMPLETED:
            return "Quest Completed";
        case RELATIONSHIP_EVENT_QUEST_FAILED:
            return "Quest Failed";
        case RELATIONSHIP_EVENT_GIFT_GIVEN:
            return "Gift Given";
        case RELATIONSHIP_EVENT_DIALOGUE_CHOICE:
            return "Dialogue Choice";
        case RELATIONSHIP_EVENT_BETRAYAL:
            return "Betrayal";
        case RELATIONSHIP_EVENT_HELP_PROVIDED:
            return "Help Provided";
        case RELATIONSHIP_EVENT_HARM_CAUSED:
            return "Harm Caused";
        case RELATIONSHIP_EVENT_SECRET_REVEALED:
            return "Secret Revealed";
        case RELATIONSHIP_EVENT_PROMISE_KEPT:
            return "Promise Kept";
        case RELATIONSHIP_EVENT_PROMISE_BROKEN:
            return "Promise Broken";
        default:
            return "Unknown Event";
    }
}

bool relationship_meets_status(const Relationship* relationship, RelationshipStatus required_status) {
    if (!relationship) return false;

    return relationship->status >= required_status;
}

const RelationshipEvent* relationship_get_latest_event(const Relationship* relationship) {
    if (!relationship || relationship->event_count == 0) {
        return NULL;
    }

    return &relationship->events[relationship->event_count - 1];
}

void relationship_lock(Relationship* relationship) {
    if (!relationship) return;
    relationship->is_locked = true;
    LOG_DEBUG("Relationship with %s locked", relationship->npc_id);
}

void relationship_unlock(Relationship* relationship) {
    if (!relationship) return;
    relationship->is_locked = false;
    LOG_DEBUG("Relationship with %s unlocked", relationship->npc_id);
}

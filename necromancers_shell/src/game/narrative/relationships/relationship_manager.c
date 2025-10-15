/**
 * @file relationship_manager.c
 * @brief Relationship manager implementation
 */

#include "relationship_manager.h"
#include "../../../utils/logger.h"
#include <stdlib.h>
#include <string.h>

RelationshipManager* relationship_manager_create(void) {
    RelationshipManager* manager = malloc(sizeof(RelationshipManager));
    if (!manager) {
        LOG_ERROR("relationship_manager_create: malloc failed");
        return NULL;
    }

    manager->relationship_capacity = 16;
    manager->relationship_count = 0;
    manager->relationships = calloc(manager->relationship_capacity, sizeof(Relationship*));

    if (!manager->relationships) {
        LOG_ERROR("relationship_manager_create: calloc failed");
        free(manager);
        return NULL;
    }

    LOG_DEBUG("Relationship manager created");
    return manager;
}

void relationship_manager_destroy(RelationshipManager* manager) {
    if (!manager) return;

    /* Destroy all relationships */
    for (size_t i = 0; i < manager->relationship_count; i++) {
        relationship_destroy(manager->relationships[i]);
    }

    free(manager->relationships);
    free(manager);

    LOG_DEBUG("Relationship manager destroyed");
}

void relationship_manager_add_relationship(RelationshipManager* manager, Relationship* relationship) {
    if (!manager || !relationship) return;

    /* Resize if needed */
    if (manager->relationship_count >= manager->relationship_capacity) {
        size_t new_capacity = manager->relationship_capacity * 2;
        Relationship** new_relationships = realloc(manager->relationships,
                                                     new_capacity * sizeof(Relationship*));
        if (!new_relationships) {
            LOG_ERROR("relationship_manager_add_relationship: realloc failed");
            return;
        }

        manager->relationships = new_relationships;
        manager->relationship_capacity = new_capacity;

        /* Zero new memory */
        for (size_t i = manager->relationship_count; i < new_capacity; i++) {
            manager->relationships[i] = NULL;
        }
    }

    manager->relationships[manager->relationship_count++] = relationship;
    LOG_DEBUG("Added relationship with: %s", relationship->npc_id);
}

Relationship* relationship_manager_get_or_create(RelationshipManager* manager, const char* npc_id) {
    if (!manager || !npc_id) return NULL;

    /* Check if relationship exists */
    Relationship* relationship = relationship_manager_get(manager, npc_id);
    if (relationship) {
        return relationship;
    }

    /* Create new relationship */
    relationship = relationship_create(npc_id);
    if (!relationship) {
        return NULL;
    }

    relationship_manager_add_relationship(manager, relationship);
    return relationship;
}

Relationship* relationship_manager_get(const RelationshipManager* manager, const char* npc_id) {
    if (!manager || !npc_id) return NULL;

    for (size_t i = 0; i < manager->relationship_count; i++) {
        if (strcmp(manager->relationships[i]->npc_id, npc_id) == 0) {
            return manager->relationships[i];
        }
    }

    return NULL;
}

Relationship** relationship_manager_get_all(const RelationshipManager* manager, size_t* count_out) {
    if (!manager || !count_out) {
        if (count_out) *count_out = 0;
        return NULL;
    }

    if (manager->relationship_count == 0) {
        *count_out = 0;
        return NULL;
    }

    /* Allocate result array */
    Relationship** result = malloc(manager->relationship_count * sizeof(Relationship*));
    if (!result) {
        LOG_ERROR("relationship_manager_get_all: malloc failed");
        *count_out = 0;
        return NULL;
    }

    /* Copy all relationships */
    for (size_t i = 0; i < manager->relationship_count; i++) {
        result[i] = manager->relationships[i];
    }

    *count_out = manager->relationship_count;
    return result;
}

Relationship** relationship_manager_get_by_status(const RelationshipManager* manager,
                                                   RelationshipStatus status,
                                                   size_t* count_out) {
    if (!manager || !count_out) {
        if (count_out) *count_out = 0;
        return NULL;
    }

    /* Count matching relationships */
    size_t count = 0;
    for (size_t i = 0; i < manager->relationship_count; i++) {
        if (manager->relationships[i]->status == status) {
            count++;
        }
    }

    if (count == 0) {
        *count_out = 0;
        return NULL;
    }

    /* Allocate result array */
    Relationship** result = malloc(count * sizeof(Relationship*));
    if (!result) {
        LOG_ERROR("relationship_manager_get_by_status: malloc failed");
        *count_out = 0;
        return NULL;
    }

    /* Fill array */
    size_t idx = 0;
    for (size_t i = 0; i < manager->relationship_count; i++) {
        if (manager->relationships[i]->status == status) {
            result[idx++] = manager->relationships[i];
        }
    }

    *count_out = count;
    return result;
}

Relationship** relationship_manager_get_meeting_status(const RelationshipManager* manager,
                                                         RelationshipStatus min_status,
                                                         size_t* count_out) {
    if (!manager || !count_out) {
        if (count_out) *count_out = 0;
        return NULL;
    }

    /* Count matching relationships */
    size_t count = 0;
    for (size_t i = 0; i < manager->relationship_count; i++) {
        if (relationship_meets_status(manager->relationships[i], min_status)) {
            count++;
        }
    }

    if (count == 0) {
        *count_out = 0;
        return NULL;
    }

    /* Allocate result array */
    Relationship** result = malloc(count * sizeof(Relationship*));
    if (!result) {
        LOG_ERROR("relationship_manager_get_meeting_status: malloc failed");
        *count_out = 0;
        return NULL;
    }

    /* Fill array */
    size_t idx = 0;
    for (size_t i = 0; i < manager->relationship_count; i++) {
        if (relationship_meets_status(manager->relationships[i], min_status)) {
            result[idx++] = manager->relationships[i];
        }
    }

    *count_out = count;
    return result;
}

Relationship** relationship_manager_get_high_trust(const RelationshipManager* manager, size_t* count_out) {
    if (!manager || !count_out) {
        if (count_out) *count_out = 0;
        return NULL;
    }

    /* Count high trust relationships */
    size_t count = 0;
    for (size_t i = 0; i < manager->relationship_count; i++) {
        if (manager->relationships[i]->trust >= 75) {
            count++;
        }
    }

    if (count == 0) {
        *count_out = 0;
        return NULL;
    }

    /* Allocate result array */
    Relationship** result = malloc(count * sizeof(Relationship*));
    if (!result) {
        LOG_ERROR("relationship_manager_get_high_trust: malloc failed");
        *count_out = 0;
        return NULL;
    }

    /* Fill array */
    size_t idx = 0;
    for (size_t i = 0; i < manager->relationship_count; i++) {
        if (manager->relationships[i]->trust >= 75) {
            result[idx++] = manager->relationships[i];
        }
    }

    *count_out = count;
    return result;
}

Relationship** relationship_manager_get_high_fear(const RelationshipManager* manager, size_t* count_out) {
    if (!manager || !count_out) {
        if (count_out) *count_out = 0;
        return NULL;
    }

    /* Count high fear relationships */
    size_t count = 0;
    for (size_t i = 0; i < manager->relationship_count; i++) {
        if (manager->relationships[i]->fear >= 75) {
            count++;
        }
    }

    if (count == 0) {
        *count_out = 0;
        return NULL;
    }

    /* Allocate result array */
    Relationship** result = malloc(count * sizeof(Relationship*));
    if (!result) {
        LOG_ERROR("relationship_manager_get_high_fear: malloc failed");
        *count_out = 0;
        return NULL;
    }

    /* Fill array */
    size_t idx = 0;
    for (size_t i = 0; i < manager->relationship_count; i++) {
        if (manager->relationships[i]->fear >= 75) {
            result[idx++] = manager->relationships[i];
        }
    }

    *count_out = count;
    return result;
}

Relationship** relationship_manager_get_romances(const RelationshipManager* manager, size_t* count_out) {
    if (!manager || !count_out) {
        if (count_out) *count_out = 0;
        return NULL;
    }

    /* Count active romances */
    size_t count = 0;
    for (size_t i = 0; i < manager->relationship_count; i++) {
        if (manager->relationships[i]->is_romance_active) {
            count++;
        }
    }

    if (count == 0) {
        *count_out = 0;
        return NULL;
    }

    /* Allocate result array */
    Relationship** result = malloc(count * sizeof(Relationship*));
    if (!result) {
        LOG_ERROR("relationship_manager_get_romances: malloc failed");
        *count_out = 0;
        return NULL;
    }

    /* Fill array */
    size_t idx = 0;
    for (size_t i = 0; i < manager->relationship_count; i++) {
        if (manager->relationships[i]->is_romance_active) {
            result[idx++] = manager->relationships[i];
        }
    }

    *count_out = count;
    return result;
}

Relationship** relationship_manager_get_rivals(const RelationshipManager* manager, size_t* count_out) {
    if (!manager || !count_out) {
        if (count_out) *count_out = 0;
        return NULL;
    }

    /* Count rivals */
    size_t count = 0;
    for (size_t i = 0; i < manager->relationship_count; i++) {
        if (manager->relationships[i]->is_rival) {
            count++;
        }
    }

    if (count == 0) {
        *count_out = 0;
        return NULL;
    }

    /* Allocate result array */
    Relationship** result = malloc(count * sizeof(Relationship*));
    if (!result) {
        LOG_ERROR("relationship_manager_get_rivals: malloc failed");
        *count_out = 0;
        return NULL;
    }

    /* Fill array */
    size_t idx = 0;
    for (size_t i = 0; i < manager->relationship_count; i++) {
        if (manager->relationships[i]->is_rival) {
            result[idx++] = manager->relationships[i];
        }
    }

    *count_out = count;
    return result;
}

void relationship_manager_modify_trust(RelationshipManager* manager, const char* npc_id, int delta) {
    if (!manager || !npc_id) return;

    Relationship* relationship = relationship_manager_get_or_create(manager, npc_id);
    if (relationship) {
        relationship_modify_trust(relationship, delta);
    }
}

void relationship_manager_modify_respect(RelationshipManager* manager, const char* npc_id, int delta) {
    if (!manager || !npc_id) return;

    Relationship* relationship = relationship_manager_get_or_create(manager, npc_id);
    if (relationship) {
        relationship_modify_respect(relationship, delta);
    }
}

void relationship_manager_modify_fear(RelationshipManager* manager, const char* npc_id, int delta) {
    if (!manager || !npc_id) return;

    Relationship* relationship = relationship_manager_get_or_create(manager, npc_id);
    if (relationship) {
        relationship_modify_fear(relationship, delta);
    }
}

void relationship_manager_add_event(RelationshipManager* manager,
                                   const char* npc_id,
                                   RelationshipEventType type,
                                   int trust_delta,
                                   int respect_delta,
                                   int fear_delta,
                                   const char* description) {
    if (!manager || !npc_id) return;

    Relationship* relationship = relationship_manager_get_or_create(manager, npc_id);
    if (relationship) {
        relationship_add_event(relationship, type, trust_delta, respect_delta, fear_delta, description);
    }
}

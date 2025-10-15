#include "alliance_manager.h"
#include <stdlib.h>
#include <string.h>

AllianceManager* alliance_manager_create(void) {
    AllianceManager* manager = malloc(sizeof(AllianceManager));
    if (!manager) {
        return NULL;
    }

    /* Initialize fields */
    for (size_t i = 0; i < MAX_ALLIANCES; i++) {
        manager->alliances[i] = NULL;
    }
    manager->alliance_count = 0;

    return manager;
}

void alliance_manager_destroy(AllianceManager* manager) {
    if (!manager) {
        return;
    }

    /* Destroy all alliances */
    for (size_t i = 0; i < manager->alliance_count; i++) {
        alliance_destroy(manager->alliances[i]);
    }

    free(manager);
}

bool alliance_manager_add(AllianceManager* manager, const char* npc_id, AllianceType type) {
    if (!manager || !npc_id) {
        return false;
    }

    /* Check if alliance already exists */
    if (alliance_manager_find(manager, npc_id)) {
        return false;
    }

    /* Check if manager is full */
    if (manager->alliance_count >= MAX_ALLIANCES) {
        return false;
    }

    /* Create new alliance */
    Alliance* alliance = alliance_create(npc_id, type);
    if (!alliance) {
        return false;
    }

    /* Add to manager */
    manager->alliances[manager->alliance_count] = alliance;
    manager->alliance_count++;

    return true;
}

bool alliance_manager_remove(AllianceManager* manager, const char* npc_id) {
    if (!manager || !npc_id) {
        return false;
    }

    /* Find alliance index */
    for (size_t i = 0; i < manager->alliance_count; i++) {
        if (strcmp(manager->alliances[i]->npc_id, npc_id) == 0) {
            /* Found - destroy and remove */
            alliance_destroy(manager->alliances[i]);

            /* Shift remaining alliances down */
            for (size_t j = i; j < manager->alliance_count - 1; j++) {
                manager->alliances[j] = manager->alliances[j + 1];
            }

            manager->alliances[manager->alliance_count - 1] = NULL;
            manager->alliance_count--;

            return true;
        }
    }

    return false; /* Not found */
}

Alliance* alliance_manager_find(const AllianceManager* manager, const char* npc_id) {
    if (!manager || !npc_id) {
        return NULL;
    }

    /* Linear search for alliance */
    for (size_t i = 0; i < manager->alliance_count; i++) {
        if (strcmp(manager->alliances[i]->npc_id, npc_id) == 0) {
            return manager->alliances[i];
        }
    }

    return NULL;
}

Alliance* alliance_manager_get(const AllianceManager* manager, size_t index) {
    if (!manager || index >= manager->alliance_count) {
        return NULL;
    }

    return manager->alliances[index];
}

size_t alliance_manager_count_by_type(const AllianceManager* manager, AllianceType type) {
    if (!manager) {
        return 0;
    }

    size_t count = 0;
    for (size_t i = 0; i < manager->alliance_count; i++) {
        if (manager->alliances[i]->type == type) {
            count++;
        }
    }

    return count;
}

size_t alliance_manager_coordination_allies(const AllianceManager* manager) {
    if (!manager) {
        return 0;
    }

    size_t count = 0;
    for (size_t i = 0; i < manager->alliance_count; i++) {
        if (alliance_supports_coordination(manager->alliances[i]->type)) {
            count++;
        }
    }

    return count;
}

size_t alliance_manager_hostile_count(const AllianceManager* manager) {
    return alliance_manager_count_by_type(manager, ALLIANCE_HOSTILE);
}

float alliance_manager_average_trust(const AllianceManager* manager) {
    if (!manager || manager->alliance_count == 0) {
        return 0.0f;
    }

    float total_trust = 0.0f;
    for (size_t i = 0; i < manager->alliance_count; i++) {
        total_trust += manager->alliances[i]->trust_level;
    }

    return total_trust / (float)manager->alliance_count;
}

void alliance_manager_advance_time(AllianceManager* manager, uint32_t days) {
    if (!manager) {
        return;
    }

    for (size_t i = 0; i < manager->alliance_count; i++) {
        alliance_advance_time(manager->alliances[i], days);
    }
}

bool alliance_manager_has_coordination(const AllianceManager* manager) {
    return alliance_manager_coordination_allies(manager) > 0;
}

AllianceType alliance_manager_strongest_alliance(const AllianceManager* manager) {
    if (!manager || manager->alliance_count == 0) {
        return ALLIANCE_HOSTILE;
    }

    AllianceType strongest = ALLIANCE_HOSTILE;
    for (size_t i = 0; i < manager->alliance_count; i++) {
        if (manager->alliances[i]->type > strongest) {
            strongest = manager->alliances[i]->type;
        }
    }

    return strongest;
}

size_t alliance_manager_oath_count(const AllianceManager* manager) {
    if (!manager) {
        return 0;
    }

    size_t count = 0;
    for (size_t i = 0; i < manager->alliance_count; i++) {
        if (manager->alliances[i]->phylactery_oath) {
            count++;
        }
    }

    return count;
}

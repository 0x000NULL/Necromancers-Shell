/**
 * @file territory_status.c
 * @brief Implementation of territory control and alert system
 */

#include "territory_status.h"
#include "../../utils/logger.h"
#include "../../utils/hash_table.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define ALERT_DECAY_TIME_HOURS 4    /**< Hours between alert level decays */
#define REINFORCEMENT_THRESHOLD 75   /**< Alert threshold for reinforcements */

/**
 * @brief Territory status manager structure
 */
struct TerritoryStatusManager {
    HashTable* statuses;  /**< Key: location_id, Value: TerritoryStatus* */
};

/* Helper functions */

static TerritoryStatus* get_or_create_status(TerritoryStatusManager* manager, uint32_t location_id) {
    char key[32];
    snprintf(key, sizeof(key), "%u", location_id);

    TerritoryStatus* status = hash_table_get(manager->statuses, key);
    if (status) {
        return status;
    }

    /* Create new status */
    status = malloc(sizeof(TerritoryStatus));
    if (!status) {
        LOG_ERROR("Failed to allocate TerritoryStatus");
        return NULL;
    }

    status->location_id = location_id;
    status->control_percentage = 0;
    status->dominant_faction = FACTION_NEUTRAL;
    status->alert_level = ALERT_NONE;
    status->stability = STABILITY_CHAOTIC;
    status->defense_strength = 100;
    status->last_activity_time = 0;
    status->alert_decay_time = 0;
    status->resource_modifier = 1.0f;
    status->under_siege = false;
    status->reinforcements_called = false;
    status->garrison_strength = 50;

    hash_table_put(manager->statuses, key, status);
    LOG_DEBUG("Created territory status for location %u", location_id);
    return status;
}

static void update_stability(TerritoryStatus* status) {
    status->stability = territory_status_calculate_stability(status->control_percentage);
}

static void update_dominant_faction(TerritoryStatus* status) {
    if (status->control_percentage >= 50) {
        status->dominant_faction = FACTION_PLAYER;
    } else if (status->control_percentage >= 25) {
        status->dominant_faction = FACTION_NEUTRAL;
    } else {
        status->dominant_faction = FACTION_LIVING;
    }
}

static void update_resource_modifier(TerritoryStatus* status) {
    /* Base modifier from control percentage */
    float base = 0.5f + (status->control_percentage / 100.0f) * 1.5f;

    /* Penalty from alert level */
    float alert_penalty = 1.0f;
    switch (status->alert_level) {
        case ALERT_NONE:     alert_penalty = 1.0f; break;
        case ALERT_LOW:      alert_penalty = 1.0f; break;
        case ALERT_MEDIUM:   alert_penalty = 0.9f; break;
        case ALERT_HIGH:     alert_penalty = 0.75f; break;
        case ALERT_CRITICAL: alert_penalty = 0.5f; break;
        default: alert_penalty = 1.0f; break;
    }

    /* Bonus from stability */
    float stability_bonus = 1.0f;
    switch (status->stability) {
        case STABILITY_CHAOTIC:    stability_bonus = 0.8f; break;
        case STABILITY_CONTESTED:  stability_bonus = 0.9f; break;
        case STABILITY_CONTROLLED: stability_bonus = 1.0f; break;
        case STABILITY_DOMINATED:  stability_bonus = 1.2f; break;
        default: stability_bonus = 1.0f; break;
    }

    status->resource_modifier = base * alert_penalty * stability_bonus;

    /* Clamp to reasonable range */
    if (status->resource_modifier < 0.5f) status->resource_modifier = 0.5f;
    if (status->resource_modifier > 2.0f) status->resource_modifier = 2.0f;
}

/* Callback for hash_table_foreach */

static void free_status_callback(const char* key, void* value, void* userdata) {
    (void)key;
    (void)userdata;
    free(value);
}

typedef struct {
    uint32_t* results;
    size_t count;
    size_t max_results;
    AlertLevel filter_alert;
} AlertFilterContext;

static void filter_by_alert_callback(const char* key, void* value, void* userdata) {
    (void)key;
    AlertFilterContext* ctx = (AlertFilterContext*)userdata;
    TerritoryStatus* status = (TerritoryStatus*)value;

    if (status && status->alert_level == ctx->filter_alert && ctx->count < ctx->max_results) {
        ctx->results[ctx->count++] = status->location_id;
    }
}

typedef struct {
    uint32_t* results;
    size_t count;
    size_t max_results;
    uint64_t current_time;
} ControlFilterContext;

static void filter_controlled_callback(const char* key, void* value, void* userdata) {
    (void)key;
    ControlFilterContext* ctx = (ControlFilterContext*)userdata;
    TerritoryStatus* status = (TerritoryStatus*)value;

    if (status && status->control_percentage > 50 && ctx->count < ctx->max_results) {
        ctx->results[ctx->count++] = status->location_id;
    }
}

typedef struct {
    uint64_t current_time;
} UpdateContext;

static void update_status_callback(const char* key, void* value, void* userdata) {
    (void)key;
    UpdateContext* ctx = (UpdateContext*)userdata;
    TerritoryStatus* status = (TerritoryStatus*)value;

    if (!status) return;

    /* Decay alert if enough time has passed */
    if (status->alert_level > ALERT_NONE &&
        ctx->current_time >= status->alert_decay_time) {

        status->alert_level--;
        status->alert_decay_time = ctx->current_time + (ALERT_DECAY_TIME_HOURS * 3600);

        LOG_DEBUG("Alert decayed to %s for location %u",
                  territory_status_alert_name(status->alert_level),
                  status->location_id);
    }

    /* Update modifiers */
    update_stability(status);
    update_dominant_faction(status);
    update_resource_modifier(status);

    /* Check if reinforcements arrive */
    if (status->reinforcements_called) {
        uint64_t time_since_call = ctx->current_time - status->last_activity_time;
        if (time_since_call >= 7200) {  /* 2 hours */
            status->garrison_strength += 50;
            status->reinforcements_called = false;
            LOG_INFO("Reinforcements arrived at location %u", status->location_id);
        }
    }
}

/* Public API */

TerritoryStatusManager* territory_status_create(void) {
    TerritoryStatusManager* manager = malloc(sizeof(TerritoryStatusManager));
    if (!manager) {
        LOG_ERROR("Failed to allocate TerritoryStatusManager");
        return NULL;
    }

    manager->statuses = hash_table_create(100);
    if (!manager->statuses) {
        free(manager);
        LOG_ERROR("Failed to create hash table for territory statuses");
        return NULL;
    }

    LOG_DEBUG("Created territory status manager");
    return manager;
}

void territory_status_destroy(TerritoryStatusManager* manager) {
    if (!manager) return;

    if (manager->statuses) {
        hash_table_foreach(manager->statuses, free_status_callback, NULL);
        hash_table_destroy(manager->statuses);
    }

    free(manager);
    LOG_DEBUG("Destroyed territory status manager");
}

TerritoryStatus* territory_status_get(TerritoryStatusManager* manager, uint32_t location_id) {
    if (!manager) return NULL;
    return get_or_create_status(manager, location_id);
}

bool territory_status_set_control(TerritoryStatusManager* manager,
                                   uint32_t location_id,
                                   uint8_t control_percentage) {
    if (!manager || control_percentage > 100) return false;

    TerritoryStatus* status = get_or_create_status(manager, location_id);
    if (!status) return false;

    status->control_percentage = control_percentage;
    update_stability(status);
    update_dominant_faction(status);
    update_resource_modifier(status);

    LOG_DEBUG("Set control for location %u to %u%%", location_id, control_percentage);
    return true;
}

AlertLevel territory_status_raise_alert(TerritoryStatusManager* manager,
                                         uint32_t location_id,
                                         uint8_t amount,
                                         uint64_t current_time) {
    if (!manager) return ALERT_NONE;

    TerritoryStatus* status = get_or_create_status(manager, location_id);
    if (!status) return ALERT_NONE;

    /* Increase alert level */
    int new_level = status->alert_level + amount;
    if (new_level >= ALERT_LEVEL_COUNT) {
        new_level = ALERT_LEVEL_COUNT - 1;
    }

    status->alert_level = (AlertLevel)new_level;
    status->last_activity_time = current_time;
    status->alert_decay_time = current_time + (ALERT_DECAY_TIME_HOURS * 3600);

    /* Call reinforcements at high alert */
    if (status->alert_level >= ALERT_HIGH && !status->reinforcements_called) {
        status->reinforcements_called = true;
        LOG_WARN("Reinforcements called to location %u due to high alert", location_id);
    }

    update_resource_modifier(status);

    LOG_DEBUG("Alert raised to %s for location %u",
              territory_status_alert_name(status->alert_level),
              location_id);

    return status->alert_level;
}

AlertLevel territory_status_decay_alert(TerritoryStatusManager* manager,
                                         uint32_t location_id,
                                         uint64_t current_time) {
    if (!manager) return ALERT_NONE;

    TerritoryStatus* status = get_or_create_status(manager, location_id);
    if (!status) return ALERT_NONE;

    if (status->alert_level > ALERT_NONE &&
        current_time >= status->alert_decay_time) {

        status->alert_level--;
        status->alert_decay_time = current_time + (ALERT_DECAY_TIME_HOURS * 3600);
        update_resource_modifier(status);

        LOG_DEBUG("Alert decayed to %s for location %u",
                  territory_status_alert_name(status->alert_level),
                  location_id);
    }

    return status->alert_level;
}

void territory_status_update_all(TerritoryStatusManager* manager, uint64_t current_time) {
    if (!manager) return;

    UpdateContext ctx = { .current_time = current_time };
    hash_table_foreach(manager->statuses, update_status_callback, &ctx);
}

float territory_status_resource_modifier(const TerritoryStatus* status) {
    if (!status) return 1.0f;
    return status->resource_modifier;
}

float territory_status_detection_modifier(const TerritoryStatus* status) {
    if (!status) return 1.0f;

    switch (status->alert_level) {
        case ALERT_NONE:     return 1.0f;
        case ALERT_LOW:      return 1.1f;
        case ALERT_MEDIUM:   return 1.25f;
        case ALERT_HIGH:     return 1.5f;
        case ALERT_CRITICAL: return 2.0f;
        default:             return 1.0f;
    }
}

bool territory_status_needs_reinforcements(const TerritoryStatus* status) {
    if (!status) return false;
    return status->alert_level >= ALERT_HIGH && !status->reinforcements_called;
}

const char* territory_status_alert_name(AlertLevel level) {
    switch (level) {
        case ALERT_NONE:     return "None";
        case ALERT_LOW:      return "Low";
        case ALERT_MEDIUM:   return "Medium";
        case ALERT_HIGH:     return "High";
        case ALERT_CRITICAL: return "Critical";
        default:             return "Unknown";
    }
}

const char* territory_status_faction_name(FactionType faction) {
    switch (faction) {
        case FACTION_PLAYER:   return "Player";
        case FACTION_LIVING:   return "Living";
        case FACTION_HUNTERS:  return "Hunters";
        case FACTION_RIVAL:    return "Rival Necromancer";
        case FACTION_NEUTRAL:  return "Neutral";
        default:               return "Unknown";
    }
}

const char* territory_status_stability_name(StabilityLevel stability) {
    switch (stability) {
        case STABILITY_CHAOTIC:    return "Chaotic";
        case STABILITY_CONTESTED:  return "Contested";
        case STABILITY_CONTROLLED: return "Controlled";
        case STABILITY_DOMINATED:  return "Dominated";
        default:                   return "Unknown";
    }
}

StabilityLevel territory_status_calculate_stability(uint8_t control_percentage) {
    if (control_percentage < 25) {
        return STABILITY_CHAOTIC;
    } else if (control_percentage < 50) {
        return STABILITY_CONTESTED;
    } else if (control_percentage < 75) {
        return STABILITY_CONTROLLED;
    } else {
        return STABILITY_DOMINATED;
    }
}

size_t territory_status_get_by_alert(const TerritoryStatusManager* manager,
                                      AlertLevel alert_level,
                                      uint32_t* results,
                                      size_t max_results) {
    if (!manager || !results) return 0;

    AlertFilterContext ctx = {
        .results = results,
        .count = 0,
        .max_results = max_results,
        .filter_alert = alert_level
    };

    hash_table_foreach(manager->statuses, filter_by_alert_callback, &ctx);
    return ctx.count;
}

size_t territory_status_get_controlled(const TerritoryStatusManager* manager,
                                        uint32_t* results,
                                        size_t max_results) {
    if (!manager || !results) return 0;

    ControlFilterContext ctx = {
        .results = results,
        .count = 0,
        .max_results = max_results,
        .current_time = 0
    };

    hash_table_foreach(manager->statuses, filter_controlled_callback, &ctx);
    return ctx.count;
}

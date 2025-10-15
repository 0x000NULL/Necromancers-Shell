/**
 * @file death_network.c
 * @brief Implementation of Death Network system
 */

#include "death_network.h"
#include "../../utils/logger.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Maximum nodes in network */
#define MAX_DEATH_NODES 256

/* Death signature thresholds */
#define SIGNATURE_DORMANT 20
#define SIGNATURE_WEAK 40
#define SIGNATURE_MODERATE 60
#define SIGNATURE_STRONG 80

/* Signature decay rate per hour (reduces by 1 every N hours) */
#define SIGNATURE_DECAY_HOURS 24

/**
 * @brief Death Network structure
 */
struct DeathNetwork {
    DeathNode nodes[MAX_DEATH_NODES];  /**< Array of death nodes */
    size_t node_count;                  /**< Number of active nodes */
    uint32_t current_time_hours;        /**< Current game time in hours */
    uint32_t total_deaths_tracked;      /**< Total deaths across network */
};

/* ========================================================================
 * Creation and Destruction
 * ======================================================================== */

DeathNetwork* death_network_create(void) {
    DeathNetwork* network = calloc(1, sizeof(DeathNetwork));
    if (!network) {
        LOG_ERROR("Failed to allocate death network");
        return NULL;
    }

    /* Initialize with current time seed for random events */
    srand((unsigned int)time(NULL));

    LOG_DEBUG("Death network created");
    return network;
}

void death_network_destroy(DeathNetwork* network) {
    if (!network) return;

    LOG_DEBUG("Destroying death network (tracked %u deaths)",
              network->total_deaths_tracked);
    free(network);
}

/* ========================================================================
 * Node Management
 * ======================================================================== */

bool death_network_add_location(DeathNetwork* network,
                                 uint32_t location_id,
                                 DeathSignature base_signature,
                                 uint32_t max_corpses,
                                 uint8_t regen_rate) {
    if (!network) return false;

    if (network->node_count >= MAX_DEATH_NODES) {
        LOG_ERROR("Death network is full (max %d nodes)", MAX_DEATH_NODES);
        return false;
    }

    /* Check for duplicate */
    for (size_t i = 0; i < network->node_count; i++) {
        if (network->nodes[i].location_id == location_id) {
            LOG_WARN("Location %u already in death network", location_id);
            return false;
        }
    }

    /* Initialize new node */
    DeathNode* node = &network->nodes[network->node_count];
    memset(node, 0, sizeof(DeathNode));

    node->location_id = location_id;
    node->signature = base_signature;
    node->base_signature = base_signature;
    node->max_corpses = max_corpses;
    node->regen_rate = regen_rate;
    node->is_active = true;

    /* Start with 50% of max corpses */
    node->available_corpses = max_corpses / 2;

    /* Default quality distribution (adjust per location type) */
    node->quality_poor = 50;
    node->quality_average = 30;
    node->quality_good = 15;
    node->quality_excellent = 4;
    node->quality_legendary = 1;

    network->node_count++;

    LOG_DEBUG("Added location %u to death network (signature: %u, max corpses: %u)",
              location_id, base_signature, max_corpses);

    return true;
}

const DeathNode* death_network_get_node(const DeathNetwork* network,
                                         uint32_t location_id) {
    if (!network) return NULL;

    for (size_t i = 0; i < network->node_count; i++) {
        if (network->nodes[i].location_id == location_id) {
            return &network->nodes[i];
        }
    }

    return NULL;
}

/* ========================================================================
 * Network Updates
 * ======================================================================== */

/**
 * @brief Regenerate corpses for a node based on time passage
 */
static void regenerate_corpses(DeathNode* node, uint32_t hours) {
    if (!node->is_active) return;

    /* Calculate corpses to add based on regen rate and time */
    /* regen_rate = corpses per 24 hours */
    if (node->regen_rate == 0) return;

    uint32_t hours_per_corpse = 24 / node->regen_rate;
    uint32_t corpses_to_add = hours / hours_per_corpse;

    if (corpses_to_add > 0) {
        node->available_corpses += corpses_to_add;
        if (node->available_corpses > node->max_corpses) {
            node->available_corpses = node->max_corpses;
        }
        node->total_deaths += corpses_to_add;
    }
}

/**
 * @brief Decay death signature over time
 */
static void decay_signature(DeathNode* node, uint32_t hours) {
    /* Signature decays back toward base over time */
    uint32_t decay_amount = hours / SIGNATURE_DECAY_HOURS;

    if (node->signature > node->base_signature) {
        /* Decay down */
        uint32_t diff = (uint32_t)(node->signature - node->base_signature);
        if (decay_amount > diff) {
            node->signature = node->base_signature;
        } else {
            node->signature -= (DeathSignature)decay_amount;
        }
    } else if (node->signature < node->base_signature) {
        /* Grow back up */
        uint32_t diff = (uint32_t)(node->base_signature - node->signature);
        if (decay_amount > diff) {
            node->signature = node->base_signature;
        } else {
            node->signature += (DeathSignature)decay_amount;
        }
    }
}

/**
 * @brief Randomly trigger a death event (5% chance per day)
 */
static bool trigger_random_event(DeathNetwork* network, DeathNode* node, uint32_t hours) {
    /* 5% chance per 24 hours */
    if ((rand() % 100) < (int)(hours * 5 / 24)) {
        /* Random event type */
        DeathEventType event_type = (DeathEventType)(rand() % DEATH_EVENT_COUNT);

        /* Random death count based on event type */
        uint32_t death_count;
        switch (event_type) {
            case DEATH_EVENT_PLAGUE:
                death_count = 10 + (rand() % 20);  /* 10-30 deaths */
                break;
            case DEATH_EVENT_BATTLE:
                death_count = 5 + (rand() % 15);   /* 5-20 deaths */
                break;
            case DEATH_EVENT_NATURAL:
                death_count = 1 + (rand() % 3);    /* 1-3 deaths */
                break;
            default:
                death_count = 1 + (rand() % 5);    /* 1-5 deaths */
                break;
        }

        DeathEvent event = {
            .location_id = node->location_id,
            .type = event_type,
            .death_count = death_count,
            .avg_quality = (DeathQuality)(rand() % DEATH_QUALITY_LEGENDARY),
            .timestamp_hours = network->current_time_hours
        };

        death_network_trigger_event(network, &event);
        return true;
    }

    return false;
}

void death_network_update(DeathNetwork* network, uint32_t hours_passed) {
    if (!network || hours_passed == 0) return;

    network->current_time_hours += hours_passed;

    for (size_t i = 0; i < network->node_count; i++) {
        DeathNode* node = &network->nodes[i];

        /* Update time trackers */
        node->hours_since_harvest += hours_passed;
        node->hours_since_event += hours_passed;

        /* Regenerate corpses */
        regenerate_corpses(node, hours_passed);

        /* Decay signature */
        decay_signature(node, hours_passed);

        /* Random death events */
        trigger_random_event(network, node, hours_passed);
    }

    LOG_TRACE("Death network updated (+%u hours)", hours_passed);
}

/* ========================================================================
 * Death Events
 * ======================================================================== */

bool death_network_trigger_event(DeathNetwork* network, const DeathEvent* event) {
    if (!network || !event) return false;

    /* Find node */
    DeathNode* node = NULL;
    for (size_t i = 0; i < network->node_count; i++) {
        if (network->nodes[i].location_id == event->location_id) {
            node = &network->nodes[i];
            break;
        }
    }

    if (!node) {
        LOG_WARN("Death event for unknown location %u", event->location_id);
        return false;
    }

    /* Increase death signature based on event magnitude */
    uint32_t signature_increase = event->death_count / 2;
    if (signature_increase > 30) signature_increase = 30;  /* Cap at +30 */

    node->signature += (DeathSignature)signature_increase;
    if (node->signature > 100) node->signature = 100;

    /* Add corpses */
    node->available_corpses += event->death_count;
    if (node->available_corpses > node->max_corpses) {
        node->available_corpses = node->max_corpses;
    }

    /* Update tracking */
    node->total_deaths += event->death_count;
    node->last_event_type = event->type;
    node->hours_since_event = 0;

    network->total_deaths_tracked += event->death_count;

    LOG_INFO("Death event at location %u: %s (%u deaths, signature: %u)",
             event->location_id,
             death_event_type_name(event->type),
             event->death_count,
             node->signature);

    return true;
}

/* ========================================================================
 * Corpse Harvesting
 * ======================================================================== */

DeathQuality death_network_roll_quality(const DeathNode* node) {
    if (!node) return DEATH_QUALITY_POOR;

    /* Roll 1-100 */
    int roll = (rand() % 100) + 1;
    int threshold = 0;

    threshold += node->quality_poor;
    if (roll <= threshold) return DEATH_QUALITY_POOR;

    threshold += node->quality_average;
    if (roll <= threshold) return DEATH_QUALITY_AVERAGE;

    threshold += node->quality_good;
    if (roll <= threshold) return DEATH_QUALITY_GOOD;

    threshold += node->quality_excellent;
    if (roll <= threshold) return DEATH_QUALITY_EXCELLENT;

    return DEATH_QUALITY_LEGENDARY;
}

uint32_t death_network_harvest_corpses(DeathNetwork* network,
                                        uint32_t location_id,
                                        uint32_t count,
                                        DeathQuality* qualities) {
    if (!network) return 0;

    /* Find node */
    DeathNode* node = NULL;
    for (size_t i = 0; i < network->node_count; i++) {
        if (network->nodes[i].location_id == location_id) {
            node = &network->nodes[i];
            break;
        }
    }

    if (!node) {
        LOG_WARN("Attempted to harvest from unknown location %u", location_id);
        return 0;
    }

    /* Cap at available corpses */
    uint32_t harvested = count;
    if (harvested > node->available_corpses) {
        harvested = node->available_corpses;
    }

    /* Roll qualities */
    if (qualities) {
        for (uint32_t i = 0; i < harvested; i++) {
            qualities[i] = death_network_roll_quality(node);
        }
    }

    /* Remove corpses */
    node->available_corpses -= harvested;
    node->hours_since_harvest = 0;

    /* Reduce signature slightly from harvesting */
    if (node->signature > 5) {
        node->signature -= 5;
    } else {
        node->signature = 0;
    }

    LOG_DEBUG("Harvested %u corpses from location %u (%u remaining)",
              harvested, location_id, node->available_corpses);

    return harvested;
}

/* ========================================================================
 * Network Scanning
 * ======================================================================== */

/**
 * @brief Comparison function for sorting locations by signature
 */
static int compare_nodes_by_signature(const void* a, const void* b) {
    const DeathNode* node_a = (const DeathNode*)a;
    const DeathNode* node_b = (const DeathNode*)b;

    /* Sort descending (highest signature first) */
    if (node_b->signature > node_a->signature) return 1;
    if (node_b->signature < node_a->signature) return -1;
    return 0;
}

size_t death_network_scan(const DeathNetwork* network,
                          uint32_t center_location_id,
                          uint32_t* results,
                          size_t max_results) {
    if (!network || !results || max_results == 0) return 0;

    /* For now, return all locations except center */
    /* TODO: Implement range-based scanning using location_graph */

    size_t count = 0;
    DeathNode sorted[MAX_DEATH_NODES];
    size_t sorted_count = 0;

    /* Copy all nodes except center */
    for (size_t i = 0; i < network->node_count; i++) {
        if (network->nodes[i].location_id != center_location_id) {
            sorted[sorted_count++] = network->nodes[i];
        }
    }

    /* Sort by signature */
    qsort(sorted, sorted_count, sizeof(DeathNode), compare_nodes_by_signature);

    /* Return top results */
    for (size_t i = 0; i < sorted_count && count < max_results; i++) {
        results[count++] = sorted[i].location_id;
    }

    return count;
}

size_t death_network_get_hotspots(const DeathNetwork* network,
                                   uint32_t* results,
                                   size_t max_results) {
    if (!network || !results || max_results == 0) return 0;

    /* Copy all nodes */
    DeathNode sorted[MAX_DEATH_NODES];
    memcpy(sorted, network->nodes, network->node_count * sizeof(DeathNode));

    /* Sort by signature */
    qsort(sorted, network->node_count, sizeof(DeathNode), compare_nodes_by_signature);

    /* Return top results with signature > 50 */
    size_t count = 0;
    for (size_t i = 0; i < network->node_count && count < max_results; i++) {
        if (sorted[i].signature >= SIGNATURE_MODERATE) {
            results[count++] = sorted[i].location_id;
        }
    }

    return count;
}

/* ========================================================================
 * Quality and Statistics
 * ======================================================================== */

uint32_t death_quality_to_soul_energy(DeathQuality quality) {
    switch (quality) {
        case DEATH_QUALITY_POOR:      return 10;
        case DEATH_QUALITY_AVERAGE:   return 20;
        case DEATH_QUALITY_GOOD:      return 35;
        case DEATH_QUALITY_EXCELLENT: return 50;
        case DEATH_QUALITY_LEGENDARY: return 100;
        default:                      return 10;
    }
}

bool death_network_set_quality_distribution(DeathNetwork* network,
                                             uint32_t location_id,
                                             uint8_t poor,
                                             uint8_t average,
                                             uint8_t good,
                                             uint8_t excellent,
                                             uint8_t legendary) {
    if (!network) return false;

    /* Validate percentages sum to 100 */
    if (poor + average + good + excellent + legendary != 100) {
        LOG_ERROR("Quality distribution must sum to 100 (got %u)",
                  poor + average + good + excellent + legendary);
        return false;
    }

    /* Find node */
    for (size_t i = 0; i < network->node_count; i++) {
        if (network->nodes[i].location_id == location_id) {
            network->nodes[i].quality_poor = poor;
            network->nodes[i].quality_average = average;
            network->nodes[i].quality_good = good;
            network->nodes[i].quality_excellent = excellent;
            network->nodes[i].quality_legendary = legendary;
            return true;
        }
    }

    LOG_WARN("Cannot set quality distribution for unknown location %u", location_id);
    return false;
}

DeathSignature death_network_get_activity_level(const DeathNetwork* network) {
    if (!network || network->node_count == 0) return 0;

    uint32_t total = 0;
    for (size_t i = 0; i < network->node_count; i++) {
        total += network->nodes[i].signature;
    }

    return (DeathSignature)(total / network->node_count);
}

uint32_t death_network_get_total_corpses(const DeathNetwork* network) {
    if (!network) return 0;

    uint32_t total = 0;
    for (size_t i = 0; i < network->node_count; i++) {
        total += network->nodes[i].available_corpses;
    }

    return total;
}

void death_network_get_stats(const DeathNetwork* network,
                             size_t* total_locations,
                             uint32_t* total_corpses,
                             uint32_t* total_deaths,
                             DeathSignature* avg_signature) {
    if (!network) {
        if (total_locations) *total_locations = 0;
        if (total_corpses) *total_corpses = 0;
        if (total_deaths) *total_deaths = 0;
        if (avg_signature) *avg_signature = 0;
        return;
    }

    if (total_locations) *total_locations = network->node_count;
    if (total_corpses) *total_corpses = death_network_get_total_corpses(network);
    if (total_deaths) *total_deaths = network->total_deaths_tracked;
    if (avg_signature) *avg_signature = death_network_get_activity_level(network);
}

/* ========================================================================
 * String Utilities
 * ======================================================================== */

const char* death_event_type_name(DeathEventType type) {
    switch (type) {
        case DEATH_EVENT_NATURAL:   return "Natural";
        case DEATH_EVENT_BATTLE:    return "Battle";
        case DEATH_EVENT_PLAGUE:    return "Plague";
        case DEATH_EVENT_EXECUTION: return "Execution";
        case DEATH_EVENT_SACRIFICE: return "Sacrifice";
        case DEATH_EVENT_ACCIDENT:  return "Accident";
        case DEATH_EVENT_MURDER:    return "Murder";
        default:                    return "Unknown";
    }
}

const char* death_quality_name(DeathQuality quality) {
    switch (quality) {
        case DEATH_QUALITY_POOR:      return "Poor";
        case DEATH_QUALITY_AVERAGE:   return "Average";
        case DEATH_QUALITY_GOOD:      return "Good";
        case DEATH_QUALITY_EXCELLENT: return "Excellent";
        case DEATH_QUALITY_LEGENDARY: return "Legendary";
        default:                      return "Unknown";
    }
}

const char* death_signature_description(DeathSignature signature) {
    if (signature <= SIGNATURE_DORMANT) return "Dormant";
    if (signature <= SIGNATURE_WEAK) return "Weak";
    if (signature <= SIGNATURE_MODERATE) return "Moderate";
    if (signature <= SIGNATURE_STRONG) return "Strong";
    return "Overwhelming";
}

#include "village_event.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Outcome names */
static const char* OUTCOME_NAMES[] = {
    "None",
    "Spared",
    "Partial Harvest",
    "Full Harvest",
    "Defended"
};

/* Category names */
static const char* CATEGORY_NAMES[] = {
    "Children",
    "Warriors",
    "Elders",
    "Adults",
    "Clergy",
    "Criminals"
};

/* Soul energy per soul by quality */
#define ENERGY_PER_QUALITY 10.0f

VillageEvent* village_event_create(const char* village_id, const char* village_name,
                                   uint32_t total_population) {
    if (!village_id || !village_name) {
        return NULL;
    }

    VillageEvent* event = malloc(sizeof(VillageEvent));
    if (!event) {
        return NULL;
    }

    /* Initialize identity */
    strncpy(event->village_id, village_id, sizeof(event->village_id) - 1);
    event->village_id[sizeof(event->village_id) - 1] = '\0';
    strncpy(event->village_name, village_name, sizeof(event->village_name) - 1);
    event->village_name[sizeof(event->village_name) - 1] = '\0';
    event->description[0] = '\0';

    /* Initialize population */
    event->total_population = total_population;
    event->breakdown_count = 0;

    /* Initialize soul data */
    event->average_soul_quality = 0.0f;
    event->total_soul_energy = 0;

    /* Initialize corruption cost */
    event->total_corruption_cost = 0;
    event->children_corruption = 0;

    /* Initialize event state */
    event->event_triggered = false;
    event->trigger_day = 0;
    event->outcome = OUTCOME_NONE;
    event->resolution_day = 0;

    /* Initialize consequences */
    event->triggers_divine_attention = false;
    event->locks_revenant_path = false;
    event->locks_wraith_path = false;

    event->moral_consequence[0] = '\0';

    return event;
}

void village_event_destroy(VillageEvent* event) {
    if (event) {
        free(event);
    }
}

bool village_event_add_population(VillageEvent* event, PopulationCategory category,
                                  uint32_t count, float soul_quality,
                                  uint8_t corruption_per_soul) {
    if (!event) {
        return false;
    }

    if (event->breakdown_count >= MAX_POPULATION_BREAKDOWN) {
        return false;
    }

    PopulationBreakdown* breakdown = &event->breakdown[event->breakdown_count];
    breakdown->category = category;
    breakdown->count = count;
    breakdown->average_soul_quality = soul_quality;
    breakdown->corruption_per_soul = corruption_per_soul;

    event->breakdown_count++;
    return true;
}

bool village_event_calculate_totals(VillageEvent* event) {
    if (!event) {
        return false;
    }

    float total_quality = 0.0f;
    uint32_t total_souls = 0;
    uint32_t total_corruption = 0;
    uint32_t total_energy = 0;
    uint32_t children_corruption = 0;

    /* Sum across all categories */
    for (size_t i = 0; i < event->breakdown_count; i++) {
        PopulationBreakdown* breakdown = &event->breakdown[i];

        total_souls += breakdown->count;
        total_quality += breakdown->average_soul_quality * breakdown->count;
        total_corruption += breakdown->corruption_per_soul * breakdown->count;

        /* Calculate energy from this category */
        float category_energy = (breakdown->average_soul_quality / 100.0f) *
                               ENERGY_PER_QUALITY * breakdown->count;
        total_energy += (uint32_t)category_energy;

        /* Track children corruption separately */
        if (breakdown->category == POP_CHILDREN) {
            children_corruption = breakdown->corruption_per_soul * breakdown->count;
        }
    }

    /* Calculate averages */
    if (total_souls > 0) {
        event->average_soul_quality = total_quality / total_souls;
    } else {
        event->average_soul_quality = 0.0f;
    }

    event->total_soul_energy = total_energy;
    event->total_corruption_cost = total_corruption;
    event->children_corruption = children_corruption;

    return true;
}

bool village_event_trigger(VillageEvent* event, uint32_t day) {
    if (!event) {
        return false;
    }

    if (event->event_triggered) {
        return false; /* Already triggered */
    }

    event->event_triggered = true;
    event->trigger_day = day;

    return true;
}

bool village_event_spare(VillageEvent* event, uint32_t day) {
    if (!event) {
        return false;
    }

    event->outcome = OUTCOME_SPARED;
    event->resolution_day = day;

    /* No corruption from sparing */
    event->triggers_divine_attention = false;
    event->locks_revenant_path = false;
    event->locks_wraith_path = false;

    snprintf(event->moral_consequence, sizeof(event->moral_consequence),
             "You spared %s. %u lives continue. No corruption gained. "
             "Your humanity remains intact.",
             event->village_name, event->total_population);

    return true;
}

bool village_event_partial_harvest(VillageEvent* event, uint32_t day,
                                   uint32_t* souls_harvested, uint32_t* energy_gained,
                                   uint32_t* corruption_gained) {
    if (!event || !souls_harvested || !energy_gained || !corruption_gained) {
        return false;
    }

    event->outcome = OUTCOME_PARTIAL_HARVEST;
    event->resolution_day = day;

    /* Harvest only warriors and criminals */
    uint32_t warriors = village_event_get_population_count(event, POP_WARRIORS);
    uint32_t criminals = village_event_get_population_count(event, POP_CRIMINALS);

    *souls_harvested = warriors + criminals;

    /* Calculate energy and corruption for harvested categories */
    uint32_t energy = 0;
    uint32_t corruption = 0;

    for (size_t i = 0; i < event->breakdown_count; i++) {
        PopulationBreakdown* breakdown = &event->breakdown[i];
        if (breakdown->category == POP_WARRIORS || breakdown->category == POP_CRIMINALS) {
            float cat_energy = (breakdown->average_soul_quality / 100.0f) *
                              ENERGY_PER_QUALITY * breakdown->count;
            energy += (uint32_t)cat_energy;
            corruption += breakdown->corruption_per_soul * breakdown->count;
        }
    }

    *energy_gained = energy;
    *corruption_gained = corruption;

    /* Partial harvest doesn't trigger divine attention */
    event->triggers_divine_attention = false;
    event->locks_revenant_path = false;
    event->locks_wraith_path = false;

    snprintf(event->moral_consequence, sizeof(event->moral_consequence),
             "You harvested %u souls from %s (warriors and criminals only). "
             "Children and innocents spared. Corruption: +%u%%.",
             *souls_harvested, event->village_name, *corruption_gained);

    return true;
}

bool village_event_full_harvest(VillageEvent* event, uint32_t day,
                                uint32_t* souls_harvested, uint32_t* energy_gained,
                                uint32_t* corruption_gained) {
    if (!event || !souls_harvested || !energy_gained || !corruption_gained) {
        return false;
    }

    event->outcome = OUTCOME_FULL_HARVEST;
    event->resolution_day = day;

    /* Harvest entire population */
    *souls_harvested = event->total_population;
    *energy_gained = event->total_soul_energy;
    *corruption_gained = event->total_corruption_cost;

    /* Full harvest triggers major consequences */
    event->triggers_divine_attention = true;
    event->locks_revenant_path = true;  /* Can no longer resurrect to mortal life */
    event->locks_wraith_path = true;    /* Too corrupted for Wraith path */

    uint32_t children = village_event_get_population_count(event, POP_CHILDREN);

    snprintf(event->moral_consequence, sizeof(event->moral_consequence),
             "You harvested all %u souls from %s. Including %u children. "
             "Divine attention triggered. Corruption: +%u%%. "
             "Revenant and Wraith paths permanently locked.",
             *souls_harvested, event->village_name, children, *corruption_gained);

    return true;
}

bool village_event_check_path_locks(const VillageEvent* event, bool* revenant_locked,
                                    bool* wraith_locked) {
    if (!event || !revenant_locked || !wraith_locked) {
        return false;
    }

    *revenant_locked = event->locks_revenant_path;
    *wraith_locked = event->locks_wraith_path;

    return true;
}

uint32_t village_event_get_population_count(const VillageEvent* event,
                                            PopulationCategory category) {
    if (!event) {
        return 0;
    }

    for (size_t i = 0; i < event->breakdown_count; i++) {
        if (event->breakdown[i].category == category) {
            return event->breakdown[i].count;
        }
    }

    return 0;
}

bool village_event_triggers_divine_attention(const VillageEvent* event) {
    if (!event) {
        return false;
    }
    return event->triggers_divine_attention;
}

const char* village_event_outcome_name(VillageOutcome outcome) {
    if (outcome < 0 || outcome >= 5) {
        return "Unknown";
    }
    return OUTCOME_NAMES[outcome];
}

const char* village_event_category_name(PopulationCategory category) {
    if (category < 0 || category >= 6) {
        return "Unknown";
    }
    return CATEGORY_NAMES[category];
}

const char* village_event_get_moral_consequence(const VillageEvent* event) {
    if (!event) {
        return "Unknown event";
    }

    if (event->outcome == OUTCOME_NONE) {
        return "Event not yet resolved";
    }

    return event->moral_consequence;
}

uint32_t village_event_calculate_children_corruption(const VillageEvent* event) {
    if (!event) {
        return 0;
    }

    return event->children_corruption;
}

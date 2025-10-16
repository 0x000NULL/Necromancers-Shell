/**
 * split_routing.c - Soul Split-Routing System Implementation
 */

#include "split_routing.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Afterlife information */
static const struct {
    const char* name;
    bool fragment_compatible;
} AFTERLIFE_INFO[AFTERLIFE_COUNT] = {
    {"Heaven", true},
    {"Hell", true},
    {"Limbo", true},
    {"The Void", false}  /* Void cannot accept fragments */
};

/* Default reunification timer (in game years) */
#define DEFAULT_REUNIFICATION_YEARS 1000

SplitRoutingManager* split_routing_manager_create(void) {
    SplitRoutingManager* manager = calloc(1, sizeof(SplitRoutingManager));
    if (!manager) {
        return NULL;
    }

    manager->next_route_id = 1;
    return manager;
}

void split_routing_manager_destroy(SplitRoutingManager* manager) {
    free(manager);
}

bool split_routing_validate_percentages(const int* percentages, size_t count) {
    if (!percentages || count < 2 || count > MAX_SPLIT_DESTINATIONS) {
        return false;
    }

    int sum = 0;
    for (size_t i = 0; i < count; i++) {
        if (percentages[i] <= 0 || percentages[i] > 100) {
            return false;
        }
        sum += percentages[i];
    }

    return (sum == 100);
}

int split_routing_create_route(SplitRoutingManager* manager, int soul_id,
                               const char* soul_name,
                               const AfterlifeDestination* destinations,
                               const int* percentages, size_t count,
                               int game_day) {
    if (!manager || !soul_name || !destinations || !percentages) {
        return -1;
    }

    /* Validate count */
    if (count < 2 || count > MAX_SPLIT_DESTINATIONS) {
        return -1;
    }

    /* Validate percentages */
    if (!split_routing_validate_percentages(percentages, count)) {
        return -1;
    }

    /* Check capacity */
    if (manager->route_count >= MAX_SPLIT_ROUTES) {
        return -1;
    }

    /* Validate all destinations support fragments */
    for (size_t i = 0; i < count; i++) {
        if (!split_routing_is_fragment_compatible(destinations[i])) {
            return -1; /* The Void cannot accept fragments */
        }
    }

    /* Create new route */
    SplitRoutedSoul* route = &manager->routes[manager->route_count];
    route->route_id = manager->next_route_id++;
    route->soul_id = soul_id;
    strncpy(route->soul_name, soul_name, sizeof(route->soul_name) - 1);
    route->soul_name[sizeof(route->soul_name) - 1] = '\0';

    /* Set up fragments */
    route->fragment_count = count;
    for (size_t i = 0; i < count; i++) {
        route->fragments[i].destination = destinations[i];
        route->fragments[i].percentage = percentages[i];
        route->fragments[i].experience_gained = 0;
    }

    route->reunification_offered = true;
    route->years_until_reunification = DEFAULT_REUNIFICATION_YEARS;
    route->status = ROUTE_ROUTING;
    route->creation_day = game_day;

    manager->route_count++;
    manager->routes_created++;

    /* Transition to SPLIT status */
    route->status = ROUTE_SPLIT;

    return route->route_id;
}

int split_routing_advance_time(SplitRoutingManager* manager, int years) {
    if (!manager || years <= 0) {
        return 0;
    }

    int reunified_count = 0;

    for (size_t i = 0; i < manager->route_count; i++) {
        SplitRoutedSoul* route = &manager->routes[i];

        if (route->status != ROUTE_SPLIT) {
            continue;
        }

        route->years_until_reunification -= years;

        /* Automatic reunification when timer reaches 0 */
        if (route->years_until_reunification <= 0) {
            int total_experience = 0;
            if (split_routing_reunify(manager, route->route_id, &total_experience)) {
                reunified_count++;
            }
        }
    }

    return reunified_count;
}

bool split_routing_reunify(SplitRoutingManager* manager, int route_id,
                           int* total_experience) {
    if (!manager) {
        return false;
    }

    /* Find route */
    SplitRoutedSoul* route = NULL;
    for (size_t i = 0; i < manager->route_count; i++) {
        if (manager->routes[i].route_id == route_id) {
            route = &manager->routes[i];
            break;
        }
    }

    if (!route || route->status == ROUTE_REUNIFIED) {
        return false;
    }

    /* Calculate total experience from all fragments */
    int experience = 0;
    for (size_t i = 0; i < route->fragment_count; i++) {
        /* Simulate experience based on percentage and destination */
        int base_exp = 100; /* Base experience per fragment */
        int fragment_exp = (base_exp * route->fragments[i].percentage) / 100;

        /* Different destinations grant different bonuses */
        switch (route->fragments[i].destination) {
            case AFTERLIFE_HEAVEN:
                fragment_exp = (fragment_exp * 120) / 100; /* +20% bonus */
                break;
            case AFTERLIFE_HELL:
                fragment_exp = (fragment_exp * 110) / 100; /* +10% bonus */
                break;
            case AFTERLIFE_LIMBO:
                fragment_exp = (fragment_exp * 90) / 100;  /* -10% penalty */
                break;
            case AFTERLIFE_VOID:
                fragment_exp = 0; /* No experience from Void */
                break;
            default:
                break;
        }

        route->fragments[i].experience_gained = fragment_exp;
        experience += fragment_exp;
    }

    if (total_experience) {
        *total_experience = experience;
    }

    /* Mark as reunified */
    route->status = ROUTE_REUNIFIED;
    manager->reunifications_performed++;
    manager->routes_completed++;

    return true;
}

const SplitRoutedSoul* split_routing_get_route(const SplitRoutingManager* manager,
                                               int route_id) {
    if (!manager) {
        return NULL;
    }

    for (size_t i = 0; i < manager->route_count; i++) {
        if (manager->routes[i].route_id == route_id) {
            return &manager->routes[i];
        }
    }

    return NULL;
}

const SplitRoutedSoul* split_routing_get_all_routes(const SplitRoutingManager* manager,
                                                    size_t* count_out) {
    if (!manager) {
        if (count_out) {
            *count_out = 0;
        }
        return NULL;
    }

    if (count_out) {
        *count_out = manager->route_count;
    }

    return manager->routes;
}

const char* split_routing_get_afterlife_name(AfterlifeDestination destination) {
    if (destination < 0 || destination >= AFTERLIFE_COUNT) {
        return "Unknown";
    }
    return AFTERLIFE_INFO[destination].name;
}

bool split_routing_is_fragment_compatible(AfterlifeDestination destination) {
    if (destination < 0 || destination >= AFTERLIFE_COUNT) {
        return false;
    }
    return AFTERLIFE_INFO[destination].fragment_compatible;
}

const char* split_routing_status_to_string(RouteStatus status) {
    switch (status) {
        case ROUTE_ROUTING:
            return "ROUTING";
        case ROUTE_SPLIT:
            return "SPLIT";
        case ROUTE_REUNIFIED:
            return "REUNIFIED";
        default:
            return "UNKNOWN";
    }
}

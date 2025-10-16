/**
 * split_routing.h - Soul Split-Routing System
 *
 * Unlocked after Trial 2 (Wisdom). Allows routing soul fragments to multiple
 * afterlives simultaneously to resolve edge cases. Souls can be reunified
 * after experiencing multiple destinations.
 */

#ifndef SPLIT_ROUTING_H
#define SPLIT_ROUTING_H

#include <stdbool.h>
#include <stddef.h>

/* Maximum destinations for split routing */
#define MAX_SPLIT_DESTINATIONS 4

/* Maximum active split routes */
#define MAX_SPLIT_ROUTES 100

/* Afterlife destination IDs */
typedef enum {
    AFTERLIFE_HEAVEN,
    AFTERLIFE_HELL,
    AFTERLIFE_LIMBO,
    AFTERLIFE_VOID,
    AFTERLIFE_COUNT
} AfterlifeDestination;

/**
 * Split route status
 */
typedef enum {
    ROUTE_ROUTING,     /* Currently being routed */
    ROUTE_SPLIT,       /* Fragments active in destinations */
    ROUTE_REUNIFIED    /* Soul reunified */
} RouteStatus;

/**
 * Soul fragment in a destination
 */
typedef struct {
    AfterlifeDestination destination;
    int percentage;              /* Allocation percentage (0-100) */
    int experience_gained;       /* Experience from this destination */
} SoulFragment;

/**
 * Split-routed soul
 */
typedef struct {
    int route_id;                           /* Unique route identifier */
    int soul_id;                            /* Original soul ID */
    char soul_name[64];                     /* Soul name */

    SoulFragment fragments[MAX_SPLIT_DESTINATIONS];
    size_t fragment_count;                  /* Number of destinations (2-4) */

    bool reunification_offered;             /* Can be reunified */
    int years_until_reunification;          /* Timer until automatic reunification */
    RouteStatus status;                     /* Current route status */

    int creation_day;                       /* Game day route was created */
} SplitRoutedSoul;

/**
 * Split-routing manager
 */
typedef struct {
    SplitRoutedSoul routes[MAX_SPLIT_ROUTES];
    size_t route_count;

    int routes_created;         /* Total routes created */
    int routes_completed;       /* Routes that finished */
    int reunifications_performed; /* Manual reunifications */

    int next_route_id;          /* Next route ID to assign */
} SplitRoutingManager;

/**
 * Create split-routing manager
 *
 * Returns: Newly allocated manager, or NULL on failure
 */
SplitRoutingManager* split_routing_manager_create(void);

/**
 * Destroy split-routing manager
 *
 * Params:
 *   manager - Manager to destroy
 */
void split_routing_manager_destroy(SplitRoutingManager* manager);

/**
 * Create a split route
 *
 * Routes a soul to multiple afterlives with specified allocation percentages.
 * Percentages must sum to 100.
 *
 * Params:
 *   manager - Routing manager
 *   soul_id - Soul to split
 *   soul_name - Name of the soul
 *   destinations - Array of destination IDs
 *   percentages - Array of allocation percentages (must sum to 100)
 *   count - Number of destinations (2-4)
 *   game_day - Current game day
 *
 * Returns: Route ID on success, -1 on failure
 */
int split_routing_create_route(SplitRoutingManager* manager, int soul_id,
                               const char* soul_name,
                               const AfterlifeDestination* destinations,
                               const int* percentages, size_t count,
                               int game_day);

/**
 * Advance time for all routes
 *
 * Decrements reunification timers. Triggers automatic reunifications
 * when timers reach 0.
 *
 * Params:
 *   manager - Routing manager
 *   years - Number of years to advance
 *
 * Returns: Number of routes automatically reunified
 */
int split_routing_advance_time(SplitRoutingManager* manager, int years);

/**
 * Manually reunify a split route
 *
 * Merges soul fragments back together. Calculates combined experience
 * from all destinations.
 *
 * Params:
 *   manager - Routing manager
 *   route_id - Route to reunify
 *   total_experience - Output: combined experience from fragments
 *
 * Returns: True on success, false if route invalid/already reunified
 */
bool split_routing_reunify(SplitRoutingManager* manager, int route_id,
                           int* total_experience);

/**
 * Get route information
 *
 * Params:
 *   manager - Routing manager
 *   route_id - Route to query
 *
 * Returns: Pointer to route, or NULL if not found
 */
const SplitRoutedSoul* split_routing_get_route(const SplitRoutingManager* manager,
                                               int route_id);

/**
 * Get all active routes
 *
 * Params:
 *   manager - Routing manager
 *   count_out - Output: number of active routes
 *
 * Returns: Pointer to routes array
 */
const SplitRoutedSoul* split_routing_get_all_routes(const SplitRoutingManager* manager,
                                                    size_t* count_out);

/**
 * Get afterlife name
 *
 * Params:
 *   destination - Afterlife destination
 *
 * Returns: Name string ("Heaven", "Hell", etc.)
 */
const char* split_routing_get_afterlife_name(AfterlifeDestination destination);

/**
 * Check if afterlife supports fragmentation
 *
 * The Void doesn't support soul fragments.
 *
 * Params:
 *   destination - Afterlife destination
 *
 * Returns: True if fragments are compatible
 */
bool split_routing_is_fragment_compatible(AfterlifeDestination destination);

/**
 * Get route status as string
 *
 * Params:
 *   status - Route status
 *
 * Returns: Status string ("ROUTING", "SPLIT", "REUNIFIED")
 */
const char* split_routing_status_to_string(RouteStatus status);

/**
 * Validate split routing percentages
 *
 * Ensures percentages sum to 100 and are all positive.
 *
 * Params:
 *   percentages - Array of percentages
 *   count - Number of percentages
 *
 * Returns: True if valid, false otherwise
 */
bool split_routing_validate_percentages(const int* percentages, size_t count);

#endif /* SPLIT_ROUTING_H */

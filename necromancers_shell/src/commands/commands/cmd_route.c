/**
 * @file cmd_route.c
 * @brief Implementation of 'route' command - displays pathfinding route
 */

#include "commands.h"
#include "../../game/game_state.h"
#include "../../game/world/location_graph.h"
#include "../../game/world/world_map.h"
#include "../../game/world/territory.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ANSI color codes for output */
#define COLOR_RESET   "\033[0m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_RED     "\033[31m"
#define COLOR_DIM     "\033[2m"

/**
 * @brief Display pathfinding route to destination
 *
 * Usage: route <location_name|location_id> [--show-map]
 */
CommandResult cmd_route(ParsedCommand* cmd) {
    GameState* game = game_state_get_instance();
    if (!game) {
        return command_result_error(EXEC_ERROR_INTERNAL, "Game state not initialized");
    }

    if (!game->location_graph) {
        return command_result_error(EXEC_ERROR_INTERNAL, "Location graph not initialized");
    }

    /* Get destination argument */
    const char* dest_arg = parsed_command_get_arg(cmd, 0);
    if (!dest_arg) {
        return command_result_error(EXEC_ERROR_COMMAND_FAILED, "Missing destination argument");
    }

    /* Try to parse as location ID first, then search by name */
    uint32_t dest_id = 0;
    char* endptr;
    long id = strtol(dest_arg, &endptr, 10);

    if (*endptr == '\0' && id > 0) {
        /* Successfully parsed as integer */
        dest_id = (uint32_t)id;

        /* Verify location exists */
        Location* loc = territory_manager_get_location(game->territory, dest_id);
        if (!loc) {
            char error[256];
            snprintf(error, sizeof(error), "Location ID %u not found", dest_id);
            return command_result_error(EXEC_ERROR_COMMAND_FAILED, error);
        }
    } else {
        /* Search by name */
        Location* loc = territory_manager_get_location_by_name(game->territory, dest_arg);
        if (!loc) {
            char error[256];
            snprintf(error, sizeof(error), "Location '%s' not found", dest_arg);
            return command_result_error(EXEC_ERROR_COMMAND_FAILED, error);
        }
        dest_id = loc->id;
    }

    /* Check if destination is current location */
    if (dest_id == game->current_location_id) {
        return command_result_error(EXEC_ERROR_COMMAND_FAILED, "You are already at this location");
    }

    /* Find path */
    PathfindingResult result;
    bool found = location_graph_find_path(game->location_graph,
                                          game->current_location_id,
                                          dest_id,
                                          &result);

    if (!found || !result.path_found) {
        Location* dest = territory_manager_get_location(game->territory, dest_id);
        char error[256];
        snprintf(error, sizeof(error),
                 "No route found to %s (may require discovering connecting locations)",
                 dest ? dest->name : "destination");
        return command_result_error(EXEC_ERROR_COMMAND_FAILED, error);
    }

    /* Build result message */
    char message[4096];
    int pos = 0;

    /* Header */
    Location* dest_loc = territory_manager_get_location(game->territory, dest_id);
    pos += snprintf(message + pos, sizeof(message) - pos,
                    "\n%s=== Route to %s ===%s\n\n",
                    COLOR_CYAN, dest_loc ? dest_loc->name : "destination", COLOR_RESET);

    /* Route summary */
    pos += snprintf(message + pos, sizeof(message) - pos,
                    "%sDistance:%s %zu location%s\n",
                    COLOR_GREEN, COLOR_RESET,
                    result.path_length - 1,
                    (result.path_length - 1) == 1 ? "" : "s");

    pos += snprintf(message + pos, sizeof(message) - pos,
                    "%sTravel Time:%s %u hour%s\n",
                    COLOR_GREEN, COLOR_RESET,
                    result.total_travel_time,
                    result.total_travel_time == 1 ? "" : "s");

    pos += snprintf(message + pos, sizeof(message) - pos,
                    "%sDanger Level:%s %u\n\n",
                    result.total_danger > 50 ? COLOR_RED : COLOR_YELLOW,
                    COLOR_RESET,
                    result.total_danger);

    /* Path steps */
    pos += snprintf(message + pos, sizeof(message) - pos,
                    "%sPath:%s\n", COLOR_CYAN, COLOR_RESET);

    for (size_t i = 0; i < result.path_length; i++) {
        Location* loc = territory_manager_get_location(game->territory, result.path[i]);
        if (!loc) continue;

        const char* marker = (i == 0) ? "@" : (i == result.path_length - 1) ? "*" : "->";

        /* Get connection info for travel time */
        if (i < result.path_length - 1) {
            LocationConnection conn;
            if (location_graph_get_connection(game->location_graph,
                                             result.path[i],
                                             result.path[i + 1],
                                             &conn)) {
                pos += snprintf(message + pos, sizeof(message) - pos,
                                "  %s %s (%u hour%s, danger: %u)\n",
                                marker, loc->name,
                                conn.travel_time_hours,
                                conn.travel_time_hours == 1 ? "" : "s",
                                conn.danger_level);
            } else {
                pos += snprintf(message + pos, sizeof(message) - pos,
                                "  %s %s\n", marker, loc->name);
            }
        } else {
            pos += snprintf(message + pos, sizeof(message) - pos,
                            "  %s %s\n", marker, loc->name);
        }
    }

    /* Show map with path if requested */
    if (parsed_command_has_flag(cmd, "show-map") && game->world_map) {
        MapRenderOptions opts = map_render_options_default();
        opts.width = 60;
        opts.height = 20;
        opts.show_legend = false;
        opts.highlight_path = result.path;
        opts.highlight_path_length = result.path_length;
        opts.highlight_location_id = dest_id;

        char map_buffer[4096];
        size_t written = world_map_render(game->world_map,
                                          game->current_location_id,
                                          &opts,
                                          map_buffer,
                                          sizeof(map_buffer));

        if (written > 0) {
            pos += snprintf(message + pos, sizeof(message) - pos,
                            "\n%sMap:%s\n%s\n",
                            COLOR_CYAN, COLOR_RESET,
                            map_buffer);
        }
    }

    /* Add hint */
    pos += snprintf(message + pos, sizeof(message) - pos,
                    "\n%sHint:%s Use 'connect %s' to travel to the first location\n",
                    COLOR_DIM, COLOR_RESET,
                    result.path_length > 1 ?
                        (territory_manager_get_location(game->territory, result.path[1]) ?
                         territory_manager_get_location(game->territory, result.path[1])->name : "next") :
                        "destination");

    /* Free path */
    pathfinding_result_free(&result);

    return command_result_success(message);
}

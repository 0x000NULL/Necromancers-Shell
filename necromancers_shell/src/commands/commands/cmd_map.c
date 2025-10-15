/**
 * @file cmd_map.c
 * @brief Implementation of 'map' command - displays world map
 */

#include "commands.h"
#include "../../game/game_state.h"
#include "../../game/world/world_map.h"
#include "../../game/world/territory_status.h"
#include <stdio.h>
#include <string.h>

/* ANSI color codes for output */
#define COLOR_RESET   "\033[0m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_DIM     "\033[2m"

/**
 * @brief Display world map with current location
 *
 * Usage: map [--width <n>] [--height <n>] [--no-legend] [--show-all]
 */
CommandResult cmd_map(ParsedCommand* cmd) {
    GameState* game = game_state_get_instance();
    if (!game) {
        return command_result_error(EXEC_ERROR_INTERNAL, "Game state not initialized");
    }

    if (!game->world_map) {
        return command_result_error(EXEC_ERROR_INTERNAL, "World map not initialized");
    }

    /* Parse options */
    MapRenderOptions opts = map_render_options_default();

    /* Custom width */
    if (parsed_command_has_flag(cmd, "width")) {
        const ArgumentValue* width_arg = parsed_command_get_flag(cmd, "width");
        if (width_arg && width_arg->type == ARG_TYPE_INT) {
            int64_t width = width_arg->value.int_value;
            if (width < 20 || width > 120) {
                return command_result_error(EXEC_ERROR_COMMAND_FAILED,
                                           "Width must be between 20 and 120");
            }
            opts.width = (uint16_t)width;
        }
    }

    /* Custom height */
    if (parsed_command_has_flag(cmd, "height")) {
        const ArgumentValue* height_arg = parsed_command_get_flag(cmd, "height");
        if (height_arg && height_arg->type == ARG_TYPE_INT) {
            int64_t height = height_arg->value.int_value;
            if (height < 10 || height > 40) {
                return command_result_error(EXEC_ERROR_COMMAND_FAILED,
                                           "Height must be between 10 and 40");
            }
            opts.height = (uint16_t)height;
        }
    }

    /* Hide legend */
    if (parsed_command_has_flag(cmd, "no-legend")) {
        opts.show_legend = false;
    }

    /* Show all locations including undiscovered */
    if (parsed_command_has_flag(cmd, "show-all")) {
        opts.show_undiscovered = true;
    } else {
        opts.show_undiscovered = false;
    }

    /* Render map */
    char map_buffer[8192];
    size_t written = world_map_render(game->world_map,
                                       game->current_location_id,
                                       &opts,
                                       map_buffer,
                                       sizeof(map_buffer));

    if (written == 0) {
        return command_result_error(EXEC_ERROR_INTERNAL, "Failed to render map");
    }

    /* Build result message */
    char result[9216];
    int pos = 0;

    /* Add header */
    pos += snprintf(result + pos, sizeof(result) - pos,
                    "\n%s=== World Map ===%s\n\n",
                    COLOR_CYAN, COLOR_RESET);

    /* Add map */
    pos += snprintf(result + pos, sizeof(result) - pos, "%s", map_buffer);

    /* Add current location info */
    Location* current = territory_manager_get_location(game->territory,
                                                        game->current_location_id);
    if (current) {
        pos += snprintf(result + pos, sizeof(result) - pos,
                        "\n%sCurrent Location:%s %s (@)\n",
                        COLOR_GREEN, COLOR_RESET, current->name);

        /* Get territory status */
        if (game->territory_status) {
            TerritoryStatus* status = territory_status_get(game->territory_status,
                                                           game->current_location_id);
            if (status) {
                pos += snprintf(result + pos, sizeof(result) - pos,
                                "Control: %u%% | Alert: %s | Stability: %s\n",
                                status->control_percentage,
                                territory_status_alert_name(status->alert_level),
                                territory_status_stability_name(status->stability));
            }
        }
    }

    /* Add hint */
    pos += snprintf(result + pos, sizeof(result) - pos,
                    "\n%sHint:%s Use 'route <location>' to plot a path\n",
                    COLOR_DIM, COLOR_RESET);

    return command_result_success(result);
}

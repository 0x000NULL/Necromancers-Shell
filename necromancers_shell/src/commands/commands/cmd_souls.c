/**
 * @file cmd_souls.c
 * @brief Souls command implementation
 *
 * Display soul inventory with filtering and sorting options.
 */

#define _GNU_SOURCE
#include "commands.h"
#include "../../game/game_state.h"
#include "../../game/souls/soul_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern GameState* g_game_state;

/**
 * Parse soul type from string
 */
static bool parse_soul_type(const char* str, SoulType* type) {
    if (!str || !type) return false;

    if (strcmp(str, "common") == 0) {
        *type = SOUL_TYPE_COMMON;
    } else if (strcmp(str, "warrior") == 0) {
        *type = SOUL_TYPE_WARRIOR;
    } else if (strcmp(str, "mage") == 0) {
        *type = SOUL_TYPE_MAGE;
    } else if (strcmp(str, "innocent") == 0) {
        *type = SOUL_TYPE_INNOCENT;
    } else if (strcmp(str, "corrupted") == 0) {
        *type = SOUL_TYPE_CORRUPTED;
    } else if (strcmp(str, "ancient") == 0) {
        *type = SOUL_TYPE_ANCIENT;
    } else {
        return false;
    }
    return true;
}

/**
 * Parse sort criteria from string
 */
static bool parse_sort_criteria(const char* str, SoulSortCriteria* criteria) {
    if (!str || !criteria) return false;

    if (strcmp(str, "id") == 0) {
        *criteria = SOUL_SORT_ID;
    } else if (strcmp(str, "type") == 0) {
        *criteria = SOUL_SORT_TYPE;
    } else if (strcmp(str, "quality") == 0) {
        *criteria = SOUL_SORT_QUALITY;
    } else if (strcmp(str, "energy") == 0) {
        *criteria = SOUL_SORT_ENERGY;
    } else {
        return false;
    }
    return true;
}

CommandResult cmd_souls(ParsedCommand* cmd) {
    if (!g_game_state || !g_game_state->souls) {
        return command_result_error(EXEC_ERROR_INTERNAL,
                                     "Game state not initialized");
    }

    /* Build filter */
    SoulFilter filter = {0};
    filter.type = -1; /* All types by default */
    filter.quality_min = 0;
    filter.quality_max = 100;
    filter.bound_filter = -1; /* All souls by default */

    /* Parse --type flag */
    if (parsed_command_has_flag(cmd, "type")) {
        const ArgumentValue* type_arg = parsed_command_get_flag(cmd, "type");
        if (type_arg && type_arg->type == ARG_TYPE_STRING) {
            const char* type_str = type_arg->value.str_value;
            SoulType type;
            if (!parse_soul_type(type_str, &type)) {
                char error_msg[128];
                snprintf(error_msg, sizeof(error_msg), "Invalid soul type: %s", type_str);
                return command_result_error(EXEC_ERROR_INVALID_COMMAND, error_msg);
            }
            filter.type = (int)type;
        }
    }

    /* Parse --quality-min flag */
    if (parsed_command_has_flag(cmd, "quality-min")) {
        const ArgumentValue* qmin_arg = parsed_command_get_flag(cmd, "quality-min");
        if (qmin_arg && qmin_arg->type == ARG_TYPE_INT) {
            filter.quality_min = (uint8_t)qmin_arg->value.int_value;
            if (filter.quality_min > 100) {
                filter.quality_min = 100;
            }
        }
    }

    /* Parse --quality-max flag */
    if (parsed_command_has_flag(cmd, "quality-max")) {
        const ArgumentValue* qmax_arg = parsed_command_get_flag(cmd, "quality-max");
        if (qmax_arg && qmax_arg->type == ARG_TYPE_INT) {
            filter.quality_max = (uint8_t)qmax_arg->value.int_value;
            if (filter.quality_max > 100) {
                filter.quality_max = 100;
            }
        }
    }

    /* Parse --bound flag */
    if (parsed_command_has_flag(cmd, "bound")) {
        filter.bound_filter = 1; /* Only bound souls */
    }

    /* Parse --free flag */
    if (parsed_command_has_flag(cmd, "free")) {
        filter.bound_filter = 0; /* Only unbound souls */
    }

    /* Parse --sort flag to determine if we need to sort */
    SoulSortCriteria sort_criteria = SOUL_SORT_ID;
    bool should_sort = false;
    if (parsed_command_has_flag(cmd, "sort")) {
        const ArgumentValue* sort_arg = parsed_command_get_flag(cmd, "sort");
        if (sort_arg && sort_arg->type == ARG_TYPE_STRING) {
            const char* sort_str = sort_arg->value.str_value;
            if (!parse_sort_criteria(sort_str, &sort_criteria)) {
                char error_msg[128];
                snprintf(error_msg, sizeof(error_msg), "Invalid sort criteria: %s", sort_str);
                return command_result_error(EXEC_ERROR_INVALID_COMMAND, error_msg);
            }
            should_sort = true;
        }
    }

    /* Sort manager if requested (this affects the order of results) */
    if (should_sort) {
        soul_manager_sort(g_game_state->souls, sort_criteria);
    }

    /* Get filtered souls */
    Soul** results = NULL;
    size_t count = 0;
    results = soul_manager_get_filtered(g_game_state->souls, &filter, &count);

    /* Build output string using dynamic buffer */
    char* output = NULL;
    size_t output_size = 0;
    FILE* stream = open_memstream(&output, &output_size);
    if (!stream) {
        free(results);
        return command_result_error(EXEC_ERROR_INTERNAL,
                                     "Failed to allocate output buffer");
    }

    fprintf(stream, "=== Soul Inventory ===\n\n");

    if (count == 0) {
        fprintf(stream, "No souls match the specified criteria.\n");
    } else {
        /* Table header */
        fprintf(stream, "%-6s %-12s %-8s %-8s %-10s\n",
                "ID", "Type", "Quality", "Energy", "Status");
        fprintf(stream, "%-6s %-12s %-8s %-8s %-10s\n",
                "------", "------------", "--------", "--------", "----------");

        /* Table rows */
        for (size_t i = 0; i < count; i++) {
            const Soul* soul = results[i];
            const char* status = soul->bound ? "Bound" : "Free";

            fprintf(stream, "%-6u %-12s %-8u %-8u %-10s\n",
                    soul->id,
                    soul_type_name(soul->type),
                    soul->quality,
                    soul->energy,
                    status);
        }

        fprintf(stream, "\nTotal: %zu soul(s)\n", count);

        /* Calculate total energy */
        uint64_t total_energy = 0;
        for (size_t i = 0; i < count; i++) {
            total_energy += results[i]->energy;
        }
        fprintf(stream, "Combined Energy: %lu\n", total_energy);
    }

    fclose(stream);
    free(results);

    CommandResult result = command_result_success(output);
    free(output);
    return result;
}

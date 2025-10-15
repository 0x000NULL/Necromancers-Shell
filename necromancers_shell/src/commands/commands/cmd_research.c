/**
 * @file cmd_research.c
 * @brief Research command - View and manage research projects
 */

#include "commands.h"
#include "../../game/game_state.h"
#include "../../game/progression/research.h"
#include "../../terminal/ui_feedback.h"
#include "../../utils/logger.h"
#include "../parser.h"
#include <stdio.h>
#include <string.h>

CommandResult cmd_research(ParsedCommand* cmd) {
    (void)cmd; /* Suppress unused parameter warning */

    GameState* state = game_state_get_instance();
    if (!state) {
        return command_result_error(EXEC_ERROR_INTERNAL, "Game state not initialized");
    }

    /* TODO: Add research manager to GameState */
    return command_result_error(EXEC_ERROR_INTERNAL,
                               "Research system not yet integrated into GameState");

    #if 0
    /* Get subcommand */
    const char* subcommand = parsed_command_get_arg(cmd, 0);

    /* No args - list all available projects */
    if (!subcommand) {
        uint32_t results[50];
        size_t count = research_get_available(state->research, state->player_level, results, 50);

        if (count == 0) {
            ui_print_info("No research projects available at your level");
            return command_result_success("No projects available");
        }

        ui_print_success("Available Research Projects:");
        printf("\n");

        for (size_t i = 0; i < count; i++) {
            const ResearchProject* project = research_get_project(state->research, results[i]);
            if (project) {
                display_project(project, false);
            }
        }

        printf("\nUse 'research info <id>' for details\n");
        printf("Use 'research start <id>' to begin a project\n");

        return command_result_success("Listed available projects");
    }

    /* research info <id> */
    if (strcmp(subcommand, "info") == 0) {
        const char* id_str = parsed_command_get_arg(cmd, 1);
        if (!id_str) {
            return command_result_error("Usage: research info <project_id>");
        }

        uint32_t project_id = (uint32_t)atoi(id_str);
        const ResearchProject* project = research_get_project(state->research, project_id);

        if (!project) {
            return command_result_error("Project not found");
        }

        display_project(project, true);
        return command_result_success("Displayed project info");
    }

    /* research start <id> */
    if (strcmp(subcommand, "start") == 0) {
        const char* id_str = parsed_command_get_arg(cmd, 1);
        if (!id_str) {
            return command_result_error("Usage: research start <project_id>");
        }

        uint32_t project_id = (uint32_t)atoi(id_str);
        const ResearchProject* project = research_get_project(state->research, project_id);

        if (!project) {
            return command_result_error("Project not found");
        }

        /* Check if can start */
        uint32_t available_energy = resources_get_soul_energy(&state->resources);
        uint32_t available_mana = resources_get_mana(&state->resources);

        if (!research_can_start(state->research, project_id, state->player_level,
                                available_energy, available_mana)) {
            ui_print_error("Cannot start project - check requirements and resources");
            return command_result_error("Requirements not met");
        }

        /* Deduct costs */
        if (!resources_spend_soul_energy(&state->resources, project->soul_energy_cost)) {
            return command_result_error("Insufficient soul energy");
        }
        if (!resources_spend_mana(&state->resources, project->mana_cost)) {
            resources_add_soul_energy(&state->resources, project->soul_energy_cost); // Refund
            return command_result_error("Insufficient mana");
        }

        /* Start project */
        if (!research_start(state->research, project_id)) {
            // Refund resources
            resources_add_soul_energy(&state->resources, project->soul_energy_cost);
            resources_add_mana(&state->resources, project->mana_cost);
            return command_result_error("Failed to start project");
        }

        ui_print_success("Research started: %s", project->name);
        printf("Time required: %u hours\n", project->time_hours);

        return command_result_success("Started research");
    }

    /* research current */
    if (strcmp(subcommand, "current") == 0) {
        uint32_t current_id = research_get_current(state->research);

        if (current_id == 0) {
            ui_print_info("No research currently in progress");
            return command_result_success("No current research");
        }

        const ResearchProject* project = research_get_project(state->research, current_id);
        if (project) {
            ui_print_success("Current Research:");
            printf("\n");
            display_project(project, true);
        }

        return command_result_success("Displayed current research");
    }

    /* research cancel */
    if (strcmp(subcommand, "cancel") == 0) {
        if (!research_cancel_current(state->research)) {
            return command_result_error("No research to cancel");
        }

        ui_print_warning("Research cancelled (resources not refunded)");
        return command_result_success("Cancelled research");
    }

    /* research completed */
    if (strcmp(subcommand, "completed") == 0) {
        uint32_t results[50];
        size_t count = research_get_completed(state->research, results, 50);

        if (count == 0) {
            ui_print_info("No completed research projects");
            return command_result_success("No completed projects");
        }

        ui_print_success("Completed Research Projects:");
        printf("\n");

        for (size_t i = 0; i < count; i++) {
            const ResearchProject* project = research_get_project(state->research, results[i]);
            if (project) {
                printf("  [%u] %s - Unlocked: %s\n",
                       project->id, project->name, project->unlock_name);
            }
        }

        return command_result_success("Listed completed projects");
    }

    return command_result_error("Unknown subcommand. Use: info, start, current, cancel, completed");
    #endif
}

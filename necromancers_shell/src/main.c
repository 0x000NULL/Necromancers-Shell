/**
 * Necromancer's Shell - Main Entry Point
 * Phase 2: Core Game Systems
 */

#include "core/memory.h"
#include "core/timing.h"
#include "core/events.h"
#include "terminal/ncurses_wrapper.h"
#include "terminal/colors.h"
#include "terminal/input_handler.h"
#include "utils/logger.h"
#include "commands/command_system.h"
#include "commands/commands/commands.h"
#include "commands/registry.h"
#include "game/game_state.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

/* Global state */
GameState* g_game_state = NULL;
static volatile bool g_running = true;

/**
 * Signal handler for graceful shutdown
 */
static void signal_handler(int signum) {
    (void)signum;
    g_running = false;
}

/**
 * Register all game commands
 */
static void register_game_commands(void) {
    extern CommandRegistry* g_command_registry;
    int registered = 0;

    /* Souls command */
    {
        static FlagDefinition souls_flags[] = {
            { .name = "type", .short_name = 't', .type = ARG_TYPE_STRING, .required = false,
              .description = "Filter by soul type (common,warrior,mage,innocent,corrupted,ancient)" },
            { .name = "quality-min", .short_name = 0, .type = ARG_TYPE_INT, .required = false,
              .description = "Minimum quality (0-100)" },
            { .name = "quality-max", .short_name = 0, .type = ARG_TYPE_INT, .required = false,
              .description = "Maximum quality (0-100)" },
            { .name = "bound", .short_name = 'b', .type = ARG_TYPE_BOOL, .required = false,
              .description = "Show only bound souls" },
            { .name = "free", .short_name = 'f', .type = ARG_TYPE_BOOL, .required = false,
              .description = "Show only free souls" },
            { .name = "sort", .short_name = 's', .type = ARG_TYPE_STRING, .required = false,
              .description = "Sort by (id,type,quality,energy,captured)" }
        };

        CommandInfo info = {
            .name = "souls",
            .description = "Display soul inventory",
            .usage = "souls [--type <type>] [--quality-min <n>] [--quality-max <n>] [--bound] [--free] [--sort <criteria>]",
            .help_text = "Shows your collected souls with optional filtering and sorting.\n"
                        "Use flags to filter by type, quality range, binding status, or sort results.",
            .function = cmd_souls,
            .flags = souls_flags,
            .flag_count = 6,
            .min_args = 0,
            .max_args = 0,
            .hidden = false
        };
        if (command_registry_register(g_command_registry, &info)) registered++;
    }

    /* Harvest command */
    {
        static FlagDefinition harvest_flags[] = {
            { .name = "count", .short_name = 'c', .type = ARG_TYPE_INT, .required = false,
              .description = "Number of corpses to harvest (default: 10, max: 100)" }
        };

        CommandInfo info = {
            .name = "harvest",
            .description = "Harvest souls from corpses",
            .usage = "harvest [--count <n>]",
            .help_text = "Collects souls from corpses at your current location.\n"
                        "Soul type and quality depend on the location type.",
            .function = cmd_harvest,
            .flags = harvest_flags,
            .flag_count = 1,
            .min_args = 0,
            .max_args = 0,
            .hidden = false
        };
        if (command_registry_register(g_command_registry, &info)) registered++;
    }

    /* Scan command */
    {
        CommandInfo info = {
            .name = "scan",
            .description = "Scan for connected locations",
            .usage = "scan",
            .help_text = "Shows all locations connected to your current position.\n"
                        "Displays status and resources for discovered locations.",
            .function = cmd_scan,
            .flags = NULL,
            .flag_count = 0,
            .min_args = 0,
            .max_args = 0,
            .hidden = false
        };
        if (command_registry_register(g_command_registry, &info)) registered++;
    }

    /* Probe command */
    {
        CommandInfo info = {
            .name = "probe",
            .description = "Investigate a location",
            .usage = "probe <location_id_or_name>",
            .help_text = "Gets detailed information about a specific location.\n"
                        "If the location is undiscovered and connected, it will be discovered.",
            .function = cmd_probe,
            .flags = NULL,
            .flag_count = 0,
            .min_args = 1,
            .max_args = 1,
            .hidden = false
        };
        if (command_registry_register(g_command_registry, &info)) registered++;
    }

    /* Connect command */
    {
        CommandInfo info = {
            .name = "connect",
            .description = "Travel to a location",
            .usage = "connect <location_id_or_name>",
            .help_text = "Travels to a connected, discovered location.\n"
                        "Travel takes 1-3 hours of game time.",
            .function = cmd_connect,
            .flags = NULL,
            .flag_count = 0,
            .min_args = 1,
            .max_args = 1,
            .hidden = false
        };
        if (command_registry_register(g_command_registry, &info)) registered++;
    }

    /* Raise command */
    {
        static FlagDefinition raise_flags[] = {
            { .name = "soul", .short_name = 's', .type = ARG_TYPE_INT, .required = false,
              .description = "Soul ID to bind to minion" }
        };

        CommandInfo info = {
            .name = "raise",
            .description = "Raise an undead minion",
            .usage = "raise <type> [name] [--soul <id>]",
            .help_text = "Raises an undead minion from corpses. Costs soul energy.\n"
                        "Types: zombie, skeleton, ghoul, wraith, wight, revenant\n"
                        "Optional: provide a name or bind a soul for stat bonuses.",
            .function = cmd_raise,
            .flags = raise_flags,
            .flag_count = 1,
            .min_args = 1,
            .max_args = 2,
            .hidden = false
        };
        if (command_registry_register(g_command_registry, &info)) registered++;
    }

    /* Bind command */
    {
        CommandInfo info = {
            .name = "bind",
            .description = "Bind soul to minion",
            .usage = "bind <minion_id> <soul_id>",
            .help_text = "Binds a soul to a minion for stat bonuses.\n"
                        "Soul quality affects the strength of the bonus.",
            .function = cmd_bind,
            .flags = NULL,
            .flag_count = 0,
            .min_args = 2,
            .max_args = 2,
            .hidden = false
        };
        if (command_registry_register(g_command_registry, &info)) registered++;
    }

    /* Banish command */
    {
        CommandInfo info = {
            .name = "banish",
            .description = "Banish a minion",
            .usage = "banish <minion_id>",
            .help_text = "Banishes (destroys) a minion from your army.\n"
                        "If the minion has a bound soul, it returns to your collection.",
            .function = cmd_banish,
            .flags = NULL,
            .flag_count = 0,
            .min_args = 1,
            .max_args = 1,
            .hidden = false
        };
        if (command_registry_register(g_command_registry, &info)) registered++;
    }

    /* Minions command */
    {
        CommandInfo info = {
            .name = "minions",
            .description = "List all minions",
            .usage = "minions",
            .help_text = "Displays your complete minion army.\n"
                        "Shows stats, levels, and bound souls for each minion.",
            .function = cmd_minions,
            .flags = NULL,
            .flag_count = 0,
            .min_args = 0,
            .max_args = 0,
            .hidden = false
        };
        if (command_registry_register(g_command_registry, &info)) registered++;
    }

    LOG_INFO("Registered %d game commands", registered);
}

/**
 * Display welcome banner
 */
static void display_welcome(void) {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║                                                        ║\n");
    printf("║           NECROMANCER'S SHELL - v0.2.0                ║\n");
    printf("║         Dark Fantasy Terminal RPG                     ║\n");
    printf("║                                                        ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n");
    printf("\n");
    printf("Type 'help' for available commands.\n");
    printf("Type 'quit' or 'exit' to leave.\n");
    printf("\n");
}

/**
 * Main entry point
 */
int main(void) {
    int exit_code = EXIT_SUCCESS;

    /* Setup signal handlers */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    /* Initialize logger */
    if (!logger_init("necromancer_shell.log", LOG_LEVEL_INFO)) {
        fprintf(stderr, "Failed to initialize logger\n");
        return EXIT_FAILURE;
    }

    LOG_INFO("=== Necromancer's Shell Starting ===");
    LOG_INFO("Phase 2: Core Game Systems");

    /* Initialize command system (includes built-in commands) */
    if (!command_system_init()) {
        LOG_ERROR("Failed to initialize command system");
        logger_shutdown();
        return EXIT_FAILURE;
    }

    /* Register game commands */
    register_game_commands();

    /* Initialize game state */
    g_game_state = game_state_create();
    if (!g_game_state) {
        LOG_ERROR("Failed to create game state");
        command_system_shutdown();
        logger_shutdown();
        return EXIT_FAILURE;
    }

    /* Display welcome message */
    display_welcome();

    /* Display starting location */
    Location* start_loc = game_state_get_current_location(g_game_state);
    if (start_loc) {
        printf("You awaken in the %s...\n", start_loc->name);
        printf("%s\n\n", start_loc->description);
    }

    LOG_INFO("Entering main loop");

    /* Main game loop - simple REPL for now */
    char input_buffer[1024];
    while (g_running) {
        /* Display prompt */
        printf("> ");
        fflush(stdout);

        /* Read input */
        if (fgets(input_buffer, sizeof(input_buffer), stdin) == NULL) {
            break; /* EOF or error */
        }

        /* Remove trailing newline */
        size_t len = strlen(input_buffer);
        if (len > 0 && input_buffer[len - 1] == '\n') {
            input_buffer[len - 1] = '\0';
        }

        /* Skip empty input */
        if (input_buffer[0] == '\0') {
            continue;
        }

        /* Execute command */
        CommandResult result = command_system_execute(input_buffer);

        /* Display result */
        if (result.success) {
            if (result.output && result.output[0] != '\0') {
                printf("%s\n", result.output);
            }
        } else {
            /* Print error */
            if (result.error_message && result.error_message[0] != '\0') {
                fprintf(stderr, "Error: %s\n", result.error_message);
            }
        }

        /* Check if quit was requested */
        if (result.should_exit) {
            g_running = false;
        }

        /* Free result strings */
        free(result.output);
        free(result.error_message);
    }

    LOG_INFO("Shutting down");

    /* Cleanup */
    game_state_destroy(g_game_state);
    g_game_state = NULL;

    command_system_shutdown();
    logger_shutdown();

    printf("\nFarewell, Necromancer. Your dark deeds are recorded in history...\n\n");

    return exit_code;
}

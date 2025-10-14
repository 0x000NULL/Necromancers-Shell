/**
 * Example Integration - Command System Usage
 *
 * This file demonstrates how to integrate the command system into the main game loop.
 * Copy this code into main.c or adapt as needed.
 */

#include "command_system.h"
#include "../terminal/ui_feedback.h"
#include "../utils/logger.h"
#include <stdio.h>
#include <stdbool.h>

/* Example: Basic integration in main() */
void example_basic_integration(void) {
    /* Initialize logger */
    logger_init("necromancer.log", LOG_LEVEL_INFO);

    /* Initialize command system */
    if (!command_system_init()) {
        fprintf(stderr, "Failed to initialize command system\n");
        logger_shutdown();
        return;
    }

    printf("Necromancer's Shell - Command System Active\n");
    printf("Type 'help' for available commands, 'quit' to exit\n\n");

    /* Main loop */
    bool running = true;
    while (running) {
        /* Process one command */
        CommandResult result = command_system_process_input("necromancer> ");

        /* Display result */
        ui_feedback_command_result(&result);

        /* Check for exit */
        if (result.should_exit) {
            running = false;
        }

        /* Clean up result */
        command_result_destroy(&result);
    }

    /* Cleanup */
    command_system_shutdown();
    logger_shutdown();
}

/* Example: Execute command directly without prompt */
void example_execute_command(void) {
    /* Initialize systems */
    logger_init(NULL, LOG_LEVEL_WARN);  /* NULL = stdout only */
    command_system_init();

    /* Execute a command directly */
    CommandResult result = command_system_execute("help");
    ui_feedback_command_result(&result);
    command_result_destroy(&result);

    /* Execute another command */
    result = command_system_execute("status --verbose");
    ui_feedback_command_result(&result);
    command_result_destroy(&result);

    /* Cleanup */
    command_system_shutdown();
    logger_shutdown();
}

/* Example: Register custom command */
CommandResult cmd_custom(ParsedCommand* cmd) {
    (void)cmd;
    return command_result_success("Custom command executed!");
}

void example_register_custom_command(void) {
    logger_init(NULL, LOG_LEVEL_INFO);
    command_system_init();

    /* Define custom command */
    CommandInfo custom = {
        .name = "custom",
        .description = "A custom command",
        .usage = "custom",
        .help_text = "This is a custom command example.",
        .function = cmd_custom,
        .flags = NULL,
        .flag_count = 0,
        .min_args = 0,
        .max_args = 0,
        .hidden = false
    };

    /* Register it */
    if (command_system_register_command(&custom)) {
        printf("Custom command registered!\n");

        /* Execute it */
        CommandResult result = command_system_execute("custom");
        ui_feedback_command_result(&result);
        command_result_destroy(&result);
    }

    command_system_shutdown();
    logger_shutdown();
}

/* Example: Access command system components */
void example_access_components(void) {
    logger_init(NULL, LOG_LEVEL_INFO);
    command_system_init();

    /* Get history */
    CommandHistory* history = command_system_get_history();
    if (history) {
        printf("History size: %zu\n", command_history_size(history));
        printf("History capacity: %zu\n", command_history_capacity(history));
    }

    /* Get autocomplete */
    Autocomplete* ac = command_system_get_autocomplete();
    if (ac) {
        char** completions = NULL;
        size_t count = 0;
        if (autocomplete_get_completions(ac, "he", &completions, &count)) {
            printf("Completions for 'he': ");
            for (size_t i = 0; i < count; i++) {
                printf("%s ", completions[i]);
            }
            printf("\n");
            autocomplete_free_completions(completions, count);
        }
    }

    /* Get registry */
    CommandRegistry* registry = command_system_get_registry();
    if (registry) {
        printf("Registered commands: %zu\n", command_registry_count(registry));

        char** names = NULL;
        size_t count = 0;
        if (command_registry_get_all_names(registry, &names, &count)) {
            printf("Command names: ");
            for (size_t i = 0; i < count; i++) {
                printf("%s ", names[i]);
            }
            printf("\n");
            command_registry_free_names(names, count);
        }
    }

    command_system_shutdown();
    logger_shutdown();
}

/*
 * To use in your main.c:
 *
 * 1. Include the command system header:
 *    #include "commands/command_system.h"
 *
 * 2. In your main() function, replace the existing game loop:
 *
 *    int main(void) {
 *        // Initialize logger
 *        logger_init("necromancer.log", LOG_LEVEL_INFO);
 *
 *        // Initialize command system
 *        if (!command_system_init()) {
 *            fprintf(stderr, "Failed to initialize command system\n");
 *            return 1;
 *        }
 *
 *        // Main game loop
 *        bool running = true;
 *        while (running) {
 *            CommandResult result = command_system_process_input("necromancer> ");
 *            ui_feedback_command_result(&result);
 *
 *            if (result.should_exit) {
 *                running = false;
 *            }
 *
 *            command_result_destroy(&result);
 *        }
 *
 *        // Cleanup
 *        command_system_shutdown();
 *        logger_shutdown();
 *
 *        return 0;
 *    }
 */

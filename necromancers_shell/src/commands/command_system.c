#include "command_system.h"
#include "commands/commands.h"
#include "../terminal/ui_feedback.h"
#include "../utils/logger.h"
#include "../core/state_manager.h"
#include <stdlib.h>

/* Global command system state */
static struct {
    bool initialized;
    CommandRegistry* registry;
    InputHandler* input_handler;
} g_command_system = {
    .initialized = false,
    .registry = NULL,
    .input_handler = NULL
};

/* Global registry reference for commands that need it */
CommandRegistry* g_command_registry = NULL;

/* External references that commands might need */
StateManager* g_state_manager = NULL;  /* Set by game initialization */

bool command_system_init(void) {
    if (g_command_system.initialized) {
        LOG_WARN("Command system already initialized");
        return true;
    }

    LOG_INFO("Initializing command system");

    /* Initialize UI feedback */
    if (!ui_feedback_init()) {
        LOG_ERROR("Failed to initialize UI feedback");
        return false;
    }

    /* Create registry */
    g_command_system.registry = command_registry_create();
    if (!g_command_system.registry) {
        LOG_ERROR("Failed to create command registry");
        ui_feedback_shutdown();
        return false;
    }

    /* Set global registry reference */
    g_command_registry = g_command_system.registry;

    /* Register built-in commands */
    int registered = register_builtin_commands(g_command_system.registry);
    LOG_INFO("Registered %d built-in commands", registered);

    /* Create input handler */
    g_command_system.input_handler = input_handler_create(g_command_system.registry);
    if (!g_command_system.input_handler) {
        LOG_ERROR("Failed to create input handler");
        command_registry_destroy(g_command_system.registry);
        g_command_registry = NULL;
        ui_feedback_shutdown();
        return false;
    }

    g_command_system.initialized = true;
    LOG_INFO("Command system initialized successfully");

    return true;
}

void command_system_shutdown(void) {
    if (!g_command_system.initialized) return;

    LOG_INFO("Shutting down command system");

    /* Destroy input handler (saves history) */
    input_handler_destroy(g_command_system.input_handler);
    g_command_system.input_handler = NULL;

    /* Destroy registry */
    command_registry_destroy(g_command_system.registry);
    g_command_system.registry = NULL;
    g_command_registry = NULL;

    /* Shutdown UI feedback */
    ui_feedback_shutdown();

    g_command_system.initialized = false;
    LOG_INFO("Command system shutdown complete");
}

bool command_system_is_initialized(void) {
    return g_command_system.initialized;
}

CommandResult command_system_process_input(const char* prompt) {
    if (!g_command_system.initialized) {
        return command_result_error(EXEC_ERROR_INTERNAL,
                                   "Command system not initialized");
    }

    return input_handler_read_and_execute(g_command_system.input_handler, prompt);
}

CommandResult command_system_execute(const char* input) {
    if (!g_command_system.initialized) {
        return command_result_error(EXEC_ERROR_INTERNAL,
                                   "Command system not initialized");
    }

    return input_handler_execute(g_command_system.input_handler, input);
}

CommandRegistry* command_system_get_registry(void) {
    return g_command_system.registry;
}

CommandHistory* command_system_get_history(void) {
    if (!g_command_system.input_handler) return NULL;
    return input_handler_get_history(g_command_system.input_handler);
}

Autocomplete* command_system_get_autocomplete(void) {
    if (!g_command_system.input_handler) return NULL;
    return input_handler_get_autocomplete(g_command_system.input_handler);
}

InputHandler* command_system_get_input_handler(void) {
    return g_command_system.input_handler;
}

bool command_system_register_command(const CommandInfo* info) {
    if (!g_command_system.initialized || !info) return false;

    bool result = command_registry_register(g_command_system.registry, info);

    if (result) {
        /* Rebuild autocomplete index */
        Autocomplete* ac = command_system_get_autocomplete();
        if (ac) {
            autocomplete_rebuild(ac);
        }
        LOG_INFO("Registered command: %s", info->name);
    }

    return result;
}

bool command_system_unregister_command(const char* name) {
    if (!g_command_system.initialized || !name) return false;

    bool result = command_registry_unregister(g_command_system.registry, name);

    if (result) {
        /* Rebuild autocomplete index */
        Autocomplete* ac = command_system_get_autocomplete();
        if (ac) {
            autocomplete_rebuild(ac);
        }
        LOG_INFO("Unregistered command: %s", name);
    }

    return result;
}

#include "core/state_manager.h"
#include "utils/logger.h"
#include <stdlib.h>
#include <string.h>

/* Maximum state stack depth */
#define MAX_STATE_STACK 16

/* State registration info */
typedef struct {
    StateCallbacks callbacks;
    void* userdata;
    bool registered;
} StateInfo;

/* State stack entry */
typedef struct {
    GameState state;
} StackEntry;

/* State manager structure */
struct StateManager {
    StateInfo states[STATE_COUNT];  /* Registered state info */
    StackEntry stack[MAX_STATE_STACK];  /* State stack */
    size_t stack_depth;
};

/* State names for debugging */
static const char* g_state_names[] = {
    [STATE_NONE] = "NONE",
    [STATE_INIT] = "INIT",
    [STATE_MAIN_MENU] = "MAIN_MENU",
    [STATE_GAME_WORLD] = "GAME_WORLD",
    [STATE_DIALOGUE] = "DIALOGUE",
    [STATE_INVENTORY] = "INVENTORY",
    [STATE_PAUSE] = "PAUSE",
    [STATE_SHUTDOWN] = "SHUTDOWN"
};

const char* state_manager_state_name(GameState state) {
    if (state >= 0 && state < STATE_COUNT) {
        return g_state_names[state];
    }
    return "UNKNOWN";
}

StateManager* state_manager_create(void) {
    StateManager* manager = calloc(1, sizeof(StateManager));
    if (!manager) {
        LOG_ERROR("Failed to allocate state manager");
        return NULL;
    }

    manager->stack_depth = 0;

    LOG_DEBUG("Created state manager");
    return manager;
}

void state_manager_destroy(StateManager* manager) {
    if (!manager) return;

    /* Clear all states (calls exit callbacks) */
    state_manager_clear(manager);

    free(manager);
    LOG_DEBUG("Destroyed state manager");
}

bool state_manager_register(StateManager* manager, GameState state,
                            const StateCallbacks* callbacks, void* userdata) {
    if (!manager || state <= STATE_NONE || state >= STATE_COUNT) {
        LOG_ERROR("Invalid state manager or state");
        return false;
    }

    if (manager->states[state].registered) {
        LOG_WARN("State %s already registered, overwriting", state_manager_state_name(state));
    }

    /* Copy callbacks (NULL callbacks are fine) */
    if (callbacks) {
        manager->states[state].callbacks = *callbacks;
    } else {
        memset(&manager->states[state].callbacks, 0, sizeof(StateCallbacks));
    }

    manager->states[state].userdata = userdata;
    manager->states[state].registered = true;

    LOG_DEBUG("Registered state: %s", state_manager_state_name(state));
    return true;
}

bool state_manager_push(StateManager* manager, GameState state) {
    if (!manager || state <= STATE_NONE || state >= STATE_COUNT) {
        LOG_ERROR("Invalid state manager or state");
        return false;
    }

    if (!manager->states[state].registered) {
        LOG_ERROR("State %s not registered", state_manager_state_name(state));
        return false;
    }

    if (manager->stack_depth >= MAX_STATE_STACK) {
        LOG_ERROR("State stack overflow");
        return false;
    }

    /* Pause current state if any */
    if (manager->stack_depth > 0) {
        GameState current = manager->stack[manager->stack_depth - 1].state;
        StateInfo* info = &manager->states[current];
        if (info->callbacks.on_pause) {
            info->callbacks.on_pause(info->userdata);
        }
        LOG_DEBUG("Paused state: %s", state_manager_state_name(current));
    }

    /* Push new state */
    manager->stack[manager->stack_depth].state = state;
    manager->stack_depth++;

    /* Enter new state */
    StateInfo* info = &manager->states[state];
    if (info->callbacks.on_enter) {
        info->callbacks.on_enter(info->userdata);
    }

    LOG_INFO("Pushed state: %s (depth: %zu)", state_manager_state_name(state), manager->stack_depth);
    return true;
}

bool state_manager_pop(StateManager* manager) {
    if (!manager) {
        LOG_ERROR("Invalid state manager");
        return false;
    }

    if (manager->stack_depth == 0) {
        LOG_WARN("Cannot pop from empty state stack");
        return false;
    }

    /* Exit current state */
    GameState current = manager->stack[manager->stack_depth - 1].state;
    StateInfo* info = &manager->states[current];
    if (info->callbacks.on_exit) {
        info->callbacks.on_exit(info->userdata);
    }

    LOG_DEBUG("Exited state: %s", state_manager_state_name(current));

    /* Pop state */
    manager->stack_depth--;

    /* Resume previous state if any */
    if (manager->stack_depth > 0) {
        GameState previous = manager->stack[manager->stack_depth - 1].state;
        StateInfo* prev_info = &manager->states[previous];
        if (prev_info->callbacks.on_resume) {
            prev_info->callbacks.on_resume(prev_info->userdata);
        }
        LOG_INFO("Popped to state: %s (depth: %zu)", state_manager_state_name(previous), manager->stack_depth);
    } else {
        LOG_INFO("Popped state, stack now empty");
    }

    return true;
}

bool state_manager_change(StateManager* manager, GameState state) {
    if (!manager || state <= STATE_NONE || state >= STATE_COUNT) {
        LOG_ERROR("Invalid state manager or state");
        return false;
    }

    if (!manager->states[state].registered) {
        LOG_ERROR("State %s not registered", state_manager_state_name(state));
        return false;
    }

    /* Exit current state if any */
    if (manager->stack_depth > 0) {
        GameState current = manager->stack[manager->stack_depth - 1].state;
        StateInfo* info = &manager->states[current];
        if (info->callbacks.on_exit) {
            info->callbacks.on_exit(info->userdata);
        }
        LOG_DEBUG("Exited state: %s", state_manager_state_name(current));

        /* Replace top of stack */
        manager->stack[manager->stack_depth - 1].state = state;
    } else {
        /* Empty stack, push new state */
        manager->stack[0].state = state;
        manager->stack_depth = 1;
    }

    /* Enter new state */
    StateInfo* info = &manager->states[state];
    if (info->callbacks.on_enter) {
        info->callbacks.on_enter(info->userdata);
    }

    LOG_INFO("Changed to state: %s", state_manager_state_name(state));
    return true;
}

void state_manager_clear(StateManager* manager) {
    if (!manager) return;

    /* Pop all states (calling exit callbacks) */
    while (manager->stack_depth > 0) {
        state_manager_pop(manager);
    }

    LOG_DEBUG("Cleared state stack");
}

void state_manager_update(StateManager* manager, double delta_time) {
    if (!manager || manager->stack_depth == 0) return;

    /* Update current state */
    GameState current = manager->stack[manager->stack_depth - 1].state;
    StateInfo* info = &manager->states[current];

    if (info->callbacks.on_update) {
        info->callbacks.on_update(delta_time, info->userdata);
    }
}

void state_manager_render(StateManager* manager) {
    if (!manager || manager->stack_depth == 0) return;

    /* Render current state */
    GameState current = manager->stack[manager->stack_depth - 1].state;
    StateInfo* info = &manager->states[current];

    if (info->callbacks.on_render) {
        info->callbacks.on_render(info->userdata);
    }
}

GameState state_manager_current(const StateManager* manager) {
    if (!manager || manager->stack_depth == 0) {
        return STATE_NONE;
    }

    return manager->stack[manager->stack_depth - 1].state;
}

GameState state_manager_previous(const StateManager* manager) {
    if (!manager || manager->stack_depth < 2) {
        return STATE_NONE;
    }

    return manager->stack[manager->stack_depth - 2].state;
}

size_t state_manager_depth(const StateManager* manager) {
    return manager ? manager->stack_depth : 0;
}

bool state_manager_is_registered(const StateManager* manager, GameState state) {
    if (!manager || state <= STATE_NONE || state >= STATE_COUNT) {
        return false;
    }

    return manager->states[state].registered;
}

#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include <stddef.h>
#include <stdbool.h>

/**
 * State Manager - Game State Machine
 *
 * Manages game states and transitions between them.
 * Supports state stack for pause/resume functionality.
 *
 * Usage:
 *   StateManager* sm = state_manager_create();
 *   state_manager_register(sm, STATE_MENU, &menu_callbacks);
 *   state_manager_push(sm, STATE_MENU);
 *   state_manager_update(sm, delta_time);
 *   state_manager_render(sm);
 *   state_manager_destroy(sm);
 */

/* Game states */
typedef enum {
    STATE_NONE = 0,
    STATE_INIT,
    STATE_MAIN_MENU,
    STATE_GAME_WORLD,
    STATE_DIALOGUE,
    STATE_INVENTORY,
    STATE_PAUSE,
    STATE_SHUTDOWN,
    STATE_COUNT
} GameState;

/* Forward declarations */
typedef struct StateManager StateManager;

/* State callbacks */
typedef struct {
    void (*on_enter)(void* userdata);   /* Called when state becomes active */
    void (*on_exit)(void* userdata);    /* Called when state becomes inactive */
    void (*on_update)(double delta, void* userdata);  /* Called every frame when active */
    void (*on_render)(void* userdata);  /* Called for rendering when active */
    void (*on_pause)(void* userdata);   /* Called when state is pushed over */
    void (*on_resume)(void* userdata);  /* Called when state becomes top again */
} StateCallbacks;

/**
 * Create state manager
 *
 * @return State manager pointer or NULL on failure
 */
StateManager* state_manager_create(void);

/**
 * Destroy state manager
 *
 * @param manager State manager to destroy
 */
void state_manager_destroy(StateManager* manager);

/**
 * Register state callbacks
 *
 * @param manager State manager
 * @param state Game state
 * @param callbacks Callback functions (can have NULL callbacks)
 * @param userdata User data passed to callbacks
 * @return true on success
 */
bool state_manager_register(StateManager* manager, GameState state,
                            const StateCallbacks* callbacks, void* userdata);

/**
 * Push a new state onto the stack
 * Pauses current state and activates new state
 *
 * @param manager State manager
 * @param state Game state to push
 * @return true on success
 */
bool state_manager_push(StateManager* manager, GameState state);

/**
 * Pop current state from stack
 * Deactivates current state and resumes previous
 *
 * @param manager State manager
 * @return true on success
 */
bool state_manager_pop(StateManager* manager);

/**
 * Change to a new state (replaces current)
 * Exits current state and enters new state
 *
 * @param manager State manager
 * @param state Game state to change to
 * @return true on success
 */
bool state_manager_change(StateManager* manager, GameState state);

/**
 * Clear all states from stack
 *
 * @param manager State manager
 */
void state_manager_clear(StateManager* manager);

/**
 * Update current state
 *
 * @param manager State manager
 * @param delta_time Time since last update
 */
void state_manager_update(StateManager* manager, double delta_time);

/**
 * Render current state
 *
 * @param manager State manager
 */
void state_manager_render(StateManager* manager);

/**
 * Get current active state
 *
 * @param manager State manager
 * @return Current game state
 */
GameState state_manager_current(const StateManager* manager);

/**
 * Get previous state (if any)
 *
 * @param manager State manager
 * @return Previous game state or STATE_NONE
 */
GameState state_manager_previous(const StateManager* manager);

/**
 * Get stack depth
 *
 * @param manager State manager
 * @return Number of states on stack
 */
size_t state_manager_depth(const StateManager* manager);

/**
 * Check if a state is registered
 *
 * @param manager State manager
 * @param state Game state
 * @return true if registered
 */
bool state_manager_is_registered(const StateManager* manager, GameState state);

/**
 * Get state name (for debugging)
 *
 * @param state Game state
 * @return State name string
 */
const char* state_manager_state_name(GameState state);

#endif /* STATE_MANAGER_H */

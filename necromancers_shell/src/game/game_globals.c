/**
 * Necromancer's Shell - Global Game State
 *
 * This file holds global game state that needs to be accessed
 * from multiple modules. It's separate from main.c so it can
 * be linked into unit tests.
 */

#include "game_state.h"

/* Global game state pointer - initialized in main() or test setup */
GameState* g_game_state = NULL;

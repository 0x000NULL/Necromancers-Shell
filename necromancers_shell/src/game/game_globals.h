/**
 * Necromancer's Shell - Global Game State Header
 *
 * Declares global game state that needs to be accessed
 * from multiple modules.
 */

#ifndef NECROMANCERS_GAME_GLOBALS_H
#define NECROMANCERS_GAME_GLOBALS_H

#include "game_state.h"

/* Global game state pointer - accessible from all modules */
extern GameState* g_game_state;

#endif /* NECROMANCERS_GAME_GLOBALS_H */

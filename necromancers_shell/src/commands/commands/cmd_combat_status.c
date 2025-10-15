/**
 * @file cmd_combat_status.c
 * @brief Combat status command implementation
 */

#include "commands.h"
#include "../executor.h"
#include "../../game/game_state.h"
#include "../../game/combat/combat.h"
#include "../../game/combat/combatant.h"
#include <stdio.h>
#include <string.h>

/* External game state */
extern GameState* g_game_state;

/**
 * @brief Create HP bar visualization
 */
static void format_hp_bar(char* buf, size_t buf_size, uint32_t current, uint32_t max) {
    int bar_width = 10;
    int filled = (int)((float)current / (float)max * bar_width);

    char bar[32] = "[";
    for (int i = 0; i < bar_width; i++) {
        if (i < filled) {
            strcat(bar, "=");
        } else {
            strcat(bar, "-");
        }
    }
    strcat(bar, "]");

    snprintf(buf, buf_size, "%s %u/%u", bar, current, max);
}

/**
 * @brief Execute combat status command
 *
 * Usage: status (during combat)
 */
CommandResult cmd_combat_status(ParsedCommand* cmd) {
    if (!cmd || !g_game_state) {
        return command_result_error(EXEC_ERROR_COMMAND_FAILED, "Invalid state");
    }

    /* Check if in combat */
    if (!g_game_state->combat) {
        return command_result_error(EXEC_ERROR_COMMAND_FAILED, "Not in combat. This version of 'status' is for combat only.");
    }

    CombatState* combat = g_game_state->combat;

    /* Build status output */
    char output[2048];
    char* ptr = output;
    size_t remaining = sizeof(output);
    int written;

    /* Header */
    written = snprintf(ptr, remaining, "\n=== COMBAT STATUS - Turn %u ===\n\n", combat->turn_number);
    ptr += written;
    remaining -= written;

    /* Combat phase */
    const char* phase_name = "Unknown";
    switch (combat->phase) {
        case COMBAT_PHASE_INIT:        phase_name = "Initializing"; break;
        case COMBAT_PHASE_PLAYER_TURN: phase_name = "Player Turn"; break;
        case COMBAT_PHASE_ENEMY_TURN:  phase_name = "Enemy Turn"; break;
        case COMBAT_PHASE_RESOLUTION:  phase_name = "Resolution"; break;
        case COMBAT_PHASE_END:         phase_name = "Ended"; break;
    }
    written = snprintf(ptr, remaining, "Phase: %s\n\n", phase_name);
    ptr += written;
    remaining -= written;

    /* Player forces */
    written = snprintf(ptr, remaining, "YOUR FORCES:\n");
    ptr += written;
    remaining -= written;

    for (uint8_t i = 0; i < combat->player_force_count; i++) {
        Combatant* c = combat->player_forces[i];

        char hp_bar[64];
        format_hp_bar(hp_bar, sizeof(hp_bar), c->health, c->health_max);

        const char* status = "";
        if (!combatant_is_alive(c)) {
            status = " [DEAD]";
        } else if (c->is_defending) {
            status = " [DEFENDING]";
        } else if (c->has_acted_this_turn) {
            status = " [ACTED]";
        }

        written = snprintf(ptr, remaining, "  [%s] %-20s %s%s\n",
            c->id, c->name, hp_bar, status);
        ptr += written;
        remaining -= written;
    }

    written = snprintf(ptr, remaining, "\n");
    ptr += written;
    remaining -= written;

    /* Enemy forces */
    written = snprintf(ptr, remaining, "ENEMIES:\n");
    ptr += written;
    remaining -= written;

    for (uint8_t i = 0; i < combat->enemy_force_count; i++) {
        Combatant* c = combat->enemy_forces[i];

        char hp_bar[64];
        format_hp_bar(hp_bar, sizeof(hp_bar), c->health, c->health_max);

        const char* status = "";
        if (!combatant_is_alive(c)) {
            status = " [DEAD]";
        } else if (c->is_defending) {
            status = " [DEFENDING]";
        }

        written = snprintf(ptr, remaining, "  [%s] %-20s %s%s\n",
            c->id, c->name, hp_bar, status);
        ptr += written;
        remaining -= written;
    }

    written = snprintf(ptr, remaining, "\n");
    ptr += written;
    remaining -= written;

    /* Active combatant */
    if (combat->phase == COMBAT_PHASE_PLAYER_TURN) {
        Combatant* active = combat_get_active_combatant(combat);
        if (active) {
            written = snprintf(ptr, remaining, "Active: [%s] %s\n", active->id, active->name);
            ptr += written;
            remaining -= written;
        }
    }

    /* Available commands */
    if (combat->phase == COMBAT_PHASE_PLAYER_TURN && combat->player_can_act) {
        written = snprintf(ptr, remaining, "\nCommands: attack <target>, defend, flee\n");
        ptr += written;
        remaining -= written;
    }

    /* Recent combat log (last 5 messages) */
    written = snprintf(ptr, remaining, "\n--- Recent Events ---\n");
    ptr += written;
    remaining -= written;

    const char* log_messages[5];
    size_t log_count = combat_get_log_messages(combat, 5, log_messages);

    for (size_t i = 0; i < log_count; i++) {
        written = snprintf(ptr, remaining, "%s\n", log_messages[i]);
        ptr += written;
        remaining -= written;
    }

    return command_result_success(output);
}

/**
 * @file combat_ui.c
 * @brief Combat UI rendering implementation
 */

#include "combat_ui.h"
#include "combat.h"
#include "combatant.h"
#include "../../terminal/colors.h"
#include <stdio.h>
#include <string.h>

/**
 * @brief Get HP bar color based on percentage
 */
const char* combat_ui_get_hp_color(uint32_t current, uint32_t max) {
    if (max == 0) {
        return "\033[0;31m";  /* Red for invalid */
    }

    float percent = (float)current / (float)max;

    if (percent > 0.70f) {
        return "\033[0;32m";  /* Green */
    } else if (percent > 0.30f) {
        return "\033[0;33m";  /* Yellow */
    } else {
        return "\033[0;31m";  /* Red */
    }
}

/**
 * @brief Format health bar
 */
void combat_ui_format_health_bar(char* buf, size_t buf_size,
                                  uint32_t current, uint32_t max, int width) {
    if (!buf || buf_size == 0) {
        return;
    }

    if (width <= 0) {
        width = 20;
    }

    /* Calculate filled portion */
    int filled = 0;
    if (max > 0) {
        filled = (int)((float)current / (float)max * width);
    }

    /* Build bar */
    char bar[128];
    size_t pos = 0;
    bar[pos++] = '[';

    /* Add bar characters with bounds checking */
    for (int i = 0; i < width && i < 60 && pos < sizeof(bar) - 2; i++) {
        bar[pos++] = (i < filled) ? '=' : '-';
    }

    bar[pos++] = ']';
    bar[pos] = '\0';

    /* Calculate percentage */
    int percent = 0;
    if (max > 0) {
        percent = (int)((float)current / (float)max * 100.0f);
    }

    /* Format with color */
    const char* color = combat_ui_get_hp_color(current, max);
    snprintf(buf, buf_size, "%s%s %u/%u (%d%%)\033[0m",
             color, bar, current, max, percent);
}

/**
 * @brief Format single combatant line
 */
void combat_ui_format_combatant(char* buf, size_t buf_size,
                                 const Combatant* combatant) {
    if (!buf || !combatant) {
        return;
    }

    /* Format health bar */
    char hp_bar[256];
    combat_ui_format_health_bar(hp_bar, sizeof(hp_bar),
                                combatant->health, combatant->health_max, 10);

    /* Status indicators */
    char status[128];
    if (!combatant_is_alive(combatant)) {
        snprintf(status, sizeof(status), " [DEAD]");
    } else if (combatant->is_defending) {
        snprintf(status, sizeof(status), " [DEFENDING]");
    } else if (combatant->has_acted_this_turn) {
        snprintf(status, sizeof(status), " [ACTED]");
    } else {
        status[0] = '\0';  /* Empty string if no status */
    }

    /* Format line */
    snprintf(buf, buf_size, "  [%s] %-20s %s%s",
             combatant->id, combatant->name, hp_bar, status);
}

/**
 * @brief Get phase name
 */
const char* combat_ui_phase_name(const CombatState* combat) {
    if (!combat) {
        return "Unknown";
    }

    switch (combat->phase) {
        case COMBAT_PHASE_INIT:        return "Initializing";
        case COMBAT_PHASE_PLAYER_TURN: return "Player Turn";
        case COMBAT_PHASE_ENEMY_TURN:  return "Enemy Turn";
        case COMBAT_PHASE_RESOLUTION:  return "Resolution";
        case COMBAT_PHASE_END:         return "Ended";
        default:                       return "Unknown";
    }
}

/**
 * @brief Render turn order
 */
void combat_ui_render_turn_order(const CombatState* combat) {
    if (!combat) {
        return;
    }

    printf("\n--- Turn Order ---\n");

    for (uint8_t i = 0; i < combat->turn_order_count; i++) {
        const Combatant* c = combat->turn_order[i];
        if (!c) continue;

        /* Indicators */
        char indicator = ' ';
        if (i == combat->current_turn_index) {
            indicator = '>';  /* Active */
        } else if (!combatant_is_alive(c)) {
            indicator = 'X';  /* Dead */
        } else if (c->has_acted_this_turn) {
            indicator = '*';  /* Acted */
        } else {
            indicator = '+';  /* Alive */
        }

        printf("  %c [%s] %s (Init: %u)\n",
               indicator, c->id, c->name, c->initiative);
    }
}

/**
 * @brief Render combat log
 */
void combat_ui_render_log(const CombatState* combat, uint8_t message_count) {
    if (!combat) {
        return;
    }

    if (message_count > 10) {
        message_count = 10;
    }

    printf("\n--- Recent Events ---\n");

    const char* messages[10];
    size_t count = combat_get_log_messages(combat, message_count, messages);

    for (size_t i = 0; i < count; i++) {
        printf("  > %s\n", messages[i]);
    }
}

/**
 * @brief Render available commands
 */
void combat_ui_render_commands(const CombatState* combat) {
    if (!combat) {
        return;
    }

    printf("\n--- Available Commands ---\n");

    if (combat->phase == COMBAT_PHASE_PLAYER_TURN && combat->player_can_act) {
        printf("  attack <target>  - Attack an enemy (e.g., 'attack E1')\n");
        printf("  defend           - Enter defensive stance (+50%% defense)\n");
        printf("  flee             - Attempt to escape combat\n");
        printf("  cast <spell> <target> - Cast a spell (drain, bolt, weaken)\n");
        printf("  status           - Show detailed combat status\n");
    } else {
        printf("  status           - Show detailed combat status\n");
        printf("  (Waiting for turn...)\n");
    }
}

/**
 * @brief Clear screen
 */
void combat_ui_clear_screen(void) {
    /* ANSI escape code to clear screen and move cursor to top */
    printf("\033[2J\033[H");
}

/**
 * @brief Render full combat screen
 */
void combat_ui_render(const CombatState* combat) {
    if (!combat) {
        return;
    }

    /* Header */
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════════════╗\n");
    printf("║  COMBAT - Turn %u                                              ║\n", combat->turn_number);
    printf("║  Phase: %-50s ║\n", combat_ui_phase_name(combat));
    printf("╠═══════════════════════════════════════════════════════════════╣\n");

    /* Player forces */
    printf("║  YOUR FORCES:                                                 ║\n");
    for (uint8_t i = 0; i < combat->player_force_count; i++) {
        char line[256];
        combat_ui_format_combatant(line, sizeof(line), combat->player_forces[i]);
        printf("║  %-60s║\n", line);
    }

    printf("║                                                               ║\n");

    /* Enemy forces */
    printf("║  ENEMIES:                                                     ║\n");
    for (uint8_t i = 0; i < combat->enemy_force_count; i++) {
        char line[256];
        combat_ui_format_combatant(line, sizeof(line), combat->enemy_forces[i]);
        printf("║  %-60s║\n", line);
    }

    printf("╠═══════════════════════════════════════════════════════════════╣\n");

    /* Combat log (last 3 messages) */
    const char* messages[3];
    size_t log_count = combat_get_log_messages(combat, 3, messages);

    for (size_t i = 0; i < 3; i++) {
        if (i < log_count) {
            printf("║  > %-58s║\n", messages[i]);
        } else {
            printf("║  %-60s║\n", "");
        }
    }

    printf("╠═══════════════════════════════════════════════════════════════╣\n");

    /* Active combatant and commands */
    if (combat->phase == COMBAT_PHASE_PLAYER_TURN && combat->player_can_act) {
        Combatant* active = combat_get_active_combatant(combat);
        if (active) {
            printf("║  Active: [%s] %-46s║\n", active->id, active->name);
        }
        printf("║  Commands: attack <target>, defend, flee, cast <spell>    ║\n");
    } else if (combat->phase == COMBAT_PHASE_ENEMY_TURN) {
        printf("║  Enemy turn in progress...                                ║\n");
        printf("║                                                               ║\n");
    } else {
        printf("║                                                               ║\n");
        printf("║                                                               ║\n");
    }

    printf("╚═══════════════════════════════════════════════════════════════╝\n");
    printf("\n");
}

/**
 * @brief Render victory screen
 */
void combat_ui_render_victory(const CombatState* combat) {
    if (!combat) {
        return;
    }

    printf("\n");
    printf("\033[1;32m");  /* Bright green */
    printf("╔═══════════════════════════════════════════════════════════════╗\n");
    printf("║                                                               ║\n");
    printf("║                         VICTORY!                              ║\n");
    printf("║                                                               ║\n");
    printf("║              All enemies have been defeated!                  ║\n");
    printf("║                                                               ║\n");
    printf("╚═══════════════════════════════════════════════════════════════╝\n");
    printf("\033[0m");  /* Reset color */
    printf("\n");

    /* Show final combat log */
    combat_ui_render_log(combat, 5);
    printf("\n");
}

/**
 * @brief Render defeat screen
 */
void combat_ui_render_defeat(const CombatState* combat) {
    if (!combat) {
        return;
    }

    printf("\n");
    printf("\033[1;31m");  /* Bright red */
    printf("╔═══════════════════════════════════════════════════════════════╗\n");
    printf("║                                                               ║\n");
    printf("║                          DEFEAT                               ║\n");
    printf("║                                                               ║\n");
    printf("║              All your forces have fallen...                   ║\n");
    printf("║                                                               ║\n");
    printf("╚═══════════════════════════════════════════════════════════════╝\n");
    printf("\033[0m");  /* Reset color */
    printf("\n");

    /* Show final combat log */
    combat_ui_render_log(combat, 5);
    printf("\n");
}

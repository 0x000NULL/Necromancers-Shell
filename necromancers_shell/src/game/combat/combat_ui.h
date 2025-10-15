/**
 * @file combat_ui.h
 * @brief Combat screen rendering system
 */

#ifndef COMBAT_UI_H
#define COMBAT_UI_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Forward declarations */
typedef struct CombatState CombatState;
typedef struct Combatant Combatant;

/**
 * @brief Render full combat screen
 *
 * Layout:
 * ┌─────────────────────────────────────┐
 * │ COMBAT - Turn 3                     │
 * ├─────────────────────────────────────┤
 * │ YOUR FORCES:                        │
 * │ [M1] Zombie "Groaner"     [45/60]   │
 * │ [M2] Skeleton "Bones"     [30/30]   │
 * │                                     │
 * │ ENEMIES:                            │
 * │ [E1] Guard                [20/60]   │
 * │ [E2] Priest               [55/60]   │
 * ├─────────────────────────────────────┤
 * │ > Zombie attacks Guard for 15 dmg  │
 * │ > Guard defends (+50% defense)     │
 * │ > Priest heals Guard for 10 HP     │
 * ├─────────────────────────────────────┤
 * │ Active: [M1] Zombie "Groaner"       │
 * │ Commands: attack, defend, flee      │
 * └─────────────────────────────────────┘
 *
 * @param combat Combat state to render
 */
void combat_ui_render(const CombatState* combat);

/**
 * @brief Format combatant health bar
 *
 * Format: [==========----------] 45/60 (75%)
 *
 * Colors:
 * - Green: > 70% HP
 * - Yellow: 30-70% HP
 * - Red: < 30% HP
 *
 * @param buf Output buffer
 * @param buf_size Buffer size
 * @param current Current HP
 * @param max Maximum HP
 * @param width Bar width in characters (default 20)
 */
void combat_ui_format_health_bar(char* buf, size_t buf_size,
                                  uint32_t current, uint32_t max, int width);

/**
 * @brief Render turn order display
 *
 * Shows initiative order with indicators:
 * - > Current active combatant
 * - * Has acted this turn
 * - + Alive
 * - X Dead
 *
 * @param combat Combat state
 */
void combat_ui_render_turn_order(const CombatState* combat);

/**
 * @brief Render combat log (last N messages)
 *
 * @param combat Combat state
 * @param message_count Number of messages to show (max 10)
 */
void combat_ui_render_log(const CombatState* combat, uint8_t message_count);

/**
 * @brief Render available commands for current phase
 *
 * @param combat Combat state
 */
void combat_ui_render_commands(const CombatState* combat);

/**
 * @brief Render single combatant line
 *
 * Format: [ID] Name          [HP] Status
 *
 * @param buf Output buffer
 * @param buf_size Buffer size
 * @param combatant Combatant to render
 */
void combat_ui_format_combatant(char* buf, size_t buf_size,
                                 const Combatant* combatant);

/**
 * @brief Get HP bar color based on percentage
 *
 * @param current Current HP
 * @param max Maximum HP
 * @return ANSI color code (green, yellow, or red)
 */
const char* combat_ui_get_hp_color(uint32_t current, uint32_t max);

/**
 * @brief Render combat phase name
 *
 * @param combat Combat state
 * @return Human-readable phase name
 */
const char* combat_ui_phase_name(const CombatState* combat);

/**
 * @brief Clear and prepare combat screen
 *
 * Clears terminal and sets up for combat rendering
 */
void combat_ui_clear_screen(void);

/**
 * @brief Render combat victory screen
 *
 * @param combat Combat state
 */
void combat_ui_render_victory(const CombatState* combat);

/**
 * @brief Render combat defeat screen
 *
 * @param combat Combat state
 */
void combat_ui_render_defeat(const CombatState* combat);

#endif /* COMBAT_UI_H */

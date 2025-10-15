#include "combat.h"
#include "combat_rewards.h"
#include "../game_state.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

/* External game state */
extern GameState* g_game_state;

/* Helper: Compare combatants by initiative (for qsort) */
static int compare_initiative(const void* a, const void* b) {
    const Combatant* ca = *(const Combatant**)a;
    const Combatant* cb = *(const Combatant**)b;

    /* Higher initiative goes first */
    if (ca->initiative > cb->initiative) return -1;
    if (ca->initiative < cb->initiative) return 1;
    return 0;
}

CombatState* combat_state_create(void) {
    CombatState* combat = calloc(1, sizeof(CombatState));
    if (!combat) {
        return NULL;
    }

    combat->phase = COMBAT_PHASE_INIT;
    combat->turn_number = 0;
    combat->player_force_count = 0;
    combat->enemy_force_count = 0;
    combat->turn_order_count = 0;
    combat->current_turn_index = 0;
    combat->log_count = 0;
    combat->log_head = 0;
    combat->combat_start_time = 0;
    combat->outcome = COMBAT_OUTCOME_NONE;
    combat->player_can_act = false;

    return combat;
}

void combat_state_destroy(CombatState* combat) {
    if (!combat) {
        return;
    }

    /* Free all player combatants */
    for (uint8_t i = 0; i < combat->player_force_count; i++) {
        combatant_destroy(combat->player_forces[i]);
    }

    /* Free all enemy combatants */
    for (uint8_t i = 0; i < combat->enemy_force_count; i++) {
        combatant_destroy(combat->enemy_forces[i]);
    }

    free(combat);
}

bool combat_add_player_combatant(CombatState* combat, Combatant* combatant) {
    if (!combat || !combatant) {
        return false;
    }

    if (combat->player_force_count >= COMBAT_MAX_COMBATANTS) {
        return false;
    }

    combat->player_forces[combat->player_force_count++] = combatant;
    return true;
}

bool combat_add_enemy_combatant(CombatState* combat, Combatant* combatant) {
    if (!combat || !combatant) {
        return false;
    }

    if (combat->enemy_force_count >= COMBAT_MAX_COMBATANTS) {
        return false;
    }

    combat->enemy_forces[combat->enemy_force_count++] = combatant;
    return true;
}

bool combat_initialize(CombatState* combat) {
    if (!combat) {
        return false;
    }

    /* Roll initiative for all combatants */
    for (uint8_t i = 0; i < combat->player_force_count; i++) {
        combatant_roll_initiative(combat->player_forces[i]);
    }

    for (uint8_t i = 0; i < combat->enemy_force_count; i++) {
        combatant_roll_initiative(combat->enemy_forces[i]);
    }

    /* Calculate turn order */
    combat_calculate_turn_order(combat);

    /* Start combat */
    combat->turn_number = 1;
    combat->phase = COMBAT_PHASE_PLAYER_TURN;
    combat->current_turn_index = 0;
    combat->player_can_act = true;

    combat_log_message(combat, "=== COMBAT START ===");
    combat_log_message(combat, "Turn %u begins", combat->turn_number);

    return true;
}

void combat_calculate_turn_order(CombatState* combat) {
    if (!combat) {
        return;
    }

    /* Build combined array of all combatants */
    combat->turn_order_count = 0;

    for (uint8_t i = 0; i < combat->player_force_count; i++) {
        combat->turn_order[combat->turn_order_count++] = combat->player_forces[i];
    }

    for (uint8_t i = 0; i < combat->enemy_force_count; i++) {
        combat->turn_order[combat->turn_order_count++] = combat->enemy_forces[i];
    }

    /* Sort by initiative (highest first) */
    qsort(combat->turn_order, combat->turn_order_count, sizeof(Combatant*), compare_initiative);
}

void combat_update(CombatState* combat, double delta_time) {
    (void)delta_time;  /* Not used in turn-based combat */

    if (!combat) {
        return;
    }

    switch (combat->phase) {
        case COMBAT_PHASE_INIT:
            /* Should call combat_initialize() before updating */
            break;

        case COMBAT_PHASE_PLAYER_TURN:
            /* Wait for player command - do nothing */
            /* Commands will call combat_advance_turn() when done */
            break;

        case COMBAT_PHASE_ENEMY_TURN:
            /* Process AI for current combatant */
            combat_process_ai_turn(combat);
            combat_advance_turn(combat);
            break;

        case COMBAT_PHASE_RESOLUTION:
            /* Check victory/defeat conditions */
            if (combat_check_victory(combat)) {
                combat_end(combat, COMBAT_OUTCOME_VICTORY);
            } else if (combat_check_defeat(combat)) {
                combat_end(combat, COMBAT_OUTCOME_DEFEAT);
            } else {
                /* Start new turn */
                combat->turn_number++;
                combat->current_turn_index = 0;

                /* Reset all combatant turn flags */
                for (uint8_t i = 0; i < combat->turn_order_count; i++) {
                    combatant_reset_turn_flags(combat->turn_order[i]);
                }

                combat_log_message(combat, "--- Turn %u ---", combat->turn_number);
                combat->phase = COMBAT_PHASE_PLAYER_TURN;
                combat->player_can_act = true;
            }
            break;

        case COMBAT_PHASE_END:
            /* Combat is over - do nothing */
            break;
    }
}

void combat_advance_turn(CombatState* combat) {
    if (!combat) {
        return;
    }

    /* Mark current combatant as having acted */
    if (combat->current_turn_index < combat->turn_order_count) {
        Combatant* current = combat->turn_order[combat->current_turn_index];
        current->has_acted_this_turn = true;
    }

    /* Move to next combatant */
    combat->current_turn_index++;

    /* Check if we've processed all combatants */
    if (combat->current_turn_index >= combat->turn_order_count) {
        /* All combatants have acted - move to resolution phase */
        combat->phase = COMBAT_PHASE_RESOLUTION;
        combat->player_can_act = false;
        return;
    }

    /* Find next living combatant */
    while (combat->current_turn_index < combat->turn_order_count) {
        Combatant* next = combat->turn_order[combat->current_turn_index];

        if (!combatant_is_alive(next)) {
            /* Skip dead combatants */
            combat->current_turn_index++;
            continue;
        }

        /* Found next living combatant */
        if (next->is_player_controlled) {
            combat->phase = COMBAT_PHASE_PLAYER_TURN;
            combat->player_can_act = true;
        } else {
            combat->phase = COMBAT_PHASE_ENEMY_TURN;
            combat->player_can_act = false;
        }
        return;
    }

    /* All remaining combatants are dead - go to resolution */
    combat->phase = COMBAT_PHASE_RESOLUTION;
    combat->player_can_act = false;
}

bool combat_check_victory(const CombatState* combat) {
    if (!combat) {
        return false;
    }

    return combat_count_living_enemy_forces(combat) == 0;
}

bool combat_check_defeat(const CombatState* combat) {
    if (!combat) {
        return false;
    }

    return combat_count_living_player_forces(combat) == 0;
}

void combat_log_message(CombatState* combat, const char* format, ...) {
    if (!combat || !format) {
        return;
    }

    va_list args;
    va_start(args, format);

    /* Write to circular buffer */
    vsnprintf(combat->log_messages[combat->log_head],
              COMBAT_LOG_MESSAGE_SIZE,
              format,
              args);

    va_end(args);

    /* Advance head */
    combat->log_head = (combat->log_head + 1) % COMBAT_LOG_SIZE;

    /* Track total count (capped at buffer size) */
    if (combat->log_count < COMBAT_LOG_SIZE) {
        combat->log_count++;
    }
}

size_t combat_get_log_messages(const CombatState* combat, size_t count, const char** out_messages) {
    if (!combat || !out_messages || count == 0) {
        return 0;
    }

    /* Cap at available messages */
    if (count > combat->log_count) {
        count = combat->log_count;
    }

    /* Get messages in reverse order (most recent first) */
    for (size_t i = 0; i < count; i++) {
        /* Calculate index (wrap around) */
        int index = (int)combat->log_head - 1 - (int)i;
        if (index < 0) {
            index += COMBAT_LOG_SIZE;
        }

        out_messages[i] = combat->log_messages[index];
    }

    return count;
}

Combatant* combat_find_combatant(const CombatState* combat, const char* id) {
    if (!combat || !id) {
        return NULL;
    }

    /* Search player forces */
    for (uint8_t i = 0; i < combat->player_force_count; i++) {
        if (strcmp(combat->player_forces[i]->id, id) == 0) {
            return combat->player_forces[i];
        }
    }

    /* Search enemy forces */
    for (uint8_t i = 0; i < combat->enemy_force_count; i++) {
        if (strcmp(combat->enemy_forces[i]->id, id) == 0) {
            return combat->enemy_forces[i];
        }
    }

    return NULL;
}

Combatant* combat_get_active_combatant(const CombatState* combat) {
    if (!combat || combat->current_turn_index >= combat->turn_order_count) {
        return NULL;
    }

    return combat->turn_order[combat->current_turn_index];
}

uint8_t combat_count_living_player_forces(const CombatState* combat) {
    if (!combat) {
        return 0;
    }

    uint8_t count = 0;
    for (uint8_t i = 0; i < combat->player_force_count; i++) {
        if (combatant_is_alive(combat->player_forces[i])) {
            count++;
        }
    }

    return count;
}

uint8_t combat_count_living_enemy_forces(const CombatState* combat) {
    if (!combat) {
        return 0;
    }

    uint8_t count = 0;
    for (uint8_t i = 0; i < combat->enemy_force_count; i++) {
        if (combatant_is_alive(combat->enemy_forces[i])) {
            count++;
        }
    }

    return count;
}

void combat_process_ai_turn(CombatState* combat) {
    if (!combat) {
        return;
    }

    Combatant* current = combat_get_active_combatant(combat);
    if (!current || !current->ai_func) {
        return;
    }

    /* Call AI function */
    current->ai_func(current, combat);
}

void combat_end(CombatState* combat, CombatOutcome outcome) {
    if (!combat) {
        return;
    }

    combat->phase = COMBAT_PHASE_END;
    combat->outcome = outcome;
    combat->player_can_act = false;

    /* Log outcome */
    switch (outcome) {
        case COMBAT_OUTCOME_VICTORY:
            combat_log_message(combat, "=== VICTORY ===");

            /* Calculate and apply rewards */
            if (g_game_state) {
                CombatRewards* rewards = combat_calculate_rewards(combat);
                if (rewards) {
                    /* Apply rewards to game state */
                    combat_apply_rewards(rewards, g_game_state);

                    /* Log rewards */
                    char reward_msg[512];
                    combat_rewards_format(rewards, reward_msg, sizeof(reward_msg));
                    combat_log_message(combat, "%s", reward_msg);

                    /* Also log summary */
                    combat_log_message(combat, "Gained %u XP and %u soul energy",
                        rewards->experience_total, rewards->soul_energy_gained);

                    if (rewards->soul_count > 0) {
                        combat_log_message(combat, "Harvested %u souls from the fallen",
                            rewards->soul_count);
                    }

                    combat_rewards_destroy(rewards);
                }
            }
            break;

        case COMBAT_OUTCOME_DEFEAT:
            combat_log_message(combat, "=== DEFEAT ===");
            combat_log_message(combat, "Your forces have been defeated.");
            /* TODO: Apply penalties in future */
            break;

        case COMBAT_OUTCOME_FLED:
            combat_log_message(combat, "=== FLED ===");
            combat_log_message(combat, "You escaped from combat.");
            break;

        default:
            break;
    }
}

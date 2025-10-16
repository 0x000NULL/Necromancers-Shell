/**
 * @file divine_summons_event.c
 * @brief Divine summons event implementation
 */

#include "divine_summons_event.h"
#include "event_scheduler.h"
#include "../game_state.h"
#include "../../utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Forward declaration for archon trial functions */
extern bool archon_trial_activate_path(void* manager, uint8_t corruption, float consciousness);

#define SUMMONS_EVENT_ID 155
#define SUMMONS_TRIGGER_DAY 155
#define SUMMONS_DEADLINE_DAYS 7

/* Global state */
static DivineSummonsEvent g_divine_summons = {
    .state = SUMMONS_NOT_RECEIVED,
    .trigger_day = 155,
    .event_registered = false,
    .trials_unlocked = false,
    .response_deadline = 162
};

bool divine_summons_event_callback(GameState* state, uint32_t event_id) {
    if (!state) {
        LOG_ERROR("divine_summons_event_callback: NULL state");
        return false;
    }

    (void)event_id;

    LOG_INFO("=== DIVINE SUMMONS EVENT (Day %u) ===", state->resources.day_count);

    printf("\n");
    printf("═══════════════════════════════════════════════════════════\n");
    printf("           SUMMONS FROM THE DIVINE COUNCIL\n");
    printf("═══════════════════════════════════════════════════════════\n");
    printf("\n");
    printf("The Death Network shudders. Every soul in the queue pauses.\n");
    printf("Something ancient has taken notice of you.\n");
    printf("\n");
    printf("A presence manifests—not a message, but a command etched\n");
    printf("directly into your consciousness. Seven voices speaking as one:\n");
    printf("\n");
    printf("  \"ADMINISTRATOR. YOUR ACTIONS HAVE BEEN OBSERVED.\"\n");
    printf("\n");
    printf("  \"YOU HAVE VIOLATED THE NATURAL ORDER. RAISED THE DEAD.\n");
    printf("   DISRUPTED THE FLOW. CLAIMED POWER NOT MEANT FOR MORTALS.\"\n");
    printf("\n");
    printf("  \"YET... YOU HAVE ALSO SHOWN RESTRAINT. QUESTIONING.\n");
    printf("   A DESIRE TO UNDERSTAND RATHER THAN MERELY CONSUME.\"\n");
    printf("\n");
    printf("  \"THE SEVEN ARCHITECTS SUMMON YOU TO STAND JUDGMENT.\"\n");
    printf("\n");
    printf("  \"DAY 162. NULL SPACE COORDINATES: DIVINE THRESHOLD.\n");
    printf("   COME ALONE. PREPARED TO DEFEND YOUR EXISTENCE.\n");
    printf("   OR FACE THE FOURTH PURGE UNPREPARED.\"\n");
    printf("\n");
    printf("  \"THIS IS NOT A REQUEST.\"\n");
    printf("\n");
    printf("  \"- Keldrin, Voice of Divine Judgment\"\n");
    printf("\n");
    printf("═══════════════════════════════════════════════════════════\n");
    printf("\n");
    printf("Seven days to respond. Seven trials to prove yourself.\n");
    printf("The Archon path—if you dare to walk it.\n");
    printf("\n");
    printf("Use 'invoke divine_council' to acknowledge the summons.\n");
    printf("Or ignore it, and face the consequences.\n");
    printf("\n");

    /* Set summons deadline */
    g_divine_summons.response_deadline = state->resources.day_count + SUMMONS_DEADLINE_DAYS;
    g_divine_summons.state = SUMMONS_RECEIVED;

    /* Set event flag for quest triggers */
    if (state->event_scheduler) {
        event_scheduler_set_flag(state->event_scheduler, "divine_summons_received");
        LOG_INFO("Set flag: divine_summons_received");
    }

    /* Log the summons */
    LOG_INFO("Divine Council summoned player (deadline: Day %u)", g_divine_summons.response_deadline);

    return true;
}

bool divine_summons_register_event(EventScheduler* scheduler, GameState* state) {
    if (!scheduler || !state) {
        LOG_ERROR("divine_summons_register_event: NULL scheduler or state");
        return false;
    }

    if (g_divine_summons.event_registered) {
        LOG_WARN("Divine summons event already registered");
        return false;
    }

    ScheduledEvent event = {
        .id = SUMMONS_EVENT_ID,
        .trigger_type = EVENT_TRIGGER_DAY,
        .trigger_value = SUMMONS_TRIGGER_DAY,
        .triggered = false,
        .completed = false,
        .repeatable = false,
        .priority = EVENT_PRIORITY_CRITICAL,
        .callback = divine_summons_event_callback,
        .requires_flag = true,
        .min_day = 155,
        .max_day = 0
    };

    snprintf(event.name, sizeof(event.name), "%s", "Divine Council Summons");
    snprintf(event.description, sizeof(event.description), "%s",
             "The Seven Architects call you to judgment");
    snprintf(event.required_flag, sizeof(event.required_flag), "%s", "thessara_paths_revealed");

    bool success = event_scheduler_register(scheduler, event);
    if (success) {
        g_divine_summons.event_registered = true;
        LOG_INFO("Divine summons event registered for Day 155 (requires: thessara_paths_revealed)");
    } else {
        LOG_ERROR("Failed to register Divine summons event");
    }

    return success;
}

bool divine_summons_acknowledge(GameState* state) {
    if (!state) {
        LOG_ERROR("divine_summons_acknowledge: NULL state");
        return false;
    }

    if (g_divine_summons.state != SUMMONS_RECEIVED) {
        printf("You have not been summoned by the Divine Council yet.\n");
        return false;
    }

    /* Check if deadline passed */
    if (state->resources.day_count > g_divine_summons.response_deadline) {
        printf("\n");
        printf("You have ignored the Divine Council's summons.\n");
        printf("The deadline has passed. The Archon path is now closed.\n");
        printf("\n");
        g_divine_summons.state = SUMMONS_IGNORED;

        if (state->event_scheduler) {
            event_scheduler_set_flag(state->event_scheduler, "divine_summons_ignored");
        }

        return false;
    }

    printf("\n");
    printf("═══════════════════════════════════════════════════════════\n");
    printf("         ACKNOWLEDGING THE DIVINE SUMMONS\n");
    printf("═══════════════════════════════════════════════════════════\n");
    printf("\n");
    printf("You reach out through the Death Network, directing your\n");
    printf("consciousness toward the divine signatures.\n");
    printf("\n");
    printf("YOUR VOICE: \"I acknowledge the summons. I will stand before\n");
    printf("            the Seven Architects and face judgment.\"\n");
    printf("\n");
    printf("A response echoes back—Keldrin's voice, cold and precise:\n");
    printf("\n");
    printf("KELDRIN: \"So be it. The Seven Trials will test your worthiness.\n");
    printf("         Pass them all, and you may earn our amnesty.\n");
    printf("         Fail, and the Fourth Purge will claim you with the rest.\n");
    printf("\n");
    printf("         The first trial begins now. Prove your POWER.\n");
    printf("         You will face Seraphim, our enforcer, in single combat.\n");
    printf("         Show us you have the strength to reshape reality—\n");
    printf("         and the mercy to wield it wisely.\"\n");
    printf("\n");
    printf("═══════════════════════════════════════════════════════════\n");
    printf("\n");
    printf("TRIAL 1 UNLOCKED: Test of Power\n");
    printf("Use 'ritual archon_trial 1' to begin the first trial.\n");
    printf("\n");

    g_divine_summons.state = SUMMONS_ACKNOWLEDGED;
    g_divine_summons.trials_unlocked = true;

    /* Unlock Trial 1 in archon trial system */
    if (state->archon_trials) {
        archon_trial_activate_path(state->archon_trials,
                                   state->corruption.corruption,
                                   state->consciousness.stability);
        LOG_INFO("Archon trial path activated (Trial 1 unlocked)");
    } else {
        LOG_WARN("Archon trial system not initialized");
    }

    /* Set flags */
    if (state->event_scheduler) {
        event_scheduler_set_flag(state->event_scheduler, "divine_summons_acknowledged");
        event_scheduler_set_flag(state->event_scheduler, "trial_1_unlocked");
        LOG_INFO("Set flags: divine_summons_acknowledged, trial_1_unlocked");
    }

    LOG_INFO("Player acknowledged Divine summons (Day %u, deadline was Day %u)",
             state->resources.day_count, g_divine_summons.response_deadline);

    return true;
}

bool divine_summons_is_ignored(const GameState* state) {
    if (!state) {
        return false;
    }

    /* Check if deadline passed without acknowledgment */
    if (g_divine_summons.state == SUMMONS_RECEIVED &&
        state->resources.day_count > g_divine_summons.response_deadline) {
        return true;
    }

    return g_divine_summons.state == SUMMONS_IGNORED;
}

DivineSummonsState divine_summons_get_state(const GameState* state) {
    (void)state;
    return g_divine_summons.state;
}

bool divine_summons_was_received(const GameState* state) {
    (void)state;
    return g_divine_summons.state != SUMMONS_NOT_RECEIVED;
}

bool divine_summons_trials_unlocked(const GameState* state) {
    (void)state;
    return g_divine_summons.trials_unlocked;
}

void divine_summons_reset_for_testing(void) {
    g_divine_summons.state = SUMMONS_NOT_RECEIVED;
    g_divine_summons.trigger_day = 155;
    g_divine_summons.event_registered = false;
    g_divine_summons.trials_unlocked = false;
    g_divine_summons.response_deadline = 162;
}

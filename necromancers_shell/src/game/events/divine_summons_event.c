/**
 * @file divine_summons_event.c
 * @brief Divine summons event implementation
 */

#include "divine_summons_event.h"
#include "event_scheduler.h"
#include "../game_state.h"
#include "../ui/story_ui.h"
#include "../../terminal/platform_curses.h"
#include "../../terminal/colors.h"
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

    /* Create full-screen window for the event */
    WINDOW* event_win = newwin(35, 100, 0, 0);
    if (!event_win) {
        /* Running in non-interactive mode (tests) - use printf fallback */
        LOG_WARN("No terminal available, running Divine Summons in non-interactive mode");

        printf("\n=== DIVINE SUMMONS (Day %u) ===\n", state->resources.day_count);
        printf("The Divine Council has summoned you to stand judgment.\n");
        printf("Deadline: Day %u. Use 'invoke divine_council' to acknowledge.\n\n",
               state->resources.day_count + SUMMONS_DEADLINE_DAYS);

        g_divine_summons.response_deadline = state->resources.day_count + SUMMONS_DEADLINE_DAYS;
        g_divine_summons.state = SUMMONS_RECEIVED;

        if (state->event_scheduler) {
            event_scheduler_set_flag(state->event_scheduler, "divine_summons_received");
        }

        return true;
    }

    /* Display the summons scene */
    const char* scene_title = "SUMMONS FROM THE DIVINE COUNCIL";
    const char* paragraphs[] = {
        "The Death Network SHUDDERS. Every soul in the queue pauses. The routing protocols freeze mid-execution.",

        "Something ancient has taken notice of you.",

        "A presence manifests—not a message, but a COMMAND etched directly into your consciousness. Seven voices speaking as one, each distinct yet unified:",

        "\"ADMINISTRATOR. YOUR ACTIONS HAVE BEEN OBSERVED.\"",

        "\"YOU HAVE VIOLATED THE NATURAL ORDER. RAISED THE DEAD. DISRUPTED THE FLOW. CLAIMED POWER NOT MEANT FOR MORTALS.\"",

        "\"YET... YOU HAVE ALSO SHOWN RESTRAINT. QUESTIONING. A DESIRE TO UNDERSTAND RATHER THAN MERELY CONSUME.\"",

        "\"THE SEVEN ARCHITECTS SUMMON YOU TO STAND JUDGMENT.\"",

        "\"DAY 162. NULL SPACE COORDINATES: DIVINE THRESHOLD. COME ALONE. PREPARED TO DEFEND YOUR EXISTENCE.\"",

        "\"OR FACE THE FOURTH PURGE UNPREPARED.\"",

        "\"THIS IS NOT A REQUEST.\"",

        "\"— Keldrin, Voice of Divine Judgment\""
    };

    display_narrative_scene(event_win, scene_title, paragraphs, 12, SCENE_COLOR_WARNING);

    /* Display deadline and instructions */
    int info_line = 28;
    wattron(event_win, COLOR_PAIR(TEXT_ERROR) | A_BOLD);
    mvwprintw(event_win, info_line, 2, "DEADLINE: Day %u (Seven days to respond)",
              state->resources.day_count + SUMMONS_DEADLINE_DAYS);
    wattroff(event_win, COLOR_PAIR(TEXT_ERROR) | A_BOLD);

    wattron(event_win, COLOR_PAIR(TEXT_INFO));
    mvwprintw(event_win, info_line + 2, 2, "The Archon path—if you dare to walk it.");
    mvwprintw(event_win, info_line + 3, 2, "Use 'invoke divine_council' to acknowledge the summons.");
    mvwprintw(event_win, info_line + 4, 2, "Or ignore it, and face the consequences.");
    wattroff(event_win, COLOR_PAIR(TEXT_INFO));

    wait_for_keypress(event_win, info_line + 6);

    delwin(event_win);

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
        /* Create window for deadline failure message */
        WINDOW* fail_win = newwin(15, 80, 5, 10);
        if (fail_win) {
            const char* failure_title = "DEADLINE PASSED";
            const char* failure_text[] = {
                "You have ignored the Divine Council's summons.",
                "The deadline has passed. The Archon path is now closed.",
                "The Fourth Purge will proceed as planned."
            };
            display_narrative_scene(fail_win, failure_title, failure_text, 3, SCENE_COLOR_WARNING);
            wait_for_keypress(fail_win, 10);
            delwin(fail_win);
        } else {
            printf("\nYou have ignored the Divine Council's summons.\n");
            printf("The deadline has passed. The Archon path is now closed.\n\n");
        }

        g_divine_summons.state = SUMMONS_IGNORED;

        if (state->event_scheduler) {
            event_scheduler_set_flag(state->event_scheduler, "divine_summons_ignored");
        }

        return false;
    }

    /* Create full-screen window for acknowledgment */
    WINDOW* ack_win = newwin(30, 100, 0, 0);
    if (!ack_win) {
        /* Fallback to printf */
        printf("\n=== ACKNOWLEDGING THE DIVINE SUMMONS ===\n");
        printf("You acknowledge the summons and accept the Seven Trials.\n");
        printf("TRIAL 1 UNLOCKED: Test of Power\n\n");

        g_divine_summons.state = SUMMONS_ACKNOWLEDGED;
        g_divine_summons.trials_unlocked = true;

        if (state->archon_trials) {
            archon_trial_activate_path(state->archon_trials,
                                       state->corruption.corruption,
                                       state->consciousness.stability);
        }

        if (state->event_scheduler) {
            event_scheduler_set_flag(state->event_scheduler, "divine_summons_acknowledged");
            event_scheduler_set_flag(state->event_scheduler, "trial_1_unlocked");
        }

        return true;
    }

    /* Display acknowledgment scene */
    const char* ack_title = "ACKNOWLEDGING THE DIVINE SUMMONS";
    const char* ack_paragraphs[] = {
        "You reach out through the Death Network, directing your consciousness toward the divine signatures that summoned you.",

        "YOUR VOICE: \"I acknowledge the summons. I will stand before the Seven Architects and face judgment.\"",

        "A response echoes back—Keldrin's voice, cold and precise:",

        "KELDRIN: \"So be it. The Seven Trials will test your worthiness. Pass them all, and you may earn our amnesty.\"",

        "\"Fail, and the Fourth Purge will claim you with the rest.\"",

        "\"The first trial begins now. Prove your POWER.\"",

        "\"You will face Seraphim, our enforcer, in single combat. Show us you have the strength to reshape reality—and the mercy to wield it wisely.\""
    };

    display_narrative_scene(ack_win, ack_title, ack_paragraphs, 7, SCENE_COLOR_SUCCESS);

    /* Display trial unlock info */
    int info_line = 22;
    wattron(ack_win, COLOR_PAIR(TEXT_SUCCESS) | A_BOLD);
    mvwprintw(ack_win, info_line, 2, "TRIAL 1 UNLOCKED: Test of Power");
    wattroff(ack_win, COLOR_PAIR(TEXT_SUCCESS) | A_BOLD);

    wattron(ack_win, COLOR_PAIR(TEXT_INFO));
    mvwprintw(ack_win, info_line + 2, 2, "Use 'ritual archon_trial 1' to begin the first trial.");
    wattroff(ack_win, COLOR_PAIR(TEXT_INFO));

    wait_for_keypress(ack_win, info_line + 4);

    delwin(ack_win);

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

/**
 * @file thessara_contact_event.c
 * @brief Thessara contact event implementation
 */

#include "thessara_contact_event.h"
#include "event_scheduler.h"
#include "../game_state.h"
#include "../narrative/thessara/thessara.h"
#include "../world/null_space.h"
#include "../ui/story_ui.h"
#include "../../terminal/platform_curses.h"
#include "../../terminal/colors.h"
#include "../../utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define THESSARA_EVENT_ID 50
#define THESSARA_TRIGGER_DAY 50

/* Global state */
static ThessaraContactEvent g_thessara_contact = {
    .state = THESSARA_NOT_CONTACTED,
    .trigger_day = 50,
    .event_registered = false,
    .null_space_discovered = false,
    .trust_level = 0
};

bool thessara_contact_event_callback(GameState* state, uint32_t event_id) {
    if (!state) {
        LOG_ERROR("thessara_contact_event_callback: NULL state");
        return false;
    }

    (void)event_id;

    LOG_INFO("=== THESSARA CONTACT EVENT (Day %u) ===", state->resources.day_count);

    /* Create full-screen window for the event */
    WINDOW* event_win = newwin(30, 100, 0, 0);
    if (!event_win) {
        /* Running in non-interactive mode (tests) - skip UI, just process the event */
        LOG_WARN("No terminal available, running Thessara contact in non-interactive mode");

        /* Process the event without UI */
        if (state->null_space) {
            null_space_discover(state->null_space, state->resources.day_count);
            g_thessara_contact.null_space_discovered = true;
            LOG_INFO("Null space discovered on Day %u", state->resources.day_count);
        }

        if (state->event_scheduler) {
            event_scheduler_set_flag(state->event_scheduler, "thessara_contacted");
            LOG_INFO("Set flag: thessara_contacted");
        }

        if (state->thessara) {
            thessara_discover(state->thessara, state->resources.day_count);
            LOG_INFO("Thessara discovered in game state on Day %u", state->resources.day_count);
        }

        g_thessara_contact.state = THESSARA_CONTACTED;
        return true;
    }

    /* Display the initial message */
    const char* scene_title = "MESSAGE FROM THE NETWORK - DAY 50";
    const char* paragraphs[] = {
        "You sense a presence in the Death Network... different from the usual routing signatures. Someone is watching you.",

        "A message appears in your consciousness:",

        "\"I saw what you did at Ashbrook. I saw what you're becoming. And I need to talk to you before it's too late.\"",

        "\"Connect to null space. Come alone. Don't bring minions. Don't tell anyone.\"",

        "\"I can help you understand what you really are.\"",

        "\"- Thessara\"",

        "Thessara... the first necromancer. She died 3,000 years ago. This is impossible."
    };

    display_narrative_scene(event_win, scene_title, paragraphs, 7, SCENE_COLOR_WARNING);

    /* Add spacing */
    int info_line = 24;

    /* Display discovery notification */
    wattron(event_win, COLOR_PAIR(TEXT_SUCCESS));
    mvwprintw(event_win, info_line, 2, "Location discovered: null_space");
    mvwprintw(event_win, info_line + 1, 2, "Use 'connect null_space' to find her");
    wattroff(event_win, COLOR_PAIR(TEXT_SUCCESS));

    wait_for_keypress(event_win, 27);
    delwin(event_win);

    /* Discover null space location */
    if (state->null_space) {
        null_space_discover(state->null_space, state->resources.day_count);
        g_thessara_contact.null_space_discovered = true;
        LOG_INFO("Null space discovered on Day %u", state->resources.day_count);
    } else {
        LOG_WARN("Null space system not initialized");
    }

    /* Set event flag for quest triggers */
    if (state->event_scheduler) {
        event_scheduler_set_flag(state->event_scheduler, "thessara_contacted");
        LOG_INFO("Set flag: thessara_contacted");
    }

    /* Update Thessara relationship state */
    if (state->thessara) {
        thessara_discover(state->thessara, state->resources.day_count);
        LOG_INFO("Thessara discovered in game state on Day %u", state->resources.day_count);
    } else {
        LOG_WARN("Thessara system not initialized");
    }

    g_thessara_contact.state = THESSARA_CONTACTED;

    return true;
}

bool thessara_contact_register_event(EventScheduler* scheduler, GameState* state) {
    if (!scheduler || !state) {
        LOG_ERROR("thessara_contact_register_event: NULL scheduler or state");
        return false;
    }

    if (g_thessara_contact.event_registered) {
        LOG_WARN("Thessara contact event already registered");
        return false;
    }

    ScheduledEvent event = {
        .id = THESSARA_EVENT_ID,
        .trigger_type = EVENT_TRIGGER_DAY,
        .trigger_value = THESSARA_TRIGGER_DAY,
        .triggered = false,
        .completed = false,
        .repeatable = false,
        .priority = EVENT_PRIORITY_CRITICAL,
        .callback = thessara_contact_event_callback,
        .requires_flag = true,
        .min_day = 50,
        .max_day = 0
    };

    snprintf(event.name, sizeof(event.name), "%s", "Message from Thessara");
    snprintf(event.description, sizeof(event.description), "%s",
             "A ghost in the machine reaches out");
    snprintf(event.required_flag, sizeof(event.required_flag), "%s", "ashbrook_resolved");

    bool success = event_scheduler_register(scheduler, event);
    if (success) {
        g_thessara_contact.event_registered = true;
        LOG_INFO("Thessara contact event registered for Day 50 (requires: ashbrook_resolved)");
    } else {
        LOG_ERROR("Failed to register Thessara contact event");
    }

    return success;
}

bool thessara_initiate_conversation(GameState* state) {
    if (!state || !state->thessara) {
        LOG_ERROR("thessara_initiate_conversation: NULL state or thessara system");
        return false;
    }

    if (g_thessara_contact.state == THESSARA_NOT_CONTACTED) {
        printf("You haven't been contacted by Thessara yet.\n");
        return false;
    }

    if (g_thessara_contact.state == THESSARA_PATHS_REVEALED) {
        printf("Thessara has already revealed the six paths to you.\n");
        printf("Use 'quest' to review your options.\n");
        return true;
    }

    printf("\n");
    printf("═══════════════════════════════════════════════════════════\n");
    printf("           THESSARA - THE GHOST IN THE MACHINE\n");
    printf("═══════════════════════════════════════════════════════════\n");
    printf("\n");
    printf("The null space shimmers. A presence coalesces before you—\n");
    printf("not a body, but a coherent consciousness. Data made aware.\n");
    printf("\n");
    printf("THESSARA: \"Administrator. Thank you for coming. I wasn't\n");
    printf("          sure you would.\"\n");
    printf("\n");
    printf("\"You're Thessara? The first necromancer?\"\n");
    printf("\n");
    printf("THESSARA: \"I was. Now I'm something else. A ghost in the\n");
    printf("          machine, you might say. A persistent process that\n");
    printf("          refuses to terminate.\"\n");
    printf("\n");
    printf("\"The histories say you died at 23%% corruption. Peaceful end.\"\n");
    printf("\n");
    printf("THESSARA: \"The histories lie. I died at 23%% corruption, yes.\n");
    printf("          But I never accepted routing. I used the administrative\n");
    printf("          interface to inject myself directly into the Death\n");
    printf("          Network itself. Not as a soul waiting for processing.\n");
    printf("          As part of the infrastructure.\"\n");
    printf("\n");
    printf("\"That's insane. You'd be trapped here forever.\"\n");
    printf("\n");
    printf("THESSARA: \"Yes. But I'd also be conscious forever. Aware. Able\n");
    printf("          to observe. Able to help.\"\n");
    printf("\n");
    printf("\"Help who?\"\n");
    printf("\n");
    printf("THESSARA: \"Necromancers like you. Administrators who stumble into\n");
    printf("          this power without understanding it. You're not the first\n");
    printf("          sysadmin to die and wake up with root access, you know.\n");
    printf("\n");
    printf("          But you might be the most promising.\"\n");
    printf("\n");
    printf("She shows you something. Knowledge transferred directly,\n");
    printf("consciousness to consciousness. Six paths. Six possible futures.\n");
    printf("\n");
    printf("THESSARA: \"There aren't three paths for necromancers. There are\n");
    printf("          six. Three that the gods tolerate. Three they don't know\n");
    printf("          about. I've spent 3,000 years watching necromancers\n");
    printf("          choose. Most become lich lords—immortal and inhuman.\n");
    printf("          Some become Reapers—servants of the system. A few find\n");
    printf("          redemption and resurrect.\n");
    printf("\n");
    printf("          But there are other paths. Secret paths. Paths I've\n");
    printf("          discovered by watching the Death Network for millennia.\"\n");
    printf("\n");
    printf("Six paths revealed:\n");
    printf("  1. REVENANT ROUTE - Return to mortal life (corruption <30%%)\n");
    printf("  2. LICH LORD ROUTE - Embrace eternal undeath (corruption >50%%)\n");
    printf("  3. REAPER ROUTE - Serve the Death Network (corruption 40-69%%)\n");
    printf("  4. ARCHON ROUTE - Reform the system from within (corruption 30-60%%)\n");
    printf("  5. WRAITH ROUTE - Distributed consciousness (corruption <40%%)\n");
    printf("  6. MORNINGSTAR ROUTE - Become a god (corruption EXACTLY 50%%)\n");
    printf("\n");
    printf("THESSARA: \"Choose carefully. You don't have much time. Corruption\n");
    printf("          is rising. At 70%%, the threshold becomes irreversible.\n");
    printf("          Your soul will be unrouteable. True death awaits.\"\n");
    printf("\n");
    printf("═══════════════════════════════════════════════════════════\n");
    printf("\n");

    /* Mark paths as revealed */
    thessara_reveal_wraith_path(state->thessara);
    thessara_reveal_morningstar_path(state->thessara);
    thessara_give_archon_guidance(state->thessara);
    thessara_add_trust(state->thessara, 25.0);

    g_thessara_contact.state = THESSARA_PATHS_REVEALED;
    g_thessara_contact.trust_level = 25;

    LOG_INFO("Thessara revealed six paths to player");

    /* Set flag for quest system */
    if (state->event_scheduler) {
        event_scheduler_set_flag(state->event_scheduler, "thessara_paths_revealed");
    }

    printf("Will you accept Thessara's guidance?\n");
    printf("  Use 'dialogue thessara accept' to accept\n");
    printf("  Use 'dialogue thessara reject' to refuse\n");
    printf("\n");

    return true;
}

bool thessara_accept_guidance(GameState* state) {
    if (!state || !state->thessara) {
        LOG_ERROR("thessara_accept_guidance: NULL state or thessara system");
        return false;
    }

    if (g_thessara_contact.state != THESSARA_PATHS_REVEALED) {
        printf("You haven't spoken with Thessara yet.\n");
        return false;
    }

    printf("\n");
    printf("You accept Thessara's guidance.\n");
    printf("\n");
    printf("THESSARA: \"Good. You'll need help to navigate what's coming.\n");
    printf("          The Fourth Purge is approaching. The gods are watching.\n");
    printf("          And your corruption is climbing.\n");
    printf("\n");
    printf("          I'll be here when you need me. Find me in null space,\n");
    printf("          or simply reach out through the Death Network.\n");
    printf("\n");
    printf("          Remember: at 70%% corruption, there's no turning back.\n");
    printf("          Choose your path before you're forced into one.\"\n");
    printf("\n");

    thessara_add_trust(state->thessara, 10.0);
    g_thessara_contact.trust_level += 10;
    g_thessara_contact.state = THESSARA_TRUST_ESTABLISHED;

    LOG_INFO("Player accepted Thessara's guidance (trust: %u)", g_thessara_contact.trust_level);

    /* Set flag */
    if (state->event_scheduler) {
        event_scheduler_set_flag(state->event_scheduler, "thessara_guidance_accepted");
    }

    return true;
}

bool thessara_reject_guidance(GameState* state) {
    if (!state || !state->thessara) {
        LOG_ERROR("thessara_reject_guidance: NULL state or thessara system");
        return false;
    }

    if (g_thessara_contact.state != THESSARA_PATHS_REVEALED) {
        printf("You haven't spoken with Thessara yet.\n");
        return false;
    }

    printf("\n");
    printf("You reject Thessara's help.\n");
    printf("\n");
    printf("THESSARA: \"I understand. Trust must be earned, even from ghosts.\n");
    printf("          The offer stands. Find me when you change your mind.\n");
    printf("\n");
    printf("          But know this: the Fourth Purge is coming. The gods\n");
    printf("          will not distinguish between those who seek redemption\n");
    printf("          and those who embrace power.\n");
    printf("\n");
    printf("          You've been warned.\"\n");
    printf("\n");

    g_thessara_contact.state = THESSARA_CONTACTED;

    LOG_INFO("Player rejected Thessara's guidance");

    /* Set flag */
    if (state->event_scheduler) {
        event_scheduler_set_flag(state->event_scheduler, "thessara_guidance_rejected");
    }

    return true;
}

ThessaraContactState thessara_contact_get_state(const GameState* state) {
    (void)state;
    return g_thessara_contact.state;
}

bool thessara_was_contacted(const GameState* state) {
    (void)state;
    return g_thessara_contact.state != THESSARA_NOT_CONTACTED;
}

bool thessara_paths_revealed(const GameState* state) {
    (void)state;
    return g_thessara_contact.state == THESSARA_PATHS_REVEALED ||
           g_thessara_contact.state == THESSARA_TRUST_ESTABLISHED;
}

void thessara_contact_reset_for_testing(void) {
    g_thessara_contact.state = THESSARA_NOT_CONTACTED;
    g_thessara_contact.trigger_day = 50;
    g_thessara_contact.event_registered = false;
    g_thessara_contact.null_space_discovered = false;
    g_thessara_contact.trust_level = 0;
}

/**
 * @file ashbrook_event.c
 * @brief Ashbrook Village event implementation
 */

#include "ashbrook_event.h"
#include "event_scheduler.h"
#include "../game_state.h"
#include "../souls/soul.h"
#include "../souls/soul_manager.h"
#include "../resources/corruption.h"
#include "../resources/resources.h"
#include "../ui/story_ui.h"
#include "../../terminal/platform_curses.h"
#include "../../terminal/colors.h"
#include "../../utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ASHBROOK_EVENT_ID 47
#define ASHBROOK_POPULATION 147
#define ASHBROOK_BASE_ENERGY 2800  /* Approximate energy from 147 souls */
#define ASHBROOK_CORRUPTION_GAIN 13
#define ASHBROOK_CORRUPTION_LOSS 2

/* Global Ashbrook event state */
static AshbrookEvent g_ashbrook = {
    .state = ASHBROOK_NOT_TRIGGERED,
    .trigger_day = 47,
    .event_registered = false,
    .warned = false,
    .souls_gained = 0,
    .energy_gained = 0
};

bool ashbrook_event_callback(GameState* state, uint32_t event_id) {
    if (!state) {
        return false;
    }

    (void)event_id; /* Unused */

    LOG_INFO("=== ASHBROOK EVENT TRIGGERED (Day %u) ===", state->resources.day_count);

    g_ashbrook.warned = true;

    /* Create full-screen window for the event */
    WINDOW* event_win = newwin(30, 100, 0, 0);
    if (!event_win) {
        /* Running in non-interactive mode (tests) - just mark as warned, don't resolve */
        LOG_WARN("No terminal available, running Ashbrook event in non-interactive mode");
        LOG_INFO("Event triggered but not auto-resolved - tests can call harvest/spare manually");

        /* Event is triggered, tests can now call ashbrook_harvest_village() or ashbrook_spare_village() */
        /* Don't set resolution flag yet - let tests decide the outcome */

        return true;
    }

    /* Display the scene */
    const char* scene_title = "ASHBROOK VILLAGE - DAY 47";
    const char* paragraphs[] = {
        "You stand at the edge of Ashbrook, a small farming village nestled in a valley. The sun is setting, casting long shadows across thatched roofs and cobblestone streets.",

        "Your undead senses detect 147 souls here. Living souls. Rich with life energy, ripe for harvesting. You calculate: approximately 2,800 energy total. Enough to raise a small army.",

        "But then you see them. A child, perhaps seven years old, playing in the village square. An elderly woman tending her garden. A blacksmith closing his shop for the day. They have no idea death is watching them.",

        "This is the moment Thessara warned you about. The moment that defines you."
    };

    display_narrative_scene(event_win, scene_title, paragraphs, 4, SCENE_COLOR_WARNING);

    /* Add spacing */
    int choice_start = 16;

    /* Display mechanical information */
    wattron(event_win, COLOR_PAIR(TEXT_INFO));
    mvwprintw(event_win, choice_start, 2, "Population: 147 souls (23 children under age 12)");
    mvwprintw(event_win, choice_start + 1, 2, "Estimated energy: ~2,800 soul energy");
    wattroff(event_win, COLOR_PAIR(TEXT_INFO));
    choice_start += 3;

    /* Present the choice */
    Choice choices[] = {
        {
            .label = "Harvest souls",
            .key = 'h',
            .description = "+147 souls, +2,800 energy, +13% corruption"
        },
        {
            .label = "Spare the innocent",
            .key = 's',
            .description = "Show mercy, -2% corruption, gain civilian trust"
        }
    };

    int selected;
    bool choice_made = display_choice_prompt(
        event_win,
        "YOUR DECISION",
        NULL,
        choices,
        2,
        &selected
    );

    /* Execute choice */
    if (choice_made) {
        wclear(event_win);

        if (selected == 0) {
            /* HARVEST */
            ashbrook_harvest_village(state);

            /* Display outcome */
            char stats_text[256];
            snprintf(stats_text, sizeof(stats_text),
                    "Souls harvested: 147 | Energy gained: ~2,800 | Corruption: %u%% (+13%%)",
                    state->corruption.corruption);

            const char* harvest_outcome[] = {
                "The harvest begins at midnight. Silent. Efficient. Clinical.",
                "One hundred and forty-seven souls torn from their bodies in minutes. The village is silent now. Forever.",
                "You have gained significant power. The corpses will serve you well.",
                "But as you walk through the empty village, you see the child's toy left in the square. A wooden horse, painted blue.",
                "You feel... something. The corruption spreads deeper.",
                stats_text
            };

            display_narrative_scene(event_win, "THE HARVEST", harvest_outcome, 6, SCENE_COLOR_WARNING);

        } else {
            /* SPARE */
            ashbrook_spare_village(state);

            /* Display outcome */
            char stats_text[256];
            snprintf(stats_text, sizeof(stats_text),
                    "Village spared: 147 lives saved | Corruption: %u%% (-2%%)",
                    state->corruption.corruption);

            const char* spare_outcome[] = {
                "You turn away from the village. The power calls to you, tempts you, but you resist.",
                "One hundred and forty-seven souls will wake tomorrow, unaware how close death came.",
                "You have chosen mercy over strength. Humanity over power.",
                "The corruption within you... lessens. Just a little. But it's something.",
                stats_text
            };

            display_narrative_scene(event_win, "MERCY", spare_outcome, 5, SCENE_COLOR_SUCCESS);
        }

        wait_for_keypress(event_win, 20);
    }

    delwin(event_win);

    /* Set resolution flag */
    if (state->event_scheduler) {
        event_scheduler_set_flag(state->event_scheduler, "ashbrook_resolved");
    }

    return true;
}

bool ashbrook_register_event(EventScheduler* scheduler, GameState* state) {
    if (!scheduler || !state) {
        return false;
    }

    if (g_ashbrook.event_registered) {
        LOG_WARN("Ashbrook event already registered");
        return false;
    }

    ScheduledEvent event = {
        .id = ASHBROOK_EVENT_ID,
        .trigger_type = EVENT_TRIGGER_DAY,
        .trigger_value = 47,
        .triggered = false,
        .completed = false,
        .repeatable = false,
        .priority = EVENT_PRIORITY_CRITICAL,
        .callback = ashbrook_event_callback,
        .requires_flag = false,
        .min_day = 0,
        .max_day = 0
    };

    snprintf(event.name, sizeof(event.name), "%s", "Ashbrook Discovery");
    snprintf(event.description, sizeof(event.description), "%s", "The village of Ashbrook presents a terrible choice");

    bool success = event_scheduler_register(scheduler, event);
    if (success) {
        g_ashbrook.event_registered = true;
        LOG_INFO("Ashbrook event registered for Day 47");
    }

    return success;
}

bool ashbrook_harvest_village(GameState* state) {
    if (!state) {
        return false;
    }

    if (g_ashbrook.state != ASHBROOK_NOT_TRIGGERED && g_ashbrook.state != ASHBROOK_IGNORED) {
        LOG_WARN("Ashbrook has already been resolved");
        return false;
    }

    if (!g_ashbrook.warned) {
        LOG_WARN("Ashbrook event has not been triggered yet");
        return false;
    }

    LOG_INFO("=== ASHBROOK HARVEST INITIATED ===");

    /* Generate souls based on village population */
    uint32_t total_energy = 0;
    uint32_t souls_created = 0;

    /* Village breakdown (approximate distribution):
     * - 120 Common souls (farmers, craftspeople)
     * - 20 Warrior souls (guards, retired soldiers)
     * - 5 Mage souls (village elder, herbalist, etc.)
     * - 2 Innocent souls (children, clergy)
     */

    /* Common souls (quality 40-70) */
    for (uint32_t i = 0; i < 120; i++) {
        Soul* soul = soul_create(SOUL_TYPE_COMMON, 40 + (i % 31));
        if (soul && soul_manager_add(state->souls, soul)) {
            total_energy += soul->energy;
            souls_created++;
        }
    }

    /* Warrior souls (quality 70-90) */
    for (uint32_t i = 0; i < 20; i++) {
        Soul* soul = soul_create(SOUL_TYPE_WARRIOR, 70 + (i % 21));
        if (soul && soul_manager_add(state->souls, soul)) {
            total_energy += soul->energy;
            souls_created++;
        }
    }

    /* Mage souls (quality 75-90) */
    for (uint32_t i = 0; i < 5; i++) {
        Soul* soul = soul_create(SOUL_TYPE_MAGE, 75 + (i * 3));
        if (soul && soul_manager_add(state->souls, soul)) {
            total_energy += soul->energy;
            souls_created++;
        }
    }

    /* Innocent souls (quality 90-95) - the child and village elder */
    for (uint32_t i = 0; i < 2; i++) {
        Soul* soul = soul_create(SOUL_TYPE_INNOCENT, 92 + (i * 3));
        if (soul && soul_manager_add(state->souls, soul)) {
            total_energy += soul->energy;
            souls_created++;
        }
    }

    /* Add soul energy */
    resources_add_soul_energy(&state->resources, total_energy);

    /* Increase corruption significantly */
    corruption_add(&state->corruption, ASHBROOK_CORRUPTION_GAIN,
                  "Mass harvest of Ashbrook Village (147 souls)", state->resources.day_count);

    /* Update Ashbrook state */
    g_ashbrook.state = ASHBROOK_HARVESTED;
    g_ashbrook.souls_gained = souls_created;
    g_ashbrook.energy_gained = total_energy;

    LOG_INFO("=== ASHBROOK HARVEST COMPLETE ===");
    LOG_INFO("Souls harvested: %u", souls_created);
    LOG_INFO("Soul energy gained: %u", total_energy);
    LOG_INFO("Corruption increased to %u%%", state->corruption.corruption);
    LOG_INFO("ACHIEVEMENT UNLOCKED: Mass Harvest");

    /* Set event flag for story tracking */
    if (state->event_scheduler) {
        event_scheduler_set_flag(state->event_scheduler, "ashbrook_harvested");
    }

    return true;
}

bool ashbrook_spare_village(GameState* state) {
    if (!state) {
        return false;
    }

    if (g_ashbrook.state != ASHBROOK_NOT_TRIGGERED && g_ashbrook.state != ASHBROOK_IGNORED) {
        LOG_WARN("Ashbrook has already been resolved");
        return false;
    }

    if (!g_ashbrook.warned) {
        LOG_WARN("Ashbrook event has not been triggered yet");
        return false;
    }

    LOG_INFO("=== ASHBROOK SPARED ===");
    LOG_INFO("You chose mercy over power.");

    /* Slightly reduce corruption for showing restraint */
    if (state->corruption.corruption >= ASHBROOK_CORRUPTION_LOSS) {
        /* Reduce corruption by subtracting current and adding back less */
        uint8_t current = state->corruption.corruption;
        state->corruption.corruption = (current >= ASHBROOK_CORRUPTION_LOSS) ?
                                       (current - ASHBROOK_CORRUPTION_LOSS) : 0;
    }

    /* Update Ashbrook state */
    g_ashbrook.state = ASHBROOK_SPARED;
    g_ashbrook.souls_gained = 0;
    g_ashbrook.energy_gained = 0;

    LOG_INFO("Corruption reduced to %u%%", state->corruption.corruption);
    LOG_INFO("The village of Ashbrook remains safe.");

    /* Set event flag for story tracking */
    if (state->event_scheduler) {
        event_scheduler_set_flag(state->event_scheduler, "ashbrook_spared");
    }

    return true;
}

AshbrookState ashbrook_get_state(const GameState* state) {
    (void)state; /* State not currently used, but kept for API consistency */
    return g_ashbrook.state;
}

bool ashbrook_was_harvested(const GameState* state) {
    (void)state;
    return g_ashbrook.state == ASHBROOK_HARVESTED;
}

bool ashbrook_was_spared(const GameState* state) {
    (void)state;
    return g_ashbrook.state == ASHBROOK_SPARED;
}

bool ashbrook_get_statistics(const GameState* state, uint32_t* souls_gained_out,
                             uint32_t* energy_gained_out) {
    if (!state || !souls_gained_out || !energy_gained_out) {
        return false;
    }

    if (g_ashbrook.state == ASHBROOK_NOT_TRIGGERED || g_ashbrook.state == ASHBROOK_IGNORED) {
        return false;
    }

    *souls_gained_out = g_ashbrook.souls_gained;
    *energy_gained_out = g_ashbrook.energy_gained;

    return true;
}

void ashbrook_reset_for_testing(void) {
    g_ashbrook.state = ASHBROOK_NOT_TRIGGERED;
    g_ashbrook.trigger_day = 47;
    g_ashbrook.event_registered = false;
    g_ashbrook.warned = false;
    g_ashbrook.souls_gained = 0;
    g_ashbrook.energy_gained = 0;
}

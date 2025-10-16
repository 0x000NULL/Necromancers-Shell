/**
 * @file event_registration.c
 * @brief Centralized event registration implementation
 */

#include "event_registration.h"
#include "event_scheduler.h"
#include "ashbrook_event.h"
#include "thessara_contact_event.h"
#include "divine_summons_event.h"
#include "trial_sequence_events.h"
#include "../game_state.h"
#include "../../utils/logger.h"

uint32_t register_all_story_events(EventScheduler* scheduler, GameState* state) {
    if (!scheduler || !state) {
        LOG_ERROR("Cannot register events: scheduler or state is NULL");
        return 0;
    }

    uint32_t registered = 0;

    LOG_INFO("Beginning story event registration...");

    /* Register Ashbrook event (Day 47) */
    if (ashbrook_register_event(scheduler, state)) {
        registered++;
        LOG_INFO("✓ Registered: Ashbrook massacre event (Day 47)");
    } else {
        LOG_WARN("✗ Failed to register: Ashbrook event");
    }

    /* Register Thessara contact (Day 50) */
    if (thessara_contact_register_event(scheduler, state)) {
        registered++;
        LOG_INFO("✓ Registered: Thessara contact event (Day 50)");
    } else {
        LOG_WARN("✗ Failed to register: Thessara contact event");
    }

    /* Register Divine summons (Day 155) */
    if (divine_summons_register_event(scheduler, state)) {
        registered++;
        LOG_INFO("✓ Registered: Divine summons event (Day 155)");
    } else {
        LOG_WARN("✗ Failed to register: Divine summons event");
    }

    /* Initialize trial sequence events (progression-based, not day-based) */
    uint32_t trial_events = trial_sequence_register_events(scheduler, state);
    if (trial_events > 0) {
        registered += trial_events;
        LOG_INFO("✓ Registered: %u Archon trial sequence events", trial_events);
    } else {
        LOG_INFO("Trial sequence events initialized (progression-based)");
    }

    /* Additional story events can be registered here as they are implemented */

    LOG_INFO("Story event registration complete: %u events registered", registered);

    return registered;
}

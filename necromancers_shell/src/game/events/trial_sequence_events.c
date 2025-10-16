/**
 * @file trial_sequence_events.c
 * @brief Trial sequence event implementation
 */

#include "trial_sequence_events.h"
#include "event_scheduler.h"
#include "../game_state.h"
#include "../../utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Forward declarations for archon trial functions */
extern bool archon_trial_unlock(void* manager, uint32_t trial_id, uint8_t corruption, float consciousness);

/* Global trial sequence state */
static TrialSequenceProgress g_trial_progress = {
    .state = TRIAL_SEQ_INACTIVE,
    .trials_unlocked = 0,
    .trials_completed = 0,
    .trials_failed = 0,
    .last_completion_day = 0,
    .judgment_triggered = false
};

/* Trial names for logging and display */
static const char* trial_names[7] = {
    "Test of Power",
    "Test of Wisdom",
    "Test of Morality",
    "Test of Technical Skill",
    "Test of Resolve",
    "Test of Sacrifice",
    "Test of Leadership"
};

bool trial_sequence_on_completion(GameState* state, uint32_t trial_number) {
    if (!state || trial_number < 1 || trial_number > 7) {
        LOG_ERROR("trial_sequence_on_completion: Invalid parameters");
        return false;
    }

    LOG_INFO("=== TRIAL %u COMPLETED: %s ===", trial_number, trial_names[trial_number - 1]);

    /* Mark trial as completed */
    g_trial_progress.trials_completed |= (1 << (trial_number - 1));
    g_trial_progress.last_completion_day = state->resources.day_count;

    printf("\n");
    printf("═══════════════════════════════════════════════════════════\n");
    printf("           TRIAL %u COMPLETE: %s\n", trial_number, trial_names[trial_number - 1]);
    printf("═══════════════════════════════════════════════════════════\n");
    printf("\n");

    /* Trial-specific completion messages */
    switch (trial_number) {
        case 1:
            printf("Seraphim lowers their blade.\n");
            printf("\n");
            printf("SERAPHIM: \"You showed mercy when you could have killed.\n");
            printf("          That is the mark of an Archon. Power without\n");
            printf("          cruelty. The first trial is passed.\"\n");
            break;

        case 2:
            printf("The routing paradox resolves. 200 years of deadlocked\n");
            printf("souls flow freely through the network.\n");
            printf("\n");
            printf("KELDRIN: \"Wisdom. You saw what centuries of divine\n");
            printf("         bureaucracy could not. Trial 2 is passed.\"\n");
            break;

        case 3:
            printf("The innocent are saved. Your soul energy depleted, but\n");
            printf("100 lives spared from necromantic corruption.\n");
            printf("\n");
            printf("ANARA: \"Morality. You chose lives over power. The third\n");
            printf("       trial is passed.\"\n");
            break;

        case 4:
            printf("All 17 bugs patched. The Death Network operates more\n");
            printf("efficiently than it has in millennia.\n");
            printf("\n");
            printf("NEXUS: \"Technical mastery. You understand the system at\n");
            printf("       a level most gods do not. Trial 4 is passed.\"\n");
            break;

        case 5:
            printf("30 days without raising your corruption. The temptation\n");
            printf("was constant, but you held firm.\n");
            printf("\n");
            printf("THEROS: \"Resolve. You resisted when lesser beings would\n");
            printf("        have given in. Trial 5 is passed.\"\n");
            break;

        case 6:
            printf("The sacrifice is made. Maya's life spared, though the\n");
            printf("cost to your power was immense.\n");
            printf("\n");
            printf("SERAPH: \"Sacrifice. You gave up what you valued most for\n");
            printf("        the sake of another. Trial 6 is passed.\"\n");
            break;

        case 7:
            printf("The Regional Council is reformed. Collective corruption\n");
            printf("reduced by 10%%. A miracle of leadership.\n");
            printf("\n");
            printf("VORATHOS: \"Leadership. You changed minds without force,\n");
            printf("          hearts without coercion. The final trial is passed.\"\n");
            break;
    }

    printf("\n");

    /* Check if this completes all trials */
    if (trial_sequence_count_completed(state) == 7) {
        printf("═══════════════════════════════════════════════════════════\n");
        printf("\n");
        printf("ALL SEVEN TRIALS COMPLETE\n");
        printf("\n");
        printf("The Death Network pulses with divine energy. The Seven\n");
        printf("Architects assemble to deliver their judgment.\n");
        printf("\n");
        printf("Your worthiness will now be determined.\n");
        printf("\n");
        printf("═══════════════════════════════════════════════════════════\n");
        printf("\n");

        g_trial_progress.state = TRIAL_SEQ_COMPLETED;

        /* Trigger Divine Judgment */
        return trial_sequence_trigger_judgment(state);
    } else {
        /* Unlock next trial */
        return trial_sequence_unlock_next(state, trial_number);
    }
}

bool trial_sequence_unlock_next(GameState* state, uint32_t completed_trial) {
    if (!state || completed_trial < 1 || completed_trial >= 7) {
        LOG_ERROR("trial_sequence_unlock_next: Invalid parameters");
        return false;
    }

    uint32_t next_trial = completed_trial + 1;

    printf("TRIAL %u UNLOCKED: %s\n", next_trial, trial_names[next_trial - 1]);
    printf("Use 'ritual archon_trial %u' to begin the next trial.\n", next_trial);
    printf("\n");

    /* Mark next trial as unlocked */
    g_trial_progress.trials_unlocked |= (1 << (next_trial - 1));

    /* Unlock in archon trial system */
    if (state->archon_trials) {
        archon_trial_unlock(state->archon_trials, next_trial,
                           state->corruption.corruption,
                           state->consciousness.stability);
        LOG_INFO("Unlocked Trial %u in archon trial system", next_trial);
    }

    /* Set flag for quest/dialogue triggers */
    if (state->event_scheduler) {
        char flag_name[64];
        snprintf(flag_name, sizeof(flag_name), "trial_%u_unlocked", next_trial);
        event_scheduler_set_flag(state->event_scheduler, flag_name);
        LOG_INFO("Set flag: %s", flag_name);
    }

    return true;
}

bool trial_sequence_trigger_judgment(GameState* state) {
    if (!state) {
        LOG_ERROR("trial_sequence_trigger_judgment: NULL state");
        return false;
    }

    if (g_trial_progress.judgment_triggered) {
        LOG_WARN("Divine Judgment already triggered");
        return false;
    }

    printf("\n");
    printf("═══════════════════════════════════════════════════════════\n");
    printf("              SUMMONING THE DIVINE COUNCIL\n");
    printf("═══════════════════════════════════════════════════════════\n");
    printf("\n");
    printf("The Seven Architects gather in null space. You feel their\n");
    printf("attention focus upon you—weighing, measuring, judging.\n");
    printf("\n");
    printf("KELDRIN: \"Administrator. You have completed all seven trials.\n");
    printf("         Now we shall determine your worthiness to become\n");
    printf("         an Archon—a custodian of balance between life and death.\n");
    printf("\n");
    printf("         Each of us will cast our vote. Four approvals grant\n");
    printf("         amnesty and transformation. Fewer, and you face the\n");
    printf("         Fourth Purge with the rest of your kind.\n");
    printf("\n");
    printf("         The judgment begins now.\"\n");
    printf("\n");
    printf("═══════════════════════════════════════════════════════════\n");
    printf("\n");
    printf("Use 'invoke divine_judgment' to hear the Council's verdict.\n");
    printf("\n");

    g_trial_progress.judgment_triggered = true;

    /* Set flag */
    if (state->event_scheduler) {
        event_scheduler_set_flag(state->event_scheduler, "divine_judgment_available");
        LOG_INFO("Set flag: divine_judgment_available");
    }

    LOG_INFO("Divine Judgment triggered after completing all 7 trials");

    return true;
}

uint32_t trial_sequence_register_events(EventScheduler* scheduler, GameState* state) {
    if (!scheduler || !state) {
        LOG_ERROR("trial_sequence_register_events: NULL scheduler or state");
        return 0;
    }

    /* Trial sequence events are triggered programmatically, not by day count */
    /* They don't need to be registered in the event scheduler */
    /* This function is here for future expansion if needed */

    LOG_INFO("Trial sequence events initialized");
    return 0;
}

TrialSequenceState trial_sequence_get_state(const GameState* state) {
    (void)state;
    return g_trial_progress.state;
}

TrialSequenceProgress trial_sequence_get_progress(const GameState* state) {
    (void)state;
    return g_trial_progress;
}

bool trial_sequence_is_unlocked(const GameState* state, uint32_t trial_number) {
    (void)state;
    if (trial_number < 1 || trial_number > 7) {
        return false;
    }
    return (g_trial_progress.trials_unlocked & (1 << (trial_number - 1))) != 0;
}

bool trial_sequence_is_completed(const GameState* state, uint32_t trial_number) {
    (void)state;
    if (trial_number < 1 || trial_number > 7) {
        return false;
    }
    return (g_trial_progress.trials_completed & (1 << (trial_number - 1))) != 0;
}

bool trial_sequence_is_failed(const GameState* state, uint32_t trial_number) {
    (void)state;
    if (trial_number < 1 || trial_number > 7) {
        return false;
    }
    return (g_trial_progress.trials_failed & (1 << (trial_number - 1))) != 0;
}

uint32_t trial_sequence_count_completed(const GameState* state) {
    (void)state;
    uint32_t count = 0;
    for (uint32_t i = 0; i < 7; i++) {
        if (g_trial_progress.trials_completed & (1 << i)) {
            count++;
        }
    }
    return count;
}

uint32_t trial_sequence_count_failed(const GameState* state) {
    (void)state;
    uint32_t count = 0;
    for (uint32_t i = 0; i < 7; i++) {
        if (g_trial_progress.trials_failed & (1 << i)) {
            count++;
        }
    }
    return count;
}

bool trial_sequence_all_completed(const GameState* state) {
    return trial_sequence_count_completed(state) == 7;
}

void trial_sequence_display_progress(const GameState* state) {
    printf("\n");
    printf("═══════════════════════════════════════════════════════════\n");
    printf("              ARCHON TRIAL PROGRESS\n");
    printf("═══════════════════════════════════════════════════════════\n");
    printf("\n");

    for (uint32_t i = 1; i <= 7; i++) {
        printf("Trial %u: %-30s", i, trial_names[i - 1]);

        if (trial_sequence_is_completed(state, i)) {
            printf(" [✓ PASSED]\n");
        } else if (trial_sequence_is_failed(state, i)) {
            printf(" [✗ FAILED]\n");
        } else if (trial_sequence_is_unlocked(state, i)) {
            printf(" [  UNLOCKED]\n");
        } else {
            printf(" [  LOCKED]\n");
        }
    }

    printf("\n");
    printf("Completed: %u/7\n", trial_sequence_count_completed(state));
    printf("Failed: %u\n", trial_sequence_count_failed(state));
    printf("\n");

    if (g_trial_progress.judgment_triggered) {
        printf("Status: Awaiting Divine Judgment\n");
    } else if (g_trial_progress.state == TRIAL_SEQ_COMPLETED) {
        printf("Status: All trials complete\n");
    } else if (g_trial_progress.state == TRIAL_SEQ_ACTIVE) {
        printf("Status: Trials in progress\n");
    } else {
        printf("Status: Trials not started\n");
    }

    printf("\n");
    printf("═══════════════════════════════════════════════════════════\n");
    printf("\n");
}

void trial_sequence_reset_for_testing(void) {
    g_trial_progress.state = TRIAL_SEQ_INACTIVE;
    g_trial_progress.trials_unlocked = 0;
    g_trial_progress.trials_completed = 0;
    g_trial_progress.trials_failed = 0;
    g_trial_progress.last_completion_day = 0;
    g_trial_progress.judgment_triggered = false;
}

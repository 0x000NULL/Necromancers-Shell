/**
 * reformation_program.c - Necromancer Reformation Program Implementation
 */

#include "reformation_program.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Name pools for procedural generation */
static const char* MALE_NAMES[] = {
    "Aldric", "Branthor", "Corvus", "Darius", "Eldric", "Fenrir", "Gorath",
    "Halvor", "Ignatius", "Jareth", "Kael", "Lucius", "Marius", "Nero",
    "Osric", "Phelan", "Quintus", "Raven", "Soren", "Thorne"
};

static const char* FEMALE_NAMES[] = {
    "Althea", "Brenna", "Cassandra", "Delara", "Elysra", "Faustine",
    "Gwyndolin", "Helara", "Isolde", "Kaelith", "Lilith", "Morgana",
    "Nyx", "Ophelia", "Petra", "Quinn", "Ravenna", "Seraphine",
    "Talia", "Vesper"
};

static const char* SURNAMES[] = {
    "Blackwood", "Darkmore", "Grimshaw", "Hollowvale", "Ironhart",
    "Nightshade", "Ravenwood", "Shadowend", "Thornheart", "Voidcaller",
    "Ashborne", "Deathmarch", "Gravesend", "Netherbane", "Soulreaver"
};

#define MALE_NAMES_COUNT 20
#define FEMALE_NAMES_COUNT 20
#define SURNAMES_COUNT 15

/* Corruption reduction per approach */
static const int APPROACH_EFFECTS[] = {
    3,  /* APPROACH_DIPLOMATIC */
    5,  /* APPROACH_HARSH */
    2   /* APPROACH_INSPIRATIONAL */
};

/* Attitude change per approach */
static const int APPROACH_ATTITUDE[] = {
    5,   /* APPROACH_DIPLOMATIC: builds trust */
    -10, /* APPROACH_HARSH: damages relationship */
    8    /* APPROACH_INSPIRATIONAL: greatly improves attitude */
};

/* Resistance modifiers */
static const double RESISTANCE_MODIFIERS[] = {
    1.0,  /* RESISTANCE_LOW */
    0.6,  /* RESISTANCE_MEDIUM */
    0.4,  /* RESISTANCE_HIGH */
    0.2   /* RESISTANCE_EXTREME */
};

ReformationProgram* reformation_program_create(void) {
    ReformationProgram* program = calloc(1, sizeof(ReformationProgram));
    if (!program) {
        return NULL;
    }

    return program;
}

void reformation_program_destroy(ReformationProgram* program) {
    free(program);
}

bool reformation_program_initialize(ReformationProgram* program,
                                   int days_until_purge) {
    if (!program) {
        return false;
    }

    program->days_remaining = days_until_purge;
    program->target_count = REFORMATION_TARGET_COUNT;

    /* Generate 147 necromancers procedurally */
    for (int i = 0; i < REFORMATION_TARGET_COUNT; i++) {
        ReformationTarget* target = &program->targets[i];

        target->npc_id = 10000 + i; /* Start NPC IDs at 10000 */

        /* Generate name */
        int gender = rand() % 2;
        const char* first_name;
        if (gender == 0) {
            first_name = MALE_NAMES[rand() % MALE_NAMES_COUNT];
        } else {
            first_name = FEMALE_NAMES[rand() % FEMALE_NAMES_COUNT];
        }
        const char* surname = SURNAMES[rand() % SURNAMES_COUNT];
        snprintf(target->name, sizeof(target->name), "%s %s", first_name, surname);

        /* Corruption: 65-99% */
        target->starting_corruption = 65 + (rand() % 35);
        target->current_corruption = target->starting_corruption;
        target->corruption_reduction = 0;

        /* Resistance level (random distribution) */
        int roll = rand() % 100;
        if (roll < 30) {
            target->resistance = RESISTANCE_LOW;
        } else if (roll < 60) {
            target->resistance = RESISTANCE_MEDIUM;
        } else if (roll < 85) {
            target->resistance = RESISTANCE_HIGH;
        } else {
            target->resistance = RESISTANCE_EXTREME;
        }

        /* Initial attitude: mostly neutral to wary */
        target->attitude_score = -10 + (rand() % 20); /* -10 to +9 */

        target->sessions_held = 0;
        target->days_since_last_session = SESSION_COOLDOWN_DAYS; /* Can start immediately */
        target->reformed = false;
        target->refused = false;
    }

    return true;
}

bool reformation_program_start_session(ReformationProgram* program,
                                      int npc_id, ReformationApproach approach,
                                      int* corruption_reduced,
                                      int* attitude_change) {
    if (!program) {
        return false;
    }

    /* Find target */
    ReformationTarget* target = NULL;
    for (size_t i = 0; i < program->target_count; i++) {
        if (program->targets[i].npc_id == npc_id) {
            target = &program->targets[i];
            break;
        }
    }

    if (!target || target->refused || target->reformed) {
        return false;
    }

    /* Check cooldown */
    if (target->days_since_last_session < SESSION_COOLDOWN_DAYS) {
        return false; /* Still on cooldown */
    }

    /* Calculate corruption reduction */
    int base_reduction = APPROACH_EFFECTS[approach];
    double modifier = RESISTANCE_MODIFIERS[target->resistance];
    int reduction = (int)(base_reduction * modifier);

    if (reduction < 1) {
        reduction = 1; /* Minimum 1% reduction */
    }

    /* Apply reduction */
    target->current_corruption -= reduction;
    if (target->current_corruption < 0) {
        target->current_corruption = 0;
    }

    target->corruption_reduction = target->starting_corruption - target->current_corruption;

    /* Calculate attitude change */
    int attitude_delta = APPROACH_ATTITUDE[approach];

    /* Attitude affects future resistance */
    if (target->attitude_score > 20) {
        /* Cooperative targets are easier to reform */
        if (target->resistance > RESISTANCE_LOW) {
            target->resistance--;
        }
    }

    target->attitude_score += attitude_delta;

    /* Clamp attitude */
    if (target->attitude_score > 50) {
        target->attitude_score = 50;
    } else if (target->attitude_score < -50) {
        target->attitude_score = -50;
    }

    /* Check for refusal (very hostile targets may refuse) */
    if (target->attitude_score <= -40 && target->sessions_held >= 3) {
        target->refused = true;
        program->targets_failed++;
        return false;
    }

    /* Update session tracking */
    target->sessions_held++;
    target->days_since_last_session = 0;
    program->total_sessions++;

    /* Output results */
    if (corruption_reduced) {
        *corruption_reduced = reduction;
    }
    if (attitude_change) {
        *attitude_change = attitude_delta;
    }

    return true;
}

bool reformation_program_check_reformed(ReformationProgram* program,
                                       int npc_id) {
    if (!program) {
        return false;
    }

    /* Find target */
    ReformationTarget* target = NULL;
    for (size_t i = 0; i < program->target_count; i++) {
        if (program->targets[i].npc_id == npc_id) {
            target = &program->targets[i];
            break;
        }
    }

    if (!target || target->reformed) {
        return false;
    }

    /* Check if 20%+ reduction achieved */
    if (target->corruption_reduction >= REQUIRED_CORRUPTION_REDUCTION) {
        target->reformed = true;
        program->targets_reformed++;

        /* Update completion percentage */
        program->completion_percentage =
            ((double)program->targets_reformed / REFORMATION_TARGET_COUNT) * 100.0;

        return true;
    }

    return false;
}

void reformation_program_get_progress(const ReformationProgram* program,
                                     int* reformed, int* in_progress,
                                     int* failed, int* days_remaining,
                                     double* percentage) {
    if (!program) {
        return;
    }

    if (reformed) {
        *reformed = program->targets_reformed;
    }

    if (in_progress) {
        /* Count targets with some reduction but not reformed */
        int count = 0;
        for (size_t i = 0; i < program->target_count; i++) {
            if (!program->targets[i].reformed &&
                !program->targets[i].refused &&
                program->targets[i].corruption_reduction > 0) {
                count++;
            }
        }
        *in_progress = count;
    }

    if (failed) {
        *failed = program->targets_failed;
    }

    if (days_remaining) {
        *days_remaining = program->days_remaining;
    }

    if (percentage) {
        *percentage = program->completion_percentage;
    }
}

int reformation_program_generate_report(const ReformationProgram* program,
                                       char* buffer, size_t buffer_size) {
    if (!program || !buffer || buffer_size == 0) {
        return 0;
    }

    int in_progress = 0;
    for (size_t i = 0; i < program->target_count; i++) {
        if (!program->targets[i].reformed &&
            !program->targets[i].refused &&
            program->targets[i].corruption_reduction > 0) {
            in_progress++;
        }
    }

    return snprintf(buffer, buffer_size,
        "REFORMATION PROGRAM STATUS\n\n"
        "Reformed:     %d / %d (%.1f%%)\n"
        "In Progress:  %d\n"
        "Failed:       %d\n"
        "Sessions:     %d\n"
        "Days Until Purge: %d (%.1f years)\n\n"
        "Target: Reform %d necromancers to prevent the Fourth Purge.\n",
        program->targets_reformed, REFORMATION_TARGET_COUNT,
        program->completion_percentage,
        in_progress,
        program->targets_failed,
        program->total_sessions,
        program->days_remaining,
        program->days_remaining / 365.0,
        REFORMATION_TARGET_COUNT);
}

void reformation_program_advance_time(ReformationProgram* program, int days) {
    if (!program || days <= 0) {
        return;
    }

    program->days_remaining -= days;
    if (program->days_remaining < 0) {
        program->days_remaining = 0;
    }

    /* Advance cooldowns */
    for (size_t i = 0; i < program->target_count; i++) {
        program->targets[i].days_since_last_session += days;
    }
}

const ReformationTarget* reformation_program_get_target(
    const ReformationProgram* program, int npc_id) {
    if (!program) {
        return NULL;
    }

    for (size_t i = 0; i < program->target_count; i++) {
        if (program->targets[i].npc_id == npc_id) {
            return &program->targets[i];
        }
    }

    return NULL;
}

const ReformationTarget* reformation_program_get_all_targets(
    const ReformationProgram* program, size_t* count_out) {
    if (!program) {
        if (count_out) {
            *count_out = 0;
        }
        return NULL;
    }

    if (count_out) {
        *count_out = program->target_count;
    }

    return program->targets;
}

const char* reformation_program_resistance_to_string(ResistanceLevel resistance) {
    switch (resistance) {
        case RESISTANCE_LOW:
            return "Low";
        case RESISTANCE_MEDIUM:
            return "Medium";
        case RESISTANCE_HIGH:
            return "High";
        case RESISTANCE_EXTREME:
            return "Extreme";
        default:
            return "Unknown";
    }
}

const char* reformation_program_approach_to_string(ReformationApproach approach) {
    switch (approach) {
        case APPROACH_DIPLOMATIC:
            return "Diplomatic";
        case APPROACH_HARSH:
            return "Harsh";
        case APPROACH_INSPIRATIONAL:
            return "Inspirational";
        default:
            return "Unknown";
    }
}

const char* reformation_program_attitude_to_string(Attitude attitude) {
    switch (attitude) {
        case ATTITUDE_HOSTILE:
            return "Hostile";
        case ATTITUDE_WARY:
            return "Wary";
        case ATTITUDE_NEUTRAL:
            return "Neutral";
        case ATTITUDE_COOPERATIVE:
            return "Cooperative";
        case ATTITUDE_TRUSTING:
            return "Trusting";
        default:
            return "Unknown";
    }
}

Attitude reformation_program_calculate_attitude(int score) {
    if (score <= -20) {
        return ATTITUDE_HOSTILE;
    } else if (score <= -10) {
        return ATTITUDE_WARY;
    } else if (score <= 9) {
        return ATTITUDE_NEUTRAL;
    } else if (score <= 29) {
        return ATTITUDE_COOPERATIVE;
    } else {
        return ATTITUDE_TRUSTING;
    }
}

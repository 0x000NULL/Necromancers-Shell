/**
 * reformation_program.h - Necromancer Reformation Program
 *
 * As Archon, your primary responsibility is reforming 147 corrupt necromancers
 * before the Fourth Purge. Each necromancer requires different approaches
 * to reduce their corruption by 20%+.
 */

#ifndef REFORMATION_PROGRAM_H
#define REFORMATION_PROGRAM_H

#include <stdbool.h>
#include <stddef.h>

/* Total reformation targets */
#define REFORMATION_TARGET_COUNT 147

/* Corruption reduction required for successful reformation */
#define REQUIRED_CORRUPTION_REDUCTION 20

/* Session cooldown (days) */
#define SESSION_COOLDOWN_DAYS 3

/**
 * Resistance levels (affects reformation difficulty)
 */
typedef enum {
    RESISTANCE_LOW,      /* Easily convinced, -5% per session */
    RESISTANCE_MEDIUM,   /* Moderately resistant, -3% per session */
    RESISTANCE_HIGH,     /* Highly resistant, -2% per session */
    RESISTANCE_EXTREME   /* Nearly impossible, -1% per session */
} ResistanceLevel;

/**
 * Reformation approaches
 */
typedef enum {
    APPROACH_DIPLOMATIC,    /* -3% corruption, builds trust */
    APPROACH_HARSH,         /* -5% corruption, damages attitude */
    APPROACH_INSPIRATIONAL  /* -2% corruption, improves attitude */
} ReformationApproach;

/**
 * Necromancer attitude toward Archon
 */
typedef enum {
    ATTITUDE_HOSTILE,    /* -20 to -50 */
    ATTITUDE_WARY,       /* -10 to -19 */
    ATTITUDE_NEUTRAL,    /* -9 to +9 */
    ATTITUDE_COOPERATIVE, /* +10 to +29 */
    ATTITUDE_TRUSTING    /* +30 to +50 */
} Attitude;

/**
 * Reformation target (necromancer to reform)
 */
typedef struct {
    int npc_id;                  /* NPC identifier */
    char name[64];               /* Necromancer name */

    int starting_corruption;     /* Corruption when program began (65-99%) */
    int current_corruption;      /* Current corruption level */
    int corruption_reduction;    /* Total reduction achieved */

    ResistanceLevel resistance;  /* How resistant to reformation */
    int attitude_score;          /* -50 to +50 */

    int sessions_held;           /* Number of counseling sessions */
    int days_since_last_session; /* Cooldown tracker */

    bool reformed;               /* Corruption reduced 20%+ */
    bool refused;                /* Rejected reformation */
} ReformationTarget;

/**
 * Reformation program state
 */
typedef struct {
    ReformationTarget targets[REFORMATION_TARGET_COUNT];
    size_t target_count;

    int targets_reformed;        /* Successfully reformed (20%+ reduction) */
    int targets_in_progress;     /* Partially reformed */
    int targets_failed;          /* Refused reformation */

    int total_sessions;          /* Sessions held across all targets */
    int days_remaining;          /* Days until Fourth Purge */

    double completion_percentage; /* targets_reformed / 147 * 100 */
} ReformationProgram;

/**
 * Create reformation program
 *
 * Returns: Newly allocated program, or NULL on failure
 */
ReformationProgram* reformation_program_create(void);

/**
 * Destroy reformation program
 *
 * Params:
 *   program - Program to destroy
 */
void reformation_program_destroy(ReformationProgram* program);

/**
 * Initialize reformation program
 *
 * Generates 147 necromancers to reform.
 *
 * Params:
 *   program - Reformation program
 *   days_until_purge - Days remaining until Fourth Purge
 *
 * Returns: True on success, false on failure
 */
bool reformation_program_initialize(ReformationProgram* program,
                                   int days_until_purge);

/**
 * Start reformation session with a necromancer
 *
 * Counsels a target to reduce their corruption. Different approaches
 * have different effects.
 *
 * Params:
 *   program - Reformation program
 *   npc_id - Target NPC
 *   approach - Reformation approach to use
 *   corruption_reduced - Output: corruption reduction this session
 *   attitude_change - Output: attitude change this session
 *
 * Returns: True if session successful, false if on cooldown/refused
 */
bool reformation_program_start_session(ReformationProgram* program,
                                      int npc_id, ReformationApproach approach,
                                      int* corruption_reduced,
                                      int* attitude_change);

/**
 * Check if target is reformed
 *
 * Validates if target has achieved 20%+ corruption reduction.
 *
 * Params:
 *   program - Reformation program
 *   npc_id - Target NPC
 *
 * Returns: True if reformed successfully
 */
bool reformation_program_check_reformed(ReformationProgram* program,
                                       int npc_id);

/**
 * Get reformation progress statistics
 *
 * Params:
 *   program - Reformation program
 *   reformed - Output: successfully reformed count
 *   in_progress - Output: partially reformed count
 *   failed - Output: refused reformation count
 *   days_remaining - Output: days until purge
 *   percentage - Output: completion percentage (0-100)
 */
void reformation_program_get_progress(const ReformationProgram* program,
                                     int* reformed, int* in_progress,
                                     int* failed, int* days_remaining,
                                     double* percentage);

/**
 * Generate reformation report
 *
 * Creates formatted summary of program status.
 *
 * Params:
 *   program - Reformation program
 *   buffer - Output buffer for report
 *   buffer_size - Size of output buffer
 *
 * Returns: Number of characters written
 */
int reformation_program_generate_report(const ReformationProgram* program,
                                       char* buffer, size_t buffer_size);

/**
 * Advance time for all targets
 *
 * Decrements session cooldowns.
 *
 * Params:
 *   program - Reformation program
 *   days - Days to advance
 */
void reformation_program_advance_time(ReformationProgram* program, int days);

/**
 * Get target by NPC ID
 *
 * Params:
 *   program - Reformation program
 *   npc_id - NPC to find
 *
 * Returns: Pointer to target, or NULL if not found
 */
const ReformationTarget* reformation_program_get_target(
    const ReformationProgram* program, int npc_id);

/**
 * Get all targets
 *
 * Params:
 *   program - Reformation program
 *   count_out - Output: number of targets
 *
 * Returns: Pointer to targets array
 */
const ReformationTarget* reformation_program_get_all_targets(
    const ReformationProgram* program, size_t* count_out);

/**
 * Get resistance level as string
 *
 * Params:
 *   resistance - Resistance level
 *
 * Returns: Resistance string ("Low", "Medium", etc.)
 */
const char* reformation_program_resistance_to_string(ResistanceLevel resistance);

/**
 * Get approach as string
 *
 * Params:
 *   approach - Reformation approach
 *
 * Returns: Approach string ("Diplomatic", "Harsh", etc.)
 */
const char* reformation_program_approach_to_string(ReformationApproach approach);

/**
 * Get attitude as string
 *
 * Params:
 *   attitude - Attitude level
 *
 * Returns: Attitude string ("Hostile", "Cooperative", etc.)
 */
const char* reformation_program_attitude_to_string(Attitude attitude);

/**
 * Calculate attitude from score
 *
 * Params:
 *   score - Attitude score (-50 to +50)
 *
 * Returns: Attitude level
 */
Attitude reformation_program_calculate_attitude(int score);

#endif /* REFORMATION_PROGRAM_H */

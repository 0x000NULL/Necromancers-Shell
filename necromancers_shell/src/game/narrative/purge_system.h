/**
 * purge_system.h - Fourth Purge System
 *
 * The gods periodically purge corrupt necromancers. The Fourth Purge is
 * imminent (5 years away, accelerated to 1.5 years after Ashbrook).
 * As Archon, you can reform necromancers to prevent their extermination.
 */

#ifndef PURGE_SYSTEM_H
#define PURGE_SYSTEM_H

#include <stdbool.h>
#include <stddef.h>

/* Maximum divine enforcers deployed */
#define MAX_ENFORCERS 30

/**
 * Divine enforcer types
 */
typedef enum {
    ENFORCER_SERAPHIM,     /* Holy warriors */
    ENFORCER_HELLKNIGHT,   /* Infernal enforcers */
    ENFORCER_INEVITABLE,   /* Fate's executioners */
    ENFORCER_TYPE_COUNT
} EnforcerType;

/**
 * Divine enforcer
 */
typedef struct {
    EnforcerType type;
    int hp;
    int attack;
    int defense;
    char specialty[64];    /* Special ability */
    bool deployed;         /* Currently hunting necromancers */
} DivineEnforcer;

/**
 * Fourth Purge state
 */
typedef struct {
    int purge_number;               /* Always 4 for Fourth Purge */
    int days_until_purge;           /* Countdown timer */
    bool accelerated;               /* Ashbrook triggered early */

    DivineEnforcer enforcers[MAX_ENFORCERS];
    size_t enforcer_count;
    int enforcers_deployed;         /* Number currently hunting */

    int necromancers_killed;        /* Deaths during purge */
    int necromancers_reformed;      /* Saved by Archon */
    int reformation_target;         /* Goal: 147 necromancers */

    bool reformation_successful;    /* Reached target */
    bool archon_intervention_active; /* Player is Archon */

    int estimated_casualties_base;  /* Without Archon: 200 */
    int estimated_casualties_with_archon; /* With Archon: 50 */
} PurgeState;

/**
 * Create purge state
 *
 * Returns: Newly allocated purge state, or NULL on failure
 */
PurgeState* purge_system_create(void);

/**
 * Destroy purge state
 *
 * Params:
 *   state - Purge state to destroy
 */
void purge_system_destroy(PurgeState* state);

/**
 * Initialize Fourth Purge
 *
 * Sets up the purge with 5-year timer and enforcer roster.
 *
 * Params:
 *   state - Purge state
 *
 * Returns: True on success, false on failure
 */
bool purge_system_initialize(PurgeState* state);

/**
 * Accelerate purge timeline
 *
 * Ashbrook event triggers early purge (1.5 years instead of 5).
 * Increases enforcer deployment rate.
 *
 * Params:
 *   state - Purge state
 */
void purge_system_accelerate(PurgeState* state);

/**
 * Advance purge countdown by one day
 *
 * Decrements timer. Periodically deploys enforcers as purge approaches.
 *
 * Params:
 *   state - Purge state
 *
 * Returns: True if purge has begun (timer reached 0)
 */
bool purge_system_advance_day(PurgeState* state);

/**
 * Deploy divine enforcers
 *
 * Sends enforcement teams to hunt corrupt necromancers.
 *
 * Params:
 *   state - Purge state
 *
 * Returns: Number of enforcers deployed
 */
int purge_system_deploy_enforcers(PurgeState* state);

/**
 * Reform a necromancer
 *
 * As Archon, save a necromancer from the purge by reducing their corruption.
 *
 * Params:
 *   state - Purge state
 *   npc_id - NPC to reform
 *
 * Returns: True if reformed successfully
 */
bool purge_system_reform_necromancer(PurgeState* state, int npc_id);

/**
 * Check if reformation target reached
 *
 * Params:
 *   state - Purge state
 *
 * Returns: True if 147+ necromancers reformed
 */
bool purge_system_is_reformation_complete(const PurgeState* state);

/**
 * Calculate estimated casualties
 *
 * Predicts how many necromancers will die based on Archon intervention.
 *
 * Params:
 *   state - Purge state
 *
 * Returns: Estimated deaths
 */
int purge_system_calculate_casualties(const PurgeState* state);

/**
 * Enable Archon intervention
 *
 * Activates reformation program when player becomes Archon.
 *
 * Params:
 *   state - Purge state
 */
void purge_system_enable_archon_intervention(PurgeState* state);

/**
 * Get enforcer information
 *
 * Params:
 *   state - Purge state
 *   index - Enforcer index
 *
 * Returns: Pointer to enforcer, or NULL if invalid
 */
const DivineEnforcer* purge_system_get_enforcer(const PurgeState* state,
                                               size_t index);

/**
 * Get enforcer type name
 *
 * Params:
 *   type - Enforcer type
 *
 * Returns: Type name string
 */
const char* purge_system_get_enforcer_type_name(EnforcerType type);

/**
 * Get days remaining until purge
 *
 * Params:
 *   state - Purge state
 *
 * Returns: Days until purge begins
 */
int purge_system_get_days_remaining(const PurgeState* state);

/**
 * Get years remaining until purge
 *
 * Params:
 *   state - Purge state
 *
 * Returns: Years until purge (rounded down)
 */
double purge_system_get_years_remaining(const PurgeState* state);

#endif /* PURGE_SYSTEM_H */

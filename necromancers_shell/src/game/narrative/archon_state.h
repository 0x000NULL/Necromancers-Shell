/**
 * archon_state.h - Archon Transformation System
 *
 * After passing all 7 trials and receiving divine amnesty, the player
 * transforms into an Archon - a divine administrator of the Death Network
 * with special privileges and responsibilities.
 */

#ifndef ARCHON_STATE_H
#define ARCHON_STATE_H

#include <stdbool.h>
#include <stddef.h>

/* Maximum Code of Conduct rules */
#define MAX_CODE_OF_CONDUCT_RULES 5

/* Maximum cutscene text length */
#define MAX_CUTSCENE_LENGTH 4096

/* Minimum/maximum administrative levels */
#define MIN_ADMIN_LEVEL 3
#define MAX_ADMIN_LEVEL 10

/**
 * Administrative privileges
 */
typedef enum {
    PRIVILEGE_NETWORK_PATCHING,    /* Level 1: Fix Death Network bugs */
    PRIVILEGE_SPLIT_ROUTING,        /* Level 2: Route souls to multiple afterlives */
    PRIVILEGE_ENFORCER_COMMAND,     /* Level 5: Direct divine enforcers */
    PRIVILEGE_DIVINE_SUMMONING,     /* Level 8: Call upon gods */
    PRIVILEGE_REALITY_EDITING,      /* Level 10: Reshape Death Network */
    PRIVILEGE_COUNT
} ArchonPrivilege;

/**
 * Privilege information
 */
typedef struct {
    ArchonPrivilege privilege;
    int level_required;
    bool unlocked;
    char name[64];
    char description[256];
} PrivilegeInfo;

/**
 * Archon transformation state
 */
typedef struct {
    bool transformed;               /* False → true upon transformation */
    int transformation_day;         /* Game day of ascension */

    int administrative_level;       /* 3-10 (starts at 3) */
    int corruption_locked;          /* Corruption % at transformation (locked) */

    int necromancers_reformed;      /* 0-147 reformed */
    int reforms_per_level;          /* Reforms needed for next level (15) */

    char code_of_conduct[MAX_CODE_OF_CONDUCT_RULES][256];
    size_t rule_count;

    PrivilegeInfo privileges[PRIVILEGE_COUNT];

    bool divine_recognition[7];     /* Recognition from each god */

    /* Cutscene text */
    char transformation_cutscene[MAX_CUTSCENE_LENGTH];
} ArchonState;

/**
 * Create Archon state
 *
 * Returns: Newly allocated Archon state, or NULL on failure
 */
ArchonState* archon_state_create(void);

/**
 * Destroy Archon state
 *
 * Params:
 *   state - Archon state to destroy
 */
void archon_state_destroy(ArchonState* state);

/**
 * Perform Archon transformation
 *
 * Validates all requirements and transforms the player into an Archon.
 *
 * Params:
 *   state - Archon state
 *   all_trials_passed - True if completed all 7 trials
 *   amnesty_granted - True if Divine Council approved
 *   corruption - Current player corruption %
 *   restrictions - Code of Conduct from judgment
 *   restriction_count - Number of restrictions
 *   game_day - Current game day
 *   thessara_alive - True if Thessara not sacrificed in Trial 6
 *
 * Returns: True if transformation successful
 */
bool archon_transform(ArchonState* state, bool all_trials_passed,
                     bool amnesty_granted, int corruption,
                     const char** restrictions, size_t restriction_count,
                     int game_day, bool thessara_alive);

/**
 * Grant an administrative privilege
 *
 * Unlocks a privilege based on administrative level.
 *
 * Params:
 *   state - Archon state
 *   privilege - Privilege to grant
 *
 * Returns: True if privilege granted successfully
 */
bool archon_grant_privilege(ArchonState* state, ArchonPrivilege privilege);

/**
 * Advance administrative level
 *
 * Increases level after reforming sufficient necromancers.
 * Each level requires 15 reforms.
 *
 * Params:
 *   state - Archon state
 *
 * Returns: True if level increased, false if max level or insufficient reforms
 */
bool archon_advance_level(ArchonState* state);

/**
 * Reform a necromancer
 *
 * Adds to reformed count and progresses toward next administrative level.
 *
 * Params:
 *   state - Archon state
 *   npc_id - NPC to reform
 *
 * Returns: True if reformed successfully
 */
bool archon_reform_necromancer(ArchonState* state, int npc_id);

/**
 * Check for Code of Conduct violation
 *
 * Validates if an action violates the Code of Conduct.
 *
 * Params:
 *   state - Archon state
 *   action_description - Description of action to check
 *   corruption_increase - Corruption change from action
 *
 * Returns: True if action violates Code, false if permitted
 */
bool archon_check_code_violation(const ArchonState* state,
                                 const char* action_description,
                                 int corruption_increase);

/**
 * Get transformation cutscene text
 *
 * Multi-paragraph narrative of the transformation ceremony.
 *
 * Params:
 *   state - Archon state
 *
 * Returns: Pointer to cutscene text buffer
 */
const char* archon_get_transformation_cutscene(const ArchonState* state);

/**
 * Check if privilege is unlocked
 *
 * Params:
 *   state - Archon state
 *   privilege - Privilege to check
 *
 * Returns: True if unlocked
 */
bool archon_is_privilege_unlocked(const ArchonState* state,
                                  ArchonPrivilege privilege);

/**
 * Get privilege information
 *
 * Params:
 *   state - Archon state
 *   privilege - Privilege to query
 *
 * Returns: Pointer to privilege info, or NULL if invalid
 */
const PrivilegeInfo* archon_get_privilege_info(const ArchonState* state,
                                               ArchonPrivilege privilege);

/**
 * Get reforms needed for next level
 *
 * Params:
 *   state - Archon state
 *
 * Returns: Number of additional reforms needed
 */
int archon_get_reforms_needed_for_next_level(const ArchonState* state);

/**
 * Get privilege name
 *
 * Params:
 *   privilege - Privilege type
 *
 * Returns: Name string
 */
const char* archon_get_privilege_name(ArchonPrivilege privilege);

#endif /* ARCHON_STATE_H */

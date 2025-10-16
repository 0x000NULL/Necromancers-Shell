/**
 * network_patching.h - Death Network Patching System
 *
 * After becoming Archon, gain access to administrative tools to fix bugs
 * discovered in Trial 4. Deploy patches to improve network stability and
 * justice. Each bug fix improves overall network health.
 */

#ifndef NETWORK_PATCHING_H
#define NETWORK_PATCHING_H

#include <stdbool.h>
#include <stddef.h>

/* Total bugs in the Death Network (from Trial 4) */
#define TOTAL_NETWORK_BUGS 27

/* Maximum patch history entries */
#define MAX_PATCH_HISTORY 100

/* Maximum bug description length */
#define MAX_BUG_DESC_LENGTH 256

/**
 * Patch deployment result
 */
typedef enum {
    PATCH_SUCCESS,          /* Patch deployed successfully */
    PATCH_FAILED,           /* Deployment failed (5% chance) */
    PATCH_ALREADY_FIXED,    /* Bug already patched */
    PATCH_NOT_DISCOVERED,   /* Bug not discovered in Trial 4 */
    PATCH_ACCESS_DENIED     /* Insufficient admin level */
} PatchResult;

/**
 * Network bug information
 */
typedef struct {
    int bug_id;                           /* Unique bug identifier (1-27) */
    char description[MAX_BUG_DESC_LENGTH]; /* Bug description */
    bool discovered;                       /* Found in Trial 4 */
    bool patched;                          /* Fixed by player */
    int admin_level_required;              /* Minimum level to patch */
    double impact_percentage;              /* Health improvement per bug */
} NetworkBug;

/**
 * Patch deployment log entry
 */
typedef struct {
    int day;            /* Game day of deployment */
    int bug_id;         /* Bug that was patched */
    PatchResult result; /* Success or failure */
    char description[MAX_BUG_DESC_LENGTH]; /* What was fixed */
} PatchLogEntry;

/**
 * Network patching state
 */
typedef struct {
    NetworkBug bugs[TOTAL_NETWORK_BUGS]; /* All bugs in database */

    int bugs_discovered;    /* Bugs found in Trial 4 */
    int bugs_patched;       /* Bugs successfully fixed */
    int patches_deployed;   /* Total deployment attempts */
    int patches_failed;     /* Failed deployments */

    double network_health;  /* Overall health (0-100%) */
    int admin_level;        /* Player's administrative level */

    PatchLogEntry history[MAX_PATCH_HISTORY]; /* Deployment log */
    size_t history_count;
} NetworkPatchingState;

/**
 * Create network patching state
 *
 * Returns: Newly allocated patching state, or NULL on failure
 */
NetworkPatchingState* network_patching_create(void);

/**
 * Destroy network patching state
 *
 * Params:
 *   state - Patching state to destroy
 */
void network_patching_destroy(NetworkPatchingState* state);

/**
 * Initialize network patching system
 *
 * Loads bug database and imports discovered bugs from Trial 4.
 *
 * Params:
 *   state - Patching state
 *   discovered_bugs - Array of bug IDs found in Trial 4
 *   discovered_count - Number of bugs discovered
 *   admin_level - Player's administrative level (1-10)
 *
 * Returns: True on success, false on failure
 */
bool network_patching_initialize(NetworkPatchingState* state,
                                const int* discovered_bugs,
                                size_t discovered_count,
                                int admin_level);

/**
 * Deploy a patch to fix a bug
 *
 * Attempts to fix a discovered bug. Has 95% success rate.
 * Requires sufficient administrative level.
 *
 * Params:
 *   state - Patching state
 *   bug_id - Bug to fix (1-27)
 *   game_day - Current game day (for logging)
 *
 * Returns: Result of patch deployment
 */
PatchResult network_patching_deploy_patch(NetworkPatchingState* state,
                                          int bug_id, int game_day);

/**
 * Test a patch in sandbox environment
 *
 * Tests patch without deploying it. Always succeeds, shows expected impact.
 *
 * Params:
 *   state - Patching state
 *   bug_id - Bug to test
 *   expected_improvement - Output: health improvement if deployed
 *
 * Returns: True if test succeeded, false if bug invalid/not discovered
 */
bool network_patching_test_patch(const NetworkPatchingState* state,
                                int bug_id, double* expected_improvement);

/**
 * Get network statistics
 *
 * Params:
 *   state - Patching state
 *   total_bugs - Output: total bugs in database (27)
 *   discovered - Output: bugs discovered in Trial 4
 *   patched - Output: bugs successfully fixed
 *   health - Output: current network health (0-100%)
 */
void network_patching_get_stats(const NetworkPatchingState* state,
                               int* total_bugs, int* discovered,
                               int* patched, double* health);

/**
 * Calculate network health improvement
 *
 * Params:
 *   state - Patching state
 *
 * Returns: Current network health percentage (0-100%)
 */
double network_patching_calculate_health(const NetworkPatchingState* state);

/**
 * Get bug information
 *
 * Params:
 *   state - Patching state
 *   bug_id - Bug to query (1-27)
 *
 * Returns: Pointer to bug info, or NULL if invalid
 */
const NetworkBug* network_patching_get_bug(const NetworkPatchingState* state,
                                          int bug_id);

/**
 * Get patch history
 *
 * Params:
 *   state - Patching state
 *   count_out - Output: number of history entries
 *
 * Returns: Pointer to history array
 */
const PatchLogEntry* network_patching_get_history(const NetworkPatchingState* state,
                                                  size_t* count_out);

/**
 * Get patch result as string
 *
 * Params:
 *   result - Patch result
 *
 * Returns: Human-readable result string
 */
const char* network_patching_result_to_string(PatchResult result);

#endif /* NETWORK_PATCHING_H */

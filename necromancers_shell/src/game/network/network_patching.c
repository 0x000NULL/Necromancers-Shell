/**
 * network_patching.c - Death Network Patching System Implementation
 */

#include "network_patching.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Improvement per bug fix */
#define IMPROVEMENT_PER_BUG 3.7

/* Patch success rate */
#define BASE_SUCCESS_RATE 95

/* Bug database (from Trial 4) */
static const struct {
    int id;
    const char* description;
    int admin_level;
} BUG_DATABASE[TOTAL_NETWORK_BUGS] = {
    {1, "Soul routing infinite loop for indeterminate cases", 1},
    {2, "Memory leak in consciousness transfer protocol", 1},
    {3, "Race condition in parallel soul processing", 2},
    {4, "Buffer overflow in soul metadata parser", 1},
    {5, "Incorrect permission checking for afterlife access", 3},
    {6, "Deadlock in multi-realm transaction handler", 3},
    {7, "Integer overflow in karma calculation", 2},
    {8, "SQL injection vulnerability in soul query system", 4},
    {9, "Cross-realm scripting attack vector", 4},
    {10, "Privilege escalation via soul binding exploit", 5},
    {11, "Denial of service through malformed soul packets", 2},
    {12, "Time-of-check to time-of-use race in judgment", 3},
    {13, "Unvalidated redirect to unauthorized afterlife", 3},
    {14, "Hardcoded credentials in legacy reaper module", 5},
    {15, "Insecure random number generation for fate rolls", 2},
    {16, "Path traversal in soul archive access", 4},
    {17, "XML external entity injection in divine petitions", 4},
    {18, "Use-after-free in soul fragmentation code", 5},
    {19, "NULL pointer dereference in void routing", 1},
    {20, "Double-free in consciousness cleanup routine", 2},
    {21, "Stack overflow in recursive karma aggregation", 3},
    {22, "Heap corruption in soul merger operation", 5},
    {23, "Format string vulnerability in logging system", 2},
    {24, "Uninitialized variable in trial scoring module", 1},
    {25, "Off-by-one error in reincarnation queue", 2},
    {26, "Type confusion in polymorphic soul entities", 4},
    {27, "Missing bounds check in afterlife allocation", 3}
};

NetworkPatchingState* network_patching_create(void) {
    NetworkPatchingState* state = calloc(1, sizeof(NetworkPatchingState));
    if (!state) {
        return NULL;
    }

    /* Initialize bug database */
    for (int i = 0; i < TOTAL_NETWORK_BUGS; i++) {
        state->bugs[i].bug_id = BUG_DATABASE[i].id;
        strncpy(state->bugs[i].description, BUG_DATABASE[i].description,
                MAX_BUG_DESC_LENGTH - 1);
        state->bugs[i].admin_level_required = BUG_DATABASE[i].admin_level;
        state->bugs[i].impact_percentage = IMPROVEMENT_PER_BUG;
        state->bugs[i].discovered = false;
        state->bugs[i].patched = false;
    }

    state->network_health = 0.0;
    state->history_count = 0;

    return state;
}

void network_patching_destroy(NetworkPatchingState* state) {
    free(state);
}

bool network_patching_initialize(NetworkPatchingState* state,
                                const int* discovered_bugs,
                                size_t discovered_count,
                                int admin_level) {
    if (!state || !discovered_bugs) {
        return false;
    }

    state->admin_level = admin_level;
    state->bugs_discovered = 0;

    /* Mark bugs as discovered */
    for (size_t i = 0; i < discovered_count; i++) {
        int bug_id = discovered_bugs[i];
        if (bug_id >= 1 && bug_id <= TOTAL_NETWORK_BUGS) {
            state->bugs[bug_id - 1].discovered = true;
            state->bugs_discovered++;
        }
    }

    /* Calculate initial network health (0% if nothing patched) */
    state->network_health = network_patching_calculate_health(state);

    return true;
}

PatchResult network_patching_deploy_patch(NetworkPatchingState* state,
                                          int bug_id, int game_day) {
    if (!state || bug_id < 1 || bug_id > TOTAL_NETWORK_BUGS) {
        return PATCH_FAILED;
    }

    NetworkBug* bug = &state->bugs[bug_id - 1];

    /* Check if bug was discovered */
    if (!bug->discovered) {
        return PATCH_NOT_DISCOVERED;
    }

    /* Check if already patched */
    if (bug->patched) {
        return PATCH_ALREADY_FIXED;
    }

    /* Check admin level */
    if (state->admin_level < bug->admin_level_required) {
        return PATCH_ACCESS_DENIED;
    }

    state->patches_deployed++;

    /* 95% success rate */
    int roll = rand() % 100;
    PatchResult result;

    if (roll < BASE_SUCCESS_RATE) {
        /* Success */
        bug->patched = true;
        state->bugs_patched++;
        state->network_health = network_patching_calculate_health(state);
        result = PATCH_SUCCESS;
    } else {
        /* Failure */
        state->patches_failed++;
        result = PATCH_FAILED;
    }

    /* Log deployment */
    if (state->history_count < MAX_PATCH_HISTORY) {
        PatchLogEntry* entry = &state->history[state->history_count++];
        entry->day = game_day;
        entry->bug_id = bug_id;
        entry->result = result;
        strncpy(entry->description, bug->description, MAX_BUG_DESC_LENGTH - 1);
    }

    return result;
}

bool network_patching_test_patch(const NetworkPatchingState* state,
                                int bug_id, double* expected_improvement) {
    if (!state || bug_id < 1 || bug_id > TOTAL_NETWORK_BUGS) {
        return false;
    }

    const NetworkBug* bug = &state->bugs[bug_id - 1];

    if (!bug->discovered) {
        return false;
    }

    if (expected_improvement) {
        *expected_improvement = bug->impact_percentage;
    }

    return true;
}

void network_patching_get_stats(const NetworkPatchingState* state,
                               int* total_bugs, int* discovered,
                               int* patched, double* health) {
    if (!state) {
        return;
    }

    if (total_bugs) {
        *total_bugs = TOTAL_NETWORK_BUGS;
    }

    if (discovered) {
        *discovered = state->bugs_discovered;
    }

    if (patched) {
        *patched = state->bugs_patched;
    }

    if (health) {
        *health = state->network_health;
    }
}

double network_patching_calculate_health(const NetworkPatchingState* state) {
    if (!state) {
        return 0.0;
    }

    /* Each patched bug improves health by 3.7% */
    double health = state->bugs_patched * IMPROVEMENT_PER_BUG;

    /* Cap at 100% */
    if (health > 100.0) {
        health = 100.0;
    }

    return health;
}

const NetworkBug* network_patching_get_bug(const NetworkPatchingState* state,
                                          int bug_id) {
    if (!state || bug_id < 1 || bug_id > TOTAL_NETWORK_BUGS) {
        return NULL;
    }

    return &state->bugs[bug_id - 1];
}

const PatchLogEntry* network_patching_get_history(const NetworkPatchingState* state,
                                                  size_t* count_out) {
    if (!state) {
        if (count_out) {
            *count_out = 0;
        }
        return NULL;
    }

    if (count_out) {
        *count_out = state->history_count;
    }

    return state->history;
}

const char* network_patching_result_to_string(PatchResult result) {
    switch (result) {
        case PATCH_SUCCESS:
            return "SUCCESS";
        case PATCH_FAILED:
            return "FAILED";
        case PATCH_ALREADY_FIXED:
            return "ALREADY_FIXED";
        case PATCH_NOT_DISCOVERED:
            return "NOT_DISCOVERED";
        case PATCH_ACCESS_DENIED:
            return "ACCESS_DENIED";
        default:
            return "UNKNOWN";
    }
}

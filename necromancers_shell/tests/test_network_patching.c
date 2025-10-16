/**
 * test_network_patching.c - Unit tests for Network Patching System
 */

#include "../src/game/network/network_patching.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) do { printf("Running test: %s...", #name); tests_run++; } while(0)
#define PASS() do { printf(" PASSED\n"); tests_passed++; } while(0)

void test_create_destroy(void) {
    TEST(test_create_destroy);

    NetworkPatchingState* state = network_patching_create();
    assert(state != NULL);
    assert(state->bugs_patched == 0);
    assert(state->network_health == 0.0);

    network_patching_destroy(state);
    PASS();
}

void test_initialize(void) {
    TEST(test_initialize);

    NetworkPatchingState* state = network_patching_create();
    int discovered[] = {1, 2, 3, 5, 10};

    bool result = network_patching_initialize(state, discovered, 5, 3);
    assert(result == true);
    assert(state->bugs_discovered == 5);
    assert(state->admin_level == 3);

    network_patching_destroy(state);
    PASS();
}

void test_deploy_patch_success(void) {
    TEST(test_deploy_patch_success);

    NetworkPatchingState* state = network_patching_create();
    int discovered[] = {1};
    network_patching_initialize(state, discovered, 1, 5);

    /* Patch bug 1 */
    PatchResult result = network_patching_deploy_patch(state, 1, 100);
    assert(result == PATCH_SUCCESS || result == PATCH_FAILED);

    if (result == PATCH_SUCCESS) {
        assert(state->bugs_patched == 1);
    }

    network_patching_destroy(state);
    PASS();
}

void test_deploy_patch_not_discovered(void) {
    TEST(test_deploy_patch_not_discovered);

    NetworkPatchingState* state = network_patching_create();
    int discovered[] = {1};
    network_patching_initialize(state, discovered, 1, 5);

    /* Try to patch undiscovered bug */
    PatchResult result = network_patching_deploy_patch(state, 10, 100);
    assert(result == PATCH_NOT_DISCOVERED);

    network_patching_destroy(state);
    PASS();
}

void test_deploy_patch_access_denied(void) {
    TEST(test_deploy_patch_access_denied);

    NetworkPatchingState* state = network_patching_create();
    int discovered[] = {10}; /* Bug 10 requires level 5 */
    network_patching_initialize(state, discovered, 1, 2); /* Only level 2 */

    PatchResult result = network_patching_deploy_patch(state, 10, 100);
    assert(result == PATCH_ACCESS_DENIED);

    network_patching_destroy(state);
    PASS();
}

void test_test_patch(void) {
    TEST(test_test_patch);

    NetworkPatchingState* state = network_patching_create();
    int discovered[] = {1};
    network_patching_initialize(state, discovered, 1, 5);

    double improvement = 0.0;
    bool result = network_patching_test_patch(state, 1, &improvement);
    assert(result == true);
    assert(improvement > 0.0);

    network_patching_destroy(state);
    PASS();
}

void test_calculate_health(void) {
    TEST(test_calculate_health);

    NetworkPatchingState* state = network_patching_create();
    state->bugs_patched = 10;

    double health = network_patching_calculate_health(state);
    assert(health > 0.0 && health <= 100.0);

    network_patching_destroy(state);
    PASS();
}

void test_get_stats(void) {
    TEST(test_get_stats);

    NetworkPatchingState* state = network_patching_create();
    int discovered[] = {1, 2, 3};
    network_patching_initialize(state, discovered, 3, 5);

    int total, disc, patched;
    double health;
    network_patching_get_stats(state, &total, &disc, &patched, &health);

    assert(total == 27);
    assert(disc == 3);
    assert(patched == 0);
    assert(health == 0.0);

    network_patching_destroy(state);
    PASS();
}

void test_get_bug(void) {
    TEST(test_get_bug);

    NetworkPatchingState* state = network_patching_create();

    const NetworkBug* bug = network_patching_get_bug(state, 1);
    assert(bug != NULL);
    assert(bug->bug_id == 1);
    assert(strlen(bug->description) > 0);

    network_patching_destroy(state);
    PASS();
}

void test_result_to_string(void) {
    TEST(test_result_to_string);

    assert(strcmp(network_patching_result_to_string(PATCH_SUCCESS), "SUCCESS") == 0);
    assert(strcmp(network_patching_result_to_string(PATCH_FAILED), "FAILED") == 0);
    assert(strcmp(network_patching_result_to_string(PATCH_ACCESS_DENIED), "ACCESS_DENIED") == 0);

    PASS();
}

int main(void) {
    printf("Running Network Patching System tests...\n\n");

    test_create_destroy();
    test_initialize();
    test_deploy_patch_success();
    test_deploy_patch_not_discovered();
    test_deploy_patch_access_denied();
    test_test_patch();
    test_calculate_health();
    test_get_stats();
    test_get_bug();
    test_result_to_string();

    printf("\n========================================\n");
    printf("Tests passed: %d/%d\n", tests_passed, tests_run);
    printf("========================================\n");

    return (tests_passed == tests_run) ? 0 : 1;
}

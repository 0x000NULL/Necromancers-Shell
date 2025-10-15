/**
 * @file test_npc.c
 * @brief Unit tests for NPC system
 */

#include "../src/game/narrative/npcs/npc.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Test: Create NPC */
void test_npc_create(void) {
    NPC* npc = npc_create("seraphine", "Seraphine the Merciful", NPC_ARCHETYPE_MENTOR);

    assert(npc != NULL);
    assert(strcmp(npc->id, "seraphine") == 0);
    assert(strcmp(npc->name, "Seraphine the Merciful") == 0);
    assert(npc->archetype == NPC_ARCHETYPE_MENTOR);
    assert(npc->available == true);
    assert(npc->discovered == false);
    assert(npc->is_hostile == false);
    assert(npc->is_dead == false);
    assert(npc->interaction_count == 0);

    npc_destroy(npc);
    printf("✓ test_npc_create passed\n");
}

/* Test: Create with NULL parameters */
void test_npc_create_null(void) {
    NPC* npc1 = npc_create(NULL, "Name", NPC_ARCHETYPE_ALLY);
    assert(npc1 == NULL);

    NPC* npc2 = npc_create("id", NULL, NPC_ARCHETYPE_ALLY);
    assert(npc2 == NULL);

    printf("✓ test_npc_create_null passed\n");
}

/* Test: Destroy NULL NPC */
void test_npc_destroy_null(void) {
    npc_destroy(NULL);
    printf("✓ test_npc_destroy_null passed\n");
}

/* Test: Discover NPC */
void test_npc_discover(void) {
    NPC* npc = npc_create("test", "Test NPC", NPC_ARCHETYPE_NEUTRAL);
    assert(npc != NULL);
    assert(npc->discovered == false);

    npc_discover(npc, "forgotten_crypt");

    assert(npc->discovered == true);
    assert(npc->first_met_time > 0);
    assert(strcmp(npc->current_location, "forgotten_crypt") == 0);
    assert(strcmp(npc->home_location, "forgotten_crypt") == 0);

    npc_destroy(npc);
    printf("✓ test_npc_discover passed\n");
}

/* Test: Discover with NULL location */
void test_npc_discover_null_location(void) {
    NPC* npc = npc_create("test", "Test NPC", NPC_ARCHETYPE_NEUTRAL);
    assert(npc != NULL);

    npc_discover(npc, NULL);
    assert(npc->discovered == true);
    assert(npc->first_met_time > 0);

    npc_destroy(npc);
    printf("✓ test_npc_discover_null_location passed\n");
}

/* Test: Interact with NPC */
void test_npc_interact(void) {
    NPC* npc = npc_create("test", "Test NPC", NPC_ARCHETYPE_ALLY);
    assert(npc != NULL);

    assert(npc->interaction_count == 0);
    assert(npc->last_interaction_time == 0);

    npc_interact(npc);
    assert(npc->interaction_count == 1);
    assert(npc->last_interaction_time > 0);

    npc_interact(npc);
    assert(npc->interaction_count == 2);

    npc_destroy(npc);
    printf("✓ test_npc_interact passed\n");
}

/* Test: Dialogue state management */
void test_npc_dialogue_state(void) {
    NPC* npc = npc_create("test", "Test NPC", NPC_ARCHETYPE_MYSTERIOUS);
    assert(npc != NULL);

    /* Default state */
    assert(strcmp(npc->current_dialogue_state, "initial") == 0);

    /* Change state */
    npc_set_dialogue_state(npc, "quest_active");
    assert(strcmp(npc->current_dialogue_state, "quest_active") == 0);

    /* Add available states */
    npc_add_dialogue_state(npc, "friendly");
    npc_add_dialogue_state(npc, "hostile");
    assert(npc->dialogue_state_count == 2);

    assert(npc_has_dialogue_state(npc, "friendly") == true);
    assert(npc_has_dialogue_state(npc, "hostile") == true);
    assert(npc_has_dialogue_state(npc, "unknown") == false);

    npc_destroy(npc);
    printf("✓ test_npc_dialogue_state passed\n");
}

/* Test: Dialogue state - max capacity */
void test_npc_dialogue_state_max(void) {
    NPC* npc = npc_create("test", "Test NPC", NPC_ARCHETYPE_NEUTRAL);
    assert(npc != NULL);

    /* Fill to capacity */
    for (size_t i = 0; i < MAX_NPC_DIALOGUE_STATES; i++) {
        char state[32];
        snprintf(state, sizeof(state), "state_%zu", i);
        npc_add_dialogue_state(npc, state);
    }
    assert(npc->dialogue_state_count == MAX_NPC_DIALOGUE_STATES);

    /* Try to add beyond capacity */
    npc_add_dialogue_state(npc, "overflow");
    assert(npc->dialogue_state_count == MAX_NPC_DIALOGUE_STATES);

    npc_destroy(npc);
    printf("✓ test_npc_dialogue_state_max passed\n");
}

/* Test: Dialogue state - no duplicates */
void test_npc_dialogue_state_duplicates(void) {
    NPC* npc = npc_create("test", "Test NPC", NPC_ARCHETYPE_ALLY);
    assert(npc != NULL);

    npc_add_dialogue_state(npc, "friendly");
    assert(npc->dialogue_state_count == 1);

    /* Try to add same state again */
    npc_add_dialogue_state(npc, "friendly");
    assert(npc->dialogue_state_count == 1);

    npc_destroy(npc);
    printf("✓ test_npc_dialogue_state_duplicates passed\n");
}

/* Test: Quest management */
void test_npc_quest_management(void) {
    NPC* npc = npc_create("test", "Test NPC", NPC_ARCHETYPE_MENTOR);
    assert(npc != NULL);

    /* Add active quests */
    npc_add_active_quest(npc, "quest_1");
    npc_add_active_quest(npc, "quest_2");
    assert(npc->active_quest_count == 2);
    assert(npc->completed_quest_count == 0);

    assert(npc_has_active_quest(npc, "quest_1") == true);
    assert(npc_has_active_quest(npc, "quest_2") == true);
    assert(npc_has_active_quest(npc, "quest_3") == false);

    /* Complete a quest */
    bool completed = npc_complete_quest(npc, "quest_1");
    assert(completed == true);
    assert(npc->active_quest_count == 1);
    assert(npc->completed_quest_count == 1);

    assert(npc_has_active_quest(npc, "quest_1") == false);
    assert(npc_has_completed_quest(npc, "quest_1") == true);

    npc_destroy(npc);
    printf("✓ test_npc_quest_management passed\n");
}

/* Test: Complete nonexistent quest */
void test_npc_complete_nonexistent_quest(void) {
    NPC* npc = npc_create("test", "Test NPC", NPC_ARCHETYPE_ALLY);
    assert(npc != NULL);

    bool completed = npc_complete_quest(npc, "nonexistent");
    assert(completed == false);
    assert(npc->completed_quest_count == 0);

    npc_destroy(npc);
    printf("✓ test_npc_complete_nonexistent_quest passed\n");
}

/* Test: Quest - no duplicates */
void test_npc_quest_duplicates(void) {
    NPC* npc = npc_create("test", "Test NPC", NPC_ARCHETYPE_NEUTRAL);
    assert(npc != NULL);

    npc_add_active_quest(npc, "quest_1");
    assert(npc->active_quest_count == 1);

    /* Try to add same quest again */
    npc_add_active_quest(npc, "quest_1");
    assert(npc->active_quest_count == 1);

    npc_destroy(npc);
    printf("✓ test_npc_quest_duplicates passed\n");
}

/* Test: Unlockable memories */
void test_npc_unlockable_memories(void) {
    NPC* npc = npc_create("test", "Test NPC", NPC_ARCHETYPE_MYSTERIOUS);
    assert(npc != NULL);

    npc_add_unlockable_memory(npc, "memory_1");
    npc_add_unlockable_memory(npc, "memory_2");
    npc_add_unlockable_memory(npc, "memory_3");

    assert(npc->memory_count == 3);
    assert(strcmp(npc->unlockable_memories[0], "memory_1") == 0);
    assert(strcmp(npc->unlockable_memories[1], "memory_2") == 0);
    assert(strcmp(npc->unlockable_memories[2], "memory_3") == 0);

    npc_destroy(npc);
    printf("✓ test_npc_unlockable_memories passed\n");
}

/* Test: Unlockable memories - max capacity */
void test_npc_unlockable_memories_max(void) {
    NPC* npc = npc_create("test", "Test NPC", NPC_ARCHETYPE_MENTOR);
    assert(npc != NULL);

    /* Fill to capacity */
    for (size_t i = 0; i < MAX_NPC_MEMORIES; i++) {
        char memory[32];
        snprintf(memory, sizeof(memory), "memory_%zu", i);
        npc_add_unlockable_memory(npc, memory);
    }
    assert(npc->memory_count == MAX_NPC_MEMORIES);

    /* Try to add beyond capacity */
    npc_add_unlockable_memory(npc, "overflow");
    assert(npc->memory_count == MAX_NPC_MEMORIES);

    npc_destroy(npc);
    printf("✓ test_npc_unlockable_memories_max passed\n");
}

/* Test: Move to location */
void test_npc_move_to_location(void) {
    NPC* npc = npc_create("test", "Test NPC", NPC_ARCHETYPE_NEUTRAL);
    assert(npc != NULL);

    npc_move_to_location(npc, "crypt");
    assert(strcmp(npc->current_location, "crypt") == 0);

    npc_move_to_location(npc, "tower");
    assert(strcmp(npc->current_location, "tower") == 0);

    npc_destroy(npc);
    printf("✓ test_npc_move_to_location passed\n");
}

/* Test: NPC availability */
void test_npc_availability(void) {
    NPC* npc = npc_create("test", "Test NPC", NPC_ARCHETYPE_ALLY);
    assert(npc != NULL);

    /* Default: available */
    assert(npc_is_available(npc) == true);

    /* Dead NPCs not available */
    npc->is_dead = true;
    assert(npc_is_available(npc) == false);
    npc->is_dead = false;

    /* Hidden NPCs not available */
    npc->is_hidden = true;
    assert(npc_is_available(npc) == false);
    npc->is_hidden = false;

    /* Unavailable NPCs not available */
    npc->available = false;
    assert(npc_is_available(npc) == false);
    npc->available = true;

    assert(npc_is_available(npc) == true);

    npc_destroy(npc);
    printf("✓ test_npc_availability passed\n");
}

/* Test: Archetype to string */
void test_npc_archetype_to_string(void) {
    assert(strcmp(npc_archetype_to_string(NPC_ARCHETYPE_MENTOR), "Mentor") == 0);
    assert(strcmp(npc_archetype_to_string(NPC_ARCHETYPE_RIVAL), "Rival") == 0);
    assert(strcmp(npc_archetype_to_string(NPC_ARCHETYPE_ALLY), "Ally") == 0);
    assert(strcmp(npc_archetype_to_string(NPC_ARCHETYPE_ANTAGONIST), "Antagonist") == 0);
    assert(strcmp(npc_archetype_to_string(NPC_ARCHETYPE_NEUTRAL), "Neutral") == 0);
    assert(strcmp(npc_archetype_to_string(NPC_ARCHETYPE_MYSTERIOUS), "Mysterious") == 0);

    printf("✓ test_npc_archetype_to_string passed\n");
}

/* Test: NULL parameter handling */
void test_npc_null_parameters(void) {
    NPC* npc = npc_create("test", "Test NPC", NPC_ARCHETYPE_NEUTRAL);
    assert(npc != NULL);

    /* Functions should handle NULL gracefully */
    npc_discover(NULL, "location");
    npc_interact(NULL);
    npc_set_dialogue_state(NULL, "state");
    npc_set_dialogue_state(npc, NULL);
    npc_add_dialogue_state(NULL, "state");
    npc_add_dialogue_state(npc, NULL);
    npc_add_active_quest(NULL, "quest");
    npc_add_active_quest(npc, NULL);
    npc_add_unlockable_memory(NULL, "memory");
    npc_add_unlockable_memory(npc, NULL);
    npc_move_to_location(NULL, "location");
    npc_move_to_location(npc, NULL);

    assert(npc_has_dialogue_state(NULL, "state") == false);
    assert(npc_has_dialogue_state(npc, NULL) == false);
    assert(npc_has_active_quest(NULL, "quest") == false);
    assert(npc_has_active_quest(npc, NULL) == false);
    assert(npc_has_completed_quest(NULL, "quest") == false);
    assert(npc_has_completed_quest(npc, NULL) == false);
    assert(npc_complete_quest(NULL, "quest") == false);
    assert(npc_complete_quest(npc, NULL) == false);
    assert(npc_is_available(NULL) == false);

    npc_destroy(npc);
    printf("✓ test_npc_null_parameters passed\n");
}

/* Run all tests */
int main(void) {
    printf("Running NPC tests...\n\n");

    test_npc_create();
    test_npc_create_null();
    test_npc_destroy_null();
    test_npc_discover();
    test_npc_discover_null_location();
    test_npc_interact();
    test_npc_dialogue_state();
    test_npc_dialogue_state_max();
    test_npc_dialogue_state_duplicates();
    test_npc_quest_management();
    test_npc_complete_nonexistent_quest();
    test_npc_quest_duplicates();
    test_npc_unlockable_memories();
    test_npc_unlockable_memories_max();
    test_npc_move_to_location();
    test_npc_availability();
    test_npc_archetype_to_string();
    test_npc_null_parameters();

    printf("\n✅ All NPC tests passed! (18/18)\n");
    return 0;
}

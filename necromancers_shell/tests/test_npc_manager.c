/**
 * @file test_npc_manager.c
 * @brief Unit tests for NPC manager
 */

#include "../src/game/narrative/npcs/npc_manager.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Test: Create NPC manager */
void test_npc_manager_create(void) {
    NPCManager* manager = npc_manager_create();

    assert(manager != NULL);
    assert(manager->npc_count == 0);
    assert(manager->npc_capacity > 0);
    assert(manager->npcs != NULL);

    npc_manager_destroy(manager);
    printf("✓ test_npc_manager_create passed\n");
}

/* Test: Destroy NULL manager */
void test_npc_manager_destroy_null(void) {
    npc_manager_destroy(NULL);
    printf("✓ test_npc_manager_destroy_null passed\n");
}

/* Test: Add NPC */
void test_npc_manager_add_npc(void) {
    NPCManager* manager = npc_manager_create();
    assert(manager != NULL);

    NPC* npc = npc_create("seraphine", "Seraphine", NPC_ARCHETYPE_MENTOR);
    assert(npc != NULL);

    npc_manager_add_npc(manager, npc);
    assert(manager->npc_count == 1);

    npc_manager_destroy(manager);
    printf("✓ test_npc_manager_add_npc passed\n");
}

/* Test: Add multiple NPCs */
void test_npc_manager_add_multiple(void) {
    NPCManager* manager = npc_manager_create();
    assert(manager != NULL);

    for (int i = 0; i < 5; i++) {
        char id[32], name[64];
        snprintf(id, sizeof(id), "npc_%d", i);
        snprintf(name, sizeof(name), "NPC %d", i);

        NPC* npc = npc_create(id, name, NPC_ARCHETYPE_NEUTRAL);
        assert(npc != NULL);
        npc_manager_add_npc(manager, npc);
    }

    assert(manager->npc_count == 5);

    npc_manager_destroy(manager);
    printf("✓ test_npc_manager_add_multiple passed\n");
}

/* Test: Add with NULL parameters */
void test_npc_manager_add_null(void) {
    NPCManager* manager = npc_manager_create();
    assert(manager != NULL);

    NPC* npc = npc_create("test", "Test", NPC_ARCHETYPE_ALLY);
    assert(npc != NULL);

    npc_manager_add_npc(NULL, npc);
    npc_manager_add_npc(manager, NULL);

    assert(manager->npc_count == 0);

    npc_destroy(npc);
    npc_manager_destroy(manager);
    printf("✓ test_npc_manager_add_null passed\n");
}

/* Test: Get NPC by ID */
void test_npc_manager_get_npc(void) {
    NPCManager* manager = npc_manager_create();
    assert(manager != NULL);

    NPC* npc1 = npc_create("seraphine", "Seraphine", NPC_ARCHETYPE_MENTOR);
    NPC* npc2 = npc_create("aldric", "Aldric", NPC_ARCHETYPE_RIVAL);
    npc_manager_add_npc(manager, npc1);
    npc_manager_add_npc(manager, npc2);

    NPC* found = npc_manager_get_npc(manager, "seraphine");
    assert(found != NULL);
    assert(strcmp(found->id, "seraphine") == 0);

    found = npc_manager_get_npc(manager, "aldric");
    assert(found != NULL);
    assert(strcmp(found->id, "aldric") == 0);

    found = npc_manager_get_npc(manager, "nonexistent");
    assert(found == NULL);

    npc_manager_destroy(manager);
    printf("✓ test_npc_manager_get_npc passed\n");
}

/* Test: Get discovered NPCs */
void test_npc_manager_get_discovered(void) {
    NPCManager* manager = npc_manager_create();
    assert(manager != NULL);

    /* Add NPCs, some discovered */
    NPC* npc1 = npc_create("npc1", "NPC 1", NPC_ARCHETYPE_ALLY);
    npc_discover(npc1, "crypt");
    npc_manager_add_npc(manager, npc1);

    NPC* npc2 = npc_create("npc2", "NPC 2", NPC_ARCHETYPE_NEUTRAL);
    npc_manager_add_npc(manager, npc2);

    NPC* npc3 = npc_create("npc3", "NPC 3", NPC_ARCHETYPE_MENTOR);
    npc_discover(npc3, "tower");
    npc_manager_add_npc(manager, npc3);

    /* Get discovered */
    size_t count;
    NPC** discovered = npc_manager_get_discovered(manager, &count);

    assert(discovered != NULL);
    assert(count == 2);

    free(discovered);
    npc_manager_destroy(manager);
    printf("✓ test_npc_manager_get_discovered passed\n");
}

/* Test: Get discovered - none found */
void test_npc_manager_get_discovered_empty(void) {
    NPCManager* manager = npc_manager_create();
    assert(manager != NULL);

    /* Add undiscovered NPCs */
    NPC* npc = npc_create("npc1", "NPC 1", NPC_ARCHETYPE_NEUTRAL);
    npc_manager_add_npc(manager, npc);

    size_t count;
    NPC** discovered = npc_manager_get_discovered(manager, &count);

    assert(discovered == NULL);
    assert(count == 0);

    npc_manager_destroy(manager);
    printf("✓ test_npc_manager_get_discovered_empty passed\n");
}

/* Test: Get NPCs at location */
void test_npc_manager_get_at_location(void) {
    NPCManager* manager = npc_manager_create();
    assert(manager != NULL);

    /* Add NPCs at different locations */
    NPC* npc1 = npc_create("npc1", "NPC 1", NPC_ARCHETYPE_ALLY);
    npc_move_to_location(npc1, "crypt");
    npc_manager_add_npc(manager, npc1);

    NPC* npc2 = npc_create("npc2", "NPC 2", NPC_ARCHETYPE_NEUTRAL);
    npc_move_to_location(npc2, "tower");
    npc_manager_add_npc(manager, npc2);

    NPC* npc3 = npc_create("npc3", "NPC 3", NPC_ARCHETYPE_MENTOR);
    npc_move_to_location(npc3, "crypt");
    npc_manager_add_npc(manager, npc3);

    /* Get NPCs at crypt */
    size_t count;
    NPC** at_location = npc_manager_get_at_location(manager, "crypt", &count);

    assert(at_location != NULL);
    assert(count == 2);

    free(at_location);
    npc_manager_destroy(manager);
    printf("✓ test_npc_manager_get_at_location passed\n");
}

/* Test: Get available NPCs */
void test_npc_manager_get_available(void) {
    NPCManager* manager = npc_manager_create();
    assert(manager != NULL);

    /* Add NPCs with different availability */
    NPC* npc1 = npc_create("npc1", "NPC 1", NPC_ARCHETYPE_ALLY);
    npc_manager_add_npc(manager, npc1);

    NPC* npc2 = npc_create("npc2", "NPC 2", NPC_ARCHETYPE_NEUTRAL);
    npc2->is_dead = true;
    npc_manager_add_npc(manager, npc2);

    NPC* npc3 = npc_create("npc3", "NPC 3", NPC_ARCHETYPE_MENTOR);
    npc3->is_hidden = true;
    npc_manager_add_npc(manager, npc3);

    NPC* npc4 = npc_create("npc4", "NPC 4", NPC_ARCHETYPE_RIVAL);
    npc_manager_add_npc(manager, npc4);

    /* Get available */
    size_t count;
    NPC** available = npc_manager_get_available(manager, &count);

    assert(available != NULL);
    assert(count == 2); /* npc1 and npc4 */

    free(available);
    npc_manager_destroy(manager);
    printf("✓ test_npc_manager_get_available passed\n");
}

/* Test: Get NPCs by archetype */
void test_npc_manager_get_by_archetype(void) {
    NPCManager* manager = npc_manager_create();
    assert(manager != NULL);

    /* Add NPCs with different archetypes */
    NPC* npc1 = npc_create("npc1", "NPC 1", NPC_ARCHETYPE_MENTOR);
    npc_manager_add_npc(manager, npc1);

    NPC* npc2 = npc_create("npc2", "NPC 2", NPC_ARCHETYPE_RIVAL);
    npc_manager_add_npc(manager, npc2);

    NPC* npc3 = npc_create("npc3", "NPC 3", NPC_ARCHETYPE_MENTOR);
    npc_manager_add_npc(manager, npc3);

    /* Get mentors */
    size_t count;
    NPC** mentors = npc_manager_get_by_archetype(manager, NPC_ARCHETYPE_MENTOR, &count);

    assert(mentors != NULL);
    assert(count == 2);

    free(mentors);
    npc_manager_destroy(manager);
    printf("✓ test_npc_manager_get_by_archetype passed\n");
}

/* Test: Get NPCs by faction */
void test_npc_manager_get_by_faction(void) {
    NPCManager* manager = npc_manager_create();
    assert(manager != NULL);

    /* Add NPCs with different factions */
    NPC* npc1 = npc_create("npc1", "NPC 1", NPC_ARCHETYPE_ALLY);
    strncpy(npc1->faction, "church", sizeof(npc1->faction) - 1);
    npc_manager_add_npc(manager, npc1);

    NPC* npc2 = npc_create("npc2", "NPC 2", NPC_ARCHETYPE_NEUTRAL);
    strncpy(npc2->faction, "guild", sizeof(npc2->faction) - 1);
    npc_manager_add_npc(manager, npc2);

    NPC* npc3 = npc_create("npc3", "NPC 3", NPC_ARCHETYPE_MENTOR);
    strncpy(npc3->faction, "church", sizeof(npc3->faction) - 1);
    npc_manager_add_npc(manager, npc3);

    /* Get church members */
    size_t count;
    NPC** church = npc_manager_get_by_faction(manager, "church", &count);

    assert(church != NULL);
    assert(count == 2);

    free(church);
    npc_manager_destroy(manager);
    printf("✓ test_npc_manager_get_by_faction passed\n");
}

/* Test: Get NPCs with active quests */
void test_npc_manager_get_with_active_quests(void) {
    NPCManager* manager = npc_manager_create();
    assert(manager != NULL);

    /* Add NPCs, some with active quests */
    NPC* npc1 = npc_create("npc1", "NPC 1", NPC_ARCHETYPE_MENTOR);
    npc_add_active_quest(npc1, "quest_1");
    npc_manager_add_npc(manager, npc1);

    NPC* npc2 = npc_create("npc2", "NPC 2", NPC_ARCHETYPE_NEUTRAL);
    npc_manager_add_npc(manager, npc2);

    NPC* npc3 = npc_create("npc3", "NPC 3", NPC_ARCHETYPE_ALLY);
    npc_add_active_quest(npc3, "quest_2");
    npc_add_active_quest(npc3, "quest_3");
    npc_manager_add_npc(manager, npc3);

    /* Get NPCs with active quests */
    size_t count;
    NPC** with_quests = npc_manager_get_with_active_quests(manager, &count);

    assert(with_quests != NULL);
    assert(count == 2);

    free(with_quests);
    npc_manager_destroy(manager);
    printf("✓ test_npc_manager_get_with_active_quests passed\n");
}

/* Test: Discover NPC via manager */
void test_npc_manager_discover_npc(void) {
    NPCManager* manager = npc_manager_create();
    assert(manager != NULL);

    NPC* npc = npc_create("seraphine", "Seraphine", NPC_ARCHETYPE_MENTOR);
    npc_manager_add_npc(manager, npc);

    assert(npc->discovered == false);

    npc_manager_discover_npc(manager, "seraphine", "crypt");

    assert(npc->discovered == true);
    assert(strcmp(npc->current_location, "crypt") == 0);

    npc_manager_destroy(manager);
    printf("✓ test_npc_manager_discover_npc passed\n");
}

/* Test: Discover nonexistent NPC */
void test_npc_manager_discover_nonexistent(void) {
    NPCManager* manager = npc_manager_create();
    assert(manager != NULL);

    /* Should not crash */
    npc_manager_discover_npc(manager, "nonexistent", "location");

    npc_manager_destroy(manager);
    printf("✓ test_npc_manager_discover_nonexistent passed\n");
}

/* Test: NULL parameter handling */
void test_npc_manager_null_parameters(void) {
    NPCManager* manager = npc_manager_create();
    assert(manager != NULL);

    size_t count;

    /* Functions should handle NULL gracefully */
    assert(npc_manager_get_npc(NULL, "id") == NULL);
    assert(npc_manager_get_npc(manager, NULL) == NULL);

    assert(npc_manager_get_discovered(NULL, &count) == NULL);
    assert(npc_manager_get_at_location(NULL, "location", &count) == NULL);
    assert(npc_manager_get_at_location(manager, NULL, &count) == NULL);
    assert(npc_manager_get_available(NULL, &count) == NULL);
    assert(npc_manager_get_by_archetype(NULL, NPC_ARCHETYPE_ALLY, &count) == NULL);
    assert(npc_manager_get_by_faction(NULL, "faction", &count) == NULL);
    assert(npc_manager_get_by_faction(manager, NULL, &count) == NULL);
    assert(npc_manager_get_with_active_quests(NULL, &count) == NULL);

    npc_manager_discover_npc(NULL, "id", "location");
    npc_manager_discover_npc(manager, NULL, "location");

    npc_manager_destroy(manager);
    printf("✓ test_npc_manager_null_parameters passed\n");
}

/* Run all tests */
int main(void) {
    printf("Running NPC manager tests...\n\n");

    test_npc_manager_create();
    test_npc_manager_destroy_null();
    test_npc_manager_add_npc();
    test_npc_manager_add_multiple();
    test_npc_manager_add_null();
    test_npc_manager_get_npc();
    test_npc_manager_get_discovered();
    test_npc_manager_get_discovered_empty();
    test_npc_manager_get_at_location();
    test_npc_manager_get_available();
    test_npc_manager_get_by_archetype();
    test_npc_manager_get_by_faction();
    test_npc_manager_get_with_active_quests();
    test_npc_manager_discover_npc();
    test_npc_manager_discover_nonexistent();
    test_npc_manager_null_parameters();

    printf("\n✅ All NPC manager tests passed! (16/16)\n");
    return 0;
}

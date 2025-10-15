#include "game/resources/corruption.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

int main(void) {
    printf("Running Corruption Tiers tests...\n");

    CorruptionState corr;

    /* Test 1: Initial state */
    printf("Test 1: Initial state... ");
    corruption_init(&corr);
    assert(corr.corruption == 0);
    assert(corruption_get_tier(&corr) == CORRUPTION_TIER_0);
    assert(corruption_is_pure(&corr) == true);
    printf("PASS\n");

    /* Test 2: All 11 tiers */
    printf("Test 2: All 11 corruption tiers... ");

    corr.corruption = 5;
    assert(corruption_get_tier(&corr) == CORRUPTION_TIER_0);

    corr.corruption = 15;
    assert(corruption_get_tier(&corr) == CORRUPTION_TIER_1);

    corr.corruption = 25;
    assert(corruption_get_tier(&corr) == CORRUPTION_TIER_2);

    corr.corruption = 35;
    assert(corruption_get_tier(&corr) == CORRUPTION_TIER_3);

    corr.corruption = 45;
    assert(corruption_get_tier(&corr) == CORRUPTION_TIER_4);

    corr.corruption = 55;
    assert(corruption_get_tier(&corr) == CORRUPTION_TIER_5);

    corr.corruption = 65;
    assert(corruption_get_tier(&corr) == CORRUPTION_TIER_6);

    corr.corruption = 70;
    assert(corruption_get_tier(&corr) == CORRUPTION_TIER_7);

    corr.corruption = 75;
    assert(corruption_get_tier(&corr) == CORRUPTION_TIER_8);

    corr.corruption = 95;
    assert(corruption_get_tier(&corr) == CORRUPTION_TIER_9);

    corr.corruption = 100;
    assert(corruption_get_tier(&corr) == CORRUPTION_TIER_10);

    printf("PASS\n");

    /* Test 3: Tier boundary precision (69% vs 70% vs 71%) */
    printf("Test 3: Tier boundary precision... ");
    corr.corruption = 69;
    assert(corruption_get_tier(&corr) == CORRUPTION_TIER_6);
    assert(corruption_is_irreversible(&corr) == false);

    corr.corruption = 70;
    assert(corruption_get_tier(&corr) == CORRUPTION_TIER_7);
    assert(corruption_is_irreversible(&corr) == true);

    corr.corruption = 71;
    assert(corruption_get_tier(&corr) == CORRUPTION_TIER_8);
    assert(corruption_is_irreversible(&corr) == true);
    printf("PASS\n");

    /* Test 4: Tier names */
    printf("Test 4: Tier names... ");
    assert(strcmp(corruption_tier_name(CORRUPTION_TIER_0), "Pristine") == 0);
    assert(strcmp(corruption_tier_name(CORRUPTION_TIER_7), "IRREVERSIBLE") == 0);
    assert(strcmp(corruption_tier_name(CORRUPTION_TIER_10), "Lich Lord") == 0);
    printf("PASS\n");

    /* Test 5: Tier descriptions */
    printf("Test 5: Tier descriptions... ");
    corruption_init(&corr);
    corr.corruption = 0;
    const char* desc = corruption_get_description(&corr);
    assert(strstr(desc, "corpses as people") != NULL);

    corr.corruption = 70;
    desc = corruption_get_description(&corr);
    assert(strstr(desc, "unrouteable") != NULL);
    printf("PASS\n");

    /* Test 6: Irreversible threshold check */
    printf("Test 6: Irreversible threshold... ");
    corruption_init(&corr);
    corr.corruption = 69;
    assert(corruption_is_irreversible(&corr) == false);

    corr.corruption = 70;
    assert(corruption_is_irreversible(&corr) == true);

    corr.corruption = 85;
    assert(corruption_is_irreversible(&corr) == true);
    printf("PASS\n");

    /* Test 7: Revenant path availability */
    printf("Test 7: Revenant path availability... ");
    corruption_init(&corr);
    corr.corruption = 25;
    assert(corruption_revenant_available(&corr) == true);

    corr.corruption = 29;
    assert(corruption_revenant_available(&corr) == true);

    corr.corruption = 30;
    assert(corruption_revenant_available(&corr) == false);

    corr.corruption = 50;
    assert(corruption_revenant_available(&corr) == false);
    printf("PASS\n");

    /* Test 8: Wraith path availability */
    printf("Test 8: Wraith path availability... ");
    corruption_init(&corr);
    corr.corruption = 35;
    assert(corruption_wraith_available(&corr) == true);

    corr.corruption = 39;
    assert(corruption_wraith_available(&corr) == true);

    corr.corruption = 40;
    assert(corruption_wraith_available(&corr) == false);

    corr.corruption = 70;
    assert(corruption_wraith_available(&corr) == false);
    printf("PASS\n");

    /* Test 9: Archon path availability */
    printf("Test 9: Archon path availability... ");
    corruption_init(&corr);
    corr.corruption = 25;
    assert(corruption_archon_available(&corr) == false);  /* Too pure */

    corr.corruption = 30;
    assert(corruption_archon_available(&corr) == true);  /* Sweet spot start */

    corr.corruption = 45;
    assert(corruption_archon_available(&corr) == true);  /* Middle */

    corr.corruption = 60;
    assert(corruption_archon_available(&corr) == true);  /* Sweet spot end */

    corr.corruption = 61;
    assert(corruption_archon_available(&corr) == false);  /* Too corrupted */

    corr.corruption = 80;
    assert(corruption_archon_available(&corr) == false);  /* Way too corrupted */
    printf("PASS\n");

    /* Test 10: Pure vs Damned checks */
    printf("Test 10: Pure vs Damned checks... ");
    corruption_init(&corr);
    corr.corruption = 5;
    assert(corruption_is_pure(&corr) == true);
    assert(corruption_is_damned(&corr) == false);

    corr.corruption = 11;
    assert(corruption_is_pure(&corr) == false);

    corr.corruption = 89;
    assert(corruption_is_damned(&corr) == false);

    corr.corruption = 90;
    assert(corruption_is_damned(&corr) == true);

    corr.corruption = 100;
    assert(corruption_is_damned(&corr) == true);
    printf("PASS\n");

    /* Test 11: Path lockouts after 70% */
    printf("Test 11: Path lockouts after 70%%... ");
    corruption_init(&corr);
    corr.corruption = 70;
    assert(corruption_is_irreversible(&corr) == true);
    assert(corruption_revenant_available(&corr) == false);
    assert(corruption_wraith_available(&corr) == false);
    assert(corruption_archon_available(&corr) == false);
    /* Only Lich and Reaper paths remain at >= 70% */
    printf("PASS\n");

    /* Test 12: Corruption add/reduce still work */
    printf("Test 12: Add/reduce functions... ");
    corruption_init(&corr);
    corruption_add(&corr, 25, "Test event", 1);
    assert(corr.corruption == 25);

    corruption_add(&corr, 50, "Test event 2", 2);
    assert(corr.corruption == 75);  /* 25 + 50 */

    corruption_reduce(&corr, 10, "Redemption", 3);
    assert(corr.corruption == 65);
    printf("PASS\n");

    printf("\nAll Corruption Tiers tests passed!\n");
    return 0;
}

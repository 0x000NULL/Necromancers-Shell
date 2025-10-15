#include "game/resources/consciousness.h"
#include <stdio.h>
#include <assert.h>
#include <math.h>

#define EPSILON 0.01f

int main(void) {
    printf("Running Consciousness tests...\n");

    ConsciousnessState cons;

    /* Test 1: Initial state */
    printf("Test 1: Initial state... ");
    consciousness_init(&cons);
    assert(fabsf(cons.stability - 100.0f) < EPSILON);
    assert(fabsf(cons.decay_rate - (-0.1f)) < EPSILON);
    assert(cons.fragmentation_level == 0.0f);
    assert(cons.approaching_wraith == false);
    printf("PASS\n");

    /* Test 2: Apply monthly decay */
    printf("Test 2: Apply monthly decay... ");
    consciousness_apply_decay(&cons, 1);
    assert(fabsf(cons.stability - 99.9f) < EPSILON);  /* 100 - 0.1 */
    printf("PASS\n");

    /* Test 3: Prevent double decay in same month */
    printf("Test 3: Prevent double decay in same month... ");
    consciousness_apply_decay(&cons, 1);  /* Same month, should not decay */
    assert(fabsf(cons.stability - 99.9f) < EPSILON);  /* Should remain same */
    printf("PASS\n");

    /* Test 4: Decay for next month */
    printf("Test 4: Decay for next month... ");
    consciousness_apply_decay(&cons, 2);
    assert(fabsf(cons.stability - 99.8f) < EPSILON);  /* 99.9 - 0.1 */
    printf("PASS\n");

    /* Test 5: Restore consciousness */
    printf("Test 5: Restore consciousness... ");
    consciousness_init(&cons);
    cons.stability = 95.0f;
    consciousness_restore(&cons, 10.0f);
    assert(fabsf(cons.stability - 100.0f) < EPSILON);  /* Clamped at 100 */

    consciousness_init(&cons);
    cons.stability = 50.0f;
    consciousness_restore(&cons, 20.0f);
    assert(fabsf(cons.stability - 70.0f) < EPSILON);
    printf("PASS\n");

    /* Test 6: Reduce consciousness */
    printf("Test 6: Reduce consciousness... ");
    consciousness_init(&cons);
    consciousness_reduce(&cons, 20.0f);
    assert(fabsf(cons.stability - 80.0f) < EPSILON);
    printf("PASS\n");

    /* Test 7: Check critical threshold */
    printf("Test 7: Check critical threshold... ");
    consciousness_init(&cons);
    cons.stability = 15.0f;
    assert(consciousness_is_critical(&cons) == false);
    cons.stability = 9.5f;
    assert(consciousness_is_critical(&cons) == true);
    printf("PASS\n");

    /* Test 8: Check stable threshold */
    printf("Test 8: Check stable threshold... ");
    consciousness_init(&cons);
    cons.stability = 80.0f;
    assert(consciousness_is_stable(&cons) == true);
    cons.stability = 79.5f;
    assert(consciousness_is_stable(&cons) == false);
    printf("PASS\n");

    /* Test 9: Months until critical */
    printf("Test 9: Months until critical... ");
    consciousness_init(&cons);
    cons.stability = 100.0f;
    cons.decay_rate = -0.1f;
    uint32_t months = consciousness_months_until_critical(&cons);
    /* (100 - 10) / 0.1 = 900 months */
    assert(months == 900);

    cons.stability = 50.0f;
    months = consciousness_months_until_critical(&cons);
    /* (50 - 10) / 0.1 = 400 months */
    assert(months == 400);
    printf("PASS\n");

    /* Test 10: Fragmentation mechanics */
    printf("Test 10: Fragmentation mechanics... ");
    consciousness_init(&cons);
    consciousness_add_fragmentation(&cons, 30.0f);
    assert(fabsf(cons.fragmentation_level - 30.0f) < EPSILON);
    assert(consciousness_is_fragmenting(&cons) == false);  /* < 50% */

    consciousness_add_fragmentation(&cons, 25.0f);
    assert(fabsf(cons.fragmentation_level - 55.0f) < EPSILON);
    assert(consciousness_is_fragmenting(&cons) == true);  /* >= 50% */
    assert(cons.approaching_wraith == true);
    printf("PASS\n");

    /* Test 11: Set decay rate */
    printf("Test 11: Set decay rate... ");
    consciousness_init(&cons);
    consciousness_set_decay_rate(&cons, -0.5f);
    assert(fabsf(cons.decay_rate - (-0.5f)) < EPSILON);
    printf("PASS\n");

    /* Test 12: Description strings */
    printf("Test 12: Description strings... ");
    consciousness_init(&cons);
    cons.stability = 95.0f;
    const char* desc = consciousness_get_description(&cons);
    assert(desc != NULL);
    assert(desc[0] != '\0');
    printf("PASS\n");

    /* Test 13: Long-term decay simulation (100 months) */
    printf("Test 13: Long-term decay (100 months)... ");
    consciousness_init(&cons);
    for (uint32_t month = 1; month <= 100; month++) {
        consciousness_apply_decay(&cons, month);
    }
    /* 100 months * -0.1% = -10% total */
    assert(fabsf(cons.stability - 90.0f) < EPSILON);
    printf("PASS\n");

    printf("\nAll Consciousness tests passed!\n");
    return 0;
}

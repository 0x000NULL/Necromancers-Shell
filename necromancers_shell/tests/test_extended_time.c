#include "game/resources/resources.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

int main(void) {
    printf("Running Extended Time Tracking tests...\n");

    Resources res;
    resources_init(&res);

    /* Test 1: Initial state */
    printf("Test 1: Initial state... ");
    assert(res.day_count == 0);
    assert(res.time_hours == 0);
    assert(res.day_of_month == 1);
    assert(res.month == 0);
    assert(res.year == 0);
    printf("PASS\n");

    /* Test 2: Advance one day */
    printf("Test 2: Advance one day... ");
    resources_advance_time(&res, 24);
    assert(res.day_count == 1);
    assert(res.time_hours == 0);
    assert(res.day_of_month == 2);
    assert(res.month == 0);
    assert(res.year == 0);
    printf("PASS\n");

    /* Test 3: Advance to end of first month (30 days) */
    printf("Test 3: Advance to end of first month... ");
    resources_advance_time(&res, 24 * 28);  /* 28 more days = 30 total */
    assert(res.day_count == 29);
    assert(res.day_of_month == 30);
    assert(res.month == 0);
    assert(res.year == 0);
    printf("PASS\n");

    /* Test 4: Cross month boundary */
    printf("Test 4: Cross month boundary... ");
    resources_advance_time(&res, 24);  /* Day 30 */
    assert(res.day_count == 30);
    assert(res.day_of_month == 1);
    assert(res.month == 1);  /* Second month */
    assert(res.year == 0);
    printf("PASS\n");

    /* Test 5: Advance 11 more months (to month 12) */
    printf("Test 5: Advance to end of first year... ");
    resources_advance_time(&res, 24 * 30 * 10);  /* 10 months */
    assert(res.month == 11);
    assert(res.year == 0);
    resources_advance_time(&res, 24 * 30);  /* One more month = 12 months total */
    assert(res.month == 0);  /* Wraps to month 0 */
    assert(res.year == 1);  /* Year 1 */
    printf("PASS\n");

    /* Test 6: Test get_months_elapsed */
    printf("Test 6: get_months_elapsed... ");
    resources_init(&res);
    resources_advance_time(&res, 24 * 30 * 5);  /* 5 months */
    assert(resources_get_months_elapsed(&res) == 5);
    resources_advance_time(&res, 24 * 30 * 7);  /* 7 more = 12 total */
    assert(resources_get_months_elapsed(&res) == 12);
    assert(resources_get_years_elapsed(&res) == 1);
    printf("PASS\n");

    /* Test 7: Test format_extended_time */
    printf("Test 7: format_extended_time... ");
    resources_init(&res);
    res.year = 2;
    res.month = 5;
    res.day_of_month = 15;
    res.time_hours = 14;
    char buf[128];
    resources_format_extended_time(&res, buf, sizeof(buf));
    /* Format: "Year X, Month Y, Day Z, HH:00" */
    /* Month is displayed as 1-indexed (month+1) */
    assert(strstr(buf, "Year 2") != NULL);
    assert(strstr(buf, "Month 6") != NULL);
    assert(strstr(buf, "Day 15") != NULL);
    assert(strstr(buf, "14:00") != NULL);
    printf("PASS\n");

    /* Test 8: Long-term advance (3 years) */
    printf("Test 8: Long-term advance (3 years)... ");
    resources_init(&res);
    resources_advance_time(&res, 24 * 30 * 12 * 3);  /* 3 years */
    assert(resources_get_years_elapsed(&res) == 3);
    assert(res.year == 3);
    assert(res.month == 0);
    assert(res.day_of_month == 1);
    printf("PASS\n");

    printf("\nAll Extended Time Tracking tests passed!\n");
    return 0;
}

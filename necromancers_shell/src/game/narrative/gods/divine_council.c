#include "divine_council.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Minimum day to summon council */
#define MIN_SUMMON_DAY 162

/* Verdict names */
static const char* VERDICT_NAMES[] = {
    "None",
    "Full Amnesty",
    "Conditional Amnesty",
    "Purge Sentence",
    "Immediate Execution"
};

DivineCouncil* divine_council_create(void) {
    DivineCouncil* council = malloc(sizeof(DivineCouncil));
    if (!council) {
        return NULL;
    }

    /* Initialize gods array */
    for (size_t i = 0; i < MAX_COUNCIL_GODS; i++) {
        council->gods[i] = NULL;
    }
    council->god_count = 0;

    /* Initialize state */
    council->council_summoned = false;
    council->summon_day = 0;
    council->judgment_complete = false;

    /* Initialize verdict */
    council->verdict = VERDICT_NONE;
    council->verdict_text[0] = '\0';
    council->restriction_count = 0;

    /* Initialize votes */
    council->votes_amnesty = 0;
    council->votes_conditional = 0;
    council->votes_purge = 0;
    council->votes_death = 0;

    /* Initialize statistics */
    council->average_favor = 0;
    council->total_interactions = 0;

    return council;
}

void divine_council_destroy(DivineCouncil* council) {
    if (!council) {
        return;
    }

    /* Destroy all gods */
    for (size_t i = 0; i < council->god_count; i++) {
        god_destroy(council->gods[i]);
    }

    free(council);
}

bool divine_council_add_god(DivineCouncil* council, God* god) {
    if (!council || !god) {
        return false;
    }

    if (council->god_count >= MAX_COUNCIL_GODS) {
        return false;
    }

    council->gods[council->god_count] = god;
    council->god_count++;

    return true;
}

God* divine_council_find_god(const DivineCouncil* council, const char* god_id) {
    if (!council || !god_id) {
        return NULL;
    }

    for (size_t i = 0; i < council->god_count; i++) {
        if (strcmp(council->gods[i]->id, god_id) == 0) {
            return council->gods[i];
        }
    }

    return NULL;
}

God* divine_council_get_god(const DivineCouncil* council, size_t index) {
    if (!council || index >= council->god_count) {
        return NULL;
    }

    return council->gods[index];
}

bool divine_council_summon(DivineCouncil* council, uint32_t day) {
    if (!council) {
        return false;
    }

    if (council->council_summoned) {
        return false; /* Already summoned */
    }

    council->council_summoned = true;
    council->summon_day = day;

    /* Mark all gods as summoned */
    for (size_t i = 0; i < council->god_count; i++) {
        god_mark_summoned(council->gods[i]);
    }

    return true;
}

bool divine_council_pass_judgment(DivineCouncil* council, uint8_t player_corruption,
                                  float player_consciousness) {
    if (!council) {
        return false;
    }

    if (council->judgment_complete) {
        return false; /* Already judged */
    }

    /* Calculate average favor */
    council->average_favor = divine_council_calculate_average_favor(council);

    /* Each god votes based on favor and player state */
    for (size_t i = 0; i < council->god_count; i++) {
        God* god = council->gods[i];
        int16_t favor = god->favor;

        /* Voting logic based on favor */
        if (favor >= 40) {
            /* Strong favor - vote amnesty */
            council->votes_amnesty++;
        } else if (favor >= 0) {
            /* Weak favor - vote conditional */
            council->votes_conditional++;
        } else if (favor >= -40) {
            /* Weak disfavor - vote conditional (with restrictions) */
            council->votes_conditional++;
        } else if (favor >= -70) {
            /* Strong disfavor - vote purge */
            council->votes_purge++;
        } else {
            /* Extreme disfavor - vote death */
            council->votes_death++;
        }

        /* Mark judgment as given */
        god_mark_judgment_given(god);
    }

    /* Determine verdict by majority vote */
    if (council->votes_death >= 4) {
        /* Majority vote death (4+ of 7) */
        council->verdict = VERDICT_IMMEDIATE_DEATH;
        snprintf(council->verdict_text, sizeof(council->verdict_text), "%s", "The Council has spoken. Your crimes against the natural order warrant "
                "immediate dissolution. Your consciousness will be fragmented beyond recovery.");
    } else if (council->votes_purge >= 4) {
        /* Majority vote purge */
        council->verdict = VERDICT_PURGE;
        snprintf(council->verdict_text, sizeof(council->verdict_text), "%s", "The Council finds you guilty of necromantic excess. You are condemned to "
                "face the Fourth Purge. Your fate rests with the enforcers.");
    } else if (council->votes_amnesty >= 4) {
        /* Majority vote amnesty */
        council->verdict = VERDICT_AMNESTY;
        snprintf(council->verdict_text, sizeof(council->verdict_text), "%s", "The Council acknowledges your restraint and purpose. Full amnesty is granted. "
                "You may continue your work without divine interference.");

        /* Grant amnesty from all gods */
        for (size_t i = 0; i < council->god_count; i++) {
            god_grant_amnesty(council->gods[i]);
        }
    } else {
        /* Default to conditional */
        council->verdict = VERDICT_CONDITIONAL;
        snprintf(council->verdict_text, sizeof(council->verdict_text),
                "The Council grants conditional amnesty. Corruption: %d%%, Consciousness: %.1f%%. "
                "You must adhere to the imposed restrictions or face immediate Purge.",
                player_corruption, player_consciousness);
    }

    council->judgment_complete = true;
    return true;
}

bool divine_council_add_restriction(DivineCouncil* council, const char* restriction) {
    if (!council || !restriction) {
        return false;
    }

    if (council->restriction_count >= MAX_COUNCIL_RESTRICTIONS) {
        return false;
    }

    strncpy(council->restrictions[council->restriction_count], restriction,
            sizeof(council->restrictions[0]) - 1);
    council->restrictions[council->restriction_count][sizeof(council->restrictions[0]) - 1] = '\0';
    council->restriction_count++;

    return true;
}

int16_t divine_council_calculate_average_favor(const DivineCouncil* council) {
    if (!council || council->god_count == 0) {
        return 0;
    }

    int32_t total_favor = 0;
    for (size_t i = 0; i < council->god_count; i++) {
        total_favor += council->gods[i]->favor;
    }

    return (int16_t)(total_favor / (int32_t)council->god_count);
}

uint32_t divine_council_calculate_total_interactions(const DivineCouncil* council) {
    if (!council) {
        return 0;
    }

    uint32_t total = 0;
    for (size_t i = 0; i < council->god_count; i++) {
        total += council->gods[i]->interactions;
    }

    return total;
}

bool divine_council_can_summon(const DivineCouncil* council, uint32_t current_day) {
    if (!council) {
        return false;
    }

    if (council->council_summoned) {
        return false; /* Already summoned */
    }

    return current_day >= MIN_SUMMON_DAY;
}

bool divine_council_favor_amnesty(const DivineCouncil* council) {
    if (!council) {
        return false;
    }

    int16_t avg = divine_council_calculate_average_favor(council);
    return avg >= 20;
}

bool divine_council_favor_purge(const DivineCouncil* council) {
    if (!council) {
        return false;
    }

    int16_t avg = divine_council_calculate_average_favor(council);
    return avg <= -40;
}

const char* divine_council_verdict_name(DivineVerdict verdict) {
    if (verdict < 0 || verdict >= 5) {
        return "Unknown";
    }
    return VERDICT_NAMES[verdict];
}

const char* divine_council_get_verdict_description(const DivineCouncil* council) {
    if (!council) {
        return "No council";
    }

    if (!council->judgment_complete) {
        return "Judgment not yet passed";
    }

    return council->verdict_text;
}

size_t divine_council_count_positive_favor(const DivineCouncil* council) {
    if (!council) {
        return 0;
    }

    size_t count = 0;
    for (size_t i = 0; i < council->god_count; i++) {
        if (god_has_positive_favor(council->gods[i])) {
            count++;
        }
    }

    return count;
}

size_t divine_council_count_negative_favor(const DivineCouncil* council) {
    if (!council) {
        return 0;
    }

    size_t count = 0;
    for (size_t i = 0; i < council->god_count; i++) {
        if (god_has_negative_favor(council->gods[i])) {
            count++;
        }
    }

    return count;
}

bool divine_council_all_voted(const DivineCouncil* council) {
    if (!council) {
        return false;
    }

    uint32_t total_votes = council->votes_amnesty + council->votes_conditional +
                          council->votes_purge + council->votes_death;

    return total_votes == council->god_count;
}

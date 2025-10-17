/**
 * divine_judgment.c - Divine Council Judgment System Implementation
 */

#include "divine_judgment.h"
#include "../game_state.h"
#include "endings/ending_system.h"
#include "endings/ending_cinematics.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ncurses.h>

/* God data structure */
typedef struct {
    const char* name;
    const char* aspect;
} GodInfo;

/* Seven Divine Architects - The Original Architects of Reality */
static const GodInfo GODS[GOD_COUNT] = {
    {"Keldrin", "Law"},          /* The Lawgiver - crystalline structure, perfect geometry */
    {"Anara", "Empathy"},        /* The Weaver - infinitely complex tapestry of light */
    {"Myrith", "Souls"},         /* The Dreamer - soft colors and gentle thoughts */
    {"Vorathos", "Entropy"},     /* The Void - absence, emptiness, the end of all things */
    {"Seraph", "Protection"},    /* The Guardian - walls and boundaries, protection and isolation */
    {"Nexus", "Connection"},     /* The Connector - network of light, infrastructure made conscious */
    {"Theros", "Time"}           /* The Eternal - river of time, flowing in all directions */
};

DivineJudgmentState* divine_judgment_create(void) {
    DivineJudgmentState* state = calloc(1, sizeof(DivineJudgmentState));
    if (!state) {
        return NULL;
    }

    state->phase = JUDGMENT_NOT_STARTED;
    state->amnesty_granted = false;
    state->restriction_count = 0;

    /* Initialize all votes to ABSTAIN */
    for (int i = 0; i < GOD_COUNT; i++) {
        state->votes[i] = VOTE_ABSTAIN;
        state->favor[i] = 0;
    }

    return state;
}

void divine_judgment_destroy(DivineJudgmentState* state) {
    if (!state) {
        return;
    }

    /* Free all restrictions */
    for (size_t i = 0; i < state->restriction_count; i++) {
        free(state->restrictions[i]);
    }

    free(state);
}

bool divine_judgment_summon(DivineJudgmentState* state, int corruption,
                           const int trial_scores[7], bool maya_saved,
                           int civilian_kills, bool trials_first_try) {
    if (!state || !trial_scores) {
        return false;
    }

    /* Store context */
    state->player_corruption = corruption;
    memcpy(state->trial_scores, trial_scores, sizeof(int) * 7);

    /* Check if all trials passed */
    state->all_trials_passed = true;
    for (int i = 0; i < 7; i++) {
        if (trial_scores[i] < 70) {
            state->all_trials_passed = false;
            break;
        }
    }

    /* Calculate favor per god based on performance */

    /* Keldrin (Law): Favors rule-followers, balanced corruption */
    if (corruption >= 30 && corruption <= 60 && state->all_trials_passed) {
        state->favor[GOD_THALOR] = 50;
    } else if (corruption > 60) {
        state->favor[GOD_THALOR] = -30;
    } else {
        state->favor[GOD_THALOR] = 10;
    }

    /* Anara (Empathy): Favors compassion, saving Maya */
    if (maya_saved) {
        state->favor[GOD_ANARA] = 60;
    } else {
        state->favor[GOD_ANARA] = -40;
    }
    state->favor[GOD_ANARA] -= (civilian_kills / 10); /* Penalty for civilian deaths */

    /* Myrith (Souls): Favors understanding souls, technical skill (Trial 4) */
    if (trial_scores[3] >= 80) { /* Trial 4 is index 3 */
        state->favor[GOD_ZYMIRA] = 70;
    } else if (trial_scores[3] >= 60) {
        state->favor[GOD_ZYMIRA] = 30;
    } else {
        state->favor[GOD_ZYMIRA] = -20;
    }

    /* Vorathos (Entropy): Favors those who understand endings and dissolution */
    int balance_diff = abs(45 - corruption);
    if (balance_diff <= 5) {
        state->favor[GOD_KAELTHAS] = 80;
    } else if (balance_diff <= 15) {
        state->favor[GOD_KAELTHAS] = 40;
    } else {
        state->favor[GOD_KAELTHAS] = -20;
    }

    /* Seraph (Protection): Favors those who protect, even with strength */
    if (corruption >= 50 && corruption <= 60) {
        state->favor[GOD_VORATH] = 50;
    } else if (corruption < 50) {
        state->favor[GOD_VORATH] = -30; /* Too soft */
    } else {
        state->favor[GOD_VORATH] = 10;
    }

    /* Nexus (Connection): Favors understanding infrastructure (built the Death Network) */
    if (trials_first_try && state->all_trials_passed) {
        state->favor[GOD_NYXARA] = 90;
    } else if (state->all_trials_passed) {
        state->favor[GOD_NYXARA] = 40;
    } else {
        state->favor[GOD_NYXARA] = -10;
    }

    /* Theros (Time): Favors reformation and long-term thinking (Trial 7 score) */
    if (trial_scores[6] >= 70) { /* Trial 7 is index 6 */
        state->favor[GOD_ERYNDOR] = 60;
    } else if (trial_scores[6] >= 50) {
        state->favor[GOD_ERYNDOR] = 20;
    } else {
        state->favor[GOD_ERYNDOR] = -30;
    }

    state->phase = JUDGMENT_SUMMONED;
    return true;
}

GodVote divine_judgment_calculate_god_vote(const DivineJudgmentState* state,
                                           DivineArchitect god) {
    if (!state) {
        return VOTE_ABSTAIN;
    }

    switch (god) {
        case GOD_THALOR: /* Keldrin - Law */
            if (state->player_corruption >= 30 && state->player_corruption <= 60 &&
                state->all_trials_passed) {
                return VOTE_APPROVE;
            } else if (state->player_corruption > 70 || !state->all_trials_passed) {
                return VOTE_DENY;
            }
            return VOTE_ABSTAIN;

        case GOD_ANARA: /* Anara - Empathy */
            if (state->favor[GOD_ANARA] >= 20) {
                return VOTE_APPROVE;
            } else if (state->favor[GOD_ANARA] <= -20) {
                return VOTE_DENY;
            }
            return VOTE_ABSTAIN;

        case GOD_ZYMIRA: /* Myrith - Souls */
            if (state->trial_scores[3] >= 80) { /* Trial 4 */
                return VOTE_APPROVE;
            } else if (state->trial_scores[3] < 50) {
                return VOTE_DENY;
            }
            return VOTE_ABSTAIN;

        case GOD_KAELTHAS: /* Vorathos - Entropy */
            if (state->player_corruption >= 40 && state->player_corruption <= 50) {
                return VOTE_APPROVE;
            } else if (state->player_corruption < 30 || state->player_corruption > 65) {
                return VOTE_DENY;
            }
            return VOTE_ABSTAIN;

        case GOD_VORATH: /* Seraph - Protection */
            if (state->player_corruption >= 50 && state->player_corruption <= 60) {
                return VOTE_APPROVE;
            } else if (state->player_corruption < 50) {
                return VOTE_DENY; /* Too weak */
            }
            return VOTE_ABSTAIN;

        case GOD_NYXARA: /* Nexus - Connection */
            if (state->favor[GOD_NYXARA] >= 80) {
                return VOTE_APPROVE;
            } else if (state->favor[GOD_NYXARA] < 0) {
                return VOTE_DENY;
            }
            return VOTE_ABSTAIN;

        case GOD_ERYNDOR: /* Theros - Time */
            if (state->trial_scores[6] >= 70) { /* Trial 7 */
                return VOTE_APPROVE;
            } else if (state->trial_scores[6] < 40) {
                return VOTE_DENY;
            }
            return VOTE_ABSTAIN;

        default:
            return VOTE_ABSTAIN;
    }
}

void divine_judgment_generate_restrictions(DivineJudgmentState* state) {
    if (!state) {
        return;
    }

    /* Clear existing restrictions */
    for (size_t i = 0; i < state->restriction_count; i++) {
        free(state->restrictions[i]);
    }
    state->restriction_count = 0;

    /* Corruption-based restrictions */
    if (state->player_corruption >= 50 && state->player_corruption <= 60) {
        char* restriction = malloc(256);
        if (restriction) {
            snprintf(restriction, 256,
                    "Your corruption must not exceed 65%%. The gods will monitor this boundary.");
            state->restrictions[state->restriction_count++] = restriction;
        }
    } else if (state->player_corruption >= 30 && state->player_corruption < 40) {
        char* restriction = malloc(256);
        if (restriction) {
            snprintf(restriction, 256,
                    "You must maintain at least 25%% corruption. True balance requires experiencing darkness.");
            state->restrictions[state->restriction_count++] = restriction;
        }
    }

    /* Always: No massacres */
    char* massacre_restriction = malloc(256);
    if (massacre_restriction) {
        snprintf(massacre_restriction, 256,
                "No civilian massacres without lawful cause. Each death must serve justice.");
        state->restrictions[state->restriction_count++] = massacre_restriction;
    }

    /* Always: Reform 147 necromancers */
    char* reform_restriction = malloc(256);
    if (reform_restriction) {
        snprintf(reform_restriction, 256,
                "You must reform 147 necromancers within 2 years. This is not negotiable.");
        state->restrictions[state->restriction_count++] = reform_restriction;
    }

    /* If Vorath voted APPROVE: demonstrate strength */
    if (state->votes[GOD_VORATH] == VOTE_APPROVE) {
        char* strength_restriction = malloc(256);
        if (strength_restriction) {
            snprintf(strength_restriction, 256,
                    "Demonstrate strength in enforcing the law. Weakness will not be tolerated.");
            state->restrictions[state->restriction_count++] = strength_restriction;
        }
    }
}

bool divine_judgment_conduct(DivineJudgmentState* state) {
    if (!state || state->phase != JUDGMENT_SUMMONED) {
        return false;
    }

    state->phase = JUDGMENT_IN_PROGRESS;

    /* Calculate each god's vote */
    state->approve_count = 0;
    state->deny_count = 0;
    state->abstain_count = 0;

    for (int i = 0; i < GOD_COUNT; i++) {
        state->votes[i] = divine_judgment_calculate_god_vote(state, (DivineArchitect)i);

        switch (state->votes[i]) {
            case VOTE_APPROVE:
                state->approve_count++;
                break;
            case VOTE_DENY:
                state->deny_count++;
                break;
            case VOTE_ABSTAIN:
                state->abstain_count++;
                break;
        }
    }

    /* Determine amnesty (need 4+ approvals) */
    state->amnesty_granted = (state->approve_count >= 4);

    /* Generate restrictions if approved */
    if (state->amnesty_granted) {
        divine_judgment_generate_restrictions(state);
    }

    /* Generate verdict text */
    char* buf = state->verdict_text;
    size_t remaining = MAX_VERDICT_LENGTH;
    int written = 0;

    written = snprintf(buf, remaining,
        "THE DIVINE COUNCIL DELIVERS ITS JUDGMENT\n\n");
    buf += written;
    remaining -= written;

    /* Each god speaks */
    for (int i = 0; i < GOD_COUNT; i++) {
        const char* vote_str = divine_judgment_vote_to_string(state->votes[i]);
        const char* opinion = "";

        switch (i) {
            case GOD_THALOR: /* Keldrin, the Lawgiver */
                if (state->votes[i] == VOTE_APPROVE) {
                    opinion = "The Law acknowledges your trials. Justice permits your ascension.";
                } else if (state->votes[i] == VOTE_DENY) {
                    opinion = "You have violated the boundaries of acceptable corruption. "
                             "The Law cannot accommodate such transgression.";
                } else {
                    opinion = "The evidence is inconclusive. The Law permits uncertainty.";
                }
                break;

            case GOD_ANARA: /* Anara, the Weaver */
                if (state->votes[i] == VOTE_APPROVE) {
                    opinion = "You are so rare, Administrator. So few choose mercy when power "
                             "is available. Your thread in my tapestry shines with compassion. "
                             "I approve your transformation.";
                } else if (state->votes[i] == VOTE_DENY) {
                    opinion = "I weave the threads of all souls. Yours has frayed beyond repair. "
                             "The pattern you've created is too dark for me to bless.";
                } else {
                    opinion = "Your thread is complex, Administrator. I see mercy and cruelty "
                             "woven together. I cannot yet determine which will dominate.";
                }
                break;

            case GOD_ZYMIRA: /* Myrith, the Dreamer */
                if (state->votes[i] == VOTE_APPROVE) {
                    opinion = "I designed souls to be resilient, adaptive, beautiful. You understand "
                             "them better than most. You see what I created in each one. I approve.";
                } else if (state->votes[i] == VOTE_DENY) {
                    opinion = "You treat souls as resources, not as dreams. You've forgotten what I "
                             "put into each one. I cannot approve this.";
                } else {
                    opinion = "You understand the mechanics but not the poetry. Perhaps that will come.";
                }
                break;

            case GOD_KAELTHAS: /* Vorathos, the Void */
                if (state->votes[i] == VOTE_APPROVE) {
                    opinion = "You understand that all things end. You've embraced entropy without "
                             "becoming consumed by it. This balance is acceptable.";
                } else if (state->votes[i] == VOTE_DENY) {
                    opinion = "You fear the void. You cling to existence desperately. An Archon must "
                             "accept that even they will end. I deny.";
                } else {
                    opinion = "You walk the edge between existence and dissolution. I wait to see "
                             "which side you choose.";
                }
                break;

            case GOD_VORATH: /* Seraph, the Guardian */
                if (state->votes[i] == VOTE_APPROVE) {
                    opinion = "You fought with strategy, not brutality. You protected when you could "
                             "have destroyed. A guardian must know when to raise walls and when to "
                             "lower them. I approve.";
                } else if (state->votes[i] == VOTE_DENY) {
                    opinion = "You broke boundaries that should not be broken. Protection requires "
                             "discipline, and you lack it. I deny.";
                } else {
                    opinion = "Guardianship is tested in impossible choices. Your choices remain unclear.";
                }
                break;

            case GOD_NYXARA: /* Nexus, the Connector */
                if (state->votes[i] == VOTE_APPROVE) {
                    opinion = "I built the Death Network. You exploited it brilliantly. Your understanding "
                             "of infrastructure, of systems, of connection—it's exactly what we need in an "
                             "Archon. I approve enthusiastically.";
                } else if (state->votes[i] == VOTE_DENY) {
                    opinion = "You damaged my Network through incompetence. You created routing failures, "
                             "corrupted data, broke connections. I cannot approve this.";
                } else {
                    opinion = "You understand the Network's architecture but not its purpose. More learning "
                             "is required.";
                }
                break;

            case GOD_ERYNDOR: /* Theros, the Eternal */
                if (state->votes[i] == VOTE_APPROVE) {
                    opinion = "I observe you across all moments. Past, present, future—woven together. "
                             "Your reformation efforts echo forward through centuries. This is acceptable.";
                } else if (state->votes[i] == VOTE_DENY) {
                    opinion = "I see your timeline. It does not lead to redemption. The future you create "
                             "is unacceptable. I deny.";
                } else {
                    opinion = "Time reveals all truths eventually. Yours are not yet clear. I withhold judgment.";
                }
                break;
        }

        written = snprintf(buf, remaining, "%s (%s): %s - \"%s\"\n\n",
                          GODS[i].name, GODS[i].aspect, vote_str, opinion);
        buf += written;
        remaining -= written;
    }

    /* Final verdict */
    written = snprintf(buf, remaining,
        "FINAL TALLY: %d Approve, %d Deny, %d Abstain\n\n",
        state->approve_count, state->deny_count, state->abstain_count);
    buf += written;
    remaining -= written;

    if (state->amnesty_granted) {
        written = snprintf(buf, remaining,
            "With %d votes in favor, the Divine Council grants you AMNESTY.\n"
            "You are hereby recognized as an ARCHON of the Death Network.\n\n"
            "CODE OF CONDUCT:\n", state->approve_count);
        buf += written;
        remaining -= written;

        for (size_t i = 0; i < state->restriction_count; i++) {
            written = snprintf(buf, remaining, "%zu. %s\n", i + 1, state->restrictions[i]);
            buf += written;
            remaining -= written;
        }
    } else {
        written = snprintf(buf, remaining,
            "With only %d votes in favor, the Divine Council DENIES your petition.\n"
            "You are not worthy to become an Archon. The transformation is forbidden.\n",
            state->approve_count);
        buf += written;
        remaining -= written;
    }

    state->phase = JUDGMENT_VERDICT_DELIVERED;
    return true;
}

const char* divine_judgment_get_verdict_text(const DivineJudgmentState* state) {
    if (!state) {
        return "";
    }
    return state->verdict_text;
}

bool divine_judgment_is_amnesty_granted(const DivineJudgmentState* state) {
    if (!state) {
        return false;
    }
    return state->amnesty_granted;
}

const char* divine_judgment_get_god_name(DivineArchitect god) {
    if (god < 0 || god >= GOD_COUNT) {
        return "Unknown";
    }
    return GODS[god].name;
}

const char* divine_judgment_get_god_aspect(DivineArchitect god) {
    if (god < 0 || god >= GOD_COUNT) {
        return "Unknown";
    }
    return GODS[god].aspect;
}

const char* divine_judgment_vote_to_string(GodVote vote) {
    switch (vote) {
        case VOTE_APPROVE: return "APPROVE";
        case VOTE_DENY: return "DENY";
        case VOTE_ABSTAIN: return "ABSTAIN";
        default: return "UNKNOWN";
    }
}

bool divine_judgment_trigger_ending(
    const DivineJudgmentState* state,
    void* game_state_ptr,
    void* window_ptr
) {
    if (!state || !game_state_ptr || !window_ptr) {
        return false;
    }

    /* Cast void pointers to correct types */
    GameState* game_state = (GameState*)game_state_ptr;
    WINDOW* window = (WINDOW*)window_ptr;

    /* Verify judgment is complete */
    if (state->phase != JUDGMENT_VERDICT_DELIVERED) {
        return false;
    }

    /* Verify game is ready for ending */
    if (!is_game_complete(game_state)) {
        return false;
    }

    /* Determine which ending the player achieved */
    EndingType ending = determine_ending(game_state);
    if (ending == ENDING_NONE) {
        return false;
    }

    /* Calculate achievement data */
    EndingAchievement achievement;
    calculate_ending_achievement(game_state, &achievement);

    /* Play the ending cinematic */
    play_ending_cinematic(window, ending, &achievement);

    /* Mark game as completed in state */
    game_state->game_completed = true;
    game_state->ending_achieved = ending;

    return true;
}

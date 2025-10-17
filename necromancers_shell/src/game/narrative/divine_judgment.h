/**
 * divine_judgment.h - Divine Council Judgment System
 *
 * After completing all 7 trials, the Seven Divine Architects judge the player's
 * worthiness to become an Archon. Each god votes based on trial performance and
 * the player's moral choices. Requires 4+ approvals for amnesty.
 */

#ifndef DIVINE_JUDGMENT_H
#define DIVINE_JUDGMENT_H

#include <stdbool.h>
#include <stddef.h>

/* Maximum restrictions imposed by the Divine Council */
#define MAX_RESTRICTIONS 5

/* Maximum length of verdict text */
#define MAX_VERDICT_LENGTH 4096

/**
 * Judgment phase tracking
 */
typedef enum {
    JUDGMENT_NOT_STARTED,    /* Not yet summoned */
    JUDGMENT_SUMMONED,       /* Called before council */
    JUDGMENT_IN_PROGRESS,    /* Deliberation ongoing */
    JUDGMENT_VERDICT_DELIVERED /* Final decision made */
} JudgmentPhase;

/**
 * Individual god vote
 */
typedef enum {
    VOTE_APPROVE,   /* Supports Archon transformation */
    VOTE_DENY,      /* Rejects transformation */
    VOTE_ABSTAIN    /* No position */
} GodVote;

/**
 * Seven Divine Architects (from DivineCouncil)
 */
typedef enum {
    GOD_THALOR = 0,    /* Justice */
    GOD_ANARA,         /* Empathy */
    GOD_ZYMIRA,        /* Knowledge */
    GOD_KAELTHAS,      /* Balance */
    GOD_VORATH,        /* Destruction */
    GOD_NYXARA,        /* Fate */
    GOD_ERYNDOR,       /* Creation */
    GOD_COUNT
} DivineArchitect;

/**
 * Divine Council judgment state
 */
typedef struct DivineJudgmentState {
    JudgmentPhase phase;

    /* Individual god votes */
    GodVote votes[GOD_COUNT];

    /* Vote tallies */
    int approve_count;
    int deny_count;
    int abstain_count;

    /* Final verdict */
    bool amnesty_granted;      /* True if 4+ approvals */
    char* restrictions[MAX_RESTRICTIONS]; /* Code of Conduct */
    size_t restriction_count;

    /* Favor levels per god (-100 to +100) */
    int favor[GOD_COUNT];

    /* Verdict text (multi-god dialogue) */
    char verdict_text[MAX_VERDICT_LENGTH];

    /* Context at judgment time */
    int player_corruption;     /* Corruption % at judgment */
    int trial_scores[7];       /* Performance in all 7 trials */
    bool all_trials_passed;    /* True if completed all trials */
} DivineJudgmentState;

/**
 * Create divine judgment state
 *
 * Returns: Newly allocated judgment state, or NULL on failure
 */
DivineJudgmentState* divine_judgment_create(void);

/**
 * Destroy divine judgment state
 *
 * Params:
 *   state - Judgment state to destroy
 */
void divine_judgment_destroy(DivineJudgmentState* state);

/**
 * Summon player before the Divine Council
 *
 * Triggers after completing Trial 7. Loads god personalities and
 * calculates initial favor based on trial performance.
 *
 * Params:
 *   state - Judgment state
 *   corruption - Current player corruption %
 *   trial_scores - Array of 7 trial scores (0-100)
 *   maya_saved - True if Maya was saved in Trial 6
 *   civilian_kills - Number of civilians killed
 *   trials_first_try - True if all trials passed on first attempt
 *
 * Returns: True on success, false on failure
 */
bool divine_judgment_summon(DivineJudgmentState* state, int corruption,
                           const int trial_scores[7], bool maya_saved,
                           int civilian_kills, bool trials_first_try);

/**
 * Conduct the divine judgment
 *
 * Each god reviews the player's actions and casts a vote.
 * Final verdict requires 4+ approvals for amnesty.
 *
 * Params:
 *   state - Judgment state
 *
 * Returns: True if judgment completed successfully
 */
bool divine_judgment_conduct(DivineJudgmentState* state);

/**
 * Calculate individual god's vote
 *
 * Each god has unique criteria based on their aspect.
 *
 * Params:
 *   state - Judgment state
 *   god - Which god is voting
 *
 * Returns: The god's vote (APPROVE/DENY/ABSTAIN)
 */
GodVote divine_judgment_calculate_god_vote(const DivineJudgmentState* state,
                                           DivineArchitect god);

/**
 * Generate Code of Conduct restrictions
 *
 * Based on corruption level and god votes, impose rules the player
 * must follow as Archon.
 *
 * Params:
 *   state - Judgment state
 */
void divine_judgment_generate_restrictions(DivineJudgmentState* state);

/**
 * Get formatted verdict text
 *
 * Multi-god dialogue showing each god's opinion and the final verdict.
 *
 * Params:
 *   state - Judgment state
 *
 * Returns: Pointer to verdict text buffer (do not free)
 */
const char* divine_judgment_get_verdict_text(const DivineJudgmentState* state);

/**
 * Check if amnesty was granted
 *
 * Params:
 *   state - Judgment state
 *
 * Returns: True if 4+ gods approved
 */
bool divine_judgment_is_amnesty_granted(const DivineJudgmentState* state);

/**
 * Get god's name
 *
 * Params:
 *   god - Divine architect
 *
 * Returns: God's name string
 */
const char* divine_judgment_get_god_name(DivineArchitect god);

/**
 * Get god's aspect
 *
 * Params:
 *   god - Divine architect
 *
 * Returns: God's aspect (Justice, Empathy, etc.)
 */
const char* divine_judgment_get_god_aspect(DivineArchitect god);

/**
 * Get vote as string
 *
 * Params:
 *   vote - God vote
 *
 * Returns: "APPROVE", "DENY", or "ABSTAIN"
 */
const char* divine_judgment_vote_to_string(GodVote vote);

/**
 * Trigger ending sequence after judgment
 *
 * After the Divine Council delivers its verdict, this function determines
 * which ending the player qualifies for and plays the appropriate cinematic.
 *
 * Should be called after divine_judgment_conduct() completes.
 *
 * Params:
 *   state - Judgment state (must be JUDGMENT_VERDICT_DELIVERED)
 *   game_state - Full game state for ending determination
 *   window - ncurses window for cinematic display
 *
 * Returns: True if ending was triggered successfully
 */
bool divine_judgment_trigger_ending(
    const DivineJudgmentState* state,
    void* game_state,  /* GameState* - void to avoid circular dependency */
    void* window       /* WINDOW* - void to avoid ncurses dependency */
);

#endif /* DIVINE_JUDGMENT_H */

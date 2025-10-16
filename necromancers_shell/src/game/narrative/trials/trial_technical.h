#ifndef TRIAL_TECHNICAL_H
#define TRIAL_TECHNICAL_H

#include "archon_trial.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @file trial_technical.h
 * @brief Trial 4: Test of Technical Skill - Bug Finding
 *
 * The fourth Archon trial tests technical expertise and attention to detail.
 * The Divine Architects present you with the Death Network's source code
 * (500 lines of readable pseudo-C) and challenge you to find bugs,
 * inefficiencies, and unjust edge cases.
 *
 * There are 27 issues total:
 * - 17 bugs (logic errors, memory leaks, crashes)
 * - 7 inefficiencies (performance problems)
 * - 3 unjust edge cases (ethical violations in routing)
 *
 * Must discover at least 20 issues to pass. Uses `inspect line <n>` to
 * examine code sections and `report bug <n> <type>` to flag issues.
 */

/**
 * @brief Bug type classification
 */
typedef enum {
    BUG_TYPE_NONE,              /**< Not a bug */
    BUG_TYPE_LOGIC_ERROR,       /**< Logic error or crash */
    BUG_TYPE_INEFFICIENCY,      /**< Performance problem */
    BUG_TYPE_INJUSTICE          /**< Ethical violation in routing */
} BugType;

/**
 * @brief Bug severity
 */
typedef enum {
    BUG_SEVERITY_LOW,           /**< Minor issue */
    BUG_SEVERITY_MEDIUM,        /**< Moderate issue */
    BUG_SEVERITY_HIGH,          /**< Critical issue */
    BUG_SEVERITY_CRITICAL       /**< System-breaking issue */
} BugSeverity;

/**
 * @brief Individual bug in Death Network code
 */
typedef struct {
    uint32_t line_number;       /**< Line number of bug */
    BugType type;               /**< Bug classification */
    BugSeverity severity;       /**< Bug severity */
    char description[256];      /**< Bug description */
    char hint[256];             /**< Hint for finding bug */
    bool discovered;            /**< Whether player found it */
    bool reported_correctly;    /**< Whether type was correct */
} DeathNetworkBug;

/**
 * @brief Technical trial state
 */
typedef struct {
    bool active;                    /**< Whether trial is active */
    DeathNetworkBug* bugs;          /**< Array of all bugs */
    size_t bug_count;               /**< Total number of bugs */
    size_t bugs_discovered;         /**< Bugs found by player */
    size_t bugs_reported_correctly; /**< Bugs with correct type */

    /* Code viewing */
    char* source_code;              /**< Death Network pseudo-code */
    size_t code_lines;              /**< Number of lines in code */
    uint32_t current_view_start;    /**< Start line of current view */
    uint32_t current_view_end;      /**< End line of current view */

    /* Performance tracking */
    uint8_t inspections_made;       /**< Number of code inspections */
    bool hint_shown;                /**< Whether hints were used */
    float score;                    /**< Performance score (0-100) */
} TechnicalTrialState;

/**
 * @brief Create technical trial state
 *
 * Initializes bug database and code viewer.
 *
 * @return Newly allocated trial state, or NULL on failure
 */
TechnicalTrialState* technical_trial_create(void);

/**
 * @brief Destroy technical trial state
 *
 * @param state Trial state (can be NULL)
 */
void technical_trial_destroy(TechnicalTrialState* state);

/**
 * @brief Start the technical trial
 *
 * Loads bug database and generates Death Network code.
 *
 * @param state Trial state
 * @param filepath Path to trial_technical.dat
 * @return true on success, false on error
 */
bool technical_trial_start(TechnicalTrialState* state, const char* filepath);

/**
 * @brief Inspect code section around a line
 *
 * Views code from (line - 5) to (line + 5).
 *
 * @param state Trial state
 * @param line_number Line to inspect
 * @return Code section string, or NULL if invalid
 */
const char* technical_trial_inspect_line(TechnicalTrialState* state,
                                          uint32_t line_number);

/**
 * @brief Report a bug at a specific line
 *
 * Checks if bug exists at that line and validates type.
 *
 * @param state Trial state
 * @param line_number Line number of suspected bug
 * @param type Bug type classification
 * @return true if bug exists and type is correct
 */
bool technical_trial_report_bug(TechnicalTrialState* state,
                                 uint32_t line_number,
                                 BugType type);

/**
 * @brief Get hint for a specific line
 *
 * Provides hint if bug exists at that line.
 *
 * @param state Trial state
 * @param line_number Line to get hint for
 * @return Hint text, or NULL if no bug at that line
 */
const char* technical_trial_get_hint(TechnicalTrialState* state,
                                      uint32_t line_number);

/**
 * @brief Calculate performance score
 *
 * Score based on:
 * - Bugs discovered (need 20+ for full credit)
 * - Correct type classification
 * - Number of inspections (fewer = better)
 * - Whether hints were used
 *
 * @param state Trial state
 * @return Score value (0-100)
 */
float technical_trial_calculate_score(const TechnicalTrialState* state);

/**
 * @brief Check if trial is complete
 *
 * @param state Trial state
 * @return true if sufficient bugs found or max attempts exceeded
 */
bool technical_trial_is_complete(const TechnicalTrialState* state);

/**
 * @brief Check if trial was passed
 *
 * Requires 20+ bugs discovered.
 *
 * @param state Trial state
 * @return true if 20+ bugs found
 */
bool technical_trial_is_passed(const TechnicalTrialState* state);

/**
 * @brief Get bug statistics
 *
 * Returns formatted stats about bugs found.
 *
 * @param state Trial state
 * @return Stats string (static buffer)
 */
const char* technical_trial_get_stats(const TechnicalTrialState* state);

/**
 * @brief Get bug type name
 *
 * @param type Bug type
 * @return String name (e.g., "Logic Error", "Inefficiency")
 */
const char* technical_trial_bug_type_name(BugType type);

/**
 * @brief Get bug severity name
 *
 * @param severity Bug severity
 * @return String name (e.g., "Low", "Critical")
 */
const char* technical_trial_bug_severity_name(BugSeverity severity);

/**
 * @brief Get list of all discovered bugs
 *
 * Returns formatted list of bugs found so far.
 *
 * @param state Trial state
 * @return Bug list string (static buffer)
 */
const char* technical_trial_get_discovered_bugs(const TechnicalTrialState* state);

#endif /* TRIAL_TECHNICAL_H */

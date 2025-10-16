#define _POSIX_C_SOURCE 200809L
#include "trial_technical.h"
#include "../../../data/data_loader.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * @brief Minimum bugs required to pass
 */
#define MIN_BUGS_TO_PASS 20

/**
 * @brief Maximum bugs in database
 */
#define MAX_BUGS 50

/**
 * @brief Code view context lines
 */
#define CODE_CONTEXT_LINES 5

/**
 * @brief Static buffers for text output
 */
static char stats_buffer[512];
static char bugs_buffer[2048];
static char code_view_buffer[4096];

/**
 * @brief Sample Death Network pseudo-code (part 1)
 */
static const char* DEATH_NETWORK_CODE_PART1 =
    "  1  /* Death Network Soul Routing System v3.14 */\n"
    "  2  /* WARNING: Legacy code with known issues */\n"
    "  3  \n"
    "  4  #include <soul.h>\n"
    "  5  #include <routing.h>\n"
    "  6  \n"
    "  7  #define MAX_QUEUE 1000\n"
    "  8  #define REINCARNATION_LIMIT 999\n"
    "  9  \n"
    " 10  typedef struct {\n"
    " 11      soul_t* queue[MAX_QUEUE];\n"
    " 12      int front, rear;\n"
    " 13  } soul_queue_t;\n"
    " 14  \n"
    " 15  /* BUG: No initialization function - queue starts with garbage */\n"
    " 16  soul_queue_t reincarnation_queue;\n"
    " 17  \n"
    " 18  /* Route soul to destination */\n"
    " 19  int route_soul(soul_t* soul) {\n"
    " 20      if (!soul) return -1;\n"
    " 21      \n"
    " 22      /* Calculate karma score */\n"
    " 23      int karma = calculate_karma(soul);\n"
    " 24      \n"
    " 25      /* BUG: Children under 5 not handled - can go to Hell */\n"
    " 26      if (karma > 0) {\n"
    " 27          send_to_heaven(soul);\n"
    " 28      } else if (karma < 0) {\n"
    " 29          send_to_hell(soul);\n"
    " 30      } else {\n"
    " 31          /* Neutral - reincarnate */\n"
    " 32          queue_for_reincarnation(soul);\n"
    " 33      }\n"
    " 34      return 0;\n"
    " 35  }\n"
    " 36  \n"
    " 37  /* Queue soul for reincarnation */\n"
    " 38  void queue_for_reincarnation(soul_t* soul) {\n"
    " 39      /* BUG: No bounds check - buffer overflow possible */\n"
    " 40      reincarnation_queue.queue[reincarnation_queue.rear] = soul;\n"
    " 41      reincarnation_queue.rear++;\n"
    " 42  }\n"
    " 43  \n"
    " 44  /* Process reincarnation queue */\n"
    " 45  void process_reincarnations(void) {\n"
    " 46      while (reincarnation_queue.front < reincarnation_queue.rear) {\n"
    " 47          soul_t* soul = reincarnation_queue.queue[reincarnation_queue.front];\n"
    " 48          \n"
    " 49          /* BUG: Integer overflow - reincarnation count wraps to 0 */\n"
    " 50          soul->reincarnation_count++;\n"
    " 51          \n"
    " 52          if (soul->reincarnation_count > REINCARNATION_LIMIT) {\n"
    " 53              /* Soul exhausted - send to void */\n"
    " 54              free(soul);\n"
    " 55          } else {\n"
    " 56              reincarnate(soul);\n"
    " 57          }\n"
    " 58          \n"
    " 59          reincarnation_queue.front++;\n"
    " 60      }\n"
    " 61  }\n"
    " 62  \n"
    " 63  /* Calculate karma score */\n"
    " 64  int calculate_karma(soul_t* soul) {\n"
    " 65      int score = 0;\n"
    " 66      \n"
    " 67      /* INEFFICIENCY: O(n^2) loop for simple sum */\n"
    " 68      for (int i = 0; i < soul->deed_count; i++) {\n"
    " 69          for (int j = 0; j < soul->deed_count; j++) {\n"
    " 70              if (i == j) {\n"
    " 71                  score += soul->deeds[i].karma_value;\n"
    " 72              }\n"
    " 73          }\n"
    " 74      }\n"
    " 75      \n"
    " 76      return score;\n"
    " 77  }\n";

/**
 * @brief Sample Death Network pseudo-code (part 2)
 */
static const char* DEATH_NETWORK_CODE_PART2 =
    " 78  \n"
    " 79  /* Transfer soul data */\n"
    " 80  soul_t* transfer_soul(soul_t* source) {\n"
    " 81      /* BUG: Memory leak - allocates but caller may not free */\n"
    " 82      soul_t* dest = malloc(sizeof(soul_t));\n"
    " 83      memcpy(dest, source, sizeof(soul_t));\n"
    " 84      \n"
    " 85      /* BUG: Shallow copy - pointers shared between source/dest */\n"
    " 86      dest->deeds = source->deeds;\n"
    " 87      dest->memories = source->memories;\n"
    " 88      \n"
    " 89      return dest;\n"
    " 90  }\n"
    " 91  \n"
    " 92  /* Judge soul based on final moments */\n"
    " 93  void judge_final_moments(soul_t* soul) {\n"
    " 94      /* BUG: Null pointer dereference if no final moment */\n"
    " 95      moment_t* final = soul->final_moment;\n"
    " 96      \n"
    " 97      if (final->type == MOMENT_SACRIFICE) {\n"
    " 98          soul->karma += 1000;\n"
    " 99      } else if (final->type == MOMENT_MURDER) {\n"
    " 100      soul->karma -= 1000;\n"
    " 101  }\n"
    " 102  /* BUG: Missing closing brace */\n"
    " 103  \n"
    " 104  /* INEFFICIENCY: Linear search instead of hash table */\n"
    " 105  soul_t* find_soul_by_id(uint64_t id) {\n"
    " 106      for (int i = 0; i < global_soul_count; i++) {\n"
    " 107          if (global_souls[i].id == id) {\n"
    " 108              return &global_souls[i];\n"
    " 109          }\n"
    " 110      }\n"
    " 111      return NULL;\n"
    " 112  }\n"
    " 113  \n"
    " 114  /* Check if soul qualifies for Heaven */\n"
    " 115  bool qualifies_for_heaven(soul_t* soul) {\n"
    " 116      /* BUG: Always returns false due to assignment vs comparison */\n"
    " 117      if (soul->karma = 0) {\n"
    " 118          return false;\n"
    " 119      }\n"
    " 120      \n"
    " 121      return soul->karma >= HEAVEN_THRESHOLD;\n"
    " 122  }\n"
    " 123  \n"
    " 124  /* Eternal punishment queue */\n"
    " 125  void add_to_hell_queue(soul_t* soul) {\n"
    " 126      /* INJUSTICE: Mentally ill treated same as malicious */\n"
    " 127      /* BUG: No check for mental illness or diminished capacity */\n"
    " 128      hell_queue_push(soul);\n"
    " 129  }\n"
    " 130  \n"
    " 131  /* Soul merge for twin flames */\n"
    " 132  soul_t* merge_souls(soul_t* s1, soul_t* s2) {\n"
    " 133      /* BUG: NULL check missing */\n"
    " 134      soul_t* merged = create_soul();\n"
    " 135      \n"
    " 136      /* BUG: Use after free if souls are freed elsewhere */\n"
    " 137      merged->karma = s1->karma + s2->karma;\n"
    " 138      \n"
    " 139      /* INEFFICIENCY: Redundant memory allocation */\n"
    " 140      merged->deeds = malloc(sizeof(deed_t) * 1000);\n"
    " 141      free(merged->deeds);\n"
    " 142      merged->deeds = malloc(sizeof(deed_t) * (s1->deed_count + s2->deed_count));\n"
    " 143      \n"
    " 144      return merged;\n"
    " 145  }\n";

TechnicalTrialState* technical_trial_create(void) {
    TechnicalTrialState* state = calloc(1, sizeof(TechnicalTrialState));
    if (!state) {
        fprintf(stderr, "Failed to allocate technical trial state\n");
        return NULL;
    }

    /* Initialize state */
    state->active = false;
    state->bugs = NULL;
    state->bug_count = 0;
    state->bugs_discovered = 0;
    state->bugs_reported_correctly = 0;
    state->source_code = NULL;
    state->code_lines = 0;
    state->current_view_start = 0;
    state->current_view_end = 0;
    state->inspections_made = 0;
    state->hint_shown = false;
    state->score = 0.0f;

    return state;
}

void technical_trial_destroy(TechnicalTrialState* state) {
    if (!state) {
        return;
    }

    if (state->bugs) {
        free(state->bugs);
    }

    if (state->source_code) {
        free(state->source_code);
    }

    free(state);
}

bool technical_trial_start(TechnicalTrialState* state, const char* filepath) {
    if (!state || !filepath) {
        return false;
    }

    /* Load bug database from file */
    DataFile* file = data_file_load(filepath);
    if (!file) {
        fprintf(stderr, "Failed to load technical trial data from %s\n", filepath);
        return false;
    }

    /* Get all BUG sections */
    size_t section_count = 0;
    const DataSection** sections = data_file_get_sections(file, "BUG", &section_count);
    if (!sections || section_count == 0) {
        fprintf(stderr, "No BUG sections found in %s\n", filepath);
        data_file_destroy(file);
        return false;
    }

    /* Allocate bug array */
    state->bug_count = section_count < MAX_BUGS ? section_count : MAX_BUGS;
    state->bugs = calloc(state->bug_count, sizeof(DeathNetworkBug));
    if (!state->bugs) {
        fprintf(stderr, "Failed to allocate bug array\n");
        data_file_destroy(file);
        return false;
    }

    /* Parse bug data */
    for (size_t i = 0; i < state->bug_count; i++) {
        const DataSection* section = sections[i];
        DeathNetworkBug* bug = &state->bugs[i];

        /* Get bug fields */
        const DataValue* line_val = data_section_get(section, "line");
        const DataValue* type_val = data_section_get(section, "type");
        const DataValue* severity_val = data_section_get(section, "severity");
        const DataValue* desc_val = data_section_get(section, "description");
        const DataValue* hint_val = data_section_get(section, "hint");

        /* Parse values */
        bug->line_number = (uint32_t)data_value_get_int(line_val, 0);

        /* Parse bug type */
        const char* type_str = data_value_get_string(type_val, "logic_error");
        if (strcmp(type_str, "logic_error") == 0) {
            bug->type = BUG_TYPE_LOGIC_ERROR;
        } else if (strcmp(type_str, "inefficiency") == 0) {
            bug->type = BUG_TYPE_INEFFICIENCY;
        } else if (strcmp(type_str, "injustice") == 0) {
            bug->type = BUG_TYPE_INJUSTICE;
        } else {
            bug->type = BUG_TYPE_LOGIC_ERROR;
        }

        /* Parse severity */
        const char* severity_str = data_value_get_string(severity_val, "medium");
        if (strcmp(severity_str, "low") == 0) {
            bug->severity = BUG_SEVERITY_LOW;
        } else if (strcmp(severity_str, "medium") == 0) {
            bug->severity = BUG_SEVERITY_MEDIUM;
        } else if (strcmp(severity_str, "high") == 0) {
            bug->severity = BUG_SEVERITY_HIGH;
        } else if (strcmp(severity_str, "critical") == 0) {
            bug->severity = BUG_SEVERITY_CRITICAL;
        } else {
            bug->severity = BUG_SEVERITY_MEDIUM;
        }

        /* Copy strings */
        const char* desc = data_value_get_string(desc_val, "");
        snprintf(bug->description, sizeof(bug->description), "%s", desc);

        const char* hint = data_value_get_string(hint_val, "");
        snprintf(bug->hint, sizeof(bug->hint), "%s", hint);

        bug->discovered = false;
        bug->reported_correctly = false;
    }

    data_file_destroy(file);

    /* Load source code - concatenate parts */
    size_t len1 = strlen(DEATH_NETWORK_CODE_PART1);
    size_t len2 = strlen(DEATH_NETWORK_CODE_PART2);
    state->source_code = malloc(len1 + len2 + 1);
    if (!state->source_code) {
        fprintf(stderr, "Failed to allocate source code\n");
        return false;
    }
    strcpy(state->source_code, DEATH_NETWORK_CODE_PART1);
    strcat(state->source_code, DEATH_NETWORK_CODE_PART2);

    /* Count lines */
    state->code_lines = 0;
    for (const char* p = state->source_code; *p; p++) {
        if (*p == '\n') {
            state->code_lines++;
        }
    }

    /* Reset trial state */
    state->active = true;
    state->bugs_discovered = 0;
    state->bugs_reported_correctly = 0;
    state->current_view_start = 0;
    state->current_view_end = 0;
    state->inspections_made = 0;
    state->hint_shown = false;
    state->score = 0.0f;

    return true;
}

const char* technical_trial_inspect_line(TechnicalTrialState* state,
                                          uint32_t line_number) {
    if (!state || !state->active || !state->source_code) {
        return NULL;
    }

    state->inspections_made++;

    /* Calculate view window */
    uint32_t start = (line_number > CODE_CONTEXT_LINES) ?
                      (line_number - CODE_CONTEXT_LINES) : 1;
    uint32_t end = line_number + CODE_CONTEXT_LINES;

    if (end > state->code_lines) {
        end = state->code_lines;
    }

    state->current_view_start = start;
    state->current_view_end = end;

    /* Extract code section */
    memset(code_view_buffer, 0, sizeof(code_view_buffer));
    char* dest = code_view_buffer;
    size_t remaining = sizeof(code_view_buffer) - 1;

    const char* src = state->source_code;
    uint32_t current_line = 1;
    bool in_range = false;

    while (*src && remaining > 0) {
        if (current_line >= start && current_line <= end) {
            in_range = true;
        } else if (in_range) {
            break; /* Past the range */
        }

        if (in_range) {
            *dest++ = *src;
            remaining--;
        }

        if (*src == '\n') {
            current_line++;
        }

        src++;
    }

    *dest = '\0';
    return code_view_buffer;
}

bool technical_trial_report_bug(TechnicalTrialState* state,
                                 uint32_t line_number,
                                 BugType type) {
    if (!state || !state->active || !state->bugs) {
        return false;
    }

    /* Find bug at this line */
    for (size_t i = 0; i < state->bug_count; i++) {
        DeathNetworkBug* bug = &state->bugs[i];

        if (bug->line_number == line_number) {
            if (!bug->discovered) {
                bug->discovered = true;
                state->bugs_discovered++;
            }

            /* Check if type is correct */
            if (bug->type == type) {
                if (!bug->reported_correctly) {
                    bug->reported_correctly = true;
                    state->bugs_reported_correctly++;
                }
                return true;
            }

            return false; /* Bug exists but wrong type */
        }
    }

    return false; /* No bug at this line */
}

const char* technical_trial_get_hint(TechnicalTrialState* state,
                                      uint32_t line_number) {
    if (!state || !state->bugs) {
        return NULL;
    }

    state->hint_shown = true;

    /* Find bug at this line */
    for (size_t i = 0; i < state->bug_count; i++) {
        const DeathNetworkBug* bug = &state->bugs[i];

        if (bug->line_number == line_number) {
            return bug->hint;
        }
    }

    return NULL;
}

float technical_trial_calculate_score(const TechnicalTrialState* state) {
    if (!state) {
        return 0.0f;
    }

    /* Base score from bugs found */
    float base_score = ((float)state->bugs_discovered / (float)state->bug_count) * 100.0f;

    /* Bonus for correct classification */
    float classification_bonus = 0.0f;
    if (state->bugs_discovered > 0) {
        classification_bonus = ((float)state->bugs_reported_correctly /
                                (float)state->bugs_discovered) * 10.0f;
    }

    /* Penalty for excessive inspections */
    float inspection_penalty = 0.0f;
    if (state->inspections_made > 50) {
        inspection_penalty = (state->inspections_made - 50) * 0.5f;
    }

    /* Penalty for using hints */
    float hint_penalty = state->hint_shown ? 10.0f : 0.0f;

    float score = base_score + classification_bonus - inspection_penalty - hint_penalty;

    /* Clamp to 0-100 */
    if (score < 0.0f) score = 0.0f;
    if (score > 100.0f) score = 100.0f;

    return score;
}

bool technical_trial_is_complete(const TechnicalTrialState* state) {
    if (!state) {
        return false;
    }

    /* Complete if we've found enough bugs or exhausted all */
    return (state->bugs_discovered >= MIN_BUGS_TO_PASS) ||
           (state->bugs_discovered >= state->bug_count);
}

bool technical_trial_is_passed(const TechnicalTrialState* state) {
    if (!state) {
        return false;
    }

    return state->bugs_discovered >= MIN_BUGS_TO_PASS;
}

const char* technical_trial_get_stats(const TechnicalTrialState* state) {
    if (!state) {
        return NULL;
    }

    snprintf(stats_buffer, sizeof(stats_buffer),
             "Bugs discovered: %zu / %zu\n"
             "Correctly classified: %zu / %zu\n"
             "Inspections made: %u\n"
             "Pass threshold: %d bugs\n"
             "Status: %s",
             state->bugs_discovered,
             state->bug_count,
             state->bugs_reported_correctly,
             state->bugs_discovered,
             state->inspections_made,
             MIN_BUGS_TO_PASS,
             technical_trial_is_passed(state) ? "PASSED" : "IN PROGRESS");

    return stats_buffer;
}

const char* technical_trial_bug_type_name(BugType type) {
    switch (type) {
        case BUG_TYPE_NONE:
            return "None";
        case BUG_TYPE_LOGIC_ERROR:
            return "Logic Error";
        case BUG_TYPE_INEFFICIENCY:
            return "Inefficiency";
        case BUG_TYPE_INJUSTICE:
            return "Injustice";
        default:
            return "Unknown";
    }
}

const char* technical_trial_bug_severity_name(BugSeverity severity) {
    switch (severity) {
        case BUG_SEVERITY_LOW:
            return "Low";
        case BUG_SEVERITY_MEDIUM:
            return "Medium";
        case BUG_SEVERITY_HIGH:
            return "High";
        case BUG_SEVERITY_CRITICAL:
            return "Critical";
        default:
            return "Unknown";
    }
}

const char* technical_trial_get_discovered_bugs(const TechnicalTrialState* state) {
    if (!state || !state->bugs) {
        return NULL;
    }

    memset(bugs_buffer, 0, sizeof(bugs_buffer));
    char* dest = bugs_buffer;
    size_t remaining = sizeof(bugs_buffer) - 1;

    int count = snprintf(dest, remaining, "Discovered Bugs:\n");
    if (count > 0) {
        dest += count;
        remaining -= count;
    }

    for (size_t i = 0; i < state->bug_count && remaining > 0; i++) {
        const DeathNetworkBug* bug = &state->bugs[i];

        if (bug->discovered) {
            count = snprintf(dest, remaining,
                             "  Line %u: [%s] %s - %s\n",
                             bug->line_number,
                             technical_trial_bug_type_name(bug->type),
                             technical_trial_bug_severity_name(bug->severity),
                             bug->description);
            if (count > 0) {
                dest += count;
                remaining -= count;
            }
        }
    }

    return bugs_buffer;
}

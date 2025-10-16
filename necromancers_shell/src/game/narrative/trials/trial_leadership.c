#include "trial_leadership.h"
#include "../../../data/data_loader.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * @brief Helper to parse resistance level from string
 */
static ResistanceLevel parse_resistance(const char* str) {
    if (strcmp(str, "low") == 0) return RESISTANCE_LOW;
    if (strcmp(str, "medium") == 0) return RESISTANCE_MEDIUM;
    if (strcmp(str, "high") == 0) return RESISTANCE_HIGH;
    return RESISTANCE_MEDIUM; /* Default */
}

/**
 * @brief Helper to parse attitude from string
 */
static MemberAttitude parse_attitude(const char* str) {
    if (strcmp(str, "hostile") == 0) return ATTITUDE_HOSTILE;
    if (strcmp(str, "neutral") == 0) return ATTITUDE_NEUTRAL;
    if (strcmp(str, "supportive") == 0) return ATTITUDE_SUPPORTIVE;
    return ATTITUDE_NEUTRAL; /* Default */
}

/**
 * @brief Helper to convert attitude enum to score
 */
static int32_t attitude_to_score(MemberAttitude attitude) {
    switch (attitude) {
        case ATTITUDE_HOSTILE:    return -50;
        case ATTITUDE_NEUTRAL:    return 0;
        case ATTITUDE_SUPPORTIVE: return 50;
        default:                  return 0;
    }
}

/**
 * @brief Helper to convert score to attitude enum
 */
static MemberAttitude score_to_attitude(int32_t score) {
    if (score <= -25) return ATTITUDE_HOSTILE;
    if (score >= 25)  return ATTITUDE_SUPPORTIVE;
    return ATTITUDE_NEUTRAL;
}

/**
 * @brief Helper to apply corruption change with bounds checking
 */
static void apply_corruption_change(CouncilMemberProgress* member, float change) {
    member->corruption_current += change;

    /* Clamp to 0-100 */
    if (member->corruption_current < 0.0f) {
        member->corruption_current = 0.0f;
    }
    if (member->corruption_current > 100.0f) {
        member->corruption_current = 100.0f;
    }
}

/**
 * @brief Helper to apply attitude change with bounds checking
 */
static void apply_attitude_change(CouncilMemberProgress* member, int32_t change) {
    member->attitude_score += change;

    /* Clamp to -100 to +100 */
    if (member->attitude_score < -100) {
        member->attitude_score = -100;
    }
    if (member->attitude_score > 100) {
        member->attitude_score = 100;
    }

    /* Update attitude enum based on score */
    member->attitude = score_to_attitude(member->attitude_score);
}

LeadershipTrialState* leadership_trial_create(void) {
    LeadershipTrialState* state = calloc(1, sizeof(LeadershipTrialState));
    if (!state) {
        fprintf(stderr, "Failed to allocate leadership trial state\n");
        return NULL;
    }

    /* Initialize state */
    state->active = false;
    state->current_day = 0;
    state->days_remaining = LEADERSHIP_TRIAL_DURATION;
    state->member_count = 0;
    state->collective_corruption_start = 0.0f;
    state->collective_corruption_current = 0.0f;
    state->collective_corruption_target = 0.0f;
    state->reforms_successful = 0;
    state->council_meetings_held = 0;
    state->last_council_meeting_day = 0;
    state->target_met = false;

    /* Initialize members to empty */
    for (size_t i = 0; i < MAX_COUNCIL_MEMBERS; i++) {
        state->members[i].npc_id[0] = '\0';
        state->members[i].name[0] = '\0';
        state->members[i].corruption_start = 0.0f;
        state->members[i].corruption_current = 0.0f;
        state->members[i].corruption_target = 0.0f;
        state->members[i].resistance = RESISTANCE_MEDIUM;
        state->members[i].attitude_score = 0;
        state->members[i].attitude = ATTITUDE_NEUTRAL;
        state->members[i].reforms_applied = 0;
        state->members[i].meetings_held = 0;
        state->members[i].last_interaction_day = 0;
        state->members[i].specialty[0] = '\0';
    }

    return state;
}

void leadership_trial_destroy(LeadershipTrialState* state) {
    if (!state) {
        return;
    }

    free(state);
}

bool leadership_trial_start(LeadershipTrialState* state) {
    if (!state) {
        return false;
    }

    /* Must have members loaded */
    if (state->member_count == 0) {
        fprintf(stderr, "Cannot start leadership trial: no members loaded\n");
        return false;
    }

    /* Reset trial state */
    state->active = true;
    state->current_day = 1;
    state->days_remaining = LEADERSHIP_TRIAL_DURATION; /* Day 1 has 30 days total */
    state->reforms_successful = 0;
    state->council_meetings_held = 0;
    state->last_council_meeting_day = 0;
    state->target_met = false;

    /* Calculate starting collective corruption */
    state->collective_corruption_start = leadership_trial_calculate_collective_corruption(state);
    state->collective_corruption_current = state->collective_corruption_start;

    /* Set target (10% reduction) */
    state->collective_corruption_target = state->collective_corruption_start * 0.9f;

    /* Reset member runtime state */
    for (size_t i = 0; i < state->member_count; i++) {
        state->members[i].corruption_current = state->members[i].corruption_start;
        state->members[i].reforms_applied = 0;
        state->members[i].meetings_held = 0;
        state->members[i].last_interaction_day = 0;
    }

    return true;
}

bool leadership_trial_load_from_file(LeadershipTrialState* state, const char* filepath) {
    if (!state || !filepath) {
        return false;
    }

    DataFile* file = data_file_load(filepath);
    if (!file) {
        fprintf(stderr, "Failed to load leadership trial data from %s\n", filepath);
        return false;
    }

    /* Get MEMBER sections */
    size_t section_count = 0;
    const DataSection** sections = data_file_get_sections(file, "MEMBER", &section_count);
    if (!sections || section_count == 0) {
        fprintf(stderr, "No MEMBER sections found in %s\n", filepath);
        data_file_destroy(file);
        return false;
    }

    /* Limit to MAX_COUNCIL_MEMBERS */
    if (section_count > MAX_COUNCIL_MEMBERS) {
        section_count = MAX_COUNCIL_MEMBERS;
    }

    /* Parse each member */
    for (size_t i = 0; i < section_count; i++) {
        const DataSection* section = sections[i];
        CouncilMemberProgress* member = &state->members[i];

        /* Get member NPC ID from section identifier */
        snprintf(member->npc_id, sizeof(member->npc_id), "%s", section->section_id);

        /* Parse member data */
        const DataValue* name_val = data_section_get(section, "name");
        const DataValue* corruption_start_val = data_section_get(section, "corruption_start");
        const DataValue* corruption_target_val = data_section_get(section, "corruption_target");
        const DataValue* resistance_val = data_section_get(section, "resistance");
        const DataValue* attitude_val = data_section_get(section, "attitude_start");
        const DataValue* specialty_val = data_section_get(section, "specialty");

        /* Copy name */
        const char* name = data_value_get_string(name_val, "Unknown");
        snprintf(member->name, sizeof(member->name), "%s", name);

        /* Set corruption values */
        member->corruption_start = (float)data_value_get_int(corruption_start_val, 50);
        member->corruption_current = member->corruption_start;
        member->corruption_target = (float)data_value_get_int(corruption_target_val,
                                                                (int64_t)(member->corruption_start * 0.9f));

        /* Parse resistance */
        const char* resistance_str = data_value_get_string(resistance_val, "medium");
        member->resistance = parse_resistance(resistance_str);

        /* Parse attitude */
        const char* attitude_str = data_value_get_string(attitude_val, "neutral");
        member->attitude = parse_attitude(attitude_str);
        member->attitude_score = attitude_to_score(member->attitude);

        /* Copy specialty */
        const char* specialty = data_value_get_string(specialty_val, "general");
        snprintf(member->specialty, sizeof(member->specialty), "%s", specialty);

        /* Initialize runtime state */
        member->reforms_applied = 0;
        member->meetings_held = 0;
        member->last_interaction_day = 0;
    }

    state->member_count = section_count;
    data_file_destroy(file);
    return true;
}

bool leadership_trial_advance_day(LeadershipTrialState* state) {
    if (!state || !state->active) {
        return false;
    }

    /* Advance day */
    state->current_day++;
    state->days_remaining = LEADERSHIP_TRIAL_DURATION - state->current_day + 1;

    /* Check if trial complete (reached day 30) */
    if (state->current_day >= LEADERSHIP_TRIAL_DURATION) {
        state->active = false;
        state->target_met = leadership_trial_is_target_met(state);
    }

    /* Apply small daily corruption drift (0.1% random variation) */
    /* This simulates natural fluctuation without player intervention */
    /* In a full implementation, this would use random number generation */
    /* For now, we skip automatic drift to make testing deterministic */

    /* Update collective corruption */
    state->collective_corruption_current = leadership_trial_calculate_collective_corruption(state);

    /* Check if target met */
    state->target_met = leadership_trial_is_target_met(state);

    return true;
}

bool leadership_trial_meet_with_member(LeadershipTrialState* state,
                                        size_t member_index,
                                        GuidanceApproach approach) {
    if (!state || !state->active) {
        return false;
    }

    if (member_index >= state->member_count) {
        return false;
    }

    CouncilMemberProgress* member = &state->members[member_index];

    /* Apply guidance based on approach and resistance */
    float corruption_change = 0.0f;
    int32_t attitude_change = 0;

    switch (approach) {
        case GUIDANCE_HARSH:
            /* Effective on high resistance, damages attitude */
            corruption_change = -3.0f;
            attitude_change = -10;
            break;

        case GUIDANCE_DIPLOMATIC:
            /* Balanced approach, improves attitude */
            corruption_change = -2.0f;
            attitude_change = 5;
            break;

        case GUIDANCE_INSPIRATIONAL:
            /* Effective on low resistance/supportive, big attitude boost */
            corruption_change = -1.0f;
            attitude_change = 15;
            break;
    }

    /* Adjust effectiveness based on resistance */
    if (approach == GUIDANCE_HARSH && member->resistance != RESISTANCE_HIGH) {
        /* Harsh approach less effective on non-high resistance */
        corruption_change *= 0.5f;
    }
    if (approach == GUIDANCE_DIPLOMATIC && member->resistance != RESISTANCE_MEDIUM) {
        /* Diplomatic less effective outside medium resistance */
        corruption_change *= 0.75f;
    }
    if (approach == GUIDANCE_INSPIRATIONAL && member->resistance == RESISTANCE_HIGH) {
        /* Inspirational ineffective on high resistance */
        corruption_change *= 0.3f;
    }

    /* Apply changes */
    apply_corruption_change(member, corruption_change);
    apply_attitude_change(member, attitude_change);

    /* Record meeting */
    member->meetings_held++;
    member->last_interaction_day = state->current_day;

    /* Update collective corruption */
    state->collective_corruption_current = leadership_trial_calculate_collective_corruption(state);

    return true;
}

bool leadership_trial_council_meeting(LeadershipTrialState* state) {
    if (!state || !state->active) {
        return false;
    }

    /* Check cooldown (can only hold council meeting every 3 days) */
    if (state->last_council_meeting_day != 0 &&
        state->current_day - state->last_council_meeting_day < COUNCIL_MEETING_COOLDOWN) {
        return false;
    }

    /* Apply small group dynamic bonus to all members */
    /* Supportive members improve nearby members slightly */
    size_t supportive_count = 0;
    for (size_t i = 0; i < state->member_count; i++) {
        if (state->members[i].attitude == ATTITUDE_SUPPORTIVE) {
            supportive_count++;
        }
    }

    /* Each supportive member reduces others' corruption by 0.5% */
    float peer_bonus = supportive_count * 0.5f;
    for (size_t i = 0; i < state->member_count; i++) {
        apply_corruption_change(&state->members[i], -peer_bonus);
    }

    /* Record meeting */
    state->council_meetings_held++;
    state->last_council_meeting_day = state->current_day;

    /* Update collective corruption */
    state->collective_corruption_current = leadership_trial_calculate_collective_corruption(state);

    return true;
}

bool leadership_trial_implement_reform(LeadershipTrialState* state, ReformType reform) {
    if (!state || !state->active) {
        return false;
    }

    switch (reform) {
        case REFORM_CODE_OF_CONDUCT:
            /* All members -1%, except Mordak/Vorgath resist (no change) */
            for (size_t i = 0; i < state->member_count; i++) {
                /* Check if member is Mordak or Vorgath by name */
                const char* name = state->members[i].name;
                bool is_resistant = (strstr(name, "Mordak") != NULL ||
                                      strstr(name, "Vorgath") != NULL);
                if (!is_resistant) {
                    apply_corruption_change(&state->members[i], -1.0f);
                }
                state->members[i].reforms_applied++;
            }
            break;

        case REFORM_SOUL_ETHICS:
            /* Low corruption -2%, high corruption -0.5% (threshold 70%) */
            for (size_t i = 0; i < state->member_count; i++) {
                float change = (state->members[i].corruption_current < 70.0f) ? -2.0f : -0.5f;
                apply_corruption_change(&state->members[i], change);
                apply_attitude_change(&state->members[i], 5);
                state->members[i].reforms_applied++;
            }
            break;

        case REFORM_CORRUPTION_LIMITS:
            /* All -1.5%, but attitude -5 (enforced rule) */
            for (size_t i = 0; i < state->member_count; i++) {
                apply_corruption_change(&state->members[i], -1.5f);
                apply_attitude_change(&state->members[i], -5);
                state->members[i].reforms_applied++;
            }
            break;

        case REFORM_PEER_ACCOUNTABILITY:
            /* Supportive members influence others */
            for (size_t i = 0; i < state->member_count; i++) {
                if (state->members[i].attitude == ATTITUDE_SUPPORTIVE) {
                    /* Supportive member reduces own corruption by 1% */
                    apply_corruption_change(&state->members[i], -1.0f);
                }
                state->members[i].reforms_applied++;
            }
            /* Each supportive influences neutral/hostile */
            size_t supportive_count = 0;
            for (size_t i = 0; i < state->member_count; i++) {
                if (state->members[i].attitude == ATTITUDE_SUPPORTIVE) {
                    supportive_count++;
                }
            }
            for (size_t i = 0; i < state->member_count; i++) {
                if (state->members[i].attitude != ATTITUDE_SUPPORTIVE) {
                    float influence = supportive_count * 0.5f;
                    apply_corruption_change(&state->members[i], -influence);
                }
            }
            break;
    }

    state->reforms_successful++;

    /* Update collective corruption */
    state->collective_corruption_current = leadership_trial_calculate_collective_corruption(state);

    return true;
}

float leadership_trial_calculate_collective_corruption(const LeadershipTrialState* state) {
    if (!state || state->member_count == 0) {
        return 0.0f;
    }

    float total = 0.0f;
    for (size_t i = 0; i < state->member_count; i++) {
        total += state->members[i].corruption_current;
    }

    return total / (float)state->member_count;
}

bool leadership_trial_is_target_met(const LeadershipTrialState* state) {
    if (!state) {
        return false;
    }

    float current = leadership_trial_calculate_collective_corruption(state);
    return (current <= state->collective_corruption_target);
}

const CouncilMemberProgress* leadership_trial_get_member(
    const LeadershipTrialState* state,
    size_t member_index) {

    if (!state || member_index >= state->member_count) {
        return NULL;
    }

    return &state->members[member_index];
}

const CouncilMemberProgress* leadership_trial_get_member_by_id(
    const LeadershipTrialState* state,
    const char* npc_id) {

    if (!state || !npc_id) {
        return NULL;
    }

    for (size_t i = 0; i < state->member_count; i++) {
        if (strcmp(state->members[i].npc_id, npc_id) == 0) {
            return &state->members[i];
        }
    }

    return NULL;
}

bool leadership_trial_is_complete(const LeadershipTrialState* state) {
    if (!state) {
        return false;
    }

    return (state->current_day >= LEADERSHIP_TRIAL_DURATION);
}

bool leadership_trial_is_passed(const LeadershipTrialState* state) {
    if (!state) {
        return false;
    }

    /* Must be complete and target met */
    return (leadership_trial_is_complete(state) && state->target_met);
}

int leadership_trial_get_summary(const LeadershipTrialState* state,
                                  char* buffer,
                                  size_t buffer_size) {
    if (!state || !buffer || buffer_size == 0) {
        return 0;
    }

    int written = 0;
    size_t remaining = buffer_size;

    /* Current day */
    int n = snprintf(buffer + written, remaining,
                     "Day %u/%u (%u days remaining)\n",
                     state->current_day, LEADERSHIP_TRIAL_DURATION, state->days_remaining);
    if (n < 0 || (size_t)n >= remaining) return written;
    written += n;
    remaining -= n;

    /* Collective corruption */
    n = snprintf(buffer + written, remaining,
                 "Collective Corruption: %.1f%% (Start: %.1f%%, Target: %.1f%%)\n",
                 state->collective_corruption_current,
                 state->collective_corruption_start,
                 state->collective_corruption_target);
    if (n < 0 || (size_t)n >= remaining) return written;
    written += n;
    remaining -= n;

    /* Target status */
    n = snprintf(buffer + written, remaining,
                 "Target Met: %s\n",
                 state->target_met ? "YES" : "NO");
    if (n < 0 || (size_t)n >= remaining) return written;
    written += n;
    remaining -= n;

    /* Reforms and meetings */
    n = snprintf(buffer + written, remaining,
                 "Reforms: %u, Council Meetings: %u\n",
                 state->reforms_successful, state->council_meetings_held);
    if (n < 0 || (size_t)n >= remaining) return written;
    written += n;
    remaining -= n;

    /* Member summary */
    n = snprintf(buffer + written, remaining, "\nMembers:\n");
    if (n < 0 || (size_t)n >= remaining) return written;
    written += n;
    remaining -= n;

    for (size_t i = 0; i < state->member_count; i++) {
        const CouncilMemberProgress* m = &state->members[i];
        n = snprintf(buffer + written, remaining,
                     "  %s: %.1f%% (Target: %.1f%%, %s)\n",
                     m->name, m->corruption_current, m->corruption_target,
                     m->corruption_current <= m->corruption_target ? "MET" : "NOT MET");
        if (n < 0 || (size_t)n >= remaining) return written;
        written += n;
        remaining -= n;
    }

    return written;
}

const char* leadership_trial_resistance_name(ResistanceLevel resistance) {
    switch (resistance) {
        case RESISTANCE_LOW:    return "Low";
        case RESISTANCE_MEDIUM: return "Medium";
        case RESISTANCE_HIGH:   return "High";
        default:                return "Unknown";
    }
}

const char* leadership_trial_attitude_name(MemberAttitude attitude) {
    switch (attitude) {
        case ATTITUDE_HOSTILE:    return "Hostile";
        case ATTITUDE_NEUTRAL:    return "Neutral";
        case ATTITUDE_SUPPORTIVE: return "Supportive";
        default:                  return "Unknown";
    }
}

const char* leadership_trial_guidance_name(GuidanceApproach approach) {
    switch (approach) {
        case GUIDANCE_HARSH:          return "Harsh";
        case GUIDANCE_DIPLOMATIC:     return "Diplomatic";
        case GUIDANCE_INSPIRATIONAL:  return "Inspirational";
        default:                      return "Unknown";
    }
}

const char* leadership_trial_reform_name(ReformType reform) {
    switch (reform) {
        case REFORM_CODE_OF_CONDUCT:      return "Code of Conduct";
        case REFORM_SOUL_ETHICS:          return "Soul Ethics";
        case REFORM_CORRUPTION_LIMITS:    return "Corruption Limits";
        case REFORM_PEER_ACCOUNTABILITY:  return "Peer Accountability";
        default:                          return "Unknown";
    }
}

float leadership_trial_calculate_score(const LeadershipTrialState* state) {
    if (!state || !leadership_trial_is_complete(state)) {
        return 0.0f;
    }

    float score = 0.0f;

    /* Base score: 40 points for completing trial */
    score += 40.0f;

    /* 30 points for meeting collective target */
    if (state->target_met) {
        score += 30.0f;
    }

    /* 5 points per member who met individual target (max 30 points) */
    for (size_t i = 0; i < state->member_count; i++) {
        if (state->members[i].corruption_current <= state->members[i].corruption_target) {
            score += 5.0f;
        }
    }

    /* Bonus for supportive attitudes (up to 10 points) */
    size_t supportive_count = 0;
    for (size_t i = 0; i < state->member_count; i++) {
        if (state->members[i].attitude == ATTITUDE_SUPPORTIVE) {
            supportive_count++;
        }
    }
    score += (supportive_count / (float)state->member_count) * 10.0f;

    /* Clamp to 0-100 */
    if (score < 0.0f) score = 0.0f;
    if (score > 100.0f) score = 100.0f;

    return score;
}

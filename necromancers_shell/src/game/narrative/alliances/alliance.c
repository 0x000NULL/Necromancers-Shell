#include "alliance.h"
#include <stdlib.h>
#include <string.h>

/* Trust level requirements for alliance types */
#define TRUST_NON_AGGRESSION 20.0f
#define TRUST_INFO_EXCHANGE 40.0f
#define TRUST_FULL_ALLIANCE 60.0f

/* Trust gain from sharing */
#define TRUST_PER_KNOWLEDGE 2.0f
#define TRUST_PER_100_RESOURCES 1.0f

/* Alliance type names */
static const char* ALLIANCE_TYPE_NAMES[] = {
    "Hostile",
    "Neutral",
    "Non-Aggression Pact",
    "Information Exchange",
    "Full Alliance"
};

/* Alliance descriptions */
static const char* ALLIANCE_DESCRIPTIONS[] = {
    "Actively antagonistic - may attack on sight",
    "Indifferent - no interaction or cooperation",
    "Peaceful coexistence - will not attack",
    "Sharing knowledge and research",
    "Full cooperation - resources, knowledge, joint operations"
};

Alliance* alliance_create(const char* npc_id, AllianceType type) {
    if (!npc_id) {
        return NULL;
    }

    Alliance* alliance = malloc(sizeof(Alliance));
    if (!alliance) {
        return NULL;
    }

    /* Initialize fields */
    strncpy(alliance->npc_id, npc_id, sizeof(alliance->npc_id) - 1);
    alliance->npc_id[sizeof(alliance->npc_id) - 1] = '\0';

    alliance->type = type;
    alliance->phylactery_oath = false;
    alliance->knowledge_shared = 0;
    alliance->resources_shared = 0;
    alliance->trust_level = 0.0f;
    alliance->days_since_formed = 0;
    alliance->interactions = 0;

    return alliance;
}

void alliance_destroy(Alliance* alliance) {
    if (alliance) {
        free(alliance);
    }
}

bool alliance_upgrade(Alliance* alliance) {
    if (!alliance) {
        return false;
    }

    /* Check if already at maximum */
    if (alliance->type == ALLIANCE_FULL) {
        return false;
    }

    /* Advance to next alliance type */
    alliance->type = (AllianceType)(alliance->type + 1);
    return true;
}

bool alliance_downgrade(Alliance* alliance) {
    if (!alliance) {
        return false;
    }

    /* Check if phylactery oath prevents downgrade */
    if (alliance->phylactery_oath) {
        return false;
    }

    /* Check if already at minimum */
    if (alliance->type == ALLIANCE_HOSTILE) {
        return false;
    }

    /* Decrease to previous alliance type */
    alliance->type = (AllianceType)(alliance->type - 1);
    return true;
}

bool alliance_set_type(Alliance* alliance, AllianceType type) {
    if (!alliance) {
        return false;
    }

    /* Phylactery oath prevents downgrading */
    if (alliance->phylactery_oath && type < alliance->type) {
        return false;
    }

    alliance->type = type;
    return true;
}

bool alliance_bind_phylactery_oath(Alliance* alliance) {
    if (!alliance) {
        return false;
    }

    /* Check if already bound */
    if (alliance->phylactery_oath) {
        return false;
    }

    alliance->phylactery_oath = true;
    return true;
}

bool alliance_add_knowledge(Alliance* alliance, uint32_t count) {
    if (!alliance) {
        return false;
    }

    alliance->knowledge_shared += count;

    /* Increase trust based on knowledge shared */
    float trust_gain = count * TRUST_PER_KNOWLEDGE;
    alliance_modify_trust(alliance, trust_gain);

    return true;
}

bool alliance_add_resources(Alliance* alliance, uint32_t amount) {
    if (!alliance) {
        return false;
    }

    alliance->resources_shared += amount;

    /* Increase trust based on resources shared */
    float trust_gain = (amount / 100.0f) * TRUST_PER_100_RESOURCES;
    alliance_modify_trust(alliance, trust_gain);

    return true;
}

bool alliance_modify_trust(Alliance* alliance, float change) {
    if (!alliance) {
        return false;
    }

    alliance->trust_level += change;

    /* Clamp to valid range [0, 100] */
    if (alliance->trust_level < 0.0f) {
        alliance->trust_level = 0.0f;
    } else if (alliance->trust_level > 100.0f) {
        alliance->trust_level = 100.0f;
    }

    return true;
}

void alliance_record_interaction(Alliance* alliance) {
    if (alliance) {
        alliance->interactions++;
    }
}

void alliance_advance_time(Alliance* alliance, uint32_t days) {
    if (alliance) {
        alliance->days_since_formed += days;
    }
}

bool alliance_can_upgrade(const Alliance* alliance) {
    if (!alliance) {
        return false;
    }

    /* Check if already at maximum */
    if (alliance->type == ALLIANCE_FULL) {
        return false;
    }

    /* Check trust requirements for next level */
    AllianceType next_type = (AllianceType)(alliance->type + 1);

    switch (next_type) {
        case ALLIANCE_NON_AGGRESSION:
            return alliance->trust_level >= TRUST_NON_AGGRESSION;
        case ALLIANCE_INFO_EXCHANGE:
            return alliance->trust_level >= TRUST_INFO_EXCHANGE;
        case ALLIANCE_FULL:
            return alliance->trust_level >= TRUST_FULL_ALLIANCE;
        default:
            return true; /* HOSTILE → NEUTRAL, NEUTRAL can always happen */
    }
}

bool alliance_can_downgrade(const Alliance* alliance) {
    if (!alliance) {
        return false;
    }

    /* Phylactery oath prevents downgrade */
    if (alliance->phylactery_oath) {
        return false;
    }

    /* Check if already at minimum */
    if (alliance->type == ALLIANCE_HOSTILE) {
        return false;
    }

    return true;
}

bool alliance_meets_trust_requirement(const Alliance* alliance) {
    if (!alliance) {
        return false;
    }

    switch (alliance->type) {
        case ALLIANCE_HOSTILE:
        case ALLIANCE_NEUTRAL:
            return true; /* No trust requirements */
        case ALLIANCE_NON_AGGRESSION:
            return alliance->trust_level >= TRUST_NON_AGGRESSION;
        case ALLIANCE_INFO_EXCHANGE:
            return alliance->trust_level >= TRUST_INFO_EXCHANGE;
        case ALLIANCE_FULL:
            return alliance->trust_level >= TRUST_FULL_ALLIANCE;
        default:
            return false;
    }
}

const char* alliance_type_name(AllianceType type) {
    if (type < 0 || type >= 5) {
        return "Unknown";
    }
    return ALLIANCE_TYPE_NAMES[type];
}

const char* alliance_get_description(const Alliance* alliance) {
    if (!alliance) {
        return "Invalid alliance";
    }

    int type_index = (int)alliance->type;
    if (type_index < 0 || type_index >= 5) {
        return "Unknown alliance state";
    }

    return ALLIANCE_DESCRIPTIONS[type_index];
}

bool alliance_supports_coordination(AllianceType type) {
    /* INFO_EXCHANGE and FULL allow joint operations */
    return type >= ALLIANCE_INFO_EXCHANGE;
}

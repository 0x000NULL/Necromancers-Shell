#include "thessara.h"
#include <stdlib.h>
#include <string.h>

/* Trust level descriptions */
static const char* TRUST_DESCRIPTIONS[] = {
    "Deeply Suspicious",     /* 0-20% */
    "Wary",                  /* 21-40% */
    "Cautiously Trusting",   /* 41-60% */
    "Trusting",              /* 61-75% */
    "Deeply Trusting"        /* 76-100% */
};

/* Knowledge type names */
static const char* KNOWLEDGE_TYPE_NAMES[] = {
    "Research",
    "Spell",
    "Technique",
    "History",
    "Path Secret"
};

ThessaraRelationship* thessara_create(void) {
    ThessaraRelationship* thessara = malloc(sizeof(ThessaraRelationship));
    if (!thessara) {
        return NULL;
    }

    /* Initialize discovery */
    thessara->discovered = false;
    thessara->discovery_day = 0;

    /* Initialize connection state */
    thessara->severed = false;
    thessara->severed_day = 0;

    /* Initialize interaction tracking */
    thessara->meetings_count = 0;
    thessara->last_meeting_day = 0;

    /* Initialize knowledge transfers */
    thessara->transfer_count = 0;

    /* Initialize trust */
    thessara->trust_level = 0.0f;

    /* Initialize warnings */
    thessara->warning_count = 0;

    /* Initialize path revelations */
    thessara->wraith_path_revealed = false;
    thessara->morningstar_path_revealed = false;
    thessara->archon_guidance_given = false;

    /* Initialize mentorship metrics */
    thessara->total_guidance_time = 0;
    thessara->trials_assisted = 0;

    return thessara;
}

void thessara_destroy(ThessaraRelationship* thessara) {
    if (thessara) {
        free(thessara);
    }
}

bool thessara_discover(ThessaraRelationship* thessara, uint32_t day) {
    if (!thessara) {
        return false;
    }

    if (thessara->discovered) {
        return false; /* Already discovered */
    }

    thessara->discovered = true;
    thessara->discovery_day = day;

    /* Initial trust granted upon discovery */
    thessara->trust_level = 25.0f;

    return true;
}

bool thessara_sever_connection(ThessaraRelationship* thessara, uint32_t day) {
    if (!thessara) {
        return false;
    }

    if (thessara->severed) {
        return false; /* Already severed */
    }

    thessara->severed = true;
    thessara->severed_day = day;

    return true;
}

bool thessara_record_meeting(ThessaraRelationship* thessara, uint32_t day) {
    if (!thessara) {
        return false;
    }

    if (thessara->severed) {
        return false; /* Cannot meet after severance */
    }

    thessara->meetings_count++;
    thessara->last_meeting_day = day;

    /* Each meeting increases trust slightly */
    thessara_add_trust(thessara, 2.0f);

    return true;
}

bool thessara_transfer_knowledge(ThessaraRelationship* thessara, KnowledgeType type,
                                  const char* id, const char* description, uint32_t day) {
    if (!thessara || !id || !description) {
        return false;
    }

    if (thessara->severed) {
        return false; /* Cannot transfer after severance */
    }

    if (thessara->transfer_count >= MAX_THESSARA_KNOWLEDGE) {
        return false; /* Transfer list full */
    }

    /* Create new transfer record */
    KnowledgeTransfer* transfer = &thessara->transfers[thessara->transfer_count];
    transfer->type = type;
    strncpy(transfer->id, id, sizeof(transfer->id) - 1);
    transfer->id[sizeof(transfer->id) - 1] = '\0';
    strncpy(transfer->description, description, sizeof(transfer->description) - 1);
    transfer->description[sizeof(transfer->description) - 1] = '\0';
    transfer->day_transferred = day;

    thessara->transfer_count++;

    /* Knowledge transfer increases trust significantly */
    thessara_add_trust(thessara, 5.0f);

    return true;
}

bool thessara_add_trust(ThessaraRelationship* thessara, float amount) {
    if (!thessara || amount < 0.0f) {
        return false;
    }

    thessara->trust_level += amount;

    /* Clamp to 100% */
    if (thessara->trust_level > 100.0f) {
        thessara->trust_level = 100.0f;
    }

    return true;
}

bool thessara_reduce_trust(ThessaraRelationship* thessara, float amount) {
    if (!thessara || amount < 0.0f) {
        return false;
    }

    thessara->trust_level -= amount;

    /* Clamp to 0% */
    if (thessara->trust_level < 0.0f) {
        thessara->trust_level = 0.0f;
    }

    return true;
}

bool thessara_give_warning(ThessaraRelationship* thessara, const char* warning) {
    if (!thessara || !warning) {
        return false;
    }

    if (thessara->warning_count >= MAX_THESSARA_WARNINGS) {
        return false; /* Warning list full */
    }

    strncpy(thessara->warnings[thessara->warning_count], warning,
            sizeof(thessara->warnings[0]) - 1);
    thessara->warnings[thessara->warning_count][sizeof(thessara->warnings[0]) - 1] = '\0';
    thessara->warning_count++;

    return true;
}

bool thessara_reveal_wraith_path(ThessaraRelationship* thessara) {
    if (!thessara) {
        return false;
    }

    if (thessara->wraith_path_revealed) {
        return false; /* Already revealed */
    }

    thessara->wraith_path_revealed = true;
    return true;
}

bool thessara_reveal_morningstar_path(ThessaraRelationship* thessara) {
    if (!thessara) {
        return false;
    }

    if (thessara->morningstar_path_revealed) {
        return false; /* Already revealed */
    }

    thessara->morningstar_path_revealed = true;
    return true;
}

bool thessara_give_archon_guidance(ThessaraRelationship* thessara) {
    if (!thessara) {
        return false;
    }

    if (thessara->archon_guidance_given) {
        return false; /* Already given */
    }

    thessara->archon_guidance_given = true;
    return true;
}

void thessara_assist_trial(ThessaraRelationship* thessara) {
    if (thessara) {
        thessara->trials_assisted++;
    }
}

void thessara_add_guidance_time(ThessaraRelationship* thessara, uint32_t hours) {
    if (thessara) {
        thessara->total_guidance_time += hours;
    }
}

bool thessara_is_severed(const ThessaraRelationship* thessara) {
    if (!thessara) {
        return false;
    }
    return thessara->severed;
}

bool thessara_can_meet(const ThessaraRelationship* thessara) {
    if (!thessara) {
        return false;
    }
    return !thessara->severed;
}

size_t thessara_get_transfer_count(const ThessaraRelationship* thessara) {
    if (!thessara) {
        return 0;
    }
    return thessara->transfer_count;
}

const KnowledgeTransfer* thessara_get_transfer(const ThessaraRelationship* thessara, size_t index) {
    if (!thessara || index >= thessara->transfer_count) {
        return NULL;
    }
    return &thessara->transfers[index];
}

bool thessara_has_high_trust(const ThessaraRelationship* thessara) {
    if (!thessara) {
        return false;
    }
    return thessara->trust_level >= 75.0f;
}

const char* thessara_get_trust_description(const ThessaraRelationship* thessara) {
    if (!thessara) {
        return "Unknown";
    }

    float trust = thessara->trust_level;

    if (trust <= 20.0f) return TRUST_DESCRIPTIONS[0];
    if (trust <= 40.0f) return TRUST_DESCRIPTIONS[1];
    if (trust <= 60.0f) return TRUST_DESCRIPTIONS[2];
    if (trust <= 75.0f) return TRUST_DESCRIPTIONS[3];
    return TRUST_DESCRIPTIONS[4];
}

const char* thessara_knowledge_type_name(KnowledgeType type) {
    if (type < 0 || type >= 5) {
        return "Unknown";
    }
    return KNOWLEDGE_TYPE_NAMES[type];
}

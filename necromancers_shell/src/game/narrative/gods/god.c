#include "god.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Domain names */
static const char* DOMAIN_NAMES[] = {
    "Life",
    "Order",
    "Time",
    "Souls",
    "Entropy",
    "Boundaries",
    "Networks"
};

/* Power level names */
static const char* POWER_NAMES[] = {
    "Mortal",
    "Avatar",
    "Lesser Divine",
    "Divine Architect"
};

/* Favor level descriptions */
static const char* FAVOR_DESCRIPTIONS[] = {
    "Utterly Despised",      /* -100 to -80 */
    "Deeply Hated",          /* -79 to -60 */
    "Strongly Disfavored",   /* -59 to -40 */
    "Disfavored",            /* -39 to -20 */
    "Slightly Disfavored",   /* -19 to -1 */
    "Neutral",               /* 0 */
    "Slightly Favored",      /* 1 to 20 */
    "Favored",               /* 21 to 40 */
    "Strongly Favored",      /* 41 to 60 */
    "Highly Regarded",       /* 61 to 80 */
    "Revered"                /* 81 to 100 */
};

God* god_create(const char* id, const char* name, GodDomain domain) {
    if (!id || !name) {
        return NULL;
    }

    God* god = malloc(sizeof(God));
    if (!god) {
        return NULL;
    }

    /* Initialize identity */
    snprintf(god->id, sizeof(god->id), "%s", id);
    snprintf(god->name, sizeof(god->name), "%s", name);
    god->title[0] = '\0';
    god->description[0] = '\0';

    /* Initialize divine attributes */
    god->domain = domain;
    god->power_level = POWER_DIVINE_ARCHITECT;
    god->manifestation[0] = '\0';
    god->personality[0] = '\0';

    /* Initialize favor */
    god->favor = 0;
    god->favor_min = -100;
    god->favor_max = 100;
    god->favor_start = 0;

    /* Initialize interaction tracking */
    god->interactions = 0;
    god->summoned = false;
    god->judgment_given = false;

    /* Initialize combat */
    god->combat_possible = false;
    god->combat_difficulty = 0;

    /* Initialize arrays */
    god->dialogue_tree_count = 0;
    god->trial_count = 0;
    god->restriction_count = 0;

    /* Initialize judgment state */
    god->amnesty_granted = false;
    god->condemned = false;

    return god;
}

void god_destroy(God* god) {
    if (god) {
        free(god);
    }
}

bool god_modify_favor(God* god, int16_t change) {
    if (!god) {
        return false;
    }

    god->favor += change;

    /* Clamp to valid range */
    if (god->favor < god->favor_min) {
        god->favor = god->favor_min;
    } else if (god->favor > god->favor_max) {
        god->favor = god->favor_max;
    }

    return true;
}

bool god_set_favor(God* god, int16_t favor) {
    if (!god) {
        return false;
    }

    god->favor = favor;

    /* Clamp to valid range */
    if (god->favor < god->favor_min) {
        god->favor = god->favor_min;
    } else if (god->favor > god->favor_max) {
        god->favor = god->favor_max;
    }

    return true;
}

bool god_add_dialogue_tree(God* god, const char* tree_id) {
    if (!god || !tree_id) {
        return false;
    }

    if (god->dialogue_tree_count >= MAX_GOD_DIALOGUE_TREES) {
        return false;
    }

    strncpy(god->dialogue_trees[god->dialogue_tree_count], tree_id,
            sizeof(god->dialogue_trees[0]) - 1);
    god->dialogue_trees[god->dialogue_tree_count][sizeof(god->dialogue_trees[0]) - 1] = '\0';
    god->dialogue_tree_count++;

    return true;
}

bool god_add_trial(God* god, const char* trial_id) {
    if (!god || !trial_id) {
        return false;
    }

    if (god->trial_count >= MAX_GOD_TRIALS) {
        return false;
    }

    strncpy(god->trials[god->trial_count], trial_id, sizeof(god->trials[0]) - 1);
    god->trials[god->trial_count][sizeof(god->trials[0]) - 1] = '\0';
    god->trial_count++;

    return true;
}

bool god_add_restriction(God* god, const char* restriction) {
    if (!god || !restriction) {
        return false;
    }

    if (god->restriction_count >= MAX_GOD_RESTRICTIONS) {
        return false;
    }

    strncpy(god->restrictions[god->restriction_count], restriction,
            sizeof(god->restrictions[0]) - 1);
    god->restrictions[god->restriction_count][sizeof(god->restrictions[0]) - 1] = '\0';
    god->restriction_count++;

    return true;
}

bool god_grant_amnesty(God* god) {
    if (!god) {
        return false;
    }

    if (god->amnesty_granted) {
        return false; /* Already granted */
    }

    god->amnesty_granted = true;
    god->condemned = false; /* Cannot be both */

    /* Typically increases favor */
    god_modify_favor(god, 20);

    return true;
}

bool god_condemn(God* god) {
    if (!god) {
        return false;
    }

    if (god->condemned) {
        return false; /* Already condemned */
    }

    god->condemned = true;
    god->amnesty_granted = false; /* Cannot be both */

    /* Significantly decreases favor */
    god_modify_favor(god, -50);

    return true;
}

void god_mark_summoned(God* god) {
    if (god) {
        god->summoned = true;
    }
}

void god_mark_judgment_given(God* god) {
    if (god) {
        god->judgment_given = true;
    }
}

void god_record_interaction(God* god) {
    if (god) {
        god->interactions++;
    }
}

bool god_has_positive_favor(const God* god) {
    if (!god) {
        return false;
    }
    return god->favor > 0;
}

bool god_has_negative_favor(const God* god) {
    if (!god) {
        return false;
    }
    return god->favor < 0;
}

bool god_is_neutral(const God* god) {
    if (!god) {
        return false;
    }
    return god->favor == 0;
}

const char* god_get_favor_description(const God* god) {
    if (!god) {
        return "Unknown";
    }

    int16_t favor = god->favor;

    if (favor <= -80) return FAVOR_DESCRIPTIONS[0];
    if (favor <= -60) return FAVOR_DESCRIPTIONS[1];
    if (favor <= -40) return FAVOR_DESCRIPTIONS[2];
    if (favor <= -20) return FAVOR_DESCRIPTIONS[3];
    if (favor <= -1)  return FAVOR_DESCRIPTIONS[4];
    if (favor == 0)   return FAVOR_DESCRIPTIONS[5];
    if (favor <= 20)  return FAVOR_DESCRIPTIONS[6];
    if (favor <= 40)  return FAVOR_DESCRIPTIONS[7];
    if (favor <= 60)  return FAVOR_DESCRIPTIONS[8];
    if (favor <= 80)  return FAVOR_DESCRIPTIONS[9];
    return FAVOR_DESCRIPTIONS[10];
}

const char* god_domain_name(GodDomain domain) {
    if (domain < 0 || domain >= 7) {
        return "Unknown";
    }
    return DOMAIN_NAMES[domain];
}

const char* god_power_level_name(PowerLevel power) {
    if (power < 0 || power >= 4) {
        return "Unknown";
    }
    return POWER_NAMES[power];
}

bool god_can_be_challenged(const God* god) {
    if (!god) {
        return false;
    }
    return god->combat_possible;
}

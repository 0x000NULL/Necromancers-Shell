#include "ending.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Ending type names */
static const char* ENDING_NAMES[] = {
    "None",
    "The Revenant",
    "The Wraith Administrator",
    "The Morningstar",
    "The Archon",
    "The Lich Lord",
    "Oblivion",
    "Divine Destruction"
};

/* Initialize ending with requirements */
static void init_ending(Ending* ending, EndingType type, const char* name,
                       const char* description, const char* epilogue,
                       const EndingRequirements* requirements) {
    ending->type = type;
    strncpy(ending->name, name, sizeof(ending->name) - 1);
    ending->name[sizeof(ending->name) - 1] = '\0';
    strncpy(ending->description, description, sizeof(ending->description) - 1);
    ending->description[sizeof(ending->description) - 1] = '\0';
    strncpy(ending->epilogue, epilogue, sizeof(ending->epilogue) - 1);
    ending->epilogue[sizeof(ending->epilogue) - 1] = '\0';

    ending->requirements = *requirements;
    ending->achievement = ACHIEVEMENT_AVAILABLE;
    ending->path_locked = false;
    ending->lock_reason[0] = '\0';
    ending->day_achieved = 0;
}

EndingManager* ending_manager_create(void) {
    EndingManager* manager = malloc(sizeof(EndingManager));
    if (!manager) {
        return NULL;
    }

    manager->current_ending = ENDING_NONE;
    manager->game_over = false;
    manager->ending_day = 0;

    /* Initialize Revenant ending */
    EndingRequirements revenant_reqs = {
        .min_corruption = 0,
        .max_corruption = 30,
        .min_consciousness = 70,
        .min_divine_favor = 40,
        .gods_favor_required = 3,
        .requires_ashbrook_spared = true,
        .forbids_full_harvest = true,
        .requires_thessara_trust = false,
        .requires_council_verdict = true,
        .requires_void_battle = false,
        .requires_all_trials = false,
        .min_souls_harvested = 0,
        .min_alliances = 0
    };
    init_ending(&manager->endings[0], ENDING_REVENANT, "The Revenant",
                "You choose resurrection. The gods see your restraint, your preserved humanity. "
                "Anara, Goddess of Life, grants you a second chance. Your consciousness returns "
                "to flesh. You wake gasping, alive, mortal, human. The necromancy fades like a "
                "dark dream. You remember everything but can no longer touch death's power. "
                "You live out your days in peace, haunted but human.",
                "Years later, you die naturally. This time, there is no resurrection. "
                "No consciousness persistence. Just... rest. The necromancer is gone. "
                "Only the human remains, at peace at last.",
                &revenant_reqs);

    /* Initialize Wraith ending */
    EndingRequirements wraith_reqs = {
        .min_corruption = 20,
        .max_corruption = 40,
        .min_consciousness = 80,
        .min_divine_favor = 20,
        .gods_favor_required = 2,
        .requires_ashbrook_spared = false,
        .forbids_full_harvest = true,
        .requires_thessara_trust = true,
        .requires_council_verdict = true,
        .requires_void_battle = false,
        .requires_all_trials = true,
        .min_souls_harvested = 500,
        .min_alliances = 2
    };
    init_ending(&manager->endings[1], ENDING_WRAITH, "The Wraith Administrator",
                "You ascend beyond flesh, beyond corruption. Pure consciousness managing the "
                "flow of souls. Nexus recognizes your mastery of the Death Network. You become "
                "a Wraith - an immortal administrator of death's infrastructure. Not alive, "
                "not dead, but something in between. You process souls for eternity, maintaining "
                "the boundary between life and death. It is lonely, but necessary.",
                "Millennia pass. Civilizations rise and fall. You remain, constant and eternal, "
                "a ghost in death's machine. You remember your name. You remember being human. "
                "But it seems so long ago now.",
                &wraith_reqs);

    /* Initialize Morningstar ending */
    EndingRequirements morningstar_reqs = {
        .min_corruption = 0,
        .max_corruption = 20,
        .min_consciousness = 90,
        .min_divine_favor = 60,
        .gods_favor_required = 5,
        .requires_ashbrook_spared = true,
        .forbids_full_harvest = true,
        .requires_thessara_trust = true,
        .requires_council_verdict = true,
        .requires_void_battle = true,
        .requires_all_trials = true,
        .min_souls_harvested = 0,
        .min_alliances = 4
    };
    init_ending(&manager->endings[2], ENDING_MORNINGSTAR, "The Morningstar",
                "You achieve what Thessara achieved 3,000 years ago. You defeat Vorathos in "
                "single combat. You pass all seven trials with perfect consciousness. The "
                "Divine Council votes unanimously: APOTHEOSIS. You ascend to become the Eighth "
                "Architect, the Morningstar, the last god. Your domain: Balance between life "
                "and death, consciousness and void. You are divine now, forever.",
                "You join the Seven. Now Eight. You remember being human. You remember the "
                "struggle, the choices, the fear. It makes you a better god. You guide future "
                "necromancers with compassion, as Thessara once guided you.",
                &morningstar_reqs);

    /* Initialize Archon ending */
    EndingRequirements archon_reqs = {
        .min_corruption = 30,
        .max_corruption = 60,
        .min_consciousness = 75,
        .min_divine_favor = 0,
        .gods_favor_required = 1,
        .requires_ashbrook_spared = false,
        .forbids_full_harvest = false,
        .requires_thessara_trust = false,
        .requires_council_verdict = true,
        .requires_void_battle = false,
        .requires_all_trials = true,
        .min_souls_harvested = 1000,
        .min_alliances = 3
    };
    init_ending(&manager->endings[3], ENDING_ARCHON, "The Archon",
                "You become death's system administrator. Not quite god, not quite mortal. "
                "The Archon. You have authority over the Death Network, manage soul processing, "
                "optimize the afterlife infrastructure. You are immortal, powerful, but not "
                "divine. The gods tolerate you because you are useful. You maintain the machine "
                "they built. Corruption stains your consciousness, but you remain functional.",
                "Centuries pass. You refine the systems. Improve efficiency. Reduce suffering "
                "in soul processing. You are not good, but you are competent. That is enough. "
                "The gods rarely speak to you. You prefer it that way.",
                &archon_reqs);

    /* Initialize Lich Lord ending */
    EndingRequirements lich_reqs = {
        .min_corruption = 70,
        .max_corruption = 100,
        .min_consciousness = 40,
        .min_divine_favor = -100,
        .gods_favor_required = 0,
        .requires_ashbrook_spared = false,
        .forbids_full_harvest = false,
        .requires_thessara_trust = false,
        .requires_council_verdict = false,
        .requires_void_battle = false,
        .requires_all_trials = false,
        .min_souls_harvested = 5000,
        .min_alliances = 0
    };
    init_ending(&manager->endings[4], ENDING_LICH_LORD, "The Lich Lord",
                "You embrace corruption fully. You harvest without mercy. You raise armies of "
                "undead. You conquer the living. The gods cannot stop you - you are too powerful. "
                "You become the Lich Lord, an immortal tyrant. You rule a kingdom of corpses. "
                "You have won. But at what cost? Your consciousness fragments. Your humanity "
                "dies. Only hunger remains.",
                "A thousand years pass. Your empire spans continents. Millions of undead serve "
                "you. But you are alone. Consciousness degraded to near-zero. You barely remember "
                "being human. You barely remember anything. Just endless hunger, endless conquest. "
                "This is hell. But you chose it.",
                &lich_reqs);

    /* Initialize Oblivion ending */
    EndingRequirements oblivion_reqs = {
        .min_corruption = 0,
        .max_corruption = 50,
        .min_consciousness = 60,
        .min_divine_favor = 20,
        .gods_favor_required = 1,
        .requires_ashbrook_spared = false,
        .forbids_full_harvest = false,
        .requires_thessara_trust = true,
        .requires_council_verdict = false,
        .requires_void_battle = false,
        .requires_all_trials = false,
        .min_souls_harvested = 0,
        .min_alliances = 0
    };
    init_ending(&manager->endings[5], ENDING_OBLIVION, "Oblivion",
                "You choose dissolution. You release all bound souls. You dismantle your undead "
                "army. You step into the void willingly. Consciousness scatters. You become "
                "nothing. It is not death - death is transition. This is erasure. Complete. "
                "Final. You choose non-existence over corruption. It is a kind of peace.",
                "Nothing remains. Not even memory. The necromancer never existed. "
                "The story ends. The void is silent.",
                &oblivion_reqs);

    /* Initialize Divine Destruction ending (failure) */
    EndingRequirements destruction_reqs = {
        .min_corruption = 90,
        .max_corruption = 100,
        .min_consciousness = 0,
        .min_divine_favor = -70,
        .gods_favor_required = 0,
        .requires_ashbrook_spared = false,
        .forbids_full_harvest = false,
        .requires_thessara_trust = false,
        .requires_council_verdict = false,
        .requires_void_battle = false,
        .requires_all_trials = false,
        .min_souls_harvested = 0,
        .min_alliances = 0
    };
    init_ending(&manager->endings[6], ENDING_DIVINE_DESTRUCTION, "Divine Destruction",
                "The gods have seen enough. Your corruption is absolute. Your consciousness "
                "is gone. You are a threat to existence itself. The Seven Architects convene. "
                "The verdict is unanimous: DESTRUCTION. Divine fire consumes you. Your "
                "consciousness is shattered beyond recovery. Your souls are scattered. "
                "You are unmade. This is the bad ending.",
                "The world recovers. Your undead crumble to dust. The souls you bound are freed. "
                "Your name becomes a warning. A cautionary tale. The necromancer who went too far. "
                "Nothing remains but ash and regret.",
                &destruction_reqs);

    return manager;
}

void ending_manager_destroy(EndingManager* manager) {
    if (manager) {
        free(manager);
    }
}

bool ending_manager_lock_path(EndingManager* manager, EndingType type, const char* reason) {
    if (!manager || type <= ENDING_NONE || type > ENDING_DIVINE_DESTRUCTION) {
        return false;
    }

    Ending* ending = &manager->endings[type - 1];
    ending->path_locked = true;
    ending->achievement = ACHIEVEMENT_LOCKED;

    if (reason) {
        strncpy(ending->lock_reason, reason, sizeof(ending->lock_reason) - 1);
        ending->lock_reason[sizeof(ending->lock_reason) - 1] = '\0';
    } else {
        ending->lock_reason[0] = '\0';
    }

    return true;
}

bool ending_manager_is_available(const EndingManager* manager, EndingType type) {
    if (!manager || type <= ENDING_NONE || type > ENDING_DIVINE_DESTRUCTION) {
        return false;
    }

    const Ending* ending = &manager->endings[type - 1];
    return !ending->path_locked && ending->achievement != ACHIEVEMENT_LOCKED;
}

bool ending_manager_check_requirements(EndingManager* manager, EndingType type,
                                       uint8_t corruption, uint8_t consciousness,
                                       const int16_t divine_favor[7],
                                       bool ashbrook_spared, bool did_full_harvest,
                                       float thessara_trust, bool council_verdict_received,
                                       bool void_battle_won, uint8_t trials_completed,
                                       uint32_t total_souls_harvested, uint8_t full_alliances) {
    if (!manager || !divine_favor || type <= ENDING_NONE || type > ENDING_DIVINE_DESTRUCTION) {
        return false;
    }

    const Ending* ending = &manager->endings[type - 1];
    if (ending->path_locked) {
        return false;
    }

    const EndingRequirements* reqs = &ending->requirements;

    /* Check corruption range */
    if (corruption < reqs->min_corruption || corruption > reqs->max_corruption) {
        return false;
    }

    /* Check consciousness */
    if (consciousness < reqs->min_consciousness) {
        return false;
    }

    /* Check divine favor */
    uint8_t gods_with_favor = 0;
    bool has_min_favor = false;
    for (int i = 0; i < 7; i++) {
        if (divine_favor[i] >= reqs->min_divine_favor) {
            has_min_favor = true;
        }
        if (divine_favor[i] > 0) {
            gods_with_favor++;
        }
    }
    if (!has_min_favor || gods_with_favor < reqs->gods_favor_required) {
        return false;
    }

    /* Check event flags */
    if (reqs->requires_ashbrook_spared && !ashbrook_spared) {
        return false;
    }
    if (reqs->forbids_full_harvest && did_full_harvest) {
        return false;
    }
    if (reqs->requires_thessara_trust && thessara_trust < 75.0f) {
        return false;
    }
    if (reqs->requires_council_verdict && !council_verdict_received) {
        return false;
    }
    if (reqs->requires_void_battle && !void_battle_won) {
        return false;
    }

    /* Check special requirements */
    if (reqs->requires_all_trials && trials_completed < 7) {
        return false;
    }
    if (total_souls_harvested < reqs->min_souls_harvested) {
        return false;
    }
    if (full_alliances < reqs->min_alliances) {
        return false;
    }

    return true;
}

bool ending_manager_trigger_ending(EndingManager* manager, EndingType type, uint32_t day) {
    if (!manager || manager->game_over || type <= ENDING_NONE || type > ENDING_DIVINE_DESTRUCTION) {
        return false;
    }

    Ending* ending = &manager->endings[type - 1];
    if (ending->path_locked || ending->achievement == ACHIEVEMENT_LOCKED) {
        return false;
    }

    manager->game_over = true;
    manager->current_ending = type;
    manager->ending_day = day;

    ending->achievement = ACHIEVEMENT_UNLOCKED;
    ending->day_achieved = day;

    return true;
}

const char* ending_type_name(EndingType type) {
    if (type < 0 || type > ENDING_DIVINE_DESTRUCTION) {
        return "Unknown";
    }
    return ENDING_NAMES[type];
}

const Ending* ending_manager_get_ending(const EndingManager* manager, EndingType type) {
    if (!manager || type <= ENDING_NONE || type > ENDING_DIVINE_DESTRUCTION) {
        return NULL;
    }
    return &manager->endings[type - 1];
}

const EndingType* ending_manager_get_available_endings(const EndingManager* manager, size_t* count_out) {
    if (!manager || !count_out) {
        return NULL;
    }

    static EndingType available[7];
    size_t count = 0;

    for (int i = 0; i < 7; i++) {
        if (!manager->endings[i].path_locked && manager->endings[i].achievement != ACHIEVEMENT_LOCKED) {
            available[count++] = manager->endings[i].type;
        }
    }

    *count_out = count;
    return available;
}

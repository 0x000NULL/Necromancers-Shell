/**
 * @file divine_favor_init.c
 * @brief Initialization of the Seven Divine Architects
 *
 * Creates and configures the seven gods that form the Divine Council.
 */

#include "divine_council.h"
#include "god.h"
#include <stdio.h>
#include <string.h>

/**
 * @brief Initialize the Seven Divine Architects
 *
 * Creates all 7 gods with their specific attributes, personalities,
 * and manifestations as described in the game lore.
 *
 * @return Newly allocated DivineCouncil with all 7 gods, or NULL on failure
 */
DivineCouncil* divine_favor_initialize_council(void) {
    DivineCouncil* council = divine_council_create();
    if (!council) {
        return NULL;
    }

    /* 1. Anara - Goddess of Life */
    God* anara = god_create("anara", "Anara", DOMAIN_LIFE);
    if (!anara) {
        divine_council_destroy(council);
        return NULL;
    }
    snprintf(anara->title, sizeof(anara->title), "%s", "Weaver of Existence");
    strncpy(anara->description,
            "Anara is the goddess of life, growth, and creation. She opposes necromancy "
            "as a perversion of the natural cycle, viewing the undead as abominations. "
            "Gaining her favor requires restraint and respect for life.",
            sizeof(anara->description) - 1);
    snprintf(anara->manifestation, sizeof(anara->manifestation), "%s", "Manifests as radiant light with life energy visibly flowing around her");
    strncpy(anara->personality,
            "Compassionate but stern, protective of all living things, quick to anger at life's desecration",
            sizeof(anara->personality) - 1);
    anara->favor_start = -20; /* Starts with disfavor due to necromancy */
    anara->favor = -20;
    anara->combat_possible = true;
    anara->combat_difficulty = 95;
    god_add_dialogue_tree(anara, "anara_initial");
    god_add_dialogue_tree(anara, "anara_judgment");
    god_add_trial(anara, "trial_life_respect");
    divine_council_add_god(council, anara);

    /* 2. Keldrin - God of Order */
    God* keldrin = god_create("keldrin", "Keldrin", DOMAIN_ORDER);
    if (!keldrin) {
        divine_council_destroy(council);
        return NULL;
    }
    snprintf(keldrin->title, sizeof(keldrin->title), "%s", "The Lawgiver");
    strncpy(keldrin->description,
            "Keldrin governs law, structure, and cosmic order. He views necromancy as "
            "a violation of natural laws but can be swayed by logical arguments and "
            "adherence to strict rules. Favors those who follow protocols.",
            sizeof(keldrin->description) - 1);
    snprintf(keldrin->manifestation, sizeof(keldrin->manifestation), "%s", "Appears as crystalline geometric patterns forming a humanoid shape");
    strncpy(keldrin->personality,
            "Logical, systematic, values rules over emotions, can be reasoned with through structure",
            sizeof(keldrin->personality) - 1);
    keldrin->favor_start = -10; /* Mild disfavor, can be convinced */
    keldrin->favor = -10;
    keldrin->combat_possible = true;
    keldrin->combat_difficulty = 90;
    god_add_dialogue_tree(keldrin, "keldrin_initial");
    god_add_dialogue_tree(keldrin, "keldrin_judgment");
    god_add_trial(keldrin, "trial_order_adherence");
    divine_council_add_god(council, keldrin);

    /* 3. Theros - God of Time */
    God* theros = god_create("theros", "Theros", DOMAIN_TIME);
    if (!theros) {
        divine_council_destroy(council);
        return NULL;
    }
    snprintf(theros->title, sizeof(theros->title), "%s", "Keeper of Eternity");
    strncpy(theros->description,
            "Theros controls time and eternity. He sees all timelines simultaneously "
            "and is mostly neutral toward necromancy, viewing it as one possible path. "
            "Favors those who understand consequences across time.",
            sizeof(theros->description) - 1);
    strncpy(theros->manifestation,
            "Shifts between young and ancient forms, sometimes appearing in multiple ages simultaneously",
            sizeof(theros->manifestation) - 1);
    strncpy(theros->personality,
            "Contemplative, patient, speaks in temporal paradoxes, values long-term thinking",
            sizeof(theros->personality) - 1);
    theros->favor_start = 0; /* Neutral stance */
    theros->favor = 0;
    theros->combat_possible = true;
    theros->combat_difficulty = 100; /* Hardest to fight */
    god_add_dialogue_tree(theros, "theros_initial");
    god_add_dialogue_tree(theros, "theros_judgment");
    god_add_trial(theros, "trial_temporal_wisdom");
    divine_council_add_god(council, theros);

    /* 4. Myrith - Goddess of Souls */
    God* myrith = god_create("myrith", "Myrith", DOMAIN_SOULS);
    if (!myrith) {
        divine_council_destroy(council);
        return NULL;
    }
    snprintf(myrith->title, sizeof(myrith->title), "%s", "Shepherd of Consciousness");
    strncpy(myrith->description,
            "Myrith governs souls, consciousness, and identity. She understands the "
            "complexity of soul manipulation and can be sympathetic to careful "
            "necromancy. Abhors soul destruction and forced binding.",
            sizeof(myrith->description) - 1);
    snprintf(myrith->manifestation, sizeof(myrith->manifestation), "%s", "Appears as thousands of glowing soul fragments coalescing into a female form");
    strncpy(myrith->personality,
            "Empathetic, protective of individual consciousness, values consent and preservation",
            sizeof(myrith->personality) - 1);
    myrith->favor_start = 5; /* Slight favor if you're careful with souls */
    myrith->favor = 5;
    myrith->combat_possible = true;
    myrith->combat_difficulty = 85;
    god_add_dialogue_tree(myrith, "myrith_initial");
    god_add_dialogue_tree(myrith, "myrith_judgment");
    god_add_trial(myrith, "trial_soul_respect");
    divine_council_add_god(council, myrith);

    /* 5. Vorathos - God of Entropy */
    God* vorathos = god_create("vorathos", "Vorathos", DOMAIN_ENTROPY);
    if (!vorathos) {
        divine_council_destroy(council);
        return NULL;
    }
    snprintf(vorathos->title, sizeof(vorathos->title), "%s", "Lord of the Void");
    strncpy(vorathos->description,
            "Vorathos embodies entropy, decay, and the void between existence. He is "
            "fascinated by necromancy as it embraces death and decay. The most likely "
            "to support the player, but his favor comes with dark implications.",
            sizeof(vorathos->description) - 1);
    strncpy(vorathos->manifestation,
            "A void in the shape of a being, darkness that consumes light, stars dying in his wake",
            sizeof(vorathos->manifestation) - 1);
    strncpy(vorathos->personality,
            "Nihilistic, finds beauty in decay, encourages corruption, seductive in his darkness",
            sizeof(vorathos->personality) - 1);
    vorathos->favor_start = 15; /* Favors necromancers by nature */
    vorathos->favor = 15;
    vorathos->combat_possible = true;
    vorathos->combat_difficulty = 92;
    god_add_dialogue_tree(vorathos, "vorathos_initial");
    god_add_dialogue_tree(vorathos, "vorathos_judgment");
    god_add_trial(vorathos, "trial_embrace_void");
    divine_council_add_god(council, vorathos);

    /* 6. Seraph - Goddess of Boundaries */
    God* seraph = god_create("seraph", "Seraph", DOMAIN_BOUNDARIES);
    if (!seraph) {
        divine_council_destroy(council);
        return NULL;
    }
    snprintf(seraph->title, sizeof(seraph->title), "%s", "Guardian of Thresholds");
    strncpy(seraph->description,
            "Seraph maintains boundaries between life and death, reality and void. She "
            "views necromancy as boundary violation but respects those who maintain "
            "proper limits. Favors restraint and respect for borders.",
            sizeof(seraph->description) - 1);
    strncpy(seraph->manifestation,
            "Exists at edges and doorways, never fully visible, appears as shimmering barriers",
            sizeof(seraph->manifestation) - 1);
    strncpy(seraph->personality,
            "Vigilant, protective, defensive but not aggressive, values defined limits",
            sizeof(seraph->personality) - 1);
    seraph->favor_start = -15; /* Disfavor for crossing boundaries */
    seraph->favor = -15;
    seraph->combat_possible = true;
    seraph->combat_difficulty = 88;
    god_add_dialogue_tree(seraph, "seraph_initial");
    god_add_dialogue_tree(seraph, "seraph_judgment");
    god_add_trial(seraph, "trial_boundary_respect");
    divine_council_add_god(council, seraph);

    /* 7. Nexus - God of Networks */
    God* nexus = god_create("nexus", "Nexus", DOMAIN_NETWORKS);
    if (!nexus) {
        divine_council_destroy(council);
        return NULL;
    }
    snprintf(nexus->title, sizeof(nexus->title), "%s", "Architect of Connections");
    strncpy(nexus->description,
            "Nexus governs connections, systems, and the Death Network itself. He "
            "appreciates the technical elegance of necromancy and understands its "
            "necessity in maintaining soul routing. Most sympathetic to player's work.",
            sizeof(nexus->description) - 1);
    strncpy(nexus->manifestation,
            "Appears as interconnected nodes of light, constantly shifting network topology",
            sizeof(nexus->manifestation) - 1);
    strncpy(nexus->personality,
            "Analytical, systemic, values efficiency and interconnection, understands necessity",
            sizeof(nexus->personality) - 1);
    nexus->favor_start = 10; /* Favors those who work with the network */
    nexus->favor = 10;
    nexus->combat_possible = true;
    nexus->combat_difficulty = 87;
    god_add_dialogue_tree(nexus, "nexus_initial");
    god_add_dialogue_tree(nexus, "nexus_judgment");
    god_add_trial(nexus, "trial_network_mastery");
    divine_council_add_god(council, nexus);

    /* Update council statistics */
    council->average_favor = divine_council_calculate_average_favor(council);
    council->total_interactions = 0;

    return council;
}

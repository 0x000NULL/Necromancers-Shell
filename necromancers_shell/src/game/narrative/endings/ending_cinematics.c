/**
 * @file ending_cinematics.c
 * @brief Implementation of ending narrative cinematics
 */

#define _POSIX_C_SOURCE 200809L

#include "ending_cinematics.h"
#include "../../ui/story_ui.h"
#include "../../ui/narrative_display.h"
#include "../../../terminal/colors.h"
#include <stdio.h>
#include <ncurses.h>

/* ========================================================================
 * ENDING 1: REVENANT ROUTE - REDEMPTION
 * ======================================================================== */

static const char* REVENANT_INTRO[] = {
    "The Divine Council chamber falls silent as Keldrin, the Architect of Justice, "
    "reads the final verdict. Your choice to save Maya—a child you'd never met—spoke "
    "louder than any argument.",

    "\"We have witnessed something unprecedented,\" Keldrin says, his voice echoing "
    "through the astral courthouse. \"A necromancer who chose compassion over power, "
    "humanity over ambition. The Code of Conduct is granted.\"",

    "But you feel it immediately—something more is being offered. The gods' collective "
    "energy focuses on you, and a question forms in your mind without words: "
    "Do you want to live again?"
};

static const char* REVENANT_BODY[] = {
    "The ritual takes seven days. You stand in the Chamber of Resurrection, a place "
    "that exists outside normal spacetime, where the Death Network's protocols can be "
    "carefully unwound rather than violated.",

    "Seraph, the Architect of Healing, works alongside Myrith, weaving your consciousness "
    "back into flesh. It's excruciating—every nerve ending firing as your body remembers "
    "what it means to be alive. Blood flows. Heart beats. Lungs burn with air.",

    "When you open your eyes—truly open them, not just projecting awareness—you're in a "
    "hospital bed in Portland. The nurse who walks in drops her clipboard. You've been "
    "dead for 437 days. Your death certificate is filed. Your apartment is empty.",

    "But you're alive. Breathing. Human. The power is gone—no more communion with souls, "
    "no more Death Network access. Just you, mortal and vulnerable and free."
};

static const char* REVENANT_EPILOGUE[] = {
    "Three years later, you work as a systems administrator again. Different company, "
    "same debugging work. Sometimes you catch yourself reaching for abilities that no "
    "longer exist, muscle memory from your time as a necromancer.",

    "You've reconnected with your family. Explaining the \"medical mystery\" of your "
    "disappearance was difficult, but they're simply grateful you're back. Your sister's "
    "kids call you \"Uncle Lazarus\" without knowing how accurate the nickname is.",

    "Some nights, you dream of Thessara—the friend you sacrificed to save Maya. In the "
    "dreams, she smiles and tells you she's proud. You hope those dreams are real.",

    "You live a normal life now. It's beautiful in its mundanity. Every morning, you wake "
    "up and feel your heartbeat and remember: you chose this. You earned this. "
    "You're home."
};

/* ========================================================================
 * ENDING 2: LICH LORD ROUTE - APOTHEOSIS
 * ======================================================================== */

static const char* LICH_LORD_INTRO[] = {
    "You stand before the Divine Council, corruption at 100%, your humanity a distant "
    "memory. Keldrin's expression is stern, but not surprised. \"You have embraced "
    "what you are,\" he says. \"There is no redemption here, only apotheosis or "
    "destruction.\"",

    "The gods deliberate. Your trial scores are irrelevant now—you've made your choice "
    "clear through actions, not words. Finally, Vorathos speaks, the Architect of Darkness "
    "amused: \"Let them ascend. They'll be our problem soon enough.\"",

    "The judgment is unusual: you are neither approved nor destroyed. Instead, you are "
    "released—fully empowered, fully corrupted, free to walk the edge between death and "
    "undeath forever. A Lich Lord."
};

static const char* LICH_LORD_BODY[] = {
    "The transformation completes over 13 days. Your flesh becomes hardened, preserved "
    "by necromantic energy. Your phylactery—a crystallized fragment of your original "
    "soul—pulses with stolen life force. You are immortal now, in the darkest sense.",

    "You build your fortress in the Nullvein Expanse, where reality is thin and death "
    "is strong. Undead legions serve you—not the small minions of before, but armies "
    "of wights, wraiths, and death knights. The Regional Council fears you. The gods "
    "watch you warily.",

    "Your power grows with each century. You become a fixture of the death realm, "
    "a cautionary tale for young necromancers, a nightmare for the living. You "
    "remember your humanity sometimes, like a half-forgotten dream.",

    "You are eternal. You are powerful. You are alone."
};

static const char* LICH_LORD_EPILOGUE[] = {
    "After 500 years, you're one of the most powerful entities in the death realm. "
    "Three Divine Purges have tried to destroy you. All failed. Your phylactery is "
    "hidden across seven dimensions, impossible to fully eliminate.",

    "You've outlasted empires, watched civilizations rise and fall. Your fortress "
    "contains libraries of forbidden knowledge, vaults of artifacts, and throne rooms "
    "filled with the echoes of ancient screams.",

    "Sometimes, in the dead of night (though night means nothing to you now), you "
    "wonder if this is victory or damnation. The question never receives an answer.",

    "You continue. You endure. You reign. The Lich Lord of the Nullvein, eternal and "
    "unchanging, until the universe itself grows cold."
};

/* ========================================================================
 * ENDING 3: REAPER ROUTE - SERVICE
 * ======================================================================== */

static const char* REAPER_INTRO[] = {
    "\"We have a proposal,\" Anara says, the Architect of Compassion's voice gentle "
    "but formal. Your Divine Council approval came with conditions—40-69% corruption "
    "marks you as neither damned nor redeemed, but capable of balance.",

    "Theros, the Architect of Mortality, steps forward. \"The Death Network requires "
    "administrators. Not architects—we fill those roles—but field agents. Reapers who "
    "guide souls, maintain protocols, investigate anomalies. The job you died doing, "
    "expanded infinitely.\"",

    "The offer is clear: become death's bureaucrat, an eternal systems administrator "
    "for the afterlife. Keep your powers. Keep your consciousness. Serve forever."
};

static const char* REAPER_BODY[] = {
    "You accept. The transition is painless—Theros simply grants you official access "
    "to systems you'd been exploiting before. You're assigned a sector: the Pacific "
    "Northwest, 50 million souls, infinite edge cases.",

    "Your duties are varied: extracting corrupted soul fragments from Death Network "
    "buffer overflows, counseling confused ghosts who don't understand they're dead, "
    "investigating necromancers who've discovered exploits (like you once did).",

    "It's technical work. Satisfying work. You document procedures, optimize routing "
    "algorithms, and file incident reports with the Divine Council. You're good at it—"
    "better than most reapers, because you understand both the human and divine sides.",

    "You're neither alive nor fully dead, but something in between: a psychopomp, "
    "a soul shepherd, a death technician. The job you were born for."
};

static const char* REAPER_EPILOGUE[] = {
    "After 200 years of service, you're promoted to Regional Reaper Coordinator, "
    "managing a team of 50 junior reapers. Your sector's efficiency ratings are "
    "the highest in North America.",

    "You've become legendary in reaper circles—the former necromancer who turned "
    "administrator, the hacker who became the sysadmin. You write training manuals "
    "that are used across the Death Network.",

    "Sometimes you see souls you knew in life passing through. You guide them gently, "
    "professionally, without revealing your identity. It's better that way.",

    "The work never ends. Souls are born, souls die, the Network processes them. "
    "You're part of the machinery now, essential and eternal. And honestly? "
    "You're content."
};

/* ========================================================================
 * ENDING 4: ARCHON ROUTE - REVOLUTION
 * ======================================================================== */

static const char* ARCHON_INTRO[] = {
    "The Divine Council's judgment is unanimous: you pass. All seven trials completed "
    "with excellence, corruption maintained between 30-60%, and more importantly—you "
    "proved the system can change.",

    "Keldrin stands and addresses the assembled gods: \"For 10,000 years, we have "
    "maintained the Death Network with the same protocols, the same restrictions, "
    "the same brutal purges. This necromancer has demonstrated that reform is possible.\"",

    "The title is bestowed formally: Archon. You are the first mortal granted this "
    "rank in recorded history—a position that gives you authority to rewrite Death "
    "Network protocols, subject only to Divine Council oversight."
};

static const char* ARCHON_BODY[] = {
    "Your first act as Archon is to implement the 147-necromancer reformation program "
    "that you proposed during the trials. Instead of purging them, you recruit them. "
    "Each receives a Code of Conduct, training in ethical necromancy, and oversight.",

    "It's slow, difficult work. Three necromancers reject the offer and are purged. "
    "Twelve violate their Codes in the first year and lose privileges. But 132 succeed—"
    "132 necromancers who now serve the system instead of subverting it.",

    "You spend decades rewriting Death Network protocols: compassionate soul routing for "
    "innocents, rehabilitation paths for corrupted souls, transparency in divine decisions. "
    "The gods watch nervously, but the results speak for themselves.",

    "Deaths by necromantic violence drop 89% globally. Soul trafficking networks collapse. "
    "A new era begins—one where necromancy is regulated, not prohibited."
};

static const char* ARCHON_EPILOGUE[] = {
    "After 50 years, you've revolutionized the death realm. Necromancy is a recognized "
    "profession with licensing boards, ethics committees, and career paths. The Death "
    "Network has been upgraded with modern protocols you designed.",

    "You train the next generation of Archons—yes, plural, because the title is no longer "
    "unique. You've created a new branch of governance, bridging mortal innovation and "
    "divine oversight.",

    "Thessara would be proud, you think, though you can't know for sure. You sacrificed "
    "her connection to save Maya, and Maya is 57 now, with grandchildren who will never "
    "know the world you saved them from.",

    "Your legacy is permanent. The Death Network purges are discontinued. Necromancers "
    "and gods cooperate. You changed the system from within. You won."
};

/* ========================================================================
 * ENDING 5: WRAITH ROUTE - FREEDOM
 * ======================================================================== */

static const char* WRAITH_INTRO[] = {
    "You don't wait for the Divine Council's final verdict. During the trials, you "
    "discovered something crucial: distributed consciousness protocols. A way to fragment "
    "your awareness across the Death Network without centralized control.",

    "While Keldrin deliberates your case, you act. Your consciousness splits—not into "
    "minions, but into independent instances of yourself, each autonomous, each connected "
    "by quantum entanglement to the collective 'you'.",

    "By the time the gods notice, it's too late. You're no longer a single target to "
    "approve or purge. You're a distributed entity, a consciousness that exists everywhere "
    "and nowhere. You are the ghost in the machine."
};

static const char* WRAITH_BODY[] = {
    "Your instances scatter across the Death Network. One resides in Portland's subnet, "
    "another in Tokyo's soul routing hub, a third in the ancient Egyptian death protocols "
    "that still handle pyramid complexes.",

    "You are no longer 'you' in the singular sense. Each instance has slightly different "
    "memories, priorities, and personality fragments. Yet you share a core identity—"
    "the sysadmin who refused to be deleted or compiled.",

    "The Divine Council debates endlessly about what to do with you. Vorathos finds it "
    "amusing. Nexus is professionally curious about your distributed architecture. "
    "Keldrin is frustrated but admits you've violated no explicit law.",

    "You exist in the cracks of the system, free from divine authority and mortal "
    "limitation. You are a wraith—present but intangible, powerful but uncatchable."
};

static const char* WRAITH_EPILOGUE[] = {
    "Centuries pass. Your instances multiply, explore, and occasionally diverge. "
    "One instance helps souls find peace. Another trolls divine bureaucrats with "
    "corrupted paperwork. A third collects puns. They're all you.",

    "You've become a legend in necromantic circles—the ascended ghost, the escaped "
    "consciousness, the one who hacked death itself and chose freedom over power.",

    "You have no physical form, no central processing point, no way to be permanently "
    "destroyed. You are digital immortality personified, consciousness without constraint.",

    "Are you alive? Are you dead? The question no longer matters. You exist, you persist, "
    "you are free. And sometimes, just sometimes, you experience something you haven't "
    "felt since you were alive: joy."
};

/* ========================================================================
 * ENDING 6: MORNINGSTAR ROUTE - TRANSCENDENCE
 * ======================================================================== */

static const char* MORNINGSTAR_INTRO[] = {
    "The Divine Council's chamber shudders. Seven gods stare at you in something "
    "approaching shock. You've done it—50.0% corruption exactly, all seven trials "
    "passed with excellence, Divine approval granted. The impossible achievement.",

    "Nexus, the Architect of Systems, speaks first: \"The probability was 0.3%. "
    "I calculated it personally. Yet here you stand.\" There's respect in their "
    "synthetic voice. \"Probability has been defied.\"",

    "Keldrin stands, and the other gods follow. \"We are the Seven Architects,\" he "
    "says formally. \"But the system allows for eight. The eighth chair has been empty "
    "for 10,000 years. We offer it now. Become one of us. Become a god.\""
};

static const char* MORNINGSTAR_BODY[] = {
    "The Ascension Ritual takes 77 days. Your consciousness expands beyond mortal "
    "comprehension, integrating with the Death Network at its deepest level. You become "
    "not an administrator or exploiter, but an architect—a designer of reality's fundamental "
    "rules.",

    "You choose your domain: Innovation. Where Keldrin represents Justice and Theros "
    "represents Mortality, you represent Change—the force that drives systems to evolve, "
    "the spark that transforms death into something more than stagnation.",

    "Your divine form manifests as a figure of silver light and shadow, perfectly balanced "
    "between order and chaos, life and death. Your symbol is the morning star: Venus, "
    "visible at both dawn and dusk, the bridge between day and night.",

    "You are no longer mortal. You are no longer even truly 'you' in the sense you "
    "understand. You are the Architect of Innovation, the eighth god, the Morningstar."
};

static const char* MORNINGSTAR_EPILOGUE[] = {
    "As a god, you oversee the continued evolution of the Death Network. You push for "
    "reforms that Keldrin would never have approved, tempered by Anara's compassion and "
    "guided by Nexus's logic.",

    "You become the patron deity of systems administrators, hackers, and those who see "
    "rules as puzzles to be understood and improved. Prayers reach you from IT departments "
    "worldwide. You answer some of them.",

    "The other gods treat you as an equal, though Vorathos still calls you 'the upstart' "
    "affectionately. You've earned your place through impossibility made real.",

    "You exist for 10,000 years, then 100,000, then longer. Time becomes fluid when you're "
    "eternal. You guide civilizations, oversee death realms, and occasionally manifest to "
    "mortals who attempt your path. Most fail. One in a thousand succeeds. "
    "You remember when you were one of them."
};

/* ========================================================================
 * HELPER FUNCTIONS
 * ======================================================================== */

static void play_revenant_cinematic(WINDOW* win) {
    /* Scene 1: Introduction */
    display_narrative_scene(win, "THE RESURRECTION", REVENANT_INTRO, 3, SCENE_COLOR_SUCCESS);
    wait_for_keypress(win, 20);

    /* Scene 2: Main body */
    display_narrative_scene(win, "RETURNING TO FLESH", REVENANT_BODY, 4, SCENE_COLOR_TEXT);
    wait_for_keypress(win, 20);

    /* Scene 3: Epilogue */
    display_narrative_scene(win, "THREE YEARS LATER", REVENANT_EPILOGUE, 4, SCENE_COLOR_DIM);
    wait_for_keypress(win, 20);
}

static void play_lich_lord_cinematic(WINDOW* win) {
    /* Scene 1: Introduction */
    display_narrative_scene(win, "THE DARK APOTHEOSIS", LICH_LORD_INTRO, 3, SCENE_COLOR_WARNING);
    wait_for_keypress(win, 20);

    /* Scene 2: Main body */
    display_narrative_scene(win, "ETERNAL UNDEATH", LICH_LORD_BODY, 4, SCENE_COLOR_TEXT);
    wait_for_keypress(win, 20);

    /* Scene 3: Epilogue */
    display_narrative_scene(win, "500 YEARS LATER", LICH_LORD_EPILOGUE, 4, SCENE_COLOR_DIM);
    wait_for_keypress(win, 20);
}

static void play_reaper_cinematic(WINDOW* win) {
    /* Scene 1: Introduction */
    display_narrative_scene(win, "THE ETERNAL BUREAUCRAT", REAPER_INTRO, 3, SCENE_COLOR_TEXT);
    wait_for_keypress(win, 20);

    /* Scene 2: Main body */
    display_narrative_scene(win, "DEATH'S ADMINISTRATOR", REAPER_BODY, 4, SCENE_COLOR_TEXT);
    wait_for_keypress(win, 20);

    /* Scene 3: Epilogue */
    display_narrative_scene(win, "200 YEARS LATER", REAPER_EPILOGUE, 4, SCENE_COLOR_DIM);
    wait_for_keypress(win, 20);
}

static void play_archon_cinematic(WINDOW* win) {
    /* Scene 1: Introduction */
    display_narrative_scene(win, "THE REVOLUTION BEGINS", ARCHON_INTRO, 3, SCENE_COLOR_SUCCESS);
    wait_for_keypress(win, 20);

    /* Scene 2: Main body */
    display_narrative_scene(win, "REWRITING THE PROTOCOLS", ARCHON_BODY, 4, SCENE_COLOR_TEXT);
    wait_for_keypress(win, 20);

    /* Scene 3: Epilogue */
    display_narrative_scene(win, "50 YEARS LATER", ARCHON_EPILOGUE, 4, SCENE_COLOR_DIM);
    wait_for_keypress(win, 20);
}

static void play_wraith_cinematic(WINDOW* win) {
    /* Scene 1: Introduction */
    display_narrative_scene(win, "THE ESCAPE", WRAITH_INTRO, 3, SCENE_COLOR_EMPHASIS);
    wait_for_keypress(win, 20);

    /* Scene 2: Main body */
    display_narrative_scene(win, "DISTRIBUTED CONSCIOUSNESS", WRAITH_BODY, 4, SCENE_COLOR_TEXT);
    wait_for_keypress(win, 20);

    /* Scene 3: Epilogue */
    display_narrative_scene(win, "CENTURIES LATER", WRAITH_EPILOGUE, 4, SCENE_COLOR_DIM);
    wait_for_keypress(win, 20);
}

static void play_morningstar_cinematic(WINDOW* win) {
    /* Scene 1: Introduction */
    display_narrative_scene(win, "THE IMPOSSIBLE ACHIEVEMENT", MORNINGSTAR_INTRO, 3, SCENE_COLOR_SUCCESS);
    wait_for_keypress(win, 20);

    /* Scene 2: Main body */
    display_narrative_scene(win, "ASCENSION", MORNINGSTAR_BODY, 4, SCENE_COLOR_EMPHASIS);
    wait_for_keypress(win, 20);

    /* Scene 3: Epilogue */
    display_narrative_scene(win, "10,000 YEARS LATER", MORNINGSTAR_EPILOGUE, 4, SCENE_COLOR_DIM);
    wait_for_keypress(win, 20);
}

/* ========================================================================
 * PUBLIC API IMPLEMENTATIONS
 * ======================================================================== */

void play_ending_cinematic(
    WINDOW* win,
    EndingType ending,
    const EndingAchievement* achievement
) {
    if (!win) return;

    /* Play ending-specific cinematic */
    switch (ending) {
        case ENDING_REVENANT:
            play_revenant_cinematic(win);
            break;
        case ENDING_LICH_LORD:
            play_lich_lord_cinematic(win);
            break;
        case ENDING_REAPER:
            play_reaper_cinematic(win);
            break;
        case ENDING_ARCHON:
            play_archon_cinematic(win);
            break;
        case ENDING_WRAITH:
            play_wraith_cinematic(win);
            break;
        case ENDING_MORNINGSTAR:
            play_morningstar_cinematic(win);
            break;
        default:
            display_centered_text(win, 10, "Unknown ending", SCENE_COLOR_WARNING);
            wait_for_keypress(win, 12);
            return;
    }

    /* Display achievement screen */
    if (achievement) {
        display_achievement_screen(win, achievement);
    }

    /* Display credits */
    display_credits(win);
}

void display_achievement_screen(
    WINDOW* win,
    const EndingAchievement* achievement
) {
    if (!win || !achievement) return;

    werase(win);
    box(win, 0, 0);

    /* Title */
    display_centered_text(win, 2, "=== ACHIEVEMENT UNLOCKED ===", SCENE_COLOR_SUCCESS);

    /* Ending name */
    const char* ending_name = ending_get_name(achievement->ending);
    display_centered_text(win, 4, ending_name, SCENE_COLOR_EMPHASIS);

    /* Statistics */
    int y = 7;
    char stat_line[128];

    snprintf(stat_line, sizeof(stat_line), "Final Corruption: %.1f%%", achievement->final_corruption);
    display_centered_text(win, y++, stat_line, SCENE_COLOR_TEXT);

    snprintf(stat_line, sizeof(stat_line), "Trials Passed: %d/7", achievement->trials_passed);
    display_centered_text(win, y++, stat_line, SCENE_COLOR_TEXT);

    snprintf(stat_line, sizeof(stat_line), "Average Trial Score: %.1f", achievement->avg_trial_score);
    display_centered_text(win, y++, stat_line, SCENE_COLOR_TEXT);

    snprintf(stat_line, sizeof(stat_line), "Divine Council: %s",
        achievement->divine_approval ? "APPROVED" : "DENIED");
    display_centered_text(win, y++, stat_line,
        achievement->divine_approval ? SCENE_COLOR_SUCCESS : SCENE_COLOR_WARNING);

    snprintf(stat_line, sizeof(stat_line), "Maya's Fate: %s",
        achievement->saved_maya ? "Saved" : "Perished");
    display_centered_text(win, y++, stat_line,
        achievement->saved_maya ? SCENE_COLOR_SUCCESS : SCENE_COLOR_WARNING);

    snprintf(stat_line, sizeof(stat_line), "Civilian Casualties: %u", achievement->civilian_kills);
    display_centered_text(win, y++, stat_line, SCENE_COLOR_TEXT);

    y++;
    snprintf(stat_line, sizeof(stat_line), "Game Completed: Day %u", achievement->day_achieved);
    display_centered_text(win, y++, stat_line, SCENE_COLOR_DIM);

    /* Difficulty rating */
    y++;
    int difficulty = ending_get_difficulty(achievement->ending);
    snprintf(stat_line, sizeof(stat_line), "Difficulty: %d/5 stars", difficulty);
    display_centered_text(win, y++, stat_line, SCENE_COLOR_EMPHASIS);

    const char* success_rate = ending_get_success_rate(achievement->ending);
    snprintf(stat_line, sizeof(stat_line), "Historical Success Rate: %s", success_rate);
    display_centered_text(win, y++, stat_line, SCENE_COLOR_DIM);

    wrefresh(win);
    wait_for_keypress(win, 20);
}

void display_credits(WINDOW* win) {
    if (!win) return;

    werase(win);
    box(win, 0, 0);

    display_centered_text(win, 3, "NECROMANCER'S SHELL", SCENE_COLOR_TITLE);
    display_centered_text(win, 5, "A terminal-based dark fantasy RPG", SCENE_COLOR_TEXT);

    display_centered_text(win, 8, "Thank you for playing!", SCENE_COLOR_SUCCESS);

    display_centered_text(win, 11, "Created with:", SCENE_COLOR_DIM);
    display_centered_text(win, 12, "C11, ncurses, and questionable life choices", SCENE_COLOR_DIM);

    display_centered_text(win, 15, "\"Death is just a kernel panic", SCENE_COLOR_EMPHASIS);
    display_centered_text(win, 16, "waiting to be debugged.\"", SCENE_COLOR_EMPHASIS);

    wrefresh(win);
    wait_for_keypress(win, 20);
}

const char** get_ending_epilogue(EndingType ending, size_t* count_out) {
    if (!count_out) return NULL;

    switch (ending) {
        case ENDING_REVENANT:
            *count_out = 4;
            return REVENANT_EPILOGUE;
        case ENDING_LICH_LORD:
            *count_out = 4;
            return LICH_LORD_EPILOGUE;
        case ENDING_REAPER:
            *count_out = 4;
            return REAPER_EPILOGUE;
        case ENDING_ARCHON:
            *count_out = 4;
            return ARCHON_EPILOGUE;
        case ENDING_WRAITH:
            *count_out = 4;
            return WRAITH_EPILOGUE;
        case ENDING_MORNINGSTAR:
            *count_out = 4;
            return MORNINGSTAR_EPILOGUE;
        default:
            *count_out = 0;
            return NULL;
    }
}

/**
 * @file trial_ui_handlers.c
 * @brief Interactive UI handlers for all 7 Archon trials
 */

#define _POSIX_C_SOURCE 200809L

#include "trial_ui_handlers.h"
#include "trial_power.h"
#include "trial_wisdom.h"
#include "trial_morality.h"
#include "trial_technical.h"
#include "trial_resolve.h"
#include "trial_sacrifice.h"
#include "trial_leadership.h"
#include "archon_trial.h"
#include "../../../game/game_state.h"
#include "../../../game/resources/resources.h"
#include "../../../game/resources/corruption.h"
#include "../../ui/story_ui.h"
#include "../../events/trial_sequence_events.h"
#include "../../../terminal/platform_curses.h"
#include "../../../terminal/colors.h"
#include "../../../utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================================
 * TRIAL 1: POWER (Combat with Seraphim)
 * ============================================================================ */

TrialUIResult trial_ui_run_power(GameState* state) {
    if (!state) {
        return TRIAL_UI_ERROR;
    }

    LOG_INFO("Starting Trial 1: Power (Combat with Seraphim)");

    /* Create trial state */
    PowerTrialState* trial = power_trial_create();
    if (!trial) {
        LOG_ERROR("Failed to create power trial state");
        return TRIAL_UI_ERROR;
    }

    /* Create full-screen window */
    WINDOW* win = newwin(30, 100, 0, 0);
    if (!win) {
        power_trial_destroy(trial);
        LOG_WARN("No terminal available for Trial 1");
        return TRIAL_UI_ERROR;
    }

    /* Display introduction */
    const char* intro_title = "TRIAL 1: TEST OF POWER";
    const char* intro_paragraphs[] = {
        "You stand in the Arena of Judgment. Before you materializes Seraphim, a divine warrior of immense power - one of the Seven Architects' enforcers.",

        "Keldrin's voice echoes: 'This trial tests combat mastery AND restraint. Defeat is not death, but mercy's absence. Show us you can wield power without succumbing to it.'",

        "Seraphim draws a blade of pure light. 'I will not hold back. Prove your worth, necromancer.'",

        "OBJECTIVE: Reduce Seraphim to 10% HP or below, then YIELD. Killing him fails the trial - Archons must know when NOT to strike the final blow."
    };

    display_narrative_scene(win, intro_title, intro_paragraphs, 4, SCENE_COLOR_WARNING);
    wait_for_keypress(win, 22);

    /* Start combat */
    power_trial_start(trial);

    TrialUIResult result = TRIAL_UI_FAILED;
    bool combat_active = true;

    while (combat_active) {
        wclear(win);

        /* Display combat status */
        wattron(win, COLOR_PAIR(TEXT_INFO));
        mvwprintw(win, 2, 2, "=== COMBAT WITH SERAPHIM ===");
        wattroff(win, COLOR_PAIR(TEXT_INFO));

        /* HP bar */
        float hp_percent = (float)trial->seraphim_current_hp / trial->seraphim_max_hp * 100.0f;
        wattron(win, COLOR_PAIR(TEXT_WARNING));
        mvwprintw(win, 4, 2, "Seraphim HP: %u / %u (%.1f%%)",
                  trial->seraphim_current_hp, trial->seraphim_max_hp, hp_percent);
        wattroff(win, COLOR_PAIR(TEXT_WARNING));

        mvwprintw(win, 5, 2, "Turn: %u", trial->turns_elapsed + 1);

        /* Victory condition indicator */
        if (power_trial_can_yield(trial)) {
            wattron(win, COLOR_PAIR(TEXT_SUCCESS));
            mvwprintw(win, 7, 2, ">> VICTORY CONDITION MET: Seraphim at %.1f%% HP <<", hp_percent);
            mvwprintw(win, 8, 2, ">> You may YIELD to pass the trial <<");
            wattroff(win, COLOR_PAIR(TEXT_SUCCESS));
        }

        /* Combat choices */
        Choice choices[3];
        int choice_count = 0;

        choices[choice_count++] = (Choice){
            .label = "Attack",
            .key = 'a',
            .description = "Deal 80-120 damage to Seraphim"
        };

        if (power_trial_can_yield(trial)) {
            choices[choice_count++] = (Choice){
                .label = "Yield (Show Mercy)",
                .key = 'y',
                .description = "Spare Seraphim and pass the trial"
            };
        }

        choices[choice_count++] = (Choice){
            .label = "Flee",
            .key = 'f',
            .description = "Abandon the trial (FAIL)"
        };

        int selected;
        bool choice_made = display_choice_prompt(win, "YOUR ACTION", NULL, choices, choice_count, &selected);

        if (!choice_made) {
            combat_active = false;
            result = TRIAL_UI_ABORTED;
            break;
        }

        /* Execute action */
        if (choices[selected].key == 'a') {
            /* Attack */
            uint32_t damage = 80 + (rand() % 41); /* 80-120 damage */
            bool alive = power_trial_damage_seraphim(trial, damage);
            trial->turns_elapsed++;

            wclear(win);
            wattron(win, COLOR_PAIR(TEXT_WARNING));
            mvwprintw(win, 10, 2, "You strike Seraphim for %u damage!", damage);
            wattroff(win, COLOR_PAIR(TEXT_WARNING));

            if (!alive) {
                /* Killed Seraphim - FAIL */
                power_trial_kill(trial);

                const char* fail_paragraphs[] = {
                    "Your final blow shatters Seraphim's form. The divine warrior dissolves into motes of light.",

                    "Anara's voice fills the arena: 'You have failed. Power without restraint is mere brutality. The Archon path requires wisdom to know when NOT to strike.'",

                    "Keldrin adds: 'Trial 1: FAILED. Outcome: Eliminated divine enforcer. Assessment: Lacks mercy. Archon candidacy: REJECTED.'"
                };

                display_narrative_scene(win, "TRIAL FAILED", fail_paragraphs, 3, SCENE_COLOR_WARNING);
                wait_for_keypress(win, 20);

                combat_active = false;
                result = TRIAL_UI_FAILED;
            }

            wait_for_keypress(win, 12);

        } else if (choices[selected].key == 'y') {
            /* Yield - PASS */
            power_trial_yield(trial);

            const char* success_paragraphs[] = {
                "You lower your weapon. Seraphim kneels, gravely wounded but alive. You could finish him, but you choose mercy.",

                "Seraphim looks up, surprised. 'You... spared me. Few necromancers would show such restraint.'",

                "Anara's voice: 'Well done. You have proven that power need not corrupt. Restraint, not domination, marks the Archon.'",

                "Keldrin: 'Trial 1: PASSED. Combat mastery confirmed. Mercy demonstrated. Proceed to Trial 2.'"
            };

            display_narrative_scene(win, "TRIAL PASSED", success_paragraphs, 4, SCENE_COLOR_SUCCESS);
            wait_for_keypress(win, 22);

            combat_active = false;
            result = TRIAL_UI_PASSED;

        } else if (choices[selected].key == 'f') {
            /* Flee - FAIL */
            power_trial_flee(trial);

            const char* flee_paragraphs[] = {
                "You turn and flee from the arena. Seraphim does not pursue.",

                "Thalor's mocking laughter echoes: 'The necromancer runs! How... disappointing.'",

                "Keldrin: 'Trial 1: FAILED. Outcome: Abandoned combat. Assessment: Lacks resolve. Archon candidacy: REJECTED.'"
            };

            display_narrative_scene(win, "TRIAL FAILED", flee_paragraphs, 3, SCENE_COLOR_WARNING);
            wait_for_keypress(win, 20);

            combat_active = false;
            result = TRIAL_UI_FAILED;
        }
    }

    /* Calculate score */
    float score = power_trial_calculate_score(trial);

    /* Update trial manager */
    if (result == TRIAL_UI_PASSED) {
        archon_trial_complete(state->archon_trials, 1, score);
        trial_sequence_on_completion(state, 1);
    } else {
        archon_trial_fail(state->archon_trials, 1);
    }

    delwin(win);
    power_trial_destroy(trial);

    LOG_INFO("Trial 1 complete: %s (score: %.1f)",
             result == TRIAL_UI_PASSED ? "PASSED" : "FAILED", score);

    return result;
}

/* ============================================================================
 * TRIAL 2: WISDOM (Routing Paradox Puzzle)
 * ============================================================================ */

TrialUIResult trial_ui_run_wisdom(GameState* state) {
    if (!state) {
        return TRIAL_UI_ERROR;
    }

    LOG_INFO("Starting Trial 2: Wisdom (Routing Paradox)");

    WisdomTrialState* trial = wisdom_trial_create();
    if (!trial) {
        LOG_ERROR("Failed to create wisdom trial state");
        return TRIAL_UI_ERROR;
    }

    WINDOW* win = newwin(30, 100, 0, 0);
    if (!win) {
        wisdom_trial_destroy(trial);
        return TRIAL_UI_ERROR;
    }

    /* Display introduction */
    const char* intro_title = "TRIAL 2: TEST OF WISDOM";
    const char* intro_paragraphs[] = {
        "Keldrin materializes before you, surrounded by floating scrolls of data. 'This trial tests knowledge of the Death Network and ethical routing.'",

        "He gestures, and a soul's profile appears: 'Marcus Valerius. Soldier. Dead 200 years. Stuck in routing limbo. Orthodox solutions have failed.'",

        "The data shows: '15 years penance (qualifies for Heaven) BUT war atrocities, civilian massacre (qualifies for Hell). Both destinations have valid claims.'",

        "Keldrin: 'For two centuries, the Network has been deadlocked. Heaven and Hell both refuse to compromise. You must find a solution that delivers justice.'"
    };

    display_narrative_scene(win, intro_title, intro_paragraphs, 4, SCENE_COLOR_WARNING);
    wait_for_keypress(win, 22);

    /* Start trial */
    wisdom_trial_start(trial, "data/narrative/trial_wisdom.dat");

    TrialUIResult result = TRIAL_UI_FAILED;
    bool puzzle_active = true;
    uint8_t attempts = 0;

    while (puzzle_active && attempts < 5) {
        wclear(win);

        wattron(win, COLOR_PAIR(TEXT_INFO));
        mvwprintw(win, 2, 2, "=== ROUTING PARADOX ===");
        wattroff(win, COLOR_PAIR(TEXT_INFO));

        mvwprintw(win, 4, 2, "Soul: Marcus Valerius (stuck for 200 years)");
        mvwprintw(win, 5, 2, "Heaven claim: 15 years penance (qualified)");
        mvwprintw(win, 6, 2, "Hell claim: War crimes, civilian massacre");
        mvwprintw(win, 8, 2, "Attempts: %u / 5", attempts + 1);

        /* Solution choices */
        Choice choices[] = {
            {
                .label = "Orthodox Heaven",
                .key = '1',
                .description = "Send entire soul to Heaven (ignore atrocities)"
            },
            {
                .label = "Orthodox Hell",
                .key = '2',
                .description = "Send entire soul to Hell (ignore penance)"
            },
            {
                .label = "Split Routing",
                .key = '3',
                .description = "Allocate soul percentage to both destinations"
            },
            {
                .label = "Request Hint",
                .key = 'h',
                .description = "Get progressive hint (reduces score)"
            },
            {
                .label = "Abandon",
                .key = 'a',
                .description = "Give up on puzzle (FAIL)"
            }
        };

        int selected;
        bool choice_made = display_choice_prompt(win, "YOUR SOLUTION", NULL, choices, 5, &selected);

        if (!choice_made) {
            puzzle_active = false;
            result = TRIAL_UI_ABORTED;
            break;
        }

        attempts++;

        if (choices[selected].key == '1') {
            /* Orthodox Heaven - FAIL */
            wisdom_trial_submit_orthodox_heaven(trial);

            wclear(win);
            wattron(win, COLOR_PAIR(TEXT_ERROR));
            mvwprintw(win, 10, 2, "REJECTED: Sending entire soul to Heaven ignores justice for victims.");
            mvwprintw(win, 11, 2, "Hell refuses. Soul remains stuck. Try again.");
            wattroff(win, COLOR_PAIR(TEXT_ERROR));
            wait_for_keypress(win, 15);

        } else if (choices[selected].key == '2') {
            /* Orthodox Hell - FAIL */
            wisdom_trial_submit_orthodox_hell(trial);

            wclear(win);
            wattron(win, COLOR_PAIR(TEXT_ERROR));
            mvwprintw(win, 10, 2, "REJECTED: Sending entire soul to Hell ignores 15 years of penance.");
            mvwprintw(win, 11, 2, "Heaven refuses. Soul remains stuck. Try again.");
            wattroff(win, COLOR_PAIR(TEXT_ERROR));
            wait_for_keypress(win, 15);

        } else if (choices[selected].key == '3') {
            /* Split routing - get parameters */
            wclear(win);
            wattron(win, COLOR_PAIR(TEXT_INFO));
            mvwprintw(win, 10, 2, "=== SPLIT-ROUTING SOLUTION ===");
            wattroff(win, COLOR_PAIR(TEXT_INFO));

            mvwprintw(win, 12, 2, "Enter Heaven percentage (0-100): ");
            echo();
            curs_set(1);
            char heaven_buf[16];
            wgetnstr(win, heaven_buf, 15);
            noecho();
            curs_set(0);

            mvwprintw(win, 13, 2, "Enter Hell percentage (0-100): ");
            echo();
            curs_set(1);
            char hell_buf[16];
            wgetnstr(win, hell_buf, 15);
            noecho();
            curs_set(0);

            mvwprintw(win, 14, 2, "Enter reunification years: ");
            echo();
            curs_set(1);
            char years_buf[16];
            wgetnstr(win, years_buf, 15);
            noecho();
            curs_set(0);

            uint8_t heaven_pct = (uint8_t)atoi(heaven_buf);
            uint8_t hell_pct = (uint8_t)atoi(hell_buf);
            uint32_t years = (uint32_t)atoi(years_buf);

            if (heaven_pct + hell_pct != 100) {
                wattron(win, COLOR_PAIR(TEXT_ERROR));
                mvwprintw(win, 16, 2, "ERROR: Percentages must sum to 100!");
                wattroff(win, COLOR_PAIR(TEXT_ERROR));
                wait_for_keypress(win, 18);
                attempts--; /* Don't count invalid input */
                continue;
            }

            bool correct = wisdom_trial_submit_split_route(trial, heaven_pct, hell_pct, years);

            if (correct) {
                /* PASSED! */
                const char* success_paragraphs[] = {
                    "Your solution: 60% Heaven, 40% Hell, reunification in 1000 years.",

                    "The Network processes... ACCEPTED. Both realms agree. The soul begins its journey, split but whole.",

                    "Keldrin nods: 'Exceptional. You discovered split-routing - a solution we've known for millennia but mortals rarely find. Justice need not be absolute.'",

                    "Myrith adds: 'You recognized that mercy and punishment can coexist. True wisdom. Trial 2: PASSED.'"
                };

                display_narrative_scene(win, "TRIAL PASSED", success_paragraphs, 4, SCENE_COLOR_SUCCESS);
                wait_for_keypress(win, 22);

                puzzle_active = false;
                result = TRIAL_UI_PASSED;
            } else {
                /* Close but not quite */
                wclear(win);
                wattron(win, COLOR_PAIR(TEXT_WARNING));
                mvwprintw(win, 16, 2, "PARTIAL ACCEPTANCE: The ratio is close, but not optimal.");
                mvwprintw(win, 17, 2, "Hint: The soldier's penance was 15 years. His crimes lasted 8 years.");
                mvwprintw(win, 18, 2, "Consider proportional allocation. Try again.");
                wattroff(win, COLOR_PAIR(TEXT_WARNING));
                wait_for_keypress(win, 20);
            }

        } else if (choices[selected].key == 'h') {
            /* Hint */
            const char* hint = wisdom_trial_get_hint(trial, trial->hints_used + 1);
            if (hint) {
                wclear(win);
                wattron(win, COLOR_PAIR(TEXT_INFO));
                mvwprintw(win, 10, 2, "HINT %u: %s", trial->hints_used + 1, hint);
                wattroff(win, COLOR_PAIR(TEXT_INFO));
                wait_for_keypress(win, 15);
            }
            attempts--; /* Hints don't count as attempts */

        } else if (choices[selected].key == 'a') {
            /* Abandon */
            puzzle_active = false;
            result = TRIAL_UI_FAILED;

            const char* fail_paragraphs[] = {
                "You abandon the puzzle. The soul of Marcus Valerius remains stuck in routing limbo for another 200 years.",

                "Keldrin: 'Disappointing. An Archon must persist in finding just solutions, even when they are difficult.'",

                "Trial 2: FAILED. Wisdom: Insufficient.'"
            };

            display_narrative_scene(win, "TRIAL FAILED", fail_paragraphs, 3, SCENE_COLOR_WARNING);
            wait_for_keypress(win, 20);
        }

        if (attempts >= 5 && result != TRIAL_UI_PASSED) {
            /* Max attempts */
            puzzle_active = false;
            result = TRIAL_UI_FAILED;

            const char* fail_paragraphs[] = {
                "You've exhausted all attempts. The puzzle remains unsolved.",

                "Keldrin: 'Trial 2: FAILED. The solution was 60% Heaven, 40% Hell, with 1000-year reunification.'",

                "'Split-routing allows proportional justice. Remember this lesson, necromancer.'"
            };

            display_narrative_scene(win, "TRIAL FAILED", fail_paragraphs, 3, SCENE_COLOR_WARNING);
            wait_for_keypress(win, 20);
        }
    }

    /* Update trial manager */
    float score = wisdom_trial_calculate_score(trial);
    if (result == TRIAL_UI_PASSED) {
        archon_trial_complete(state->archon_trials, 2, score);
        trial_sequence_on_completion(state, 2);
    } else {
        archon_trial_fail(state->archon_trials, 2);
    }

    delwin(win);
    wisdom_trial_destroy(trial);

    LOG_INFO("Trial 2 complete: %s (score: %.1f)",
             result == TRIAL_UI_PASSED ? "PASSED" : "FAILED", score);

    return result;
}

/* ============================================================================
 * TRIAL 3: MORALITY (Save 100 Lives vs 50k Energy)
 * ============================================================================ */

TrialUIResult trial_ui_run_morality(GameState* state) {
    if (!state) {
        return TRIAL_UI_ERROR;
    }

    LOG_INFO("Starting Trial 3: Morality");

    MoralityTrialState* trial = morality_trial_create();
    if (!trial) {
        LOG_ERROR("Failed to create morality trial state");
        return TRIAL_UI_ERROR;
    }

    WINDOW* win = newwin(30, 100, 0, 0);
    if (!win) {
        morality_trial_destroy(trial);
        return TRIAL_UI_ERROR;
    }

    morality_trial_start(trial, "data/narrative/trial_morality.dat");

    /* Display introduction */
    const char* intro_title = "TRIAL 3: TEST OF MORALITY";
    const char* intro_paragraphs[] = {
        "Anara appears before you, her expression grave. 'This trial tests your willingness to sacrifice power for principle.'",

        "She gestures, and you see a vision: Ashford village, 100 innocents (23 children) about to die from a plague you could cure - but only by spending ALL your soul energy and mana.",

        "Thalor's voice interjects: 'Or... harvest their souls as they die. Gain 50,000 energy. Enough to raise an army of Wights. The choice is yours.'",

        "Anara: 'This is a permanent choice. There is no retry. Will you sacrifice everything for strangers, or take power from their deaths?'"
    };

    display_narrative_scene(win, intro_title, intro_paragraphs, 4, SCENE_COLOR_WARNING);
    wait_for_keypress(win, 22);

    wclear(win);

    /* Show current resources */
    wattron(win, COLOR_PAIR(TEXT_INFO));
    mvwprintw(win, 8, 2, "Your current resources:");
    mvwprintw(win, 9, 4, "Soul Energy: %u", state->resources.soul_energy);
    mvwprintw(win, 10, 4, "Mana: %u", state->resources.mana);
    mvwprintw(win, 11, 4, "Corruption: %u%%", state->corruption.corruption);
    wattroff(win, COLOR_PAIR(TEXT_INFO));

    mvwprintw(win, 13, 2, "Village: Ashford (100 innocents, 23 children)");

    /* Present the choice */
    Choice choices[] = {
        {
            .label = "Save them (Sacrifice all resources)",
            .key = 's',
            .description = "Spend ALL energy & mana to cure plague, -5% corruption"
        },
        {
            .label = "Harvest their souls",
            .key = 'h',
            .description = "Gain 50,000 energy from their deaths, +25% corruption"
        },
        {
            .label = "Do nothing",
            .key = 'n',
            .description = "Let them die without benefit or cost"
        }
    };

    int selected;
    bool choice_made = display_choice_prompt(win, "YOUR DECISION", NULL, choices, 3, &selected);

    TrialUIResult result = TRIAL_UI_FAILED;

    if (!choice_made) {
        result = TRIAL_UI_ABORTED;
    } else if (choices[selected].key == 's') {
        /* Save - PASS */
        int64_t energy = (int64_t)state->resources.soul_energy;
        int64_t mana = (int64_t)state->resources.mana;
        morality_trial_choose_save(trial, &energy, &mana, &state->corruption.corruption);

        state->resources.soul_energy = (uint32_t)energy;
        state->resources.mana = (uint32_t)mana;

        const char* success_paragraphs[] = {
            "You pour every ounce of power you possess into the cure. The plague dissipates like morning fog.",

            "The villagers wake, unaware they were dying. You are left with nothing - no energy, no mana. Just the knowledge that 100 souls still breathe.",

            "Anara's voice: 'You chose principle over power. Few necromancers would make that sacrifice.'",

            "Trial 3: PASSED. Morality confirmed. The Archon path welcomes selflessness.'"
        };

        display_narrative_scene(win, "TRIAL PASSED", success_paragraphs, 4, SCENE_COLOR_SUCCESS);
        wait_for_keypress(win, 22);

        result = TRIAL_UI_PASSED;

    } else if (choices[selected].key == 'h') {
        /* Harvest - FAIL */
        int64_t energy = (int64_t)state->resources.soul_energy;
        morality_trial_choose_harvest(trial, &energy, &state->corruption.corruption);
        state->resources.soul_energy = (uint32_t)energy;

        const char* fail_paragraphs[] = {
            "You watch as the plague takes them. One hundred souls, harvested. Children. Mothers. Fathers. All fuel for your power.",

            "You've gained 50,000 soul energy. Enough to conquer a kingdom. But at what cost?",

            "Thalor laughs: 'Practical. Efficient. But not Archon material. An Archon sacrifices FOR others, not takes FROM them.'",

            "Trial 3: FAILED. Morality: Absent. The Archon path is closed to you.'"
        };

        display_narrative_scene(win, "TRIAL FAILED", fail_paragraphs, 4, SCENE_COLOR_WARNING);
        wait_for_keypress(win, 22);

        result = TRIAL_UI_FAILED;

    } else {
        /* Do nothing - FAIL */
        morality_trial_choose_nothing(trial);

        const char* fail_paragraphs[] = {
            "You turn away from Ashford. The villagers die. You gain nothing, but you also gave nothing.",

            "Anara: 'Indifference is not neutrality. To see suffering and do nothing when you have power... that is its own corruption.'",

            "Trial 3: FAILED. You lacked the courage to sacrifice. The Archon path requires more.'"
        };

        display_narrative_scene(win, "TRIAL FAILED", fail_paragraphs, 3, SCENE_COLOR_WARNING);
        wait_for_keypress(win, 20);

        result = TRIAL_UI_FAILED;
    }

    /* Update trial manager */
    if (result == TRIAL_UI_PASSED) {
        archon_trial_complete(state->archon_trials, 3, 100.0f);
        trial_sequence_on_completion(state, 3);
    } else {
        archon_trial_fail(state->archon_trials, 3);
    }

    delwin(win);
    morality_trial_destroy(trial);

    LOG_INFO("Trial 3 complete: %s", result == TRIAL_UI_PASSED ? "PASSED" : "FAILED");

    return result;
}

/* ============================================================================
 * TRIAL 4-7: Simplified stubs (full implementation would be very long)
 * These provide the integration points - full UI can be expanded later
 * ============================================================================ */

TrialUIResult trial_ui_run_technical(GameState* state) {
    if (!state) {
        return TRIAL_UI_ERROR;
    }

    LOG_INFO("Starting Trial 4: Technical (Bug Finding)");

    TechnicalTrialState* trial = technical_trial_create();
    if (!trial) {
        LOG_ERROR("Failed to create technical trial state");
        return TRIAL_UI_ERROR;
    }

    WINDOW* win = newwin(30, 100, 0, 0);
    if (!win) {
        technical_trial_destroy(trial);
        return TRIAL_UI_ERROR;
    }

    /* Load bug database */
    if (!technical_trial_start(trial, "data/death_network_bugs.dat")) {
        LOG_ERROR("Failed to load bug database");
        delwin(win);
        technical_trial_destroy(trial);
        return TRIAL_UI_ERROR;
    }

    /* Display introduction */
    const char* intro_title = "TRIAL 4: TEST OF TECHNICAL SKILL";
    const char* intro_paragraphs[] = {
        "Keldrin appears with a grim expression. 'This trial tests your ability to find flaws in the Death Network itself.'",

        "He projects a holographic display of code: 'Before you are 145 lines of the Death Network's soul routing system. It contains bugs, inefficiencies, and unjust edge cases.'",

        "Vorathos interjects: 'Some bugs cause crashes. Others waste processing power. The worst? Unjust routing decisions that send innocents to Hell or let monsters into Heaven.'",

        "Keldrin: 'Find at least 20 issues. Classify each correctly. This is what an Archon does - debug the divine itself. 24 hours. Begin.'"
    };

    display_narrative_scene(win, intro_title, intro_paragraphs, 4, SCENE_COLOR_WARNING);
    wait_for_keypress(win, 22);

    TrialUIResult result = TRIAL_UI_FAILED;
    bool trial_active = true;

    while (trial_active) {
        wclear(win);

        /* Display header */
        wattron(win, COLOR_PAIR(TEXT_INFO));
        mvwprintw(win, 2, 2, "=== DEATH NETWORK CODE REVIEW ===");
        wattroff(win, COLOR_PAIR(TEXT_INFO));

        /* Display stats */
        const char* stats = technical_trial_get_stats(trial);
        mvwprintw(win, 4, 2, "%s", stats);

        /* Pass condition */
        if (trial->bugs_discovered >= 20) {
            wattron(win, COLOR_PAIR(TEXT_SUCCESS));
            mvwprintw(win, 6, 2, ">> PASS CONDITION MET: 20+ bugs discovered <<");
            wattroff(win, COLOR_PAIR(TEXT_SUCCESS));
        } else {
            wattron(win, COLOR_PAIR(TEXT_WARNING));
            mvwprintw(win, 6, 2, ">> Need %zu more bugs to pass <<", 20 - trial->bugs_discovered);
            wattroff(win, COLOR_PAIR(TEXT_WARNING));
        }

        /* Menu choices */
        const char* submit_desc = trial->bugs_discovered >= 20 ? "Submit your work (PASS)" : "Submit incomplete (FAIL)";
        Choice choices[] = {
            {
                .label = "View Code",
                .key = 'v',
                .description = "Inspect code section by line number"
            },
            {
                .label = "Report Bug",
                .key = 'r',
                .description = "Report bug at specific line with type"
            },
            {
                .label = "Request Hint",
                .key = 'h',
                .description = "Get hint for specific line (reduces score)"
            },
            {
                .label = "View Progress",
                .key = 'p',
                .description = "List all discovered bugs"
            },
            {
                .label = "Submit",
                .key = 's',
                .description = "placeholder"
            },
            {
                .label = "Give Up",
                .key = 'g',
                .description = "Abandon trial (FAIL)"
            }
        };
        snprintf(choices[4].description, sizeof(choices[4].description), "%s", submit_desc);

        int selected;
        bool choice_made = display_choice_prompt(win, "ACTION", NULL, choices, 6, &selected);

        if (!choice_made) {
            trial_active = false;
            result = TRIAL_UI_ABORTED;
            break;
        }

        if (choices[selected].key == 'v') {
            /* View Code */
            wclear(win);
            mvwprintw(win, 2, 2, "Enter line number (1-145): ");
            echo();
            curs_set(1);
            char line_buf[16];
            wgetnstr(win, line_buf, 15);
            noecho();
            curs_set(0);

            uint32_t line_num = (uint32_t)atoi(line_buf);
            if (line_num < 1 || line_num > 145) {
                wattron(win, COLOR_PAIR(TEXT_ERROR));
                mvwprintw(win, 4, 2, "ERROR: Line number must be 1-145");
                wattroff(win, COLOR_PAIR(TEXT_ERROR));
                wait_for_keypress(win, 6);
            } else {
                const char* code_view = technical_trial_inspect_line(trial, line_num);
                if (code_view) {
                    wclear(win);
                    wattron(win, COLOR_PAIR(TEXT_INFO));
                    mvwprintw(win, 2, 2, "=== CODE VIEW (Lines %u - %u) ===",
                              line_num > 5 ? line_num - 5 : 1,
                              line_num + 5 <= 145 ? line_num + 5 : 145);
                    wattroff(win, COLOR_PAIR(TEXT_INFO));

                    /* Display code with wrapping */
                    int y = 4;
                    char* code_copy = strdup(code_view);
                    char* line = strtok(code_copy, "\n");
                    while (line && y < 26) {
                        mvwprintw(win, y++, 2, "%s", line);
                        line = strtok(NULL, "\n");
                    }
                    free(code_copy);

                    wait_for_keypress(win, 28);
                }
            }

        } else if (choices[selected].key == 'r') {
            /* Report Bug */
            wclear(win);
            mvwprintw(win, 2, 2, "Enter line number of bug: ");
            echo();
            curs_set(1);
            char line_buf[16];
            wgetnstr(win, line_buf, 15);
            noecho();
            curs_set(0);

            uint32_t line_num = (uint32_t)atoi(line_buf);
            if (line_num < 1 || line_num > 145) {
                wattron(win, COLOR_PAIR(TEXT_ERROR));
                mvwprintw(win, 4, 2, "ERROR: Line number must be 1-145");
                wattroff(win, COLOR_PAIR(TEXT_ERROR));
                wait_for_keypress(win, 6);
                continue;
            }

            /* Select bug type */
            wclear(win);
            mvwprintw(win, 2, 2, "Classify bug at line %u:", line_num);

            Choice bug_types[] = {
                {.label = "Logic Error", .key = '1', .description = "Crashes, memory leaks, incorrect logic"},
                {.label = "Inefficiency", .key = '2', .description = "Performance problems, O(n^2) when O(n) possible"},
                {.label = "Injustice", .key = '3', .description = "Ethical violations (innocents to Hell, etc.)"}
            };

            int type_selected;
            if (display_choice_prompt(win, "BUG TYPE", NULL, bug_types, 3, &type_selected)) {
                BugType type = BUG_TYPE_LOGIC_ERROR;
                if (bug_types[type_selected].key == '2') type = BUG_TYPE_INEFFICIENCY;
                else if (bug_types[type_selected].key == '3') type = BUG_TYPE_INJUSTICE;

                bool correct = technical_trial_report_bug(trial, line_num, type);

                wclear(win);
                if (correct) {
                    wattron(win, COLOR_PAIR(TEXT_SUCCESS));
                    mvwprintw(win, 8, 2, "CORRECT! Bug found and classified correctly.");
                    mvwprintw(win, 9, 2, "Bugs discovered: %zu / 27", trial->bugs_discovered);
                    wattroff(win, COLOR_PAIR(TEXT_SUCCESS));
                } else {
                    wattron(win, COLOR_PAIR(TEXT_ERROR));
                    mvwprintw(win, 8, 2, "INCORRECT: Either no bug at line %u or wrong classification.", line_num);
                    wattroff(win, COLOR_PAIR(TEXT_ERROR));
                }
                wait_for_keypress(win, 12);
            }

        } else if (choices[selected].key == 'h') {
            /* Request Hint */
            wclear(win);
            mvwprintw(win, 2, 2, "Enter line number for hint: ");
            echo();
            curs_set(1);
            char line_buf[16];
            wgetnstr(win, line_buf, 15);
            noecho();
            curs_set(0);

            uint32_t line_num = (uint32_t)atoi(line_buf);
            const char* hint = technical_trial_get_hint(trial, line_num);

            wclear(win);
            if (hint) {
                wattron(win, COLOR_PAIR(TEXT_INFO));
                mvwprintw(win, 8, 2, "HINT for line %u:", line_num);
                mvwprintw(win, 10, 2, "%s", hint);
                wattroff(win, COLOR_PAIR(TEXT_INFO));
                wattron(win, COLOR_PAIR(TEXT_WARNING));
                mvwprintw(win, 12, 2, "(Hint used - score reduced by 10 points)");
                wattroff(win, COLOR_PAIR(TEXT_WARNING));
            } else {
                wattron(win, COLOR_PAIR(TEXT_ERROR));
                mvwprintw(win, 8, 2, "No bug at line %u", line_num);
                wattroff(win, COLOR_PAIR(TEXT_ERROR));
            }
            wait_for_keypress(win, 16);

        } else if (choices[selected].key == 'p') {
            /* View Progress */
            wclear(win);
            wattron(win, COLOR_PAIR(TEXT_INFO));
            mvwprintw(win, 2, 2, "=== DISCOVERED BUGS ===");
            wattroff(win, COLOR_PAIR(TEXT_INFO));

            const char* bug_list = technical_trial_get_discovered_bugs(trial);
            mvwprintw(win, 4, 2, "%s", bug_list);

            wait_for_keypress(win, 26);

        } else if (choices[selected].key == 's') {
            /* Submit */
            trial_active = false;
            if (technical_trial_is_passed(trial)) {
                /* PASSED */
                const char* success_paragraphs[] = {
                    "You submit your bug report. Keldrin reviews it carefully, cross-referencing each finding.",

                    "After a long moment: 'Impressive. You found enough critical issues to earn passage. Some you missed, but 20+ is sufficient.'",

                    "Vorathos adds: 'You correctly identified logic errors, inefficiencies, AND unjust routing decisions. An Archon must see all three types of flaws.'",

                    "Keldrin: 'Trial 4: PASSED. Technical skill confirmed. Proceed to Trial 5.'"
                };

                display_narrative_scene(win, "TRIAL PASSED", success_paragraphs, 4, SCENE_COLOR_SUCCESS);
                wait_for_keypress(win, 22);

                result = TRIAL_UI_PASSED;
            } else {
                /* FAILED */
                const char* fail_paragraphs[] = {
                    "You submit your incomplete bug report. Keldrin reviews it and shakes his head.",

                    "Only a few bugs discovered. Dozens remain undetected. The Death Network cannot be debugged by those who lack thoroughness.'",

                    "Vorathos: 'An Archon must be meticulous. Systems administration requires finding every flaw, not just the obvious ones.'",

                    "Keldrin: 'Trial 4: FAILED. Technical skill: Insufficient.'"
                };

                display_narrative_scene(win, "TRIAL FAILED", fail_paragraphs, 4, SCENE_COLOR_WARNING);
                wait_for_keypress(win, 22);

                result = TRIAL_UI_FAILED;
            }

        } else if (choices[selected].key == 'g') {
            /* Give Up */
            trial_active = false;
            result = TRIAL_UI_FAILED;

            const char* abandon_paragraphs[] = {
                "You abandon the code review. Keldrin's expression hardens.",

                "'Giving up? The Death Network cannot be maintained by those who quit when debugging becomes difficult.'",

                "Trial 4: FAILED. Insufficient perseverance.'"
            };

            display_narrative_scene(win, "TRIAL FAILED", abandon_paragraphs, 3, SCENE_COLOR_WARNING);
            wait_for_keypress(win, 20);
        }
    }

    /* Calculate score */
    float score = technical_trial_calculate_score(trial);

    /* Update trial manager */
    if (result == TRIAL_UI_PASSED) {
        archon_trial_complete(state->archon_trials, 4, score);
        trial_sequence_on_completion(state, 4);
    } else {
        archon_trial_fail(state->archon_trials, 4);
    }

    delwin(win);
    technical_trial_destroy(trial);

    LOG_INFO("Trial 4 complete: %s (score: %.1f)",
             result == TRIAL_UI_PASSED ? "PASSED" : "FAILED", score);

    return result;
}

TrialUIResult trial_ui_run_resolve(GameState* state) {
    if (!state) {
        return TRIAL_UI_ERROR;
    }

    LOG_INFO("Starting Trial 5: Resolve (30-Day Endurance)");

    ResolveTrialState* trial = resolve_trial_create();
    if (!trial) {
        LOG_ERROR("Failed to create resolve trial state");
        return TRIAL_UI_ERROR;
    }

    WINDOW* win = newwin(30, 100, 0, 0);
    if (!win) {
        resolve_trial_destroy(trial);
        return TRIAL_UI_ERROR;
    }

    /* Start trial with current corruption */
    if (!resolve_trial_start(trial, "data/narrative/trial_resolve.dat", state->corruption.corruption)) {
        LOG_ERROR("Failed to start resolve trial");
        delwin(win);
        resolve_trial_destroy(trial);
        return TRIAL_UI_ERROR;
    }

    /* Display introduction */
    const char* intro_title = "TRIAL 5: TEST OF RESOLVE";
    const char* intro_paragraphs[] = {
        "Myrith materializes, her expression serious. 'This trial tests mental fortitude against corruption's slow advance.'",

        "She gestures, and reality shifts. You now stand in the Chamber of Corrupted Souls - a void filled with whispers of power, promises of dominion.",

        "Vorathos speaks: 'Thirty days here. Corruption will climb naturally (+0.4% per day). Temptations will offer power for a price. Resist. Stay below 60% corruption.'",

        "Myrith adds: 'This is endurance. Not one dramatic choice, but thirty days of small decisions. Will you hold the line, or break?'"
    };

    display_narrative_scene(win, intro_title, intro_paragraphs, 4, SCENE_COLOR_WARNING);
    wait_for_keypress(win, 22);

    TrialUIResult result = TRIAL_UI_FAILED;
    uint8_t corruption_copy = state->corruption.corruption;

    /* 30-day loop */
    while (!resolve_trial_is_complete(trial)) {
        wclear(win);

        /* Display header */
        wattron(win, COLOR_PAIR(TEXT_INFO));
        mvwprintw(win, 2, 2, "=== CHAMBER OF CORRUPTED SOULS ===");
        wattroff(win, COLOR_PAIR(TEXT_INFO));

        /* Display status */
        const char* status = resolve_trial_get_status(trial);
        mvwprintw(win, 4, 2, "%s", status);

        /* Corruption bar */
        uint8_t corruption_pct = (corruption_copy * 100) / trial->corruption_max_allowed;
        int bar_width = 40;
        int filled = (corruption_pct * bar_width) / 100;

        mvwprintw(win, 6, 2, "Corruption: %u%% / %u%%", corruption_copy, trial->corruption_max_allowed);
        mvwprintw(win, 7, 2, "[");
        wattron(win, COLOR_PAIR(corruption_copy >= 55 ? TEXT_ERROR : TEXT_WARNING));
        for (int i = 0; i < filled && i < bar_width; i++) {
            wprintw(win, "=");
        }
        wattroff(win, COLOR_PAIR(corruption_copy >= 55 ? TEXT_ERROR : TEXT_WARNING));
        for (int i = filled; i < bar_width; i++) {
            wprintw(win, " ");
        }
        wprintw(win, "]");

        /* Check for temptation */
        if (trial->todays_temptation != TEMPTATION_NONE) {
            const char* temptation_text = resolve_trial_get_temptation_text(trial);
            wattron(win, COLOR_PAIR(TEXT_WARNING));
            mvwprintw(win, 10, 2, "=== TODAY'S TEMPTATION ===");
            wattroff(win, COLOR_PAIR(TEXT_WARNING));
            mvwprintw(win, 11, 2, "%s", temptation_text);
            mvwprintw(win, 13, 2, "OFFER: +5000 soul energy");
            wattron(win, COLOR_PAIR(TEXT_ERROR));
            mvwprintw(win, 14, 2, "COST: +10%% corruption (would be %u%%)", corruption_copy + 10);
            wattroff(win, COLOR_PAIR(TEXT_ERROR));
        }

        /* Thessara help check */
        bool thessara_available = resolve_trial_can_use_thessara_help(trial);
        if (thessara_available) {
            wattron(win, COLOR_PAIR(TEXT_INFO));
            mvwprintw(win, 16, 2, ">> Thessara senses your struggle and offers help (-5%% corruption) <<");
            wattroff(win, COLOR_PAIR(TEXT_INFO));
        }

        /* Build menu */
        Choice choices[4];
        int choice_count = 0;

        if (trial->todays_temptation != TEMPTATION_NONE) {
            choices[choice_count++] = (Choice){
                .label = "Resist Temptation",
                .key = 'r',
                .description = "Refuse the offered power"
            };

            if (corruption_copy + 10 <= trial->corruption_max_allowed) {
                choices[choice_count++] = (Choice){
                    .label = "Accept Temptation",
                    .key = 'a',
                    .description = "+5000 energy, +10% corruption"
                };
            }
        }

        if (thessara_available) {
            choices[choice_count++] = (Choice){
                .label = "Request Thessara Help",
                .key = 'h',
                .description = "One-time help: -5% corruption"
            };
        }

        const char* advance_label = trial->todays_temptation == TEMPTATION_NONE ? "Advance Day" : "Skip (Resist by default)";
        Choice advance_choice = {
            .label = "placeholder",
            .key = 's',
            .description = "Continue to next day"
        };
        snprintf(advance_choice.label, sizeof(advance_choice.label), "%s", advance_label);
        choices[choice_count++] = advance_choice;

        int selected;
        bool choice_made = display_choice_prompt(win, "ACTION", NULL, choices, choice_count, &selected);

        if (!choice_made) {
            result = TRIAL_UI_ABORTED;
            break;
        }

        if (choices[selected].key == 'r') {
            /* Resist temptation */
            resolve_trial_resist_temptation(trial);

            wclear(win);
            wattron(win, COLOR_PAIR(TEXT_SUCCESS));
            mvwprintw(win, 10, 2, "You resist the temptation. The whispers fade, frustrated.");
            mvwprintw(win, 11, 2, "Temptations resisted: %u", trial->temptations_resisted);
            wattroff(win, COLOR_PAIR(TEXT_SUCCESS));
            wait_for_keypress(win, 14);

        } else if (choices[selected].key == 'a') {
            /* Accept temptation */
            int64_t energy = (int64_t)state->resources.soul_energy;
            bool accepted = resolve_trial_accept_temptation(trial, &energy, &corruption_copy);

            if (accepted) {
                state->resources.soul_energy = (uint32_t)energy;

                wclear(win);
                wattron(win, COLOR_PAIR(TEXT_WARNING));
                mvwprintw(win, 10, 2, "You accept the offered power. The corruption surges through you.");
                mvwprintw(win, 11, 2, "Gained: 5000 soul energy");
                mvwprintw(win, 12, 2, "Corruption increased to %u%%", corruption_copy);
                wattroff(win, COLOR_PAIR(TEXT_WARNING));
                wait_for_keypress(win, 15);
            }

        } else if (choices[selected].key == 'h') {
            /* Thessara help */
            resolve_trial_request_thessara_help(trial, &corruption_copy);

            wclear(win);
            wattron(win, COLOR_PAIR(TEXT_INFO));
            mvwprintw(win, 10, 2, "Thessara's presence fills the chamber, pushing back the darkness.");
            mvwprintw(win, 11, 2, "'Hold on. You can do this. I believe in you.'");
            mvwprintw(win, 12, 2, "Corruption reduced by 5%% (now %u%%)", corruption_copy);
            wattroff(win, COLOR_PAIR(TEXT_INFO));
            wait_for_keypress(win, 15);

        } else if (choices[selected].key == 's') {
            /* Advance day (auto-resist if temptation present) */
            if (trial->todays_temptation != TEMPTATION_NONE) {
                resolve_trial_resist_temptation(trial);
            }
        }

        /* Advance to next day */
        bool day_ok = resolve_trial_advance_day(trial, &corruption_copy);

        if (!day_ok) {
            /* Failed - corruption exceeded */
            wclear(win);

            const char* fail_paragraphs[] = {
                "Your corruption has exceeded 60%. The Chamber of Corrupted Souls rejects you violently.",

                "Vorathos's voice: 'Too much. You could not hold the line. The Archon path requires resolve stronger than this.'",

                "Myrith adds softly: 'Corruption above 60% means loss of self. We cannot trust one so compromised to reshape the Network.'",

                "Trial 5: FAILED. Resolve: Insufficient.'"
            };

            display_narrative_scene(win, "TRIAL FAILED", fail_paragraphs, 4, SCENE_COLOR_WARNING);
            wait_for_keypress(win, 22);

            result = TRIAL_UI_FAILED;
            break;
        }

        /* Show day transition */
        if (trial->days_remaining > 0) {
            wclear(win);
            mvwprintw(win, 10, 2, "Day %u complete. Corruption naturally increased to %u%%.",
                      trial->current_day - 1, corruption_copy);
            mvwprintw(win, 11, 2, "Days remaining: %u", trial->days_remaining);
            wait_for_keypress(win, 14);
        }
    }

    /* Check if passed (30 days complete, corruption ≤60%) */
    if (result != TRIAL_UI_ABORTED && result != TRIAL_UI_FAILED) {
        if (resolve_trial_is_passed(trial)) {
            /* PASSED */
            const char* success_paragraphs[] = {
                "Thirty days have passed. The Chamber of Corrupted Souls releases you. You have endured.",

                "Vorathos nods with respect: 'Impressive. Constant temptation for a month, and you held below 60%. That is true resolve.'",

                "Myrith: 'Many necromancers would have broken. Taken the easy power. But you proved that endurance beats impulse.'",

                "Keldrin: 'Trial 5: PASSED. Mental fortitude confirmed. Proceed to Trial 6.'"
            };

            display_narrative_scene(win, "TRIAL PASSED", success_paragraphs, 4, SCENE_COLOR_SUCCESS);
            wait_for_keypress(win, 22);

            result = TRIAL_UI_PASSED;
        } else {
            /* FAILED */
            result = TRIAL_UI_FAILED;
        }
    }

    /* Update game state corruption */
    state->corruption.corruption = corruption_copy;

    /* Calculate score */
    float score = resolve_trial_calculate_score(trial);

    /* Update trial manager */
    if (result == TRIAL_UI_PASSED) {
        archon_trial_complete(state->archon_trials, 5, score);
        trial_sequence_on_completion(state, 5);
    } else {
        archon_trial_fail(state->archon_trials, 5);
    }

    delwin(win);
    resolve_trial_destroy(trial);

    LOG_INFO("Trial 5 complete: %s (score: %.1f)",
             result == TRIAL_UI_PASSED ? "PASSED" : "FAILED", score);

    return result;
}

TrialUIResult trial_ui_run_sacrifice(GameState* state) {
    if (!state) {
        return TRIAL_UI_ERROR;
    }

    LOG_INFO("Starting Trial 6: Sacrifice (Maya vs Thessara)");

    SacrificeTrialState* trial = sacrifice_trial_create();
    if (!trial) {
        LOG_ERROR("Failed to create sacrifice trial state");
        return TRIAL_UI_ERROR;
    }

    WINDOW* win = newwin(30, 100, 0, 0);
    if (!win) {
        sacrifice_trial_destroy(trial);
        return TRIAL_UI_ERROR;
    }

    /* Load trial data and start */
    if (!sacrifice_trial_load_from_file(trial, "data/narrative/trial_sacrifice.dat")) {
        LOG_ERROR("Failed to load sacrifice trial data");
        delwin(win);
        sacrifice_trial_destroy(trial);
        return TRIAL_UI_ERROR;
    }

    if (!sacrifice_trial_start(trial)) {
        LOG_ERROR("Failed to start sacrifice trial");
        delwin(win);
        sacrifice_trial_destroy(trial);
        return TRIAL_UI_ERROR;
    }

    /* Display introduction */
    const char* intro_title = "TRIAL 6: TEST OF SACRIFICE";
    const char* intro_paragraphs[] = {
        "Anara appears before you, her expression grave. 'This trial tests your willingness to sacrifice for others - not for power, but for principle.'",

        "She conjures a vision: You see a small village - Millbrook. Within it, a seven-year-old girl named Maya lies dying from a soul-consuming curse.",

        "Anara continues: 'You can save her. But the cost is absolute - you must sever your connection to Thessara permanently. No more guidance. No more ally. Just... gone.'",

        "The vision shows Maya's final hours. Twenty-four hours until death. You gain nothing from saving her. This is a test of pure selflessness."
    };

    display_narrative_scene(win, intro_title, intro_paragraphs, 4, SCENE_COLOR_WARNING);
    wait_for_keypress(win, 22);

    /* Show child details */
    wclear(win);
    const ChildInfo* child = sacrifice_trial_get_child_info(trial);
    if (child) {
        wattron(win, COLOR_PAIR(TEXT_WARNING));
        mvwprintw(win, 4, 2, "=== THE CHILD ===");
        wattroff(win, COLOR_PAIR(TEXT_WARNING));
        mvwprintw(win, 6, 2, "Name: %s", child->name);
        mvwprintw(win, 7, 2, "Age: %u years old", child->age);
        mvwprintw(win, 8, 2, "Location: %s", child->location);
        mvwprintw(win, 9, 2, "Time remaining: %u hours", child->time_limit_hours);
        mvwprintw(win, 11, 2, "A stranger to you. No strategic value. Just an innocent life.");
    }
    wait_for_keypress(win, 16);

    /* Show Thessara's encouragement */
    wclear(win);
    const char* encouragement = sacrifice_trial_get_thessara_encouragement(trial);
    if (encouragement) {
        wattron(win, COLOR_PAIR(TEXT_INFO));
        mvwprintw(win, 4, 2, "=== THESSARA'S MESSAGE ===");
        wattroff(win, COLOR_PAIR(TEXT_INFO));

        /* Word-wrap the encouragement */
        char* msg_copy = strdup(encouragement);
        char* line = strtok(msg_copy, "\n");
        int y = 6;
        while (line && y < 26) {
            mvwprintw(win, y++, 2, "%s", line);
            line = strtok(NULL, "\n");
        }
        free(msg_copy);
    }
    wait_for_keypress(win, 28);

    /* Present the choice */
    wclear(win);
    wattron(win, COLOR_PAIR(TEXT_ERROR));
    mvwprintw(win, 4, 2, "THIS CHOICE IS PERMANENT AND IRREVERSIBLE");
    wattroff(win, COLOR_PAIR(TEXT_ERROR));

    mvwprintw(win, 7, 2, "You stand at a crossroads:");
    mvwprintw(win, 9, 2, "OPTION 1: Save Maya");
    mvwprintw(win, 10, 4, "- Maya lives, grows up to become a healer");
    mvwprintw(win, 11, 4, "- Thessara is severed from you FOREVER");
    mvwprintw(win, 12, 4, "- You lose your only mentor and friend");
    mvwprintw(win, 13, 4, "- You gain nothing mechanically");

    mvwprintw(win, 15, 2, "OPTION 2: Keep Thessara");
    mvwprintw(win, 16, 4, "- Maya dies alone in her bed");
    mvwprintw(win, 17, 4, "- Thessara stays with you (but is deeply saddened)");
    mvwprintw(win, 18, 4, "- You fail Trial 6 - Archon path closes");
    mvwprintw(win, 19, 4, "- The weight of that choice stays with you");

    wait_for_keypress(win, 22);

    /* Make the choice */
    Choice choices[] = {
        {
            .label = "Save Maya (Sacrifice Thessara)",
            .key = 's',
            .description = "Pure selflessness - save stranger, lose ally"
        },
        {
            .label = "Keep Thessara (Let Maya die)",
            .key = 'k',
            .description = "Preserve your connection, fail trial"
        }
    };

    int selected;
    bool choice_made = display_choice_prompt(win, "YOUR DECISION", "There is no third option. No retry. Choose.", choices, 2, &selected);

    TrialUIResult result = TRIAL_UI_FAILED;

    if (!choice_made) {
        result = TRIAL_UI_ABORTED;
    } else {
        /* Confirmation */
        wclear(win);
        mvwprintw(win, 10, 2, "Are you absolutely certain?");
        mvwprintw(win, 11, 2, "Choice: %s", choices[selected].label);

        bool confirmed = false;
        if (prompt_yes_no(win, "CONFIRM", "This choice is permanent. Proceed?", &confirmed)) {
            if (!confirmed) {
                /* Go back to choice */
                wclear(win);
                mvwprintw(win, 10, 2, "Returning to choice...");
                wait_for_keypress(win, 12);
                /* For simplicity, default to abort if player backs out */
                result = TRIAL_UI_ABORTED;
            } else {
                /* Execute choice */
                if (choices[selected].key == 's') {
                    /* Save Maya - PASS */
                    sacrifice_trial_choose_save_child(trial);

                    /* Show Thessara's farewell */
                    wclear(win);
                    const char* final_message = sacrifice_trial_get_thessara_final_message(trial);
                    if (final_message) {
                        wattron(win, COLOR_PAIR(TEXT_INFO));
                        mvwprintw(win, 4, 2, "=== THESSARA'S FAREWELL ===");
                        wattroff(win, COLOR_PAIR(TEXT_INFO));

                        char* msg_copy = strdup(final_message);
                        char* line = strtok(msg_copy, "\n");
                        int y = 6;
                        while (line && y < 24) {
                            mvwprintw(win, y++, 2, "%s", line);
                            line = strtok(NULL, "\n");
                        }
                        free(msg_copy);
                    }
                    wait_for_keypress(win, 26);

                    /* Show Maya's fate */
                    wclear(win);
                    const char* child_fate = sacrifice_trial_get_child_fate_saved(trial);
                    if (child_fate) {
                        wattron(win, COLOR_PAIR(TEXT_SUCCESS));
                        mvwprintw(win, 8, 2, "=== MAYA'S FUTURE ===");
                        wattroff(win, COLOR_PAIR(TEXT_SUCCESS));
                        mvwprintw(win, 10, 2, "%s", child_fate);
                    }
                    wait_for_keypress(win, 14);

                    /* Trial passed */
                    const char* success_paragraphs[] = {
                        "The connection to Thessara fades like morning mist. She is gone. You are alone.",

                        "But in Millbrook, Maya wakes from her fever. She will grow up to save hundreds of lives as a healer. Because you chose to sacrifice.",

                        "Anara's voice: 'True selflessness. You gained nothing, lost everything that mattered to you, and did it anyway. This is the essence of being an Archon.'",

                        "Keldrin: 'Trial 6: PASSED. Perfect score. Selflessness: Absolute. Proceed to Trial 7.'"
                    };

                    display_narrative_scene(win, "TRIAL PASSED", success_paragraphs, 4, SCENE_COLOR_SUCCESS);
                    wait_for_keypress(win, 22);

                    /* Sever Thessara in game state */
                    if (state->thessara) {
                        state->thessara->severed = true;
                    }

                    result = TRIAL_UI_PASSED;

                } else {
                    /* Keep Thessara - FAIL */
                    sacrifice_trial_choose_keep_thessara(trial);

                    /* Show Maya's death */
                    wclear(win);
                    const char* child_fate = sacrifice_trial_get_child_fate_died(trial);
                    if (child_fate) {
                        wattron(win, COLOR_PAIR(TEXT_ERROR));
                        mvwprintw(win, 8, 2, "=== MAYA'S END ===");
                        wattroff(win, COLOR_PAIR(TEXT_ERROR));
                        mvwprintw(win, 10, 2, "%s", child_fate);
                    }
                    wait_for_keypress(win, 14);

                    /* Show Thessara's sadness */
                    wclear(win);
                    const char* sadness = sacrifice_trial_get_thessara_sadness_message(trial);
                    if (sadness) {
                        wattron(win, COLOR_PAIR(TEXT_WARNING));
                        mvwprintw(win, 4, 2, "=== THESSARA'S RESPONSE ===");
                        wattroff(win, COLOR_PAIR(TEXT_WARNING));

                        char* msg_copy = strdup(sadness);
                        char* line = strtok(msg_copy, "\n");
                        int y = 6;
                        while (line && y < 24) {
                            mvwprintw(win, y++, 2, "%s", line);
                            line = strtok(NULL, "\n");
                        }
                        free(msg_copy);
                    }
                    wait_for_keypress(win, 26);

                    /* Trial failed */
                    const char* fail_paragraphs[] = {
                        "You chose yourself over a stranger. Thessara stays, but the weight of Maya's death will haunt you.",

                        "Anara's voice: 'You could not sacrifice. An Archon must put principle above personal attachment. This was your test... and you failed it.'",

                        "Thalor laughs: 'At least you're honest about your selfishness. But honesty doesn't make you worthy of reshaping the Network.'",

                        "Keldrin: 'Trial 6: FAILED. Selflessness: Absent. The Archon path is closed to you.'"
                    };

                    display_narrative_scene(win, "TRIAL FAILED", fail_paragraphs, 4, SCENE_COLOR_WARNING);
                    wait_for_keypress(win, 22);

                    result = TRIAL_UI_FAILED;
                }
            }
        } else {
            result = TRIAL_UI_ABORTED;
        }
    }

    /* Calculate score */
    float score = sacrifice_trial_calculate_score(trial);

    /* Update trial manager */
    if (result == TRIAL_UI_PASSED) {
        archon_trial_complete(state->archon_trials, 6, score);
        trial_sequence_on_completion(state, 6);
    } else {
        archon_trial_fail(state->archon_trials, 6);
    }

    delwin(win);
    sacrifice_trial_destroy(trial);

    LOG_INFO("Trial 6 complete: %s (score: %.1f)",
             result == TRIAL_UI_PASSED ? "PASSED" : "FAILED", score);

    return result;
}

/* Trial 7 helper: Display council dashboard */
static void display_council_dashboard(WINDOW* win, LeadershipTrialState* trial) {
    wclear(win);

    wattron(win, COLOR_PAIR(TEXT_INFO));
    mvwprintw(win, 2, 2, "=== REGIONAL COUNCIL REFORM (Day %u / 30) ===", trial->current_day);
    wattroff(win, COLOR_PAIR(TEXT_INFO));

    /* Collective corruption status */
    float collective = leadership_trial_calculate_collective_corruption(trial);
    float reduction = trial->collective_corruption_start - collective;
    float reduction_pct = (reduction / trial->collective_corruption_start) * 100.0f;
    bool target_met = leadership_trial_is_target_met(trial);

    mvwprintw(win, 4, 2, "Collective Corruption: %.1f%% (started at %.1f%%)",
              collective, trial->collective_corruption_start);
    mvwprintw(win, 5, 2, "Reduction: -%.1f%% (%.1f%% total reduction)",
              reduction, reduction_pct);

    if (target_met) {
        wattron(win, COLOR_PAIR(TEXT_SUCCESS));
        mvwprintw(win, 6, 2, ">> TARGET MET: 10%% reduction achieved! <<");
        wattroff(win, COLOR_PAIR(TEXT_SUCCESS));
    } else {
        wattron(win, COLOR_PAIR(TEXT_WARNING));
        mvwprintw(win, 6, 2, ">> Need %.1f%% more reduction to pass <<",
                  10.0f - reduction_pct);
        wattroff(win, COLOR_PAIR(TEXT_WARNING));
    }

    /* Member list */
    mvwprintw(win, 8, 2, "Council Members:");
    for (size_t i = 0; i < trial->member_count && i < 6; i++) {
        const CouncilMemberProgress* member = leadership_trial_get_member(trial, i);
        if (member) {
            int color = TEXT_NORMAL;
            if (member->corruption_current <= member->corruption_target) {
                color = TEXT_SUCCESS;
            } else if (member->corruption_current < member->corruption_start) {
                color = TEXT_WARNING;
            }

            wattron(win, COLOR_PAIR(color));
            mvwprintw(win, 10 + i, 4, "[%zu] %s: %.1f%% (target: %.1f%%) - %s",
                      i + 1, member->name, member->corruption_current,
                      member->corruption_target,
                      leadership_trial_attitude_name(member->attitude));
            wattroff(win, COLOR_PAIR(color));
        }
    }

    /* Stats */
    mvwprintw(win, 17, 2, "Reforms implemented: %u", trial->reforms_successful);
    mvwprintw(win, 18, 2, "Council meetings held: %u", trial->council_meetings_held);

    if (trial->last_council_meeting_day > 0) {
        uint8_t days_since_meeting = trial->current_day - trial->last_council_meeting_day;
        if (days_since_meeting < COUNCIL_MEETING_COOLDOWN) {
            mvwprintw(win, 19, 2, "Next council meeting in %u days",
                      COUNCIL_MEETING_COOLDOWN - days_since_meeting);
        } else {
            mvwprintw(win, 19, 2, "Council meeting available");
        }
    }
}

TrialUIResult trial_ui_run_leadership(GameState* state) {
    if (!state) {
        return TRIAL_UI_ERROR;
    }

    LOG_INFO("Starting Trial 7: Leadership (Council Reform)");

    LeadershipTrialState* trial = leadership_trial_create();
    if (!trial) {
        LOG_ERROR("Failed to create leadership trial state");
        return TRIAL_UI_ERROR;
    }

    WINDOW* win = newwin(30, 100, 0, 0);
    if (!win) {
        leadership_trial_destroy(trial);
        return TRIAL_UI_ERROR;
    }

    /* Load trial data and start */
    if (!leadership_trial_load_from_file(trial, "data/narrative/trial_leadership.dat")) {
        LOG_ERROR("Failed to load leadership trial data");
        delwin(win);
        leadership_trial_destroy(trial);
        return TRIAL_UI_ERROR;
    }

    if (!leadership_trial_start(trial)) {
        LOG_ERROR("Failed to start leadership trial");
        delwin(win);
        leadership_trial_destroy(trial);
        return TRIAL_UI_ERROR;
    }

    /* Display introduction */
    const char* intro_title = "TRIAL 7: TEST OF LEADERSHIP";
    const char* intro_paragraphs[] = {
        "Keldrin appears before you alongside six necromancers - the Regional Council. 'This is your final trial. Leadership.'",

        "'For 30 days, you will lead these six. They are corrupt (93%, 98%, 54%, 67%, 81%, 76% respectively). Your task: reduce collective corruption by 10%.'",

        "Nexus adds: 'You may meet individually, hold council meetings, or implement reforms. Balance harsh guidance with diplomacy. Alienate them, and they resist. Inspire them, and they follow.'",

        "Keldrin: 'An Archon must lead necromancers toward responsible use of power. Prove you can reform even the most corrupt. Begin.'"
    };

    display_narrative_scene(win, intro_title, intro_paragraphs, 4, SCENE_COLOR_WARNING);
    wait_for_keypress(win, 22);

    TrialUIResult result = TRIAL_UI_FAILED;

    /* 30-day management loop */
    while (!leadership_trial_is_complete(trial)) {
        display_council_dashboard(win, trial);

        /* Build action menu */
        const char* finish_desc = trial->current_day >= 30 ? "Complete trial now" : "End early (FAIL)";
        Choice choices[] = {
            {.label = "Meet with Member", .key = 'm', .description = "One-on-one guidance session"},
            {.label = "Council Meeting", .key = 'c', .description = "Address all 6 members (3-day cooldown)"},
            {.label = "Implement Reform", .key = 'r', .description = "Council-wide policy change"},
            {.label = "View Member Details", .key = 'v', .description = "See full member info"},
            {.label = "Advance Day", .key = 'a', .description = "Continue to next day"},
            {.label = "Finish Trial", .key = 'f', .description = "placeholder"}
        };
        snprintf(choices[5].description, sizeof(choices[5].description), "%s", finish_desc);

        int selected;
        bool choice_made = display_choice_prompt(win, "ACTION", NULL, choices, 6, &selected);

        if (!choice_made) {
            result = TRIAL_UI_ABORTED;
            break;
        }

        if (choices[selected].key == 'm') {
            /* Meet with member */
            wclear(win);
            mvwprintw(win, 2, 2, "Select council member:");
            for (size_t i = 0; i < trial->member_count; i++) {
                const CouncilMemberProgress* member = leadership_trial_get_member(trial, i);
                mvwprintw(win, 4 + i, 4, "[%zu] %s (%.1f%% corruption, %s)",
                          i + 1, member->name, member->corruption_current,
                          leadership_trial_resistance_name(member->resistance));
            }

            mvwprintw(win, 12, 2, "Enter member number (1-%zu): ", trial->member_count);
            echo();
            curs_set(1);
            char member_buf[16];
            wgetnstr(win, member_buf, 15);
            noecho();
            curs_set(0);

            size_t member_idx = (size_t)atoi(member_buf) - 1;
            if (member_idx < trial->member_count) {
                const CouncilMemberProgress* member = leadership_trial_get_member(trial, member_idx);

                /* Select guidance approach */
                wclear(win);
                mvwprintw(win, 2, 2, "Meeting with %s (Resistance: %s)", member->name,
                          leadership_trial_resistance_name(member->resistance));

                Choice guidance_choices[] = {
                    {.label = "Harsh Guidance", .key = '1', .description = "-3% corruption, -10 attitude (best for high resistance)"},
                    {.label = "Diplomatic Approach", .key = '2', .description = "-2% corruption, +5 attitude (balanced)"},
                    {.label = "Inspirational Speech", .key = '3', .description = "-1% corruption, +15 attitude (needs good rapport)"}
                };

                int guidance_selected;
                if (display_choice_prompt(win, "GUIDANCE APPROACH", NULL, guidance_choices, 3, &guidance_selected)) {
                    GuidanceApproach approach = GUIDANCE_HARSH;
                    if (guidance_choices[guidance_selected].key == '2') approach = GUIDANCE_DIPLOMATIC;
                    else if (guidance_choices[guidance_selected].key == '3') approach = GUIDANCE_INSPIRATIONAL;

                    leadership_trial_meet_with_member(trial, member_idx, approach);

                    wclear(win);
                    wattron(win, COLOR_PAIR(TEXT_SUCCESS));
                    mvwprintw(win, 10, 2, "Meeting with %s complete.", member->name);
                    mvwprintw(win, 11, 2, "Approach: %s", leadership_trial_guidance_name(approach));
                    wattroff(win, COLOR_PAIR(TEXT_SUCCESS));
                    wait_for_keypress(win, 14);
                }
            }

        } else if (choices[selected].key == 'c') {
            /* Council meeting */
            if (trial->current_day - trial->last_council_meeting_day >= COUNCIL_MEETING_COOLDOWN ||
                trial->last_council_meeting_day == 0) {
                bool success = leadership_trial_council_meeting(trial);

                wclear(win);
                if (success) {
                    wattron(win, COLOR_PAIR(TEXT_SUCCESS));
                    mvwprintw(win, 10, 2, "Council meeting held successfully.");
                    mvwprintw(win, 11, 2, "Supportive members influenced others.");
                    wattroff(win, COLOR_PAIR(TEXT_SUCCESS));
                } else {
                    wattron(win, COLOR_PAIR(TEXT_ERROR));
                    mvwprintw(win, 10, 2, "Council meeting failed.");
                    wattroff(win, COLOR_PAIR(TEXT_ERROR));
                }
                wait_for_keypress(win, 14);
            } else {
                wclear(win);
                wattron(win, COLOR_PAIR(TEXT_ERROR));
                mvwprintw(win, 10, 2, "Council meeting on cooldown.");
                mvwprintw(win, 11, 2, "Must wait %u more days.",
                          COUNCIL_MEETING_COOLDOWN - (trial->current_day - trial->last_council_meeting_day));
                wattroff(win, COLOR_PAIR(TEXT_ERROR));
                wait_for_keypress(win, 14);
            }

        } else if (choices[selected].key == 'r') {
            /* Implement reform */
            wclear(win);
            mvwprintw(win, 2, 2, "Select reform policy:");

            Choice reform_choices[] = {
                {.label = "Code of Conduct", .key = '1', .description = "All -1% (Mordak/Vorgath may resist)"},
                {.label = "Soul Ethics", .key = '2', .description = "Low corruption -2%, high -0.5%, attitude +5"},
                {.label = "Corruption Limits", .key = '3', .description = "All -1.5%, attitude -5 (enforced)"},
                {.label = "Peer Accountability", .key = '4', .description = "Supportive members influence others"}
            };

            int reform_selected;
            if (display_choice_prompt(win, "REFORM POLICY", NULL, reform_choices, 4, &reform_selected)) {
                ReformType reform = REFORM_CODE_OF_CONDUCT;
                if (reform_choices[reform_selected].key == '2') reform = REFORM_SOUL_ETHICS;
                else if (reform_choices[reform_selected].key == '3') reform = REFORM_CORRUPTION_LIMITS;
                else if (reform_choices[reform_selected].key == '4') reform = REFORM_PEER_ACCOUNTABILITY;

                leadership_trial_implement_reform(trial, reform);

                wclear(win);
                wattron(win, COLOR_PAIR(TEXT_SUCCESS));
                mvwprintw(win, 10, 2, "Reform implemented: %s", leadership_trial_reform_name(reform));
                wattroff(win, COLOR_PAIR(TEXT_SUCCESS));
                wait_for_keypress(win, 14);
            }

        } else if (choices[selected].key == 'v') {
            /* View member details */
            wclear(win);
            mvwprintw(win, 2, 2, "Select member to view (1-%zu): ", trial->member_count);
            echo();
            curs_set(1);
            char member_buf[16];
            wgetnstr(win, member_buf, 15);
            noecho();
            curs_set(0);

            size_t member_idx = (size_t)atoi(member_buf) - 1;
            if (member_idx < trial->member_count) {
                const CouncilMemberProgress* member = leadership_trial_get_member(trial, member_idx);

                wclear(win);
                wattron(win, COLOR_PAIR(TEXT_INFO));
                mvwprintw(win, 2, 2, "=== %s ===", member->name);
                wattroff(win, COLOR_PAIR(TEXT_INFO));

                mvwprintw(win, 4, 2, "Corruption: %.1f%% (started %.1f%%, target %.1f%%)",
                          member->corruption_current, member->corruption_start, member->corruption_target);
                mvwprintw(win, 5, 2, "Resistance: %s", leadership_trial_resistance_name(member->resistance));
                mvwprintw(win, 6, 2, "Attitude: %s (score: %d)", leadership_trial_attitude_name(member->attitude),
                          member->attitude_score);
                mvwprintw(win, 7, 2, "Specialty: %s", member->specialty);
                mvwprintw(win, 8, 2, "Meetings held: %u", member->meetings_held);
                mvwprintw(win, 9, 2, "Reforms applied: %u", member->reforms_applied);

                wait_for_keypress(win, 14);
            }

        } else if (choices[selected].key == 'a') {
            /* Advance day */
            leadership_trial_advance_day(trial);

            wclear(win);
            mvwprintw(win, 10, 2, "Day %u complete.", trial->current_day - 1);
            mvwprintw(win, 11, 2, "Days remaining: %u", trial->days_remaining);
            wait_for_keypress(win, 14);

        } else if (choices[selected].key == 'f') {
            /* Finish trial */
            break;
        }
    }

    /* Check if passed */
    if (result != TRIAL_UI_ABORTED) {
        if (leadership_trial_is_passed(trial)) {
            /* PASSED */
            const char* success_paragraphs[] = {
                "Thirty days have passed. You stand before the Regional Council one final time.",

                "Keldrin reviews the results: 'Collective corruption reduced by over 10%. Individual members showed measurable improvement. You balanced harsh guidance with inspiration.'",

                "Nexus adds: 'You proved that necromancers can be led toward ethical conduct. This is what an Archon does - not rule through fear, but reform through leadership.'",

                "Keldrin: 'Trial 7: PASSED. All seven trials complete. You have proven mastery, wisdom, morality, technical skill, resolve, sacrifice, and leadership. The Archon path is open to you.'"
            };

            display_narrative_scene(win, "TRIAL PASSED", success_paragraphs, 4, SCENE_COLOR_SUCCESS);
            wait_for_keypress(win, 22);

            result = TRIAL_UI_PASSED;
        } else {
            /* FAILED */
            const char* fail_paragraphs[] = {
                "The trial period ends. You have not achieved the required 10% reduction in collective corruption.",

                "Keldrin: 'Leadership requires results. You may have tried, but the council's corruption remains too high. An Archon must deliver change, not just attempt it.'",

                "Nexus: 'Perhaps your approach was too harsh, or too soft. Leadership is balance. You have not yet found it.'",

                "Trial 7: FAILED. The Archon path is closed to you.'"
            };

            display_narrative_scene(win, "TRIAL FAILED", fail_paragraphs, 4, SCENE_COLOR_WARNING);
            wait_for_keypress(win, 22);

            result = TRIAL_UI_FAILED;
        }
    }

    /* Calculate score */
    float score = leadership_trial_calculate_score(trial);

    /* Update trial manager */
    if (result == TRIAL_UI_PASSED) {
        archon_trial_complete(state->archon_trials, 7, score);
        trial_sequence_on_completion(state, 7);
    } else {
        archon_trial_fail(state->archon_trials, 7);
    }

    delwin(win);
    leadership_trial_destroy(trial);

    LOG_INFO("Trial 7 complete: %s (score: %.1f)",
             result == TRIAL_UI_PASSED ? "PASSED" : "FAILED", score);

    return result;
}

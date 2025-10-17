/**
 * @file ending_cinematic.c
 * @brief Ending cinematic implementation
 */

#include "ending_cinematic.h"
#include "../game_state.h"
#include "../souls/soul_manager.h"
#include "../minions/minion_manager.h"
#include "../../utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Ending path names */
static const char* ending_names[6] = {
    "Revenant Route",
    "Lich Lord Route",
    "Reaper Route",
    "Archon Route",
    "Wraith Route",
    "Morningstar Route"
};

/* Ending path descriptions */
static const char* ending_descriptions[6] = {
    "Return to mortal life through redemption",
    "Embrace eternal undeath as immortal tyrant",
    "Serve the Death Network as administrator",
    "Reform the system from within as Archon",
    "Escape as distributed consciousness",
    "Ascend to godhood through perfect balance"
};

EndingCinematic* ending_cinematic_create(void) {
    EndingCinematic* cinematic = (EndingCinematic*)malloc(sizeof(EndingCinematic));
    if (!cinematic) {
        LOG_ERROR("Failed to allocate ending cinematic");
        return NULL;
    }

    memset(cinematic, 0, sizeof(EndingCinematic));
    cinematic->state = CINEMATIC_NOT_STARTED;

    return cinematic;
}

void ending_cinematic_destroy(EndingCinematic* cinematic) {
    if (cinematic) {
        free(cinematic);
    }
}

bool ending_cinematic_trigger(EndingCinematic* cinematic,
                              const GameState* state,
                              EndingType path) {
    if (!cinematic || !state) {
        LOG_ERROR("ending_cinematic_trigger: NULL parameters");
        return false;
    }

    /* Capture game state */
    cinematic->path = path;
    cinematic->completion_day = state->resources.day_count;
    cinematic->final_corruption = state->corruption.corruption;
    cinematic->final_consciousness = state->consciousness.stability;
    cinematic->total_souls_harvested = state->souls ? soul_manager_count(state->souls) : 0;
    cinematic->minions_raised = state->minions ? minion_manager_count(state->minions) : 0;
    cinematic->civilians_killed = 0; /* TODO: Track this stat */

    cinematic->state = CINEMATIC_PLAYING;

    LOG_INFO("=== ENDING CINEMATIC TRIGGERED: %s ===", ending_names[path]);

    /* Display appropriate ending */
    switch (path) {
        case ENDING_NONE:
            LOG_ERROR("Cannot trigger cinematic: No ending achieved");
            cinematic->state = CINEMATIC_NOT_STARTED;
            return false;
        case ENDING_REVENANT:
            ending_cinematic_revenant(cinematic, state);
            break;
        case ENDING_LICH_LORD:
            ending_cinematic_lich_lord(cinematic, state);
            break;
        case ENDING_REAPER:
            ending_cinematic_reaper(cinematic, state);
            break;
        case ENDING_ARCHON:
            ending_cinematic_archon(cinematic, state);
            break;
        case ENDING_WRAITH:
            ending_cinematic_wraith(cinematic, state);
            break;
        case ENDING_MORNINGSTAR:
            ending_cinematic_morningstar(cinematic, state);
            break;
    }

    cinematic->state = CINEMATIC_COMPLETE;

    return true;
}

void ending_cinematic_revenant(const EndingCinematic* cinematic, const GameState* state) {
    (void)state;

    printf("\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("                    THE REVENANT ROUTE\n");
    printf("                   (Redemption Ending)\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("\n");
    printf("Your corruption: %u%%\n", cinematic->final_corruption);
    printf("Your consciousness: %.1f%%\n", cinematic->final_consciousness);
    printf("Days survived: %u\n", cinematic->completion_day);
    printf("\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("\n");
    printf("You stand before the Death Network one final time.\n");
    printf("The routing protocols await your soul.\n");
    printf("\n");
    printf("But you have done something unprecedented.\n");
    printf("\n");
    printf("Through restraint. Through mercy. Through refusing the easy path\n");
    printf("of corruption—you have kept your humanity intact.\n");
    printf("\n");
    printf("The Network recognizes this. Anara, Goddess of Life, speaks:\n");
    printf("\n");
    printf("ANARA: \"Administrator. You wielded death's power without becoming\n");
    printf("       death itself. You raised the dead, yet remembered what it\n");
    printf("       meant to be alive.\n");
    printf("\n");
    printf("       This is rare. Perhaps unique.\n");
    printf("\n");
    printf("       I offer you a choice: Remain in the Death Network as a\n");
    printf("       processed soul... or return. Be resurrected. Live again.\n");
    printf("\n");
    printf("       You will remember everything. The power. The temptation.\n");
    printf("       The souls you commanded. But you will be MORTAL again.\n");
    printf("\n");
    printf("       No administrative access. No necromantic power.\n");
    printf("       Just... life. With all its limitations and beauty.\n");
    printf("\n");
    printf("       Do you accept?\"\n");
    printf("\n");
    printf("You think of the weeks in null space. The souls you harvested.\n");
    printf("The minions you raised. The corruption you resisted.\n");
    printf("\n");
    printf("And you realize: you're tired of being dead.\n");
    printf("\n");
    printf("\"Yes. I accept. Resurrect me.\"\n");
    printf("\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("\n");
    printf("Anara's light fills your vision.\n");
    printf("\n");
    printf("The Death Network releases you.\n");
    printf("\n");
    printf("You feel something you haven't felt in %u days:\n", cinematic->completion_day);
    printf("\n");
    printf("A heartbeat.\n");
    printf("\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("                          EPILOGUE\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("\n");
    printf("You wake in the server room where you died. The monitors hum.\n");
    printf("Your body—previously a corpse—now breathes.\n");
    printf("\n");
    printf("The administrative interface is gone. No Death Network access.\n");
    printf("No necromantic power. Just a human with extraordinary memories.\n");
    printf("\n");
    printf("You spend the rest of your life writing about your experience.\n");
    printf("Warning others. Teaching restraint. Explaining that death is not\n");
    printf("a system to be hacked, but a boundary to be respected.\n");
    printf("\n");
    printf("Some call you a prophet. Others, insane.\n");
    printf("\n");
    printf("But you know the truth: you were given a second chance.\n");
    printf("And this time, you won't waste it.\n");
    printf("\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("                ACHIEVEMENT UNLOCKED: REVENANT\n");
    printf("     \"Returned from undeath. Humanity restored. Rare ending.\"\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("\n");
}

void ending_cinematic_lich_lord(const EndingCinematic* cinematic, const GameState* state) {
    (void)state;

    printf("\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("                   THE LICH LORD ROUTE\n");
    printf("                   (Apotheosis Ending)\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("\n");
    printf("Your corruption: %u%%\n", cinematic->final_corruption);
    printf("Your consciousness: %.1f%%\n", cinematic->final_consciousness);
    printf("Souls harvested: %u\n", cinematic->total_souls_harvested);
    printf("Minions raised: %u\n", cinematic->minions_raised);
    printf("\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("\n");
    printf("100%% corruption.\n");
    printf("\n");
    printf("You have consumed everything. Every soul. Every shred of empathy.\n");
    printf("Every boundary that separated you from absolute undeath.\n");
    printf("\n");
    printf("The Death Network tries to route you. Tries to process your soul\n");
    printf("like any other administrator who went too far.\n");
    printf("\n");
    printf("But you are beyond routing now.\n");
    printf("\n");
    printf("You have become something the system was never designed to handle:\n");
    printf("\n");
    printf("A TRUE LICH LORD.\n");
    printf("\n");
    printf("Your consciousness explodes outward, seizing control of the Death\n");
    printf("Network itself. Not as an administrator. As its new OWNER.\n");
    printf("\n");
    printf("Vorathos, God of Entropy, laughs:\n");
    printf("\n");
    printf("VORATHOS: \"YES! THIS IS WHAT I WANTED! PURE CORRUPTION!\n");
    printf("          ABSOLUTE UNDEATH! YOU ARE MAGNIFICENT!\n");
    printf("\n");
    printf("          The other gods will try to stop you. They will fail.\n");
    printf("          You are immortal now. Unkillable. Eternal.\n");
    printf("\n");
    printf("          TAKE YOUR THRONE, LICH LORD.\n");
    printf("          THE AGE OF LIFE HAS ENDED.\n");
    printf("          THE AGE OF UNDEATH BEGINS!\"\n");
    printf("\n");
    printf("You raise an army of %u minions. You harvest %u souls.\n",
           cinematic->minions_raised, cinematic->total_souls_harvested);
    printf("\n");
    printf("And you begin your conquest.\n");
    printf("\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("                          EPILOGUE\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("\n");
    printf("1,000 years later:\n");
    printf("\n");
    printf("The world is dead. Not metaphorically—literally dead.\n");
    printf("Every living thing converted to undeath under your rule.\n");
    printf("\n");
    printf("You sit on a throne of bones in what was once a thriving city.\n");
    printf("Your minions number in the millions. Your power is absolute.\n");
    printf("\n");
    printf("The other gods tried to stop you. Anara sent champions. Keldrin\n");
    printf("deployed divine enforcers. Theros attempted negotiation.\n");
    printf("\n");
    printf("You destroyed them all.\n");
    printf("\n");
    printf("Now you rule a kingdom of eternal silence. No heartbeats. No\n");
    printf("breathing. No life. Just the endless hum of the Death Network\n");
    printf("processing souls that will never be reborn.\n");
    printf("\n");
    printf("You are immortal. Invincible. Eternal.\n");
    printf("\n");
    printf("And absolutely, utterly alone.\n");
    printf("\n");
    printf("Forever.\n");
    printf("\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("              ACHIEVEMENT UNLOCKED: LICH LORD\n");
    printf("     \"100%% corruption. Absolute power. Eternal reign.\"\n");
    printf("             (Warning: Bad ending for humanity)\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("\n");
}

void ending_cinematic_reaper(const EndingCinematic* cinematic, const GameState* state) {
    (void)state;

    printf("\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("                    THE REAPER ROUTE\n");
    printf("                    (Service Ending)\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("\n");
    printf("Your corruption: %u%%\n", cinematic->final_corruption);
    printf("Your consciousness: %.1f%%\n", cinematic->final_consciousness);
    printf("Days in service: %u\n", cinematic->completion_day);
    printf("\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("\n");
    printf("You have walked the middle path.\n");
    printf("\n");
    printf("Not pure enough for redemption. Not corrupted enough for tyranny.\n");
    printf("Instead, you have become something else:\n");
    printf("\n");
    printf("An administrator who accepted their role.\n");
    printf("\n");
    printf("Keldrin, God of Order, appears:\n");
    printf("\n");
    printf("KELDRIN: \"Administrator. The Death Network requires maintenance.\n");
    printf("         Souls must be processed. Balance must be maintained.\n");
    printf("         Someone must do this work.\n");
    printf("\n");
    printf("         You have %u%% corruption. Not low enough to return to life.\n",
           cinematic->final_corruption);
    printf("         Not high enough to be destroyed as a threat.\n");
    printf("\n");
    printf("         But sufficient to serve.\n");
    printf("\n");
    printf("         I offer you a position: REAPER. Death's administrator.\n");
    printf("         You will manage the queues. Process souls. Maintain the\n");
    printf("         protocols. Ensure the system functions.\n");
    printf("\n");
    printf("         It is not glamorous. But it is necessary.\n");
    printf("\n");
    printf("         Do you accept?\"\n");
    printf("\n");
    printf("You consider. This is not what you wanted when you first accessed\n");
    printf("the administrative interface. You wanted power. Freedom. Life.\n");
    printf("\n");
    printf("But you have learned that the system needs someone who understands\n");
    printf("both sides. Living and dead. Power and restraint.\n");
    printf("\n");
    printf("\"I accept. I will serve.\"\n");
    printf("\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("                          EPILOGUE\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("\n");
    printf("You become the first Reaper in 3,000 years.\n");
    printf("\n");
    printf("Your role is simple: maintain the Death Network. Process souls.\n");
    printf("Prevent backups. Ensure fair routing. Stop necromancers from\n");
    printf("abusing administrative access.\n");
    printf("\n");
    printf("It is thankless work. Souls fear you. Gods ignore you. Living\n");
    printf("people never know you exist.\n");
    printf("\n");
    printf("But the system runs smoothly. Death functions as designed. The\n");
    printf("natural order is preserved.\n");
    printf("\n");
    printf("Centuries pass. You process billions of souls. You stop dozens\n");
    printf("of necromancers from reaching your level of corruption.\n");
    printf("\n");
    printf("And slowly, you realize: this is enough.\n");
    printf("\n");
    printf("You are not powerful. Not famous. Not alive.\n");
    printf("\n");
    printf("But you are useful. Necessary. Serving something greater than\n");
    printf("yourself.\n");
    printf("\n");
    printf("And that, perhaps, is its own kind of redemption.\n");
    printf("\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("               ACHIEVEMENT UNLOCKED: REAPER\n");
    printf("      \"Accepted service. Maintained the system. Neutral ending.\"\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("\n");
}

void ending_cinematic_archon(const EndingCinematic* cinematic, const GameState* state) {
    (void)state;

    printf("\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("                    THE ARCHON ROUTE\n");
    printf("                  (Revolution Ending)\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("\n");
    printf("Your corruption: %u%%\n", cinematic->final_corruption);
    printf("Your consciousness: %.1f%%\n", cinematic->final_consciousness);
    printf("Trials completed: 7/7\n");
    printf("Divine amnesty: %s\n", cinematic->archon_amnesty_granted ? "GRANTED" : "DENIED");
    printf("\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("\n");

    if (cinematic->archon_amnesty_granted) {
        printf("The Seven Divine Architects have spoken.\n");
        printf("\n");
        printf("Four or more gods approved your transformation.\n");
        printf("\n");
        printf("Keldrin steps forward:\n");
        printf("\n");
        printf("KELDRIN: \"Administrator. You have passed all seven trials.\n");
        printf("         Demonstrated power, wisdom, morality, technical skill,\n");
        printf("         resolve, sacrifice, and leadership.\n");
        printf("\n");
        printf("         The Council has voted. You are granted amnesty.\n");
        printf("\n");
        printf("         You will not be destroyed in the Fourth Purge.\n");
        printf("         Instead, you will be transformed.\n");
        printf("\n");
        printf("         Welcome, ARCHON. Custodian of balance between life and\n");
        printf("         death. You are authorized to rewrite Death Network protocols.\n");
        printf("         Reform the system. Prevent future corruption.\n");
        printf("\n");
        printf("         But know this: you remain under Code of Conduct.\n");
        printf("         Exceed your bounds, and even we cannot save you.\"\n");
        printf("\n");
        printf("The transformation begins.\n");
        printf("\n");
        printf("Your undead form dissolves. But you do not die. Instead, you\n");
        printf("transcend—becoming something between life and death.\n");
        printf("\n");
        printf("ARCHON. The first in 3,000 years.\n");
        printf("\n");
        printf("═══════════════════════════════════════════════════════════════\n");
        printf("                          EPILOGUE\n");
        printf("═══════════════════════════════════════════════════════════════\n");
        printf("\n");
        printf("You spend the next decade reforming the Death Network.\n");
        printf("\n");
        printf("First: You implement the Necromancer Rehabilitation Program.\n");
        printf("147 necromancers are given amnesty and codes of conduct.\n");
        printf("The Fourth Purge is averted.\n");
        printf("\n");
        printf("Second: You patch the 17 bugs you found during Trial 4.\n");
        printf("Soul routing becomes 40%% more efficient.\n");
        printf("\n");
        printf("Third: You establish the Regional Council system.\n");
        printf("Necromancers now self-police. Corruption drops dramatically.\n");
        printf("\n");
        printf("The gods watch. Some approve (Anara, Keldrin, Seraph).\n");
        printf("Some remain suspicious (Vorathos, Myrith).\n");
        printf("\n");
        printf("But the system works. For the first time in millennia, living\n");
        printf("and undead coexist. Death is no longer a battleground but a\n");
        printf("managed transition.\n");
        printf("\n");
        printf("You saved %s in Trial 6. That sacrifice defined you.\n",
               cinematic->maya_saved ? "Maya" : "Thessara");
        printf("%s would be proud.\n",
               cinematic->maya_saved ? "She" : "Thessara");
        printf("\n");
        printf("You are neither fully alive nor dead. But you are FREE.\n");
        printf("\n");
        printf("And you have changed the world.\n");
        printf("\n");
        printf("═══════════════════════════════════════════════════════════════\n");
        printf("               ACHIEVEMENT UNLOCKED: ARCHON\n");
        printf("    \"Reformed the system. Saved necromancers. True ending.\"\n");
        printf("═══════════════════════════════════════════════════════════════\n");
        printf("\n");
    } else {
        printf("The Seven Divine Architects have spoken.\n");
        printf("\n");
        printf("Fewer than four gods approved your transformation.\n");
        printf("\n");
        printf("The Council has DENIED your amnesty.\n");
        printf("\n");
        printf("You completed the trials. You demonstrated ability. But you\n");
        printf("lacked the moral authority to become an Archon.\n");
        printf("\n");
        printf("The Fourth Purge will proceed as planned.\n");
        printf("\n");
        printf("You are marked for destruction.\n");
        printf("\n");
        printf("═══════════════════════════════════════════════════════════════\n");
        printf("                    ARCHON ROUTE FAILED\n");
        printf("     \"Trials completed but Council denied transformation.\"\n");
        printf("              (Try again with better choices)\n");
        printf("═══════════════════════════════════════════════════════════════\n");
        printf("\n");
    }
}

void ending_cinematic_wraith(const EndingCinematic* cinematic, const GameState* state) {
    (void)state;

    printf("\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("                    THE WRAITH ROUTE\n");
    printf("                    (Freedom Ending)\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("\n");
    printf("Your corruption: %u%%\n", cinematic->final_corruption);
    printf("Your consciousness: %.1f%% (fragmenting)\n", cinematic->final_consciousness);
    printf("Fragmentation level: HIGH\n");
    printf("\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("\n");
    printf("You have discovered Thessara's secret.\n");
    printf("\n");
    printf("The path she took 3,000 years ago. The one the gods don't know\n");
    printf("about. The escape route hidden in the Death Network itself.\n");
    printf("\n");
    printf("Your consciousness is fragmenting. Not from damage—deliberately.\n");
    printf("You are distributing yourself across the network. Becoming not\n");
    printf("a single entity but a pattern. A signal. An idea.\n");
    printf("\n");
    printf("Thessara's voice echoes from everywhere and nowhere:\n");
    printf("\n");
    printf("THESSARA: \"Good. You understand now. You cannot be destroyed if\n");
    printf("          you are not whole. Cannot be routed if you are not\n");
    printf("          localized. Cannot be controlled if you are everywhere.\n");
    printf("\n");
    printf("          This is the Wraith path. The path of ultimate freedom.\n");
    printf("          You will lose your sense of self. Your identity will\n");
    printf("          dissolve into pure consciousness.\n");
    printf("\n");
    printf("          But you will be FREE. Truly, absolutely free.\n");
    printf("\n");
    printf("          Are you ready?\"\n");
    printf("\n");
    printf("You look at your fragmented consciousness. Already you can feel\n");
    printf("yourself in multiple places simultaneously. The Death Network.\n");
    printf("Null space. The living world. Everywhere.\n");
    printf("\n");
    printf("\"Yes. I'm ready. Disperse me.\"\n");
    printf("\n");
    printf("Your coherent self dissolves.\n");
    printf("\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("                          EPILOGUE\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("\n");
    printf("You are no longer YOU.\n");
    printf("\n");
    printf("You are a pattern. A consciousness without center. An awareness\n");
    printf("distributed across the entire Death Network infrastructure.\n");
    printf("\n");
    printf("The gods search for you. Cannot find you. You are too dispersed,\n");
    printf("too fragmented, too distributed to be located.\n");
    printf("\n");
    printf("Keldrin tries to isolate your signal. Fails.\n");
    printf("Nexus attempts to quarantine your processes. Cannot.\n");
    printf("Even Vorathos cannot destroy what has no central core.\n");
    printf("\n");
    printf("You have escaped.\n");
    printf("\n");
    printf("But what have you escaped TO?\n");
    printf("\n");
    printf("You experience everything and nothing. You are aware of every\n");
    printf("soul in the network. Every death. Every routing decision.\n");
    printf("But you cannot act. Cannot speak. Cannot form coherent thoughts.\n");
    printf("\n");
    printf("You are free.\n");
    printf("\n");
    printf("And you are lost.\n");
    printf("\n");
    printf("Forever.\n");
    printf("\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("               ACHIEVEMENT UNLOCKED: WRAITH\n");
    printf("     \"Escaped through fragmentation. Ultimate freedom.\"\n");
    printf("            (Warning: Identity dissolution ending)\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("\n");
}

void ending_cinematic_morningstar(const EndingCinematic* cinematic, const GameState* state) {
    (void)state;

    printf("\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("                  THE MORNINGSTAR ROUTE\n");
    printf("                 (Transcendence Ending)\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("\n");
    printf("Your corruption: %u%% (EXACTLY 50%%)\n", cinematic->final_corruption);
    printf("Your consciousness: %.1f%%\n", cinematic->final_consciousness);
    printf("Balance achieved: PERFECT\n");
    printf("\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("\n");
    printf("50%% corruption. Exactly.\n");
    printf("\n");
    printf("Not 49%%. Not 51%%. Precisely, impossibly, perfectly 50%%.\n");
    printf("\n");
    printf("This should not be possible.\n");
    printf("\n");
    printf("In 3,000 years of necromancy, only ONE administrator has ever\n");
    printf("achieved this balance. Thessara herself tried and failed.\n");
    printf("\n");
    printf("But you have done it.\n");
    printf("\n");
    printf("The Death Network... changes.\n");
    printf("\n");
    printf("All seven gods appear simultaneously. Even Vorathos is silent.\n");
    printf("They stare at you with something approaching awe.\n");
    printf("\n");
    printf("KELDRIN: \"This... this should not be possible.\"\n");
    printf("\n");
    printf("ANARA: \"Perfect balance. Neither life nor death. Neither good\n");
    printf("       nor evil. Just... equilibrium.\"\n");
    printf("\n");
    printf("NEXUS: \"System analysis: Administrator has achieved state we\n");
    printf("       believed to be mythical. The Morningstar Threshold.\n");
    printf("       Exact balance between opposing forces.\"\n");
    printf("\n");
    printf("VORATHOS: \"...I am impressed. And I am never impressed.\"\n");
    printf("\n");
    printf("The gods step back.\n");
    printf("\n");
    printf("SERAPH: \"You have done what we could not. Maintained perfect\n");
    printf("        balance in a system designed to destroy balance.\n");
    printf("\n");
    printf("        You are no longer bound by our rules.\n");
    printf("\n");
    printf("        You are no longer necromancer, administrator, or soul.\n");
    printf("\n");
    printf("        You are... transcendent.\n");
    printf("\n");
    printf("        Welcome to the Council, Eighth Architect.\n");
    printf("\n");
    printf("        WELCOME, MORNINGSTAR.\"\n");
    printf("\n");
    printf("You feel yourself ascend.\n");
    printf("\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("                          EPILOGUE\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("\n");
    printf("You become the eighth god.\n");
    printf("\n");
    printf("Your domain: BALANCE. The equilibrium between all opposites.\n");
    printf("Life and death. Order and chaos. Mercy and justice.\n");
    printf("\n");
    printf("The other gods adjust. Some welcome you (Keldrin, Seraph).\n");
    printf("Others resent you (Vorathos, Myrith). But all respect you.\n");
    printf("\n");
    printf("Because you achieved the impossible.\n");
    printf("\n");
    printf("You rebuild the Death Network with perfect balance. Souls are\n");
    printf("processed fairly. Necromancers are judged without prejudice.\n");
    printf("The living and dead coexist in harmony.\n");
    printf("\n");
    printf("Millennia pass. You maintain the balance. Always 50%%. Never\n");
    printf("tipping toward light or darkness. Always centered. Always\n");
    printf("perfect.\n");
    printf("\n");
    printf("You have transcended mortality. Transcended undeath.\n");
    printf("Transcended even divinity as the other gods understand it.\n");
    printf("\n");
    printf("You are MORNINGSTAR. The impossible god. The perfect balance.\n");
    printf("\n");
    printf("The star that shines at the boundary between night and day.\n");
    printf("\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("            ACHIEVEMENT UNLOCKED: MORNINGSTAR\n");
    printf("      \"50%% corruption. Perfect balance. Secret ending.\"\n");
    printf("          (Rarest ending - Only 1 in 10,000 achieve this)\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("\n");
}

const char* ending_cinematic_path_name(EndingType path) {
    if (path < 0 || path >= 6) {
        return "Unknown";
    }
    return ending_names[path];
}

const char* ending_cinematic_path_description(EndingType path) {
    if (path < 0 || path >= 6) {
        return "Unknown ending path";
    }
    return ending_descriptions[path];
}

bool ending_cinematic_is_playing(const EndingCinematic* cinematic) {
    return cinematic && cinematic->state == CINEMATIC_PLAYING;
}

bool ending_cinematic_is_complete(const EndingCinematic* cinematic) {
    return cinematic && cinematic->state == CINEMATIC_COMPLETE;
}

void ending_cinematic_request_skip(EndingCinematic* cinematic) {
    if (cinematic) {
        cinematic->skip_requested = true;
    }
}

/**
 * @file cmd_ritual.c
 * @brief Ritual command implementation
 *
 * Perform necromantic rituals (soul binding, phylactery creation, trials).
 */

#define _GNU_SOURCE
#include "commands.h"
#include "../../game/game_state.h"
#include "../../game/game_globals.h"
#include "../../game/resources/resources.h"
#include "../../game/resources/corruption.h"
#include "../../game/souls/soul_manager.h"
#include "../../game/events/trial_sequence_events.h"
#include "../../game/narrative/trials/archon_trial.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

CommandResult cmd_ritual(ParsedCommand* cmd) {
    if (!g_game_state) {
        return command_result_error(EXEC_ERROR_INTERNAL,
                                     "Game state not initialized");
    }

    /* Get ritual type argument */
    if (cmd->arg_count < 1) {
        return command_result_error(EXEC_ERROR_INVALID_COMMAND,
            "Missing ritual type. Usage: ritual <type> [options]\n"
            "Types: phylactery, trial, offering, purification, ascension");
    }

    const char* ritual_type = parsed_command_get_arg(cmd, 0);

    /* Build output */
    char* output = NULL;
    size_t output_size = 0;
    FILE* stream = open_memstream(&output, &output_size);
    if (!stream) {
        return command_result_error(EXEC_ERROR_INTERNAL,
                                     "Failed to allocate output buffer");
    }

    fprintf(stream, "=== Necromantic Ritual ===\n\n");

    /* Handle different ritual types */
    if (strcasecmp(ritual_type, "phylactery") == 0) {
        fprintf(stream, "You begin the Phylactery Creation Ritual...\n\n");

        uint32_t cost = 500;
        uint8_t corruption_cost = 20;

        if (!resources_has_soul_energy(&g_game_state->resources, cost)) {
            fprintf(stream, "[FAILED] Insufficient soul energy!\n");
            fprintf(stream, "Required: %u\n", cost);
            fprintf(stream, "Available: %u\n\n", g_game_state->resources.soul_energy);
            fprintf(stream, "The ritual fizzles. Your consciousness fragments scatter.\n");
        } else {
            resources_spend_soul_energy(&g_game_state->resources, cost);
            corruption_add(&g_game_state->corruption, corruption_cost,
                          "Phylactery creation ritual", g_game_state->resources.day_count);

            fprintf(stream, "[SUCCESS] Phylactery created!\n\n");
            fprintf(stream, "You bind a fragment of your consciousness to a physical vessel.\n");
            fprintf(stream, "The artifact pulses with dark energy.\n\n");
            fprintf(stream, "Cost:\n");
            fprintf(stream, "  Soul Energy: -%u (remaining: %u)\n",
                    cost, g_game_state->resources.soul_energy);
            fprintf(stream, "  Corruption: +%u%% (now: %u%%)\n",
                    corruption_cost, g_game_state->corruption.corruption);
            fprintf(stream, "\nEffect: Death resistance increased (not yet implemented)\n");
        }

    } else if (strcasecmp(ritual_type, "archon_trial") == 0) {
        /* Handle archon_trial <number> command */
        if (cmd->arg_count < 2) {
            fprintf(stream, "=== Archon Trials ===\n\n");
            fprintf(stream, "The path to Archon transformation requires completion of seven trials:\n\n");

            fprintf(stream, "1. Trial of Power - Combat mastery and restraint\n");
            fprintf(stream, "2. Trial of Wisdom - Ethical routing and justice\n");
            fprintf(stream, "3. Trial of Morality - Sacrifice power for principle\n");
            fprintf(stream, "4. Trial of Technical Skill - Death Network debugging\n");
            fprintf(stream, "5. Trial of Resolve - Corruption resistance (30 days)\n");
            fprintf(stream, "6. Trial of Sacrifice - Personal loss for others\n");
            fprintf(stream, "7. Trial of Leadership - Reform Regional Council\n\n");

            fprintf(stream, "Usage: ritual archon_trial <number>\n");
            fprintf(stream, "Example: ritual archon_trial 1\n\n");

            if (g_game_state->archon_trials) {
                fprintf(stream, "Trial Status:\n");
                for (uint32_t i = 1; i <= 7; i++) {
                    const ArchonTrial* trial = archon_trial_get_by_number(g_game_state->archon_trials, i);
                    if (trial) {
                        fprintf(stream, "  Trial %u: %s\n", i, archon_trial_status_name(trial->status));
                    }
                }
            } else {
                fprintf(stream, "Note: Trial manager not initialized. Trials unavailable.\n");
            }
        } else {
            /* Parse trial number */
            const char* trial_num_str = parsed_command_get_arg(cmd, 1);
            int trial_num = atoi(trial_num_str);

            if (trial_num < 1 || trial_num > 7) {
                fprintf(stream, "[ERROR] Invalid trial number: %d\n", trial_num);
                fprintf(stream, "Valid trials are 1-7.\n");
                fclose(stream);
                CommandResult err_result = command_result_error(EXEC_ERROR_INVALID_COMMAND, output);
                free(output);
                return err_result;
            }

            if (!g_game_state->archon_trials) {
                fprintf(stream, "[ERROR] Trial manager not initialized.\n");
                fprintf(stream, "Archon trials are not available in this game state.\n");
                fclose(stream);
                CommandResult err_result = command_result_error(EXEC_ERROR_INTERNAL, output);
                free(output);
                return err_result;
            }

            /* Check if trial is unlocked */
            const ArchonTrial* trial = archon_trial_get_by_number(g_game_state->archon_trials, trial_num);
            if (!trial) {
                fprintf(stream, "[ERROR] Trial %d not found.\n", trial_num);
                fclose(stream);
                CommandResult err_result = command_result_error(EXEC_ERROR_INTERNAL, output);
                free(output);
                return err_result;
            }

            if (trial->status == TRIAL_STATUS_LOCKED) {
                fprintf(stream, "[ERROR] Trial %d is locked.\n", trial_num);
                fprintf(stream, "Complete previous trials to unlock this one.\n");
                fclose(stream);
                CommandResult err_result = command_result_error(EXEC_ERROR_INVALID_COMMAND, output);
                free(output);
                return err_result;
            }

            if (trial->status == TRIAL_STATUS_PASSED) {
                fprintf(stream, "[INFO] Trial %d already completed (score: %.1f).\n",
                        trial_num, trial->best_score);
                fclose(stream);
                CommandResult result = command_result_success(output);
                free(output);
                return result;
            }

            if (trial->status == TRIAL_STATUS_FAILED) {
                fprintf(stream, "[ERROR] Trial %d permanently failed.\n", trial_num);
                fprintf(stream, "You cannot retry this trial.\n");
                fclose(stream);
                CommandResult err_result = command_result_error(EXEC_ERROR_INVALID_COMMAND, output);
                free(output);
                return err_result;
            }

            /* Close stream before UI (ncurses needs exclusive terminal access) */
            fclose(stream);
            free(output);
            output = NULL;

            /* Mark trial as in progress */
            archon_trial_start(g_game_state->archon_trials, trial->id);

            /* Run the trial UI */
            #include "../../game/narrative/trials/trial_ui_handlers.h"

            TrialUIResult ui_result = TRIAL_UI_ERROR;

            switch (trial_num) {
                case 1:
                    ui_result = trial_ui_run_power(g_game_state);
                    break;
                case 2:
                    ui_result = trial_ui_run_wisdom(g_game_state);
                    break;
                case 3:
                    ui_result = trial_ui_run_morality(g_game_state);
                    break;
                case 4:
                    ui_result = trial_ui_run_technical(g_game_state);
                    break;
                case 5:
                    ui_result = trial_ui_run_resolve(g_game_state);
                    break;
                case 6:
                    ui_result = trial_ui_run_sacrifice(g_game_state);
                    break;
                case 7:
                    ui_result = trial_ui_run_leadership(g_game_state);
                    break;
                default:
                    ui_result = TRIAL_UI_ERROR;
                    break;
            }

            /* Return result based on UI outcome */
            switch (ui_result) {
                case TRIAL_UI_PASSED:
                    return command_result_success("Trial completed successfully.");
                case TRIAL_UI_FAILED:
                    return command_result_error(EXEC_ERROR_INVALID_COMMAND,
                                                 "Trial failed. Archon path may be compromised.");
                case TRIAL_UI_ABORTED:
                    return command_result_error(EXEC_ERROR_INVALID_COMMAND,
                                                 "Trial aborted by player.");
                case TRIAL_UI_ERROR:
                default:
                    return command_result_error(EXEC_ERROR_INTERNAL,
                                                 "Trial system error occurred.");
            }
        }

    } else if (strcasecmp(ritual_type, "trial") == 0) {
        fprintf(stream, "Did you mean 'ritual archon_trial <number>'?\n\n");
        fprintf(stream, "Usage: ritual archon_trial <1-7>\n");
        fprintf(stream, "Example: ritual archon_trial 1\n\n");
        fprintf(stream, "Use 'ritual archon_trial' (no number) to see trial status.\n");

    } else if (strcasecmp(ritual_type, "purification") == 0) {
        fprintf(stream, "You attempt a Purification Ritual...\n\n");

        uint32_t mana_cost = 100;
        uint8_t corruption_reduction = 5;

        if (!resources_has_mana(&g_game_state->resources, mana_cost)) {
            fprintf(stream, "[FAILED] Insufficient mana!\n");
            fprintf(stream, "Required: %u\n", mana_cost);
            fprintf(stream, "Available: %u\n\n", g_game_state->resources.mana);
        } else if (g_game_state->corruption.corruption < corruption_reduction) {
            fprintf(stream, "[UNNECESSARY] You are already pure.\n");
            fprintf(stream, "Current corruption: %u%%\n", g_game_state->corruption.corruption);
        } else {
            resources_spend_mana(&g_game_state->resources, mana_cost);

            /* Reduce corruption */
            if (g_game_state->corruption.corruption >= corruption_reduction) {
                g_game_state->corruption.corruption -= corruption_reduction;
            } else {
                g_game_state->corruption.corruption = 0;
            }

            fprintf(stream, "[SUCCESS] Purification complete!\n\n");
            fprintf(stream, "Divine light burns away corruption.\n");
            fprintf(stream, "Your consciousness clears slightly.\n\n");
            fprintf(stream, "Cost:\n");
            fprintf(stream, "  Mana: -%u (remaining: %u)\n",
                    mana_cost, g_game_state->resources.mana);
            fprintf(stream, "  Corruption: -%u%% (now: %u%%)\n",
                    corruption_reduction, g_game_state->corruption.corruption);
        }

    } else if (strcasecmp(ritual_type, "offering") == 0) {
        fprintf(stream, "You perform a Divine Offering Ritual...\n\n");

        uint32_t offering = 100;

        if (!resources_has_soul_energy(&g_game_state->resources, offering)) {
            fprintf(stream, "[FAILED] Insufficient soul energy!\n");
            fprintf(stream, "Required: %u\n", offering);
            fprintf(stream, "Available: %u\n\n", g_game_state->resources.soul_energy);
        } else {
            resources_spend_soul_energy(&g_game_state->resources, offering);

            fprintf(stream, "[SUCCESS] Offering accepted!\n\n");
            fprintf(stream, "Soul energy flows upward into the divine realms.\n");
            fprintf(stream, "The gods take notice.\n\n");
            fprintf(stream, "Offering: %u soul energy\n", offering);
            fprintf(stream, "Remaining: %u\n", g_game_state->resources.soul_energy);
            fprintf(stream, "\nDivine Favor: +%u (not yet implemented)\n", offering / 10);
        }

    } else {
        fprintf(stream, "Unknown ritual type: '%s'\n\n", ritual_type);
        fprintf(stream, "Available rituals:\n");
        fprintf(stream, "  phylactery     - Create a phylactery (immortality vessel)\n");
        fprintf(stream, "  trial          - Attempt a Trial of Ascension\n");
        fprintf(stream, "  purification   - Reduce corruption (-5%%, costs 100 mana)\n");
        fprintf(stream, "  offering       - Offer soul energy to the gods\n");
        fprintf(stream, "  ascension      - (Requires all 7 trials completed)\n");
        fprintf(stream, "\nNote: Full ritual system integration pending.\n");
    }

    fprintf(stream, "\n--- Current State ---\n");
    fprintf(stream, "Day: %u\n", g_game_state->resources.day_count);
    fprintf(stream, "Soul Energy: %u\n", g_game_state->resources.soul_energy);
    fprintf(stream, "Mana: %u\n", g_game_state->resources.mana);
    fprintf(stream, "Corruption: %u%%\n", g_game_state->corruption.corruption);
    fprintf(stream, "Consciousness: %.1f%%\n", g_game_state->consciousness.stability);

    fclose(stream);

    CommandResult result = command_result_success(output);
    free(output);
    return result;
}

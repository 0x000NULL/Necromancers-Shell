/* POSIX features (open_memstream, localtime, gettimeofday) */
#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE

#include "commands.h"
#include "../../game/game_state.h"
#include "../../game/resources/resources.h"
#include "../../game/resources/corruption.h"
#include "../../game/souls/soul_manager.h"
#include "../../game/minions/minion_manager.h"
#include "../../game/world/location.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

/* External references to game systems */
extern GameState* g_game_state;

CommandResult cmd_status(ParsedCommand* cmd) {
    if (!cmd) {
        return command_result_error(EXEC_ERROR_INVALID_COMMAND, "Invalid command");
    }

    bool verbose = parsed_command_has_flag(cmd, "verbose");

    /* Buffer for building status output */
    char* output = NULL;
    size_t output_size = 0;
    FILE* stream = open_memstream(&output, &output_size);
    if (!stream) {
        return command_result_error(EXEC_ERROR_INTERNAL, "Failed to create output buffer");
    }

    fprintf(stream, "\n=== Necromancer's Shell - Status ===\n\n");

    /* Player Info */
    if (g_game_state) {
        fprintf(stream, "Player Level: %u\n", g_game_state->player_level);
        fprintf(stream, "Experience: %lu\n", g_game_state->player_experience);
    } else {
        fprintf(stream, "Game State: Not initialized\n");
    }

    fprintf(stream, "\n=== Resources ===\n");
    if (g_game_state) {
        fprintf(stream, "Soul Energy: %u\n", g_game_state->resources.soul_energy);
        fprintf(stream, "Mana: %u/%u\n",
                g_game_state->resources.mana,
                g_game_state->resources.mana_max);

        char time_buf[64];
        resources_format_time(&g_game_state->resources, time_buf, sizeof(time_buf));
        fprintf(stream, "Time: %s (Day %u)\n",
                time_buf, g_game_state->resources.day_count);
    } else {
        fprintf(stream, "Soul Energy: N/A\n");
        fprintf(stream, "Mana: N/A\n");
        fprintf(stream, "Time: N/A\n");
    }

    fprintf(stream, "\n=== Corruption ===\n");
    if (g_game_state) {
        fprintf(stream, "Level: %u%% (%s)\n",
                g_game_state->corruption.corruption,
                corruption_level_name(corruption_get_level(&g_game_state->corruption)));
    } else {
        fprintf(stream, "Level: N/A\n");
    }

    fprintf(stream, "\n=== Location ===\n");
    if (g_game_state) {
        Location* loc = game_state_get_current_location(g_game_state);
        if (loc) {
            fprintf(stream, "%s (%s)\n", loc->name, location_type_name(loc->type));
            fprintf(stream, "Control: %u%% | Corpses: %u\n",
                    loc->control_level, loc->corpse_count);
            if (loc->discovered) {
                fprintf(stream, "Status: Discovered\n");
            }
        } else {
            fprintf(stream, "Current Location: Unknown\n");
        }
    } else {
        fprintf(stream, "Current Location: N/A\n");
    }

    fprintf(stream, "\n=== Army ===\n");
    if (g_game_state && g_game_state->minions) {
        size_t minion_count = minion_manager_count(g_game_state->minions);
        fprintf(stream, "Minions: %zu\n", minion_count);

        /* Count by type */
        if (minion_count > 0 && verbose) {
            fprintf(stream, "  Zombies: %zu\n", minion_manager_count_by_type(g_game_state->minions, MINION_TYPE_ZOMBIE));
            fprintf(stream, "  Skeletons: %zu\n", minion_manager_count_by_type(g_game_state->minions, MINION_TYPE_SKELETON));
            fprintf(stream, "  Ghouls: %zu\n", minion_manager_count_by_type(g_game_state->minions, MINION_TYPE_GHOUL));
            fprintf(stream, "  Wraiths: %zu\n", minion_manager_count_by_type(g_game_state->minions, MINION_TYPE_WRAITH));
            fprintf(stream, "  Wights: %zu\n", minion_manager_count_by_type(g_game_state->minions, MINION_TYPE_WIGHT));
            fprintf(stream, "  Revenants: %zu\n", minion_manager_count_by_type(g_game_state->minions, MINION_TYPE_REVENANT));
        }
    } else {
        fprintf(stream, "Minions: N/A\n");
    }

    fprintf(stream, "\n=== Soul Collection ===\n");
    if (g_game_state && g_game_state->souls) {
        size_t soul_count = soul_manager_count(g_game_state->souls);
        uint32_t total_energy = soul_manager_total_energy(g_game_state->souls);
        fprintf(stream, "Total Souls: %zu (energy: %u)\n", soul_count, total_energy);

        /* Count by type */
        if (soul_count > 0 && verbose && g_game_state && g_game_state->souls) {
            fprintf(stream, "  Common: %zu\n", soul_manager_count_by_type(g_game_state->souls, SOUL_TYPE_COMMON));
            fprintf(stream, "  Warrior: %zu\n", soul_manager_count_by_type(g_game_state->souls, SOUL_TYPE_WARRIOR));
            fprintf(stream, "  Mage: %zu\n", soul_manager_count_by_type(g_game_state->souls, SOUL_TYPE_MAGE));
            fprintf(stream, "  Innocent: %zu\n", soul_manager_count_by_type(g_game_state->souls, SOUL_TYPE_INNOCENT));
            fprintf(stream, "  Corrupted: %zu\n", soul_manager_count_by_type(g_game_state->souls, SOUL_TYPE_CORRUPTED));
            fprintf(stream, "  Ancient: %zu\n", soul_manager_count_by_type(g_game_state->souls, SOUL_TYPE_ANCIENT));
        }
    } else {
        fprintf(stream, "Total Souls: N/A\n");
    }

    if (!verbose) {
        fprintf(stream, "\nType 'status --verbose' for detailed information.\n");
    }

    /* System info in verbose mode */
    if (verbose) {
        fprintf(stream, "\n=== System Info ===\n");

        time_t now = time(NULL);
        struct tm* timeinfo = localtime(&now);
        char time_str[64];
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", timeinfo);
        fprintf(stream, "System Time: %s\n", time_str);

        struct timeval tv;
        gettimeofday(&tv, NULL);
        double uptime = (double)tv.tv_sec + (double)tv.tv_usec / 1000000.0;
        fprintf(stream, "Uptime: %.2f seconds\n", uptime);
    }

    fclose(stream);
    CommandResult result = command_result_success(output);
    free(output);
    return result;
}

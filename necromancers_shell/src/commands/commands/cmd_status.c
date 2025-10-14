/* POSIX features (open_memstream, localtime, gettimeofday) */
#define _POSIX_C_SOURCE 200809L

#include "commands.h"
#include "../../core/state_manager.h"
#include "../../core/timing.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

/* External references to game systems */
extern StateManager* g_state_manager;

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

    /* Game State */
    if (g_state_manager) {
        GameState current = state_manager_current(g_state_manager);
        fprintf(stream, "Game State: %s\n", state_manager_state_name(current));
        fprintf(stream, "State Depth: %zu\n", state_manager_depth(g_state_manager));
    } else {
        fprintf(stream, "Game State: Unknown (state manager not initialized)\n");
    }

    fprintf(stream, "\n");

    /* System Time */
    time_t now = time(NULL);
    struct tm* timeinfo = localtime(&now);
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", timeinfo);
    fprintf(stream, "System Time: %s\n", time_str);

    /* Uptime (if timing system available) */
    struct timeval tv;
    gettimeofday(&tv, NULL);
    double uptime = (double)tv.tv_sec + (double)tv.tv_usec / 1000000.0;
    fprintf(stream, "System Uptime: %.2f seconds\n", uptime);

    if (verbose) {
        fprintf(stream, "\n--- Verbose Information ---\n\n");

        /* Memory information (placeholder for now) */
        fprintf(stream, "Memory:\n");
        fprintf(stream, "  Total Allocated: N/A (tracking not yet implemented)\n");
        fprintf(stream, "  Active Allocations: N/A\n");
        fprintf(stream, "\n");

        /* Command system info */
        fprintf(stream, "Command System:\n");
        fprintf(stream, "  Status: Active\n");
        fprintf(stream, "  History: Enabled\n");
        fprintf(stream, "  Autocomplete: Enabled\n");
        fprintf(stream, "\n");
    }

    fprintf(stream, "Type 'status --verbose' for detailed information.\n");

    fclose(stream);
    CommandResult result = command_result_success(output);
    free(output);
    return result;
}

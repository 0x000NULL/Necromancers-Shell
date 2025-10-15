/**
 * @file cmd_message.c
 * @brief Message command implementation
 *
 * Send messages to NPCs (Regional Council, Thessara, gods).
 */

#define _GNU_SOURCE
#include "commands.h"
#include "../../game/game_state.h"
#include "../../game/game_globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

CommandResult cmd_message(ParsedCommand* cmd) {
    if (!g_game_state) {
        return command_result_error(EXEC_ERROR_INTERNAL,
                                     "Game state not initialized");
    }

    /* Get NPC ID argument */
    if (cmd->arg_count < 1) {
        return command_result_error(EXEC_ERROR_INVALID_COMMAND,
            "Missing NPC ID. Usage: message <npc_id> <message>\n"
            "Available NPCs: vorgath, seraphine, mordak, echo, whisper, archivist, thessara");
    }

    const char* npc_id = parsed_command_get_arg(cmd, 0);

    /* Get message argument */
    if (cmd->arg_count < 2) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg),
            "Missing message. Usage: message %s <message>", npc_id);
        return command_result_error(EXEC_ERROR_INVALID_COMMAND, error_msg);
    }

    const char* message = parsed_command_get_arg(cmd, 1);

    /* Build output */
    char* output = NULL;
    size_t output_size = 0;
    FILE* stream = open_memstream(&output, &output_size);
    if (!stream) {
        return command_result_error(EXEC_ERROR_INTERNAL,
                                     "Failed to allocate output buffer");
    }

    fprintf(stream, "=== Message Sent ===\n\n");
    fprintf(stream, "To: %s\n", npc_id);
    fprintf(stream, "Message: \"%s\"\n\n", message);

    /* Simulate NPC responses based on ID */
    if (strcasecmp(npc_id, "thessara") == 0) {
        fprintf(stream, "[Thessara's consciousness flickers in the void]\n\n");
        fprintf(stream, "\"I hear you, little necromancer. Your message has been received.\n");
        fprintf(stream, "The paths ahead are many, but few lead to redemption.\n");
        fprintf(stream, "Choose carefully.\"\n\n");
        fprintf(stream, "Trust level: Not yet implemented\n");
    } else if (strcasecmp(npc_id, "vorgath") == 0) {
        fprintf(stream, "[Vorgath the Unyielding responds with characteristic bluntness]\n\n");
        fprintf(stream, "\"Speak plainly, %s. I have armies to command and territories\n",
                g_game_state->player_name[0] ? g_game_state->player_name : "necromancer");
        fprintf(stream, "to conquer. Your message is noted.\"\n\n");
        fprintf(stream, "Alliance status: Not yet implemented\n");
    } else if (strcasecmp(npc_id, "seraphine") == 0) {
        fprintf(stream, "[Seraphine the Pure considers your words carefully]\n\n");
        fprintf(stream, "\"I sense the weight of your choices in your words.\n");
        fprintf(stream, "Every soul you take dims the light within you.\n");
        fprintf(stream, "Yet I sense you still seek balance. That gives me hope.\"\n\n");
        fprintf(stream, "Alliance status: Not yet implemented\n");
    } else {
        fprintf(stream, "[No response. NPC '%s' not found or unavailable]\n\n", npc_id);
        fprintf(stream, "Available NPCs:\n");
        fprintf(stream, "  Regional Council: vorgath, seraphine, mordak, echo, whisper, archivist\n");
        fprintf(stream, "  Special: thessara (requires discovery)\n");
        fprintf(stream, "  Gods: anara, keldrin, theros, myrith, vorathos, seraph, nexus\n");
        fprintf(stream, "\nNote: Full NPC system integration pending.\n");
    }

    fclose(stream);

    CommandResult result = command_result_success(output);
    free(output);
    return result;
}

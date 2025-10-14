#include "commands.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

CommandResult cmd_clear(ParsedCommand* cmd) {
    (void)cmd; /* Unused */

    /* Use ANSI escape codes to clear screen */
    /* \033[2J clears screen, \033[H moves cursor to home */
    if (isatty(STDOUT_FILENO)) {
        printf("\033[2J\033[H");
        fflush(stdout);
        return command_result_success(NULL);
    } else {
        return command_result_error(EXEC_ERROR_COMMAND_FAILED,
                                   "Cannot clear: not a terminal");
    }
}

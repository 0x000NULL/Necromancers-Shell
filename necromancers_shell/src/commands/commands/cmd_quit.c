#include "commands.h"
#include <stdio.h>
#include <stdlib.h>

CommandResult cmd_quit(ParsedCommand* cmd) {
    (void)cmd; /* Unused */

    const char* farewell = "\nFarewell, Necromancer. The shadows await your return...\n";
    return command_result_exit(farewell);
}

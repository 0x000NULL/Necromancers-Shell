/**
 * Version implementation for Necromancer's Shell
 * Provides runtime access to version information
 */

#include "core/version.h"
#include "core/version_info.h"
#include <stdio.h>

const char* version_get_string(void) {
    return VERSION_STRING;
}

int version_get_major(void) {
    return VERSION_MAJOR;
}

int version_get_minor(void) {
    return VERSION_MINOR;
}

int version_get_patch(void) {
    return VERSION_PATCH;
}

const char* version_get_build_date(void) {
    return VERSION_BUILD_DATE;
}

const char* version_get_git_hash(void) {
    return VERSION_GIT_HASH;
}

void version_print_full(FILE* stream) {
    fprintf(stream, "Necromancer's Shell v%s\n", VERSION_STRING);
    fprintf(stream, "Build Date: %s\n", VERSION_BUILD_DATE);
    fprintf(stream, "Git Hash: %s\n", VERSION_GIT_HASH);
}

void version_print_short(FILE* stream) {
    fprintf(stream, "v%s\n", VERSION_STRING);
}

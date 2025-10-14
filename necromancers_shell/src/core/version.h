/**
 * Version API for Necromancer's Shell
 * Provides runtime access to version information
 */

#ifndef NECROMANCERS_VERSION_H
#define NECROMANCERS_VERSION_H

#include <stdio.h>

/**
 * Get the version string (e.g., "0.3.0")
 * @return Version string
 */
const char* version_get_string(void);

/**
 * Get the major version number
 * @return Major version
 */
int version_get_major(void);

/**
 * Get the minor version number
 * @return Minor version
 */
int version_get_minor(void);

/**
 * Get the patch version number
 * @return Patch version
 */
int version_get_patch(void);

/**
 * Get the build date
 * @return Build date string
 */
const char* version_get_build_date(void);

/**
 * Get the git commit hash
 * @return Git hash string
 */
const char* version_get_git_hash(void);

/**
 * Print full version information to stream
 * @param stream Output stream (stdout, stderr, etc.)
 */
void version_print_full(FILE* stream);

/**
 * Print short version to stream
 * @param stream Output stream
 */
void version_print_short(FILE* stream);

#endif /* NECROMANCERS_VERSION_H */

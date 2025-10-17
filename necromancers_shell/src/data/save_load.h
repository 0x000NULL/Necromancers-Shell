/**
 * @file save_load.h
 * @brief Save/Load system for Necromancer's Shell
 *
 * Provides binary serialization and deserialization of GameState.
 * Save files use a custom binary format with version checking and CRC32 validation.
 */

#ifndef SAVE_LOAD_H
#define SAVE_LOAD_H

#include "../game/game_state.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Magic number identifying save files ("NECR")
 */
#define SAVE_MAGIC_NUMBER 0x5243454E  /* "NECR" in little-endian */

/**
 * @brief Current save file format version
 */
#define SAVE_VERSION_MAJOR 1
#define SAVE_VERSION_MINOR 0
#define SAVE_VERSION_PATCH 0

/**
 * @brief Maximum error message length
 */
#define SAVE_ERROR_MAX_LENGTH 256

/**
 * @brief Default save file path (in user home directory)
 */
#define DEFAULT_SAVE_PATH "~/.necromancers_shell_save.dat"

/**
 * @brief Save file header structure
 *
 * All multi-byte integers are stored in little-endian format
 * for cross-platform compatibility.
 */
typedef struct {
    uint32_t magic;          /* Magic number (0x5243454E = "NECR") */
    uint8_t version_major;   /* Major version number */
    uint8_t version_minor;   /* Minor version number */
    uint8_t version_patch;   /* Patch version number */
    uint8_t reserved;        /* Reserved byte for alignment */
    uint32_t checksum;       /* CRC32 checksum of data section */
    uint64_t data_length;    /* Length of data section in bytes */
} SaveFileHeader;

/**
 * @brief Save game state to file
 *
 * Serializes the entire GameState to a binary file with version
 * checking and CRC32 validation. Creates a backup of existing save
 * before overwriting.
 *
 * The save file format:
 * - Header (20 bytes): magic, version, checksum, data_length
 * - Data section (variable): serialized GameState
 *
 * @param state Game state to save
 * @param filepath Path to save file (NULL = default ~/.necromancers_shell_save.dat)
 * @return true on success, false on failure
 */
bool save_game(const GameState* state, const char* filepath);

/**
 * @brief Load game state from file
 *
 * Deserializes a GameState from a binary save file. Validates magic number,
 * version compatibility, and CRC32 checksum before loading.
 *
 * @param filepath Path to save file (NULL = default ~/.necromancers_shell_save.dat)
 * @param error_buffer Buffer to write error message on failure (can be NULL)
 * @param error_size Size of error buffer
 * @return Newly allocated GameState on success, NULL on failure
 */
GameState* load_game(const char* filepath, char* error_buffer, size_t error_size);

/**
 * @brief Validate save file format
 *
 * Checks magic number, version compatibility, and CRC32 checksum
 * without fully loading the file.
 *
 * @param filepath Path to save file
 * @return true if file is valid, false otherwise
 */
bool validate_save_file(const char* filepath);

/**
 * @brief Save game metadata as JSON
 *
 * Writes a human-readable JSON file with save metadata (version,
 * timestamp, player stats, etc.). Useful for debugging and quick
 * save file inspection.
 *
 * @param state Game state to extract metadata from
 * @param filepath Path to JSON file
 * @return true on success, false on failure
 */
bool save_metadata_json(const GameState* state, const char* filepath);

/**
 * @brief Get default save file path
 *
 * Expands ~ to user home directory and returns absolute path.
 * Caller must free returned string.
 *
 * @return Allocated string with save path, or NULL on failure
 */
char* get_default_save_path(void);

/**
 * @brief Check if save file exists
 *
 * @param filepath Path to check (NULL = default path)
 * @return true if file exists, false otherwise
 */
bool save_file_exists(const char* filepath);

/**
 * @brief Get save file size in bytes
 *
 * @param filepath Path to save file (NULL = default path)
 * @return File size in bytes, or 0 on error
 */
size_t get_save_file_size(const char* filepath);

/**
 * @brief Create backup of save file
 *
 * Renames existing save to .bak before writing new save.
 *
 * @param filepath Path to save file
 * @return true on success or if no file exists, false on error
 */
bool backup_save_file(const char* filepath);

/**
 * @brief Version compatibility check
 *
 * Determines if a save file with given version can be loaded.
 * Currently accepts same major version with any minor/patch.
 *
 * @param major Major version from save file
 * @param minor Minor version from save file
 * @param patch Patch version from save file
 * @return true if compatible, false otherwise
 */
bool is_version_compatible(uint8_t major, uint8_t minor, uint8_t patch);

#endif /* SAVE_LOAD_H */

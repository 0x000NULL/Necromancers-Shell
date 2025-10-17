/**
 * @file save_load.c
 * @brief Implementation of save/load system
 */

/* POSIX features for fmemopen and strdup */
#define _POSIX_C_SOURCE 200809L

#include "save_load.h"
#include "../utils/logger.h"
#include "../game/minions/minion_manager.h"
#include "../game/world/territory.h"
#include "../game/world/location.h"
#include "../game/narrative/quests/quest_manager.h"
#include "../game/narrative/quests/quest.h"
#include "../game/narrative/quests/quest_objective.h"
#include "../game/narrative/npcs/npc_manager.h"
#include "../game/narrative/npcs/npc.h"
#include "../game/narrative/memory/memory_manager.h"
#include "../game/narrative/memory/memory_fragment.h"
#include "../game/narrative/relationships/relationship_manager.h"
#include "../game/narrative/relationships/relationship.h"
#include "../game/narrative/dialogue/dialogue_manager.h"
#include "../game/narrative/gods/divine_council.h"
#include "../game/narrative/gods/god.h"
#include "../game/narrative/thessara/thessara.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>

/* Forward declarations for helper functions */
static bool write_uint8(FILE* fp, uint8_t value);
static bool write_uint32(FILE* fp, uint32_t value);
static bool write_uint64(FILE* fp, uint64_t value);
static bool write_float(FILE* fp, float value);
static bool write_string(FILE* fp, const char* str, size_t max_len);
static bool write_bytes(FILE* fp, const void* data, size_t size);
static bool write_bool(FILE* fp, bool value);
static bool write_int16(FILE* fp, int16_t value);
static bool write_int(FILE* fp, int value);

static bool read_uint8(FILE* fp, uint8_t* value);
static bool read_uint32(FILE* fp, uint32_t* value);
static bool read_uint64(FILE* fp, uint64_t* value);
static bool read_float(FILE* fp, float* value);
static bool read_string(FILE* fp, char* buffer, size_t max_len);
static bool read_bytes(FILE* fp, void* data, size_t size);
static bool read_bool(FILE* fp, bool* value);
static bool read_int16(FILE* fp, int16_t* value);
static bool read_int(FILE* fp, int* value);

static bool write_soul_manager(FILE* fp, const SoulManager* mgr);
static bool write_minion_manager(FILE* fp, const MinionManager* mgr);
static bool write_resources(FILE* fp, const Resources* res);
static bool write_corruption(FILE* fp, const CorruptionState* cor);
static bool write_consciousness(FILE* fp, const ConsciousnessState* con);
static bool write_location(FILE* fp, const Location* loc);
static bool write_territory_manager(FILE* fp, const TerritoryManager* mgr);
static bool write_quest_manager(FILE* fp, const QuestManager* mgr);
static bool write_npc_manager(FILE* fp, const NPCManager* mgr);
static bool write_relationship_manager(FILE* fp, const RelationshipManager* mgr);
static bool write_memory_manager(FILE* fp, const MemoryManager* mgr);
static bool write_divine_council(FILE* fp, const DivineCouncil* council);
static bool write_thessara_relationship(FILE* fp, const ThessaraRelationship* thessara);

static SoulManager* read_soul_manager(FILE* fp);
static MinionManager* read_minion_manager(FILE* fp);
static bool read_resources(FILE* fp, Resources* res);
static bool read_corruption(FILE* fp, CorruptionState* cor);
static bool read_consciousness(FILE* fp, ConsciousnessState* con);
static Location* read_location(FILE* fp);
static TerritoryManager* read_territory_manager(FILE* fp);
static QuestManager* read_quest_manager(FILE* fp);
static NPCManager* read_npc_manager(FILE* fp);
static RelationshipManager* read_relationship_manager(FILE* fp);
static MemoryManager* read_memory_manager(FILE* fp);
static DivineCouncil* read_divine_council(FILE* fp);
static ThessaraRelationship* read_thessara_relationship(FILE* fp);

static uint32_t calculate_crc32(const void* data, size_t length);
static char* expand_home_directory(const char* path);

/* CRC32 table for checksum calculation */
static uint32_t crc32_table[256];
static bool crc32_table_initialized = false;

static void init_crc32_table(void) {
    if (crc32_table_initialized) {
        return;
    }

    for (uint32_t i = 0; i < 256; i++) {
        uint32_t crc = i;
        for (int j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xEDB88320;
            } else {
                crc >>= 1;
            }
        }
        crc32_table[i] = crc;
    }
    crc32_table_initialized = true;
}

static uint32_t calculate_crc32(const void* data, size_t length) {
    init_crc32_table();

    uint32_t crc = 0xFFFFFFFF;
    const uint8_t* bytes = (const uint8_t*)data;

    for (size_t i = 0; i < length; i++) {
        uint8_t index = (crc ^ bytes[i]) & 0xFF;
        crc = (crc >> 8) ^ crc32_table[index];
    }

    return crc ^ 0xFFFFFFFF;
}

char* get_default_save_path(void) {
    const char* home = getenv("HOME");
    if (!home) {
        struct passwd* pwd = getpwuid(getuid());
        if (pwd) {
            home = pwd->pw_dir;
        }
    }

    if (!home) {
        LOG_ERROR("Could not determine home directory");
        return NULL;
    }

    size_t len = strlen(home) + strlen("/.necromancers_shell_save.dat") + 1;
    char* path = malloc(len);
    if (!path) {
        return NULL;
    }

    snprintf(path, len, "%s/.necromancers_shell_save.dat", home);
    return path;
}

static char* expand_home_directory(const char* path) {
    if (!path || path[0] != '~') {
        return strdup(path);
    }

    const char* home = getenv("HOME");
    if (!home) {
        struct passwd* pwd = getpwuid(getuid());
        if (pwd) {
            home = pwd->pw_dir;
        }
    }

    if (!home) {
        return strdup(path);
    }

    size_t len = strlen(home) + strlen(path);
    char* expanded = malloc(len);
    if (!expanded) {
        return NULL;
    }

    snprintf(expanded, len, "%s%s", home, path + 1);
    return expanded;
}

bool save_file_exists(const char* filepath) {
    char* path = filepath ? expand_home_directory(filepath) : get_default_save_path();
    if (!path) {
        return false;
    }

    bool exists = (access(path, F_OK) == 0);
    free(path);
    return exists;
}

size_t get_save_file_size(const char* filepath) {
    char* path = filepath ? expand_home_directory(filepath) : get_default_save_path();
    if (!path) {
        return 0;
    }

    struct stat st;
    size_t size = 0;
    if (stat(path, &st) == 0) {
        size = (size_t)st.st_size;
    }

    free(path);
    return size;
}

bool backup_save_file(const char* filepath) {
    char* path = filepath ? expand_home_directory(filepath) : get_default_save_path();
    if (!path) {
        return false;
    }

    if (access(path, F_OK) != 0) {
        free(path);
        return true;  /* No file to backup */
    }

    size_t backup_len = strlen(path) + 5;  /* +4 for ".bak" +1 for null */
    char* backup_path = malloc(backup_len);
    if (!backup_path) {
        free(path);
        return false;
    }

    snprintf(backup_path, backup_len, "%s.bak", path);

    bool success = (rename(path, backup_path) == 0);
    if (!success) {
        LOG_WARN("Failed to create backup: %s", strerror(errno));
    }

    free(backup_path);
    free(path);
    return success;
}

bool is_version_compatible(uint8_t major, uint8_t minor, uint8_t patch) {
    (void)minor;
    (void)patch;

    /* Currently only accept same major version */
    return (major == SAVE_VERSION_MAJOR);
}

/* Basic I/O helpers */

static bool write_uint8(FILE* fp, uint8_t value) {
    return fwrite(&value, sizeof(uint8_t), 1, fp) == 1;
}

static bool write_uint32(FILE* fp, uint32_t value) {
    /* Write in little-endian */
    uint8_t bytes[4];
    bytes[0] = value & 0xFF;
    bytes[1] = (value >> 8) & 0xFF;
    bytes[2] = (value >> 16) & 0xFF;
    bytes[3] = (value >> 24) & 0xFF;
    return fwrite(bytes, 4, 1, fp) == 1;
}

static bool write_uint64(FILE* fp, uint64_t value) {
    /* Write in little-endian */
    uint8_t bytes[8];
    for (int i = 0; i < 8; i++) {
        bytes[i] = (value >> (i * 8)) & 0xFF;
    }
    return fwrite(bytes, 8, 1, fp) == 1;
}

static bool write_float(FILE* fp, float value) {
    return fwrite(&value, sizeof(float), 1, fp) == 1;
}

static bool write_bool(FILE* fp, bool value) {
    uint8_t byte = value ? 1 : 0;
    return write_uint8(fp, byte);
}

static bool write_int16(FILE* fp, int16_t value) {
    /* Write as little-endian */
    uint8_t bytes[2];
    bytes[0] = value & 0xFF;
    bytes[1] = (value >> 8) & 0xFF;
    return fwrite(bytes, 2, 1, fp) == 1;
}

static bool write_int(FILE* fp, int value) {
    int32_t val32 = (int32_t)value;
    /* Write as little-endian */
    uint8_t bytes[4];
    for (int i = 0; i < 4; i++) {
        bytes[i] = (val32 >> (i * 8)) & 0xFF;
    }
    return fwrite(bytes, 4, 1, fp) == 1;
}

static bool write_string(FILE* fp, const char* str, size_t max_len) {
    if (!str) {
        /* Write null string as length 0 */
        return write_uint32(fp, 0);
    }

    uint32_t len = (uint32_t)strlen(str);
    if (len > max_len) {
        len = (uint32_t)max_len;
    }

    if (!write_uint32(fp, len)) {
        return false;
    }

    if (len > 0) {
        return fwrite(str, 1, len, fp) == len;
    }

    return true;
}

static bool write_bytes(FILE* fp, const void* data, size_t size) __attribute__((unused));
static bool write_bytes(FILE* fp, const void* data, size_t size) {
    if (!data || size == 0) {
        return true;
    }
    return fwrite(data, 1, size, fp) == size;
}

static bool read_uint8(FILE* fp, uint8_t* value) {
    return fread(value, sizeof(uint8_t), 1, fp) == 1;
}

static bool read_uint32(FILE* fp, uint32_t* value) {
    uint8_t bytes[4];
    if (fread(bytes, 4, 1, fp) != 1) {
        return false;
    }

    *value = bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);
    return true;
}

static bool read_uint64(FILE* fp, uint64_t* value) {
    uint8_t bytes[8];
    if (fread(bytes, 8, 1, fp) != 1) {
        return false;
    }

    *value = 0;
    for (int i = 0; i < 8; i++) {
        *value |= ((uint64_t)bytes[i]) << (i * 8);
    }
    return true;
}

static bool read_float(FILE* fp, float* value) {
    return fread(value, sizeof(float), 1, fp) == 1;
}

static bool read_bool(FILE* fp, bool* value) {
    uint8_t byte;
    if (!read_uint8(fp, &byte)) {
        return false;
    }
    *value = (byte != 0);
    return true;
}

static bool read_int16(FILE* fp, int16_t* value) {
    uint8_t bytes[2];
    if (fread(bytes, 2, 1, fp) != 1) {
        return false;
    }
    *value = (int16_t)(bytes[0] | (bytes[1] << 8));
    return true;
}

static bool read_int(FILE* fp, int* value) {
    uint8_t bytes[4];
    if (fread(bytes, 4, 1, fp) != 1) {
        return false;
    }
    int32_t val32 = bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);
    *value = (int)val32;
    return true;
}

static bool read_string(FILE* fp, char* buffer, size_t max_len) {
    uint32_t len;
    if (!read_uint32(fp, &len)) {
        return false;
    }

    if (len == 0) {
        buffer[0] = '\0';
        return true;
    }

    if (len >= max_len) {
        LOG_ERROR("String length %u exceeds buffer size %zu", len, max_len);
        return false;
    }

    if (fread(buffer, 1, len, fp) != len) {
        return false;
    }

    buffer[len] = '\0';
    return true;
}

static bool read_bytes(FILE* fp, void* data, size_t size) __attribute__((unused));
static bool read_bytes(FILE* fp, void* data, size_t size) {
    if (!data || size == 0) {
        return true;
    }
    return fread(data, 1, size, fp) == size;
}

/* Subsystem serialization */

static bool write_resources(FILE* fp, const Resources* res) {
    if (!res) {
        return false;
    }

    return write_uint32(fp, res->soul_energy) &&
           write_uint32(fp, res->mana) &&
           write_uint32(fp, res->mana_max) &&
           write_uint32(fp, res->day_count) &&
           write_uint32(fp, res->time_hours) &&
           write_uint32(fp, res->day_of_month) &&
           write_uint32(fp, res->month) &&
           write_uint32(fp, res->year);
}

static bool read_resources(FILE* fp, Resources* res) {
    if (!res) {
        return false;
    }

    return read_uint32(fp, &res->soul_energy) &&
           read_uint32(fp, &res->mana) &&
           read_uint32(fp, &res->mana_max) &&
           read_uint32(fp, &res->day_count) &&
           read_uint32(fp, &res->time_hours) &&
           read_uint32(fp, &res->day_of_month) &&
           read_uint32(fp, &res->month) &&
           read_uint32(fp, &res->year);
}

static bool write_corruption(FILE* fp, const CorruptionState* cor) {
    if (!cor) {
        return false;
    }

    if (!write_uint8(fp, cor->corruption)) {
        return false;
    }

    if (!write_uint32(fp, (uint32_t)cor->event_count)) {
        return false;
    }

    for (size_t i = 0; i < cor->event_count; i++) {
        if (!write_string(fp, cor->events[i].description, 128)) {
            return false;
        }
        if (!write_uint8(fp, (uint8_t)cor->events[i].change)) {
            return false;
        }
        if (!write_uint32(fp, cor->events[i].day)) {
            return false;
        }
    }

    return true;
}

static bool read_corruption(FILE* fp, CorruptionState* cor) {
    if (!cor) {
        return false;
    }

    if (!read_uint8(fp, &cor->corruption)) {
        return false;
    }

    uint32_t count;
    if (!read_uint32(fp, &count)) {
        return false;
    }

    cor->event_count = (size_t)count;
    if (cor->event_count > MAX_CORRUPTION_EVENTS) {
        cor->event_count = MAX_CORRUPTION_EVENTS;
    }

    for (size_t i = 0; i < cor->event_count; i++) {
        if (!read_string(fp, cor->events[i].description, 128)) {
            return false;
        }
        uint8_t change_byte;
        if (!read_uint8(fp, &change_byte)) {
            return false;
        }
        cor->events[i].change = (int8_t)change_byte;
        if (!read_uint32(fp, &cor->events[i].day)) {
            return false;
        }
    }

    return true;
}

static bool write_consciousness(FILE* fp, const ConsciousnessState* con) {
    if (!con) {
        return false;
    }

    return write_float(fp, con->stability) &&
           write_float(fp, con->decay_rate) &&
           write_uint32(fp, con->months_until_critical) &&
           write_float(fp, con->fragmentation_level) &&
           write_bool(fp, con->approaching_wraith) &&
           write_uint32(fp, con->last_decay_month);
}

static bool read_consciousness(FILE* fp, ConsciousnessState* con) {
    if (!con) {
        return false;
    }

    return read_float(fp, &con->stability) &&
           read_float(fp, &con->decay_rate) &&
           read_uint32(fp, &con->months_until_critical) &&
           read_float(fp, &con->fragmentation_level) &&
           read_bool(fp, &con->approaching_wraith) &&
           read_uint32(fp, &con->last_decay_month);
}

static bool write_soul_manager(FILE* fp, const SoulManager* mgr) {
    if (!mgr) {
        /* Write null marker */
        return write_uint32(fp, 0);
    }

    size_t count = soul_manager_count((SoulManager*)mgr);
    if (!write_uint32(fp, (uint32_t)count)) {
        return false;
    }

    /* Get all souls */
    Soul** souls = soul_manager_get_filtered((SoulManager*)mgr, NULL, &count);
    if (count > 0 && !souls) {
        return false;
    }

    for (size_t i = 0; i < count; i++) {
        Soul* soul = souls[i];

        /* Write Soul struct fields */
        if (!write_uint32(fp, soul->id) ||
            !write_uint32(fp, (uint32_t)soul->type) ||
            !write_uint8(fp, soul->quality) ||
            !write_string(fp, soul->memories, SOUL_MEMORY_MAX_LENGTH) ||
            !write_uint32(fp, soul->energy) ||
            !write_bool(fp, soul->bound) ||
            !write_uint32(fp, soul->bound_minion_id) ||
            !write_uint64(fp, (uint64_t)soul->timestamp)) {
            free(souls);
            return false;
        }
    }

    free(souls);
    return true;
}

static SoulManager* read_soul_manager(FILE* fp) {
    uint32_t count;
    if (!read_uint32(fp, &count)) {
        return NULL;
    }

    if (count == 0) {
        return NULL;  /* Null manager */
    }

    SoulManager* mgr = soul_manager_create();
    if (!mgr) {
        return NULL;
    }

    for (uint32_t i = 0; i < count; i++) {
        Soul* soul = malloc(sizeof(Soul));
        if (!soul) {
            soul_manager_destroy(mgr);
            return NULL;
        }

        uint32_t type_u32;
        uint64_t timestamp_u64;

        if (!read_uint32(fp, &soul->id) ||
            !read_uint32(fp, &type_u32) ||
            !read_uint8(fp, &soul->quality) ||
            !read_string(fp, soul->memories, SOUL_MEMORY_MAX_LENGTH) ||
            !read_uint32(fp, &soul->energy) ||
            !read_bool(fp, &soul->bound) ||
            !read_uint32(fp, &soul->bound_minion_id) ||
            !read_uint64(fp, &timestamp_u64)) {
            free(soul);
            soul_manager_destroy(mgr);
            return NULL;
        }

        soul->type = (SoulType)type_u32;
        soul->timestamp = (time_t)timestamp_u64;

        if (!soul_manager_add(mgr, soul)) {
            free(soul);
            soul_manager_destroy(mgr);
            return NULL;
        }
    }

    return mgr;
}

static bool write_minion_manager(FILE* fp, const MinionManager* mgr) {
    if (!mgr) {
        return write_uint32(fp, 0);
    }

    size_t count = minion_manager_count((MinionManager*)mgr);
    if (!write_uint32(fp, (uint32_t)count)) {
        return false;
    }

    for (size_t i = 0; i < count; i++) {
        Minion* minion = minion_manager_get_at((MinionManager*)mgr, i);
        if (!minion) {
            return false;
        }

        /* Write Minion struct fields */
        if (!write_uint32(fp, minion->id) ||
            !write_string(fp, minion->name, 64) ||
            !write_uint32(fp, (uint32_t)minion->type) ||
            !write_uint32(fp, minion->stats.health) ||
            !write_uint32(fp, minion->stats.health_max) ||
            !write_uint32(fp, minion->stats.attack) ||
            !write_uint32(fp, minion->stats.defense) ||
            !write_uint32(fp, minion->stats.speed) ||
            !write_uint8(fp, minion->stats.loyalty) ||
            !write_uint32(fp, minion->bound_soul_id) ||
            !write_uint32(fp, minion->location_id) ||
            !write_uint64(fp, minion->raised_timestamp) ||
            !write_uint32(fp, minion->experience) ||
            !write_uint8(fp, minion->level)) {
            return false;
        }
    }

    return true;
}

static MinionManager* read_minion_manager(FILE* fp) {
    uint32_t count;
    if (!read_uint32(fp, &count)) {
        return NULL;
    }

    if (count == 0) {
        return NULL;
    }

    MinionManager* mgr = minion_manager_create(count > 0 ? count : 10);
    if (!mgr) {
        return NULL;
    }

    for (uint32_t i = 0; i < count; i++) {
        Minion* minion = malloc(sizeof(Minion));
        if (!minion) {
            minion_manager_destroy(mgr);
            return NULL;
        }

        uint32_t type_u32;

        if (!read_uint32(fp, &minion->id) ||
            !read_string(fp, minion->name, 64) ||
            !read_uint32(fp, &type_u32) ||
            !read_uint32(fp, &minion->stats.health) ||
            !read_uint32(fp, &minion->stats.health_max) ||
            !read_uint32(fp, &minion->stats.attack) ||
            !read_uint32(fp, &minion->stats.defense) ||
            !read_uint32(fp, &minion->stats.speed) ||
            !read_uint8(fp, &minion->stats.loyalty) ||
            !read_uint32(fp, &minion->bound_soul_id) ||
            !read_uint32(fp, &minion->location_id) ||
            !read_uint64(fp, &minion->raised_timestamp) ||
            !read_uint32(fp, &minion->experience) ||
            !read_uint8(fp, &minion->level)) {
            free(minion);
            minion_manager_destroy(mgr);
            return NULL;
        }

        minion->type = (MinionType)type_u32;

        if (!minion_manager_add(mgr, minion)) {
            free(minion);
            minion_manager_destroy(mgr);
            return NULL;
        }
    }

    return mgr;
}

static bool write_location(FILE* fp, const Location* loc) {
    if (!loc) {
        return false;
    }

    /* Write Location fields */
    if (!write_uint32(fp, loc->id) ||
        !write_string(fp, loc->name, 64) ||
        !write_uint32(fp, (uint32_t)loc->type) ||
        !write_uint32(fp, (uint32_t)loc->status) ||
        !write_string(fp, loc->description, 512) ||
        !write_uint32(fp, loc->corpse_count) ||
        !write_uint32(fp, loc->soul_quality_avg) ||
        !write_uint8(fp, loc->control_level) ||
        !write_uint32(fp, loc->defense_strength) ||
        !write_bool(fp, loc->discovered) ||
        !write_uint64(fp, loc->discovered_timestamp)) {
        return false;
    }

    /* Write connections array */
    uint32_t conn_count = (uint32_t)loc->connection_count;
    if (!write_uint32(fp, conn_count)) {
        return false;
    }

    for (uint32_t i = 0; i < conn_count; i++) {
        if (!write_uint32(fp, loc->connected_ids[i])) {
            return false;
        }
    }

    return true;
}

static Location* read_location(FILE* fp) {
    uint32_t id;
    char name[64];
    uint32_t type_u32, status_u32;

    if (!read_uint32(fp, &id) ||
        !read_string(fp, name, 64) ||
        !read_uint32(fp, &type_u32) ||
        !read_uint32(fp, &status_u32)) {
        return NULL;
    }

    Location* loc = location_create(id, name, (LocationType)type_u32);
    if (!loc) {
        return NULL;
    }

    loc->status = (LocationStatus)status_u32;

    if (!read_string(fp, loc->description, 512) ||
        !read_uint32(fp, &loc->corpse_count) ||
        !read_uint32(fp, &loc->soul_quality_avg) ||
        !read_uint8(fp, &loc->control_level) ||
        !read_uint32(fp, &loc->defense_strength) ||
        !read_bool(fp, &loc->discovered) ||
        !read_uint64(fp, &loc->discovered_timestamp)) {
        location_destroy(loc);
        return NULL;
    }

    /* Read connections array */
    uint32_t conn_count;
    if (!read_uint32(fp, &conn_count)) {
        location_destroy(loc);
        return NULL;
    }

    for (uint32_t i = 0; i < conn_count; i++) {
        uint32_t connected_id;
        if (!read_uint32(fp, &connected_id)) {
            location_destroy(loc);
            return NULL;
        }
        if (!location_add_connection(loc, connected_id)) {
            location_destroy(loc);
            return NULL;
        }
    }

    return loc;
}

static bool write_territory_manager(FILE* fp, const TerritoryManager* mgr) {
    if (!mgr) {
        return write_uint32(fp, 0);
    }

    size_t count = territory_manager_count((TerritoryManager*)mgr);
    if (!write_uint32(fp, (uint32_t)count)) {
        return false;
    }

    /* Get all discovered locations */
    Location** results = NULL;
    size_t disc_count = 0;

    if (!territory_manager_get_discovered(mgr, &results, &disc_count)) {
        return false;
    }

    /* Write discovered count */
    if (!write_uint32(fp, (uint32_t)disc_count)) {
        territory_manager_free_results(results);
        return false;
    }

    /* Write each discovered location */
    for (size_t i = 0; i < disc_count; i++) {
        if (!write_location(fp, results[i])) {
            territory_manager_free_results(results);
            return false;
        }
    }

    territory_manager_free_results(results);
    return true;
}

static TerritoryManager* read_territory_manager(FILE* fp) {
    uint32_t count;
    if (!read_uint32(fp, &count)) {
        return NULL;
    }

    if (count == 0) {
        return NULL;
    }

    TerritoryManager* mgr = territory_manager_create();
    if (!mgr) {
        return NULL;
    }

    /* Read discovered count */
    uint32_t disc_count;
    if (!read_uint32(fp, &disc_count)) {
        territory_manager_destroy(mgr);
        return NULL;
    }

    /* Read each discovered location */
    for (uint32_t i = 0; i < disc_count; i++) {
        Location* loc = read_location(fp);
        if (!loc) {
            territory_manager_destroy(mgr);
            return NULL;
        }

        if (!territory_manager_add_location(mgr, loc)) {
            location_destroy(loc);
            territory_manager_destroy(mgr);
            return NULL;
        }
    }

    return mgr;
}
/* ==================== Quest System Serialization ==================== */

/* Write quest objective */
static bool write_quest_objective(FILE* fp, const QuestObjective* obj) {
    if (!obj) {
        return write_bool(fp, false);
    }

    if (!write_bool(fp, true)) return false;
    if (!write_string(fp, obj->id, sizeof(obj->id))) return false;
    if (!write_string(fp, obj->description, sizeof(obj->description))) return false;
    if (!write_uint32(fp, (uint32_t)obj->type)) return false;
    if (!write_string(fp, obj->target_id, sizeof(obj->target_id))) return false;
    if (!write_int(fp, obj->target_count)) return false;
    if (!write_int(fp, obj->current_count)) return false;
    if (!write_bool(fp, obj->completed)) return false;
    if (!write_bool(fp, obj->optional)) return false;
    if (!write_bool(fp, obj->hidden)) return false;
    if (!write_string(fp, obj->prerequisite_objective, sizeof(obj->prerequisite_objective))) return false;

    return true;
}

/* Read quest objective */
static QuestObjective* read_quest_objective(FILE* fp) {
    bool not_null;
    if (!read_bool(fp, &not_null)) return NULL;
    if (!not_null) return NULL;

    char id[64], description[256], target_id[64], prereq[64];
    uint32_t type_val;
    int target_count, current_count;
    bool completed, optional, hidden;

    if (!read_string(fp, id, sizeof(id))) return NULL;
    if (!read_string(fp, description, sizeof(description))) return NULL;
    if (!read_uint32(fp, &type_val)) return NULL;
    if (!read_string(fp, target_id, sizeof(target_id))) return NULL;
    if (!read_int(fp, &target_count)) return NULL;
    if (!read_int(fp, &current_count)) return NULL;
    if (!read_bool(fp, &completed)) return NULL;
    if (!read_bool(fp, &optional)) return NULL;
    if (!read_bool(fp, &hidden)) return NULL;
    if (!read_string(fp, prereq, sizeof(prereq))) return NULL;

    QuestObjective* obj = quest_objective_create(id, description, (ObjectiveType)type_val);
    if (!obj) return NULL;

    strncpy(obj->target_id, target_id, sizeof(obj->target_id) - 1);
    obj->target_id[sizeof(obj->target_id) - 1] = '\0';
    obj->target_count = target_count;
    obj->current_count = current_count;
    obj->completed = completed;
    obj->optional = optional;
    obj->hidden = hidden;
    strncpy(obj->prerequisite_objective, prereq, sizeof(obj->prerequisite_objective) - 1);
    obj->prerequisite_objective[sizeof(obj->prerequisite_objective) - 1] = '\0';

    return obj;
}

/* Write quest */
static bool write_quest(FILE* fp, const Quest* quest) {
    if (!quest) {
        return write_bool(fp, false);
    }

    if (!write_bool(fp, true)) return false;
    if (!write_string(fp, quest->id, sizeof(quest->id))) return false;
    if (!write_string(fp, quest->title, sizeof(quest->title))) return false;
    if (!write_string(fp, quest->description, sizeof(quest->description))) return false;
    if (!write_string(fp, quest->quest_giver, sizeof(quest->quest_giver))) return false;
    if (!write_uint32(fp, (uint32_t)quest->state)) return false;
    if (!write_uint64(fp, (uint64_t)quest->started_time)) return false;
    if (!write_uint64(fp, (uint64_t)quest->completed_time)) return false;

    /* Write objectives */
    if (!write_uint32(fp, (uint32_t)quest->objective_count)) return false;
    for (size_t i = 0; i < quest->objective_count; i++) {
        if (!write_quest_objective(fp, quest->objectives[i])) return false;
    }

    /* Write rewards */
    if (!write_int(fp, quest->soul_energy_reward)) return false;
    if (!write_int(fp, quest->mana_reward)) return false;
    if (!write_int(fp, quest->trust_reward)) return false;
    if (!write_int(fp, quest->respect_reward)) return false;

    /* Write unlocks */
    if (!write_string(fp, quest->unlocks_memory, sizeof(quest->unlocks_memory))) return false;
    if (!write_string(fp, quest->unlocks_quest, sizeof(quest->unlocks_quest))) return false;
    if (!write_string(fp, quest->unlocks_location, sizeof(quest->unlocks_location))) return false;

    /* Write failure conditions */
    if (!write_bool(fp, quest->can_fail)) return false;
    if (!write_bool(fp, quest->time_limited)) return false;
    if (!write_uint64(fp, (uint64_t)quest->deadline)) return false;

    return true;
}

/* Read quest */
static Quest* read_quest(FILE* fp) {
    bool not_null;
    if (!read_bool(fp, &not_null)) return NULL;
    if (!not_null) return NULL;

    char id[64], title[128], quest_giver[64];
    if (!read_string(fp, id, sizeof(id))) return NULL;
    if (!read_string(fp, title, sizeof(title))) return NULL;

    char description[512];
    if (!read_string(fp, description, sizeof(description))) return NULL;
    if (!read_string(fp, quest_giver, sizeof(quest_giver))) return NULL;

    Quest* quest = quest_create(id, title, quest_giver);
    if (!quest) return NULL;

    strncpy(quest->description, description, sizeof(quest->description) - 1);
    quest->description[sizeof(quest->description) - 1] = '\0';

    uint32_t state_val;
    uint64_t started_time, completed_time;
    if (!read_uint32(fp, &state_val)) { quest_destroy(quest); return NULL; }
    if (!read_uint64(fp, &started_time)) { quest_destroy(quest); return NULL; }
    if (!read_uint64(fp, &completed_time)) { quest_destroy(quest); return NULL; }

    quest->state = (QuestState)state_val;
    quest->started_time = (time_t)started_time;
    quest->completed_time = (time_t)completed_time;

    /* Read objectives */
    uint32_t obj_count;
    if (!read_uint32(fp, &obj_count)) { quest_destroy(quest); return NULL; }

    for (uint32_t i = 0; i < obj_count && i < MAX_QUEST_OBJECTIVES; i++) {
        QuestObjective* obj = read_quest_objective(fp);
        if (!obj) { quest_destroy(quest); return NULL; }
        quest_add_objective(quest, obj);
    }

    /* Read rewards */
    if (!read_int(fp, &quest->soul_energy_reward)) { quest_destroy(quest); return NULL; }
    if (!read_int(fp, &quest->mana_reward)) { quest_destroy(quest); return NULL; }
    if (!read_int(fp, &quest->trust_reward)) { quest_destroy(quest); return NULL; }
    if (!read_int(fp, &quest->respect_reward)) { quest_destroy(quest); return NULL; }

    /* Read unlocks */
    if (!read_string(fp, quest->unlocks_memory, sizeof(quest->unlocks_memory))) { quest_destroy(quest); return NULL; }
    if (!read_string(fp, quest->unlocks_quest, sizeof(quest->unlocks_quest))) { quest_destroy(quest); return NULL; }
    if (!read_string(fp, quest->unlocks_location, sizeof(quest->unlocks_location))) { quest_destroy(quest); return NULL; }

    /* Read failure conditions */
    uint64_t deadline;
    if (!read_bool(fp, &quest->can_fail)) { quest_destroy(quest); return NULL; }
    if (!read_bool(fp, &quest->time_limited)) { quest_destroy(quest); return NULL; }
    if (!read_uint64(fp, &deadline)) { quest_destroy(quest); return NULL; }
    quest->deadline = (time_t)deadline;

    return quest;
}

/* Write quest manager */
static bool write_quest_manager(FILE* fp, const QuestManager* mgr) {
    if (!mgr) {
        return write_uint32(fp, 0);
    }

    if (!write_uint32(fp, (uint32_t)mgr->quest_count)) return false;

    for (size_t i = 0; i < mgr->quest_count; i++) {
        if (!write_quest(fp, mgr->quests[i])) return false;
    }

    return true;
}

/* Read quest manager */
static QuestManager* read_quest_manager(FILE* fp) {
    uint32_t count;
    if (!read_uint32(fp, &count)) return NULL;

    if (count == 0) return NULL;

    QuestManager* mgr = quest_manager_create();
    if (!mgr) return NULL;

    for (uint32_t i = 0; i < count; i++) {
        Quest* quest = read_quest(fp);
        if (!quest) {
            quest_manager_destroy(mgr);
            return NULL;
        }
        quest_manager_add_quest(mgr, quest);
    }

    return mgr;
}

/* ==================== NPC System Serialization ==================== */

/* Write NPC */
static bool write_npc(FILE* fp, const NPC* npc) {
    if (!npc) {
        return write_bool(fp, false);
    }

    if (!write_bool(fp, true)) return false;
    if (!write_string(fp, npc->id, sizeof(npc->id))) return false;
    if (!write_string(fp, npc->name, sizeof(npc->name))) return false;
    if (!write_string(fp, npc->title, sizeof(npc->title))) return false;
    if (!write_string(fp, npc->description, sizeof(npc->description))) return false;
    if (!write_uint32(fp, (uint32_t)npc->archetype)) return false;
    if (!write_string(fp, npc->faction, sizeof(npc->faction))) return false;
    if (!write_uint32(fp, (uint32_t)npc->location_type)) return false;
    if (!write_string(fp, npc->current_location, sizeof(npc->current_location))) return false;
    if (!write_string(fp, npc->home_location, sizeof(npc->home_location))) return false;
    if (!write_bool(fp, npc->available)) return false;
    if (!write_bool(fp, npc->discovered)) return false;
    if (!write_uint64(fp, (uint64_t)npc->first_met_time)) return false;
    if (!write_string(fp, npc->current_dialogue_state, sizeof(npc->current_dialogue_state))) return false;

    /* Write dialogue states */
    if (!write_uint32(fp, (uint32_t)npc->dialogue_state_count)) return false;
    for (size_t i = 0; i < npc->dialogue_state_count; i++) {
        if (!write_string(fp, npc->dialogue_states[i], sizeof(npc->dialogue_states[i]))) return false;
    }

    /* Write quests */
    if (!write_uint32(fp, (uint32_t)npc->active_quest_count)) return false;
    for (size_t i = 0; i < npc->active_quest_count; i++) {
        if (!write_string(fp, npc->active_quests[i], sizeof(npc->active_quests[i]))) return false;
    }

    if (!write_uint32(fp, (uint32_t)npc->completed_quest_count)) return false;
    for (size_t i = 0; i < npc->completed_quest_count; i++) {
        if (!write_string(fp, npc->completed_quests[i], sizeof(npc->completed_quests[i]))) return false;
    }

    /* Write memories */
    if (!write_uint32(fp, (uint32_t)npc->memory_count)) return false;
    for (size_t i = 0; i < npc->memory_count; i++) {
        if (!write_string(fp, npc->unlockable_memories[i], sizeof(npc->unlockable_memories[i]))) return false;
    }

    /* Write interaction tracking */
    if (!write_int(fp, npc->interaction_count)) return false;
    if (!write_uint64(fp, (uint64_t)npc->last_interaction_time)) return false;

    /* Write flags */
    if (!write_bool(fp, npc->is_hostile)) return false;
    if (!write_bool(fp, npc->is_dead)) return false;
    if (!write_bool(fp, npc->is_hidden)) return false;

    return true;
}

/* Read NPC */
static NPC* read_npc(FILE* fp) {
    bool not_null;
    if (!read_bool(fp, &not_null)) return NULL;
    if (!not_null) return NULL;

    char id[64], name[128];
    uint32_t archetype_val;

    if (!read_string(fp, id, sizeof(id))) return NULL;
    if (!read_string(fp, name, sizeof(name))) return NULL;

    NPC* npc = (NPC*)calloc(1, sizeof(NPC));
    if (!npc) return NULL;

    strncpy(npc->id, id, sizeof(npc->id) - 1);
    npc->id[sizeof(npc->id) - 1] = '\0';
    strncpy(npc->name, name, sizeof(npc->name) - 1);
    npc->name[sizeof(npc->name) - 1] = '\0';

    if (!read_string(fp, npc->title, sizeof(npc->title))) { free(npc); return NULL; }
    if (!read_string(fp, npc->description, sizeof(npc->description))) { free(npc); return NULL; }
    if (!read_uint32(fp, &archetype_val)) { free(npc); return NULL; }
    npc->archetype = (NPCArchetype)archetype_val;

    if (!read_string(fp, npc->faction, sizeof(npc->faction))) { free(npc); return NULL; }

    uint32_t location_type_val;
    if (!read_uint32(fp, &location_type_val)) { free(npc); return NULL; }
    npc->location_type = (NPCLocationType)location_type_val;

    if (!read_string(fp, npc->current_location, sizeof(npc->current_location))) { free(npc); return NULL; }
    if (!read_string(fp, npc->home_location, sizeof(npc->home_location))) { free(npc); return NULL; }

    uint64_t first_met_time;
    if (!read_bool(fp, &npc->available)) { free(npc); return NULL; }
    if (!read_bool(fp, &npc->discovered)) { free(npc); return NULL; }
    if (!read_uint64(fp, &first_met_time)) { free(npc); return NULL; }
    npc->first_met_time = (time_t)first_met_time;

    if (!read_string(fp, npc->current_dialogue_state, sizeof(npc->current_dialogue_state))) { free(npc); return NULL; }

    /* Read dialogue states */
    uint32_t dialogue_state_count;
    if (!read_uint32(fp, &dialogue_state_count)) { free(npc); return NULL; }
    npc->dialogue_state_count = dialogue_state_count < MAX_NPC_DIALOGUE_STATES ? dialogue_state_count : MAX_NPC_DIALOGUE_STATES;
    for (size_t i = 0; i < npc->dialogue_state_count; i++) {
        if (!read_string(fp, npc->dialogue_states[i], sizeof(npc->dialogue_states[i]))) { free(npc); return NULL; }
    }

    /* Read quests */
    uint32_t active_quest_count;
    if (!read_uint32(fp, &active_quest_count)) { free(npc); return NULL; }
    npc->active_quest_count = active_quest_count < MAX_NPC_QUESTS ? active_quest_count : MAX_NPC_QUESTS;
    for (size_t i = 0; i < npc->active_quest_count; i++) {
        if (!read_string(fp, npc->active_quests[i], sizeof(npc->active_quests[i]))) { free(npc); return NULL; }
    }

    uint32_t completed_quest_count;
    if (!read_uint32(fp, &completed_quest_count)) { free(npc); return NULL; }
    npc->completed_quest_count = completed_quest_count < MAX_NPC_QUESTS ? completed_quest_count : MAX_NPC_QUESTS;
    for (size_t i = 0; i < npc->completed_quest_count; i++) {
        if (!read_string(fp, npc->completed_quests[i], sizeof(npc->completed_quests[i]))) { free(npc); return NULL; }
    }

    /* Read memories */
    uint32_t memory_count;
    if (!read_uint32(fp, &memory_count)) { free(npc); return NULL; }
    npc->memory_count = memory_count < MAX_NPC_MEMORIES ? memory_count : MAX_NPC_MEMORIES;
    for (size_t i = 0; i < npc->memory_count; i++) {
        if (!read_string(fp, npc->unlockable_memories[i], sizeof(npc->unlockable_memories[i]))) { free(npc); return NULL; }
    }

    /* Read interaction tracking */
    uint64_t last_interaction_time;
    if (!read_int(fp, &npc->interaction_count)) { free(npc); return NULL; }
    if (!read_uint64(fp, &last_interaction_time)) { free(npc); return NULL; }
    npc->last_interaction_time = (time_t)last_interaction_time;

    /* Read flags */
    if (!read_bool(fp, &npc->is_hostile)) { free(npc); return NULL; }
    if (!read_bool(fp, &npc->is_dead)) { free(npc); return NULL; }
    if (!read_bool(fp, &npc->is_hidden)) { free(npc); return NULL; }

    return npc;
}

/* Write NPC manager */
static bool write_npc_manager(FILE* fp, const NPCManager* mgr) {
    if (!mgr) {
        return write_uint32(fp, 0);
    }

    if (!write_uint32(fp, (uint32_t)mgr->npc_count)) return false;

    for (size_t i = 0; i < mgr->npc_count; i++) {
        if (!write_npc(fp, mgr->npcs[i])) return false;
    }

    return true;
}

/* Read NPC manager */
static NPCManager* read_npc_manager(FILE* fp) {
    uint32_t count;
    if (!read_uint32(fp, &count)) return NULL;

    if (count == 0) return NULL;

    NPCManager* mgr = npc_manager_create();
    if (!mgr) return NULL;

    for (uint32_t i = 0; i < count; i++) {
        NPC* npc = read_npc(fp);
        if (!npc) {
            npc_manager_destroy(mgr);
            return NULL;
        }
        npc_manager_add_npc(mgr, npc);
    }

    return mgr;
}

/* Continue in next message due to length... */
/* ==================== Relationship System Serialization ==================== */

/* Write relationship event */
static bool write_relationship_event(FILE* fp, const RelationshipEvent* event) {
    if (!write_uint32(fp, (uint32_t)event->type)) return false;
    if (!write_uint64(fp, (uint64_t)event->timestamp)) return false;
    if (!write_int(fp, event->trust_delta)) return false;
    if (!write_int(fp, event->respect_delta)) return false;
    if (!write_int(fp, event->fear_delta)) return false;
    if (!write_string(fp, event->description, sizeof(event->description))) return false;
    return true;
}

/* Read relationship event */
static bool read_relationship_event(FILE* fp, RelationshipEvent* event) {
    uint32_t type_val;
    uint64_t timestamp;

    if (!read_uint32(fp, &type_val)) return false;
    if (!read_uint64(fp, &timestamp)) return false;
    if (!read_int(fp, &event->trust_delta)) return false;
    if (!read_int(fp, &event->respect_delta)) return false;
    if (!read_int(fp, &event->fear_delta)) return false;
    if (!read_string(fp, event->description, sizeof(event->description))) return false;

    event->type = (RelationshipEventType)type_val;
    event->timestamp = (time_t)timestamp;

    return true;
}

/* Write relationship */
static bool write_relationship(FILE* fp, const Relationship* rel) {
    if (!rel) {
        return write_bool(fp, false);
    }

    if (!write_bool(fp, true)) return false;
    if (!write_string(fp, rel->npc_id, sizeof(rel->npc_id))) return false;
    if (!write_int(fp, rel->trust)) return false;
    if (!write_int(fp, rel->respect)) return false;
    if (!write_int(fp, rel->fear)) return false;
    if (!write_int(fp, rel->overall_score)) return false;
    if (!write_uint32(fp, (uint32_t)rel->status)) return false;
    if (!write_int(fp, rel->total_interactions)) return false;
    if (!write_uint64(fp, (uint64_t)rel->first_met)) return false;
    if (!write_uint64(fp, (uint64_t)rel->last_interaction)) return false;

    /* Write events */
    if (!write_uint32(fp, (uint32_t)rel->event_count)) return false;
    for (size_t i = 0; i < rel->event_count; i++) {
        if (!write_relationship_event(fp, &rel->events[i])) return false;
    }

    /* Write flags */
    if (!write_bool(fp, rel->is_romanceable)) return false;
    if (!write_bool(fp, rel->is_romance_active)) return false;
    if (!write_bool(fp, rel->is_rival)) return false;
    if (!write_bool(fp, rel->is_locked)) return false;

    return true;
}

/* Read relationship */
static Relationship* read_relationship(FILE* fp) {
    bool not_null;
    if (!read_bool(fp, &not_null)) return NULL;
    if (!not_null) return NULL;

    char npc_id[64];
    if (!read_string(fp, npc_id, sizeof(npc_id))) return NULL;

    Relationship* rel = relationship_create(npc_id);
    if (!rel) return NULL;

    uint32_t status_val;
    uint64_t first_met, last_interaction;

    if (!read_int(fp, &rel->trust)) { relationship_destroy(rel); return NULL; }
    if (!read_int(fp, &rel->respect)) { relationship_destroy(rel); return NULL; }
    if (!read_int(fp, &rel->fear)) { relationship_destroy(rel); return NULL; }
    if (!read_int(fp, &rel->overall_score)) { relationship_destroy(rel); return NULL; }
    if (!read_uint32(fp, &status_val)) { relationship_destroy(rel); return NULL; }
    rel->status = (RelationshipStatus)status_val;

    if (!read_int(fp, &rel->total_interactions)) { relationship_destroy(rel); return NULL; }
    if (!read_uint64(fp, &first_met)) { relationship_destroy(rel); return NULL; }
    if (!read_uint64(fp, &last_interaction)) { relationship_destroy(rel); return NULL; }
    rel->first_met = (time_t)first_met;
    rel->last_interaction = (time_t)last_interaction;

    /* Read events */
    uint32_t event_count;
    if (!read_uint32(fp, &event_count)) { relationship_destroy(rel); return NULL; }
    rel->event_count = event_count < MAX_RELATIONSHIP_EVENTS ? event_count : MAX_RELATIONSHIP_EVENTS;

    for (size_t i = 0; i < rel->event_count; i++) {
        if (!read_relationship_event(fp, &rel->events[i])) {
            relationship_destroy(rel);
            return NULL;
        }
    }

    /* Read flags */
    if (!read_bool(fp, &rel->is_romanceable)) { relationship_destroy(rel); return NULL; }
    if (!read_bool(fp, &rel->is_romance_active)) { relationship_destroy(rel); return NULL; }
    if (!read_bool(fp, &rel->is_rival)) { relationship_destroy(rel); return NULL; }
    if (!read_bool(fp, &rel->is_locked)) { relationship_destroy(rel); return NULL; }

    return rel;
}

/* Write relationship manager */
static bool write_relationship_manager(FILE* fp, const RelationshipManager* mgr) {
    if (!mgr) {
        return write_uint32(fp, 0);
    }

    if (!write_uint32(fp, (uint32_t)mgr->relationship_count)) return false;

    for (size_t i = 0; i < mgr->relationship_count; i++) {
        if (!write_relationship(fp, mgr->relationships[i])) return false;
    }

    return true;
}

/* Read relationship manager */
static RelationshipManager* read_relationship_manager(FILE* fp) {
    uint32_t count;
    if (!read_uint32(fp, &count)) return NULL;

    if (count == 0) return NULL;

    RelationshipManager* mgr = relationship_manager_create();
    if (!mgr) return NULL;

    for (uint32_t i = 0; i < count; i++) {
        Relationship* rel = read_relationship(fp);
        if (!rel) {
            relationship_manager_destroy(mgr);
            return NULL;
        }
        relationship_manager_add_relationship(mgr, rel);
    }

    return mgr;
}

/* ==================== Memory System Serialization ==================== */

/* Write memory fragment */
static bool write_memory_fragment(FILE* fp, const MemoryFragment* frag) {
    if (!frag) {
        return write_bool(fp, false);
    }

    if (!write_bool(fp, true)) return false;
    if (!write_string(fp, frag->id, sizeof(frag->id))) return false;
    if (!write_string(fp, frag->title, sizeof(frag->title))) return false;
    if (!write_string(fp, frag->content, sizeof(frag->content))) return false;
    if (!write_bool(fp, frag->discovered)) return false;
    if (!write_uint64(fp, (uint64_t)frag->discovery_time)) return false;
    if (!write_string(fp, frag->discovery_location, sizeof(frag->discovery_location))) return false;
    if (!write_string(fp, frag->discovery_method, sizeof(frag->discovery_method))) return false;
    if (!write_string(fp, frag->category, sizeof(frag->category))) return false;
    if (!write_int(fp, frag->chronological_order)) return false;

    /* Write related fragments */
    if (!write_uint32(fp, (uint32_t)frag->related_count)) return false;
    for (size_t i = 0; i < frag->related_count; i++) {
        if (!write_string(fp, frag->related_fragments[i], sizeof(frag->related_fragments[i]))) return false;
    }

    /* Write related NPCs */
    if (!write_uint32(fp, (uint32_t)frag->npc_count)) return false;
    for (size_t i = 0; i < frag->npc_count; i++) {
        if (!write_string(fp, frag->related_npcs[i], sizeof(frag->related_npcs[i]))) return false;
    }

    /* Write related locations */
    if (!write_uint32(fp, (uint32_t)frag->location_count)) return false;
    for (size_t i = 0; i < frag->location_count; i++) {
        if (!write_string(fp, frag->related_locations[i], sizeof(frag->related_locations[i]))) return false;
    }

    /* Write flags */
    if (!write_bool(fp, frag->key_memory)) return false;
    if (!write_bool(fp, frag->hidden)) return false;

    return true;
}

/* Read memory fragment */
static MemoryFragment* read_memory_fragment(FILE* fp) {
    bool not_null;
    if (!read_bool(fp, &not_null)) return NULL;
    if (!not_null) return NULL;

    char id[64], title[128], content[1024];
    if (!read_string(fp, id, sizeof(id))) return NULL;
    if (!read_string(fp, title, sizeof(title))) return NULL;
    if (!read_string(fp, content, sizeof(content))) return NULL;

    MemoryFragment* frag = memory_fragment_create(id, title, content);
    if (!frag) return NULL;

    uint64_t discovery_time;
    if (!read_bool(fp, &frag->discovered)) { memory_fragment_destroy(frag); return NULL; }
    if (!read_uint64(fp, &discovery_time)) { memory_fragment_destroy(frag); return NULL; }
    frag->discovery_time = (time_t)discovery_time;

    if (!read_string(fp, frag->discovery_location, sizeof(frag->discovery_location))) { memory_fragment_destroy(frag); return NULL; }
    if (!read_string(fp, frag->discovery_method, sizeof(frag->discovery_method))) { memory_fragment_destroy(frag); return NULL; }
    if (!read_string(fp, frag->category, sizeof(frag->category))) { memory_fragment_destroy(frag); return NULL; }
    if (!read_int(fp, &frag->chronological_order)) { memory_fragment_destroy(frag); return NULL; }

    /* Read related fragments */
    uint32_t related_count;
    if (!read_uint32(fp, &related_count)) { memory_fragment_destroy(frag); return NULL; }
    frag->related_count = related_count < MAX_FRAGMENT_CROSS_REFS ? related_count : MAX_FRAGMENT_CROSS_REFS;
    for (size_t i = 0; i < frag->related_count; i++) {
        if (!read_string(fp, frag->related_fragments[i], sizeof(frag->related_fragments[i]))) {
            memory_fragment_destroy(frag);
            return NULL;
        }
    }

    /* Read related NPCs */
    uint32_t npc_count;
    if (!read_uint32(fp, &npc_count)) { memory_fragment_destroy(frag); return NULL; }
    frag->npc_count = npc_count < MAX_FRAGMENT_CROSS_REFS ? npc_count : MAX_FRAGMENT_CROSS_REFS;
    for (size_t i = 0; i < frag->npc_count; i++) {
        if (!read_string(fp, frag->related_npcs[i], sizeof(frag->related_npcs[i]))) {
            memory_fragment_destroy(frag);
            return NULL;
        }
    }

    /* Read related locations */
    uint32_t location_count;
    if (!read_uint32(fp, &location_count)) { memory_fragment_destroy(frag); return NULL; }
    frag->location_count = location_count < MAX_FRAGMENT_CROSS_REFS ? location_count : MAX_FRAGMENT_CROSS_REFS;
    for (size_t i = 0; i < frag->location_count; i++) {
        if (!read_string(fp, frag->related_locations[i], sizeof(frag->related_locations[i]))) {
            memory_fragment_destroy(frag);
            return NULL;
        }
    }

    /* Read flags */
    if (!read_bool(fp, &frag->key_memory)) { memory_fragment_destroy(frag); return NULL; }
    if (!read_bool(fp, &frag->hidden)) { memory_fragment_destroy(frag); return NULL; }

    return frag;
}

/* Write memory manager */
static bool write_memory_manager(FILE* fp, const MemoryManager* mgr) {
    if (!mgr) {
        return write_uint32(fp, 0);
    }

    if (!write_uint32(fp, (uint32_t)mgr->fragment_count)) return false;

    for (size_t i = 0; i < mgr->fragment_count; i++) {
        if (!write_memory_fragment(fp, mgr->fragments[i])) return false;
    }

    return true;
}

/* Read memory manager */
static MemoryManager* read_memory_manager(FILE* fp) {
    uint32_t count;
    if (!read_uint32(fp, &count)) return NULL;

    if (count == 0) return NULL;

    MemoryManager* mgr = memory_manager_create();
    if (!mgr) return NULL;

    for (uint32_t i = 0; i < count; i++) {
        MemoryFragment* frag = read_memory_fragment(fp);
        if (!frag) {
            memory_manager_destroy(mgr);
            return NULL;
        }
        memory_manager_add_fragment(mgr, frag);
    }

    return mgr;
}

/* Continue in next message... */
/* ==================== Divine Council Serialization ==================== */

/* Write god */
static bool write_god(FILE* fp, const God* god) {
    if (!god) {
        return write_bool(fp, false);
    }

    if (!write_bool(fp, true)) return false;
    if (!write_string(fp, god->id, sizeof(god->id))) return false;
    if (!write_string(fp, god->name, sizeof(god->name))) return false;
    if (!write_string(fp, god->title, sizeof(god->title))) return false;
    if (!write_string(fp, god->description, sizeof(god->description))) return false;
    if (!write_uint32(fp, (uint32_t)god->domain)) return false;
    if (!write_uint32(fp, (uint32_t)god->power_level)) return false;
    if (!write_string(fp, god->manifestation, sizeof(god->manifestation))) return false;
    if (!write_string(fp, god->personality, sizeof(god->personality))) return false;
    if (!write_int16(fp, god->favor)) return false;
    if (!write_int16(fp, god->favor_min)) return false;
    if (!write_int16(fp, god->favor_max)) return false;
    if (!write_int16(fp, god->favor_start)) return false;
    if (!write_uint32(fp, god->interactions)) return false;
    if (!write_bool(fp, god->summoned)) return false;
    if (!write_bool(fp, god->judgment_given)) return false;
    if (!write_bool(fp, god->combat_possible)) return false;
    if (!write_uint32(fp, god->combat_difficulty)) return false;

    /* Write dialogue trees */
    if (!write_uint32(fp, (uint32_t)god->dialogue_tree_count)) return false;
    for (size_t i = 0; i < god->dialogue_tree_count; i++) {
        if (!write_string(fp, god->dialogue_trees[i], sizeof(god->dialogue_trees[i]))) return false;
    }

    /* Write trials */
    if (!write_uint32(fp, (uint32_t)god->trial_count)) return false;
    for (size_t i = 0; i < god->trial_count; i++) {
        if (!write_string(fp, god->trials[i], sizeof(god->trials[i]))) return false;
    }

    /* Write restrictions */
    if (!write_uint32(fp, (uint32_t)god->restriction_count)) return false;
    for (size_t i = 0; i < god->restriction_count; i++) {
        if (!write_string(fp, god->restrictions[i], sizeof(god->restrictions[i]))) return false;
    }

    /* Write amnesty/judgment state */
    if (!write_bool(fp, god->amnesty_granted)) return false;
    if (!write_bool(fp, god->condemned)) return false;

    return true;
}

/* Read god */
static God* read_god(FILE* fp) {
    bool not_null;
    if (!read_bool(fp, &not_null)) return NULL;
    if (!not_null) return NULL;

    char id[64], name[128];
    uint32_t domain_val;

    if (!read_string(fp, id, sizeof(id))) return NULL;
    if (!read_string(fp, name, sizeof(name))) return NULL;

    God* god = (God*)calloc(1, sizeof(God));
    if (!god) return NULL;

    strncpy(god->id, id, sizeof(god->id) - 1);
    god->id[sizeof(god->id) - 1] = '\0';
    strncpy(god->name, name, sizeof(god->name) - 1);
    god->name[sizeof(god->name) - 1] = '\0';

    if (!read_string(fp, god->title, sizeof(god->title))) { free(god); return NULL; }
    if (!read_string(fp, god->description, sizeof(god->description))) { free(god); return NULL; }
    if (!read_uint32(fp, &domain_val)) { free(god); return NULL; }
    god->domain = (GodDomain)domain_val;

    uint32_t power_level_val;
    if (!read_uint32(fp, &power_level_val)) { free(god); return NULL; }
    god->power_level = (PowerLevel)power_level_val;

    if (!read_string(fp, god->manifestation, sizeof(god->manifestation))) { free(god); return NULL; }
    if (!read_string(fp, god->personality, sizeof(god->personality))) { free(god); return NULL; }
    if (!read_int16(fp, &god->favor)) { free(god); return NULL; }
    if (!read_int16(fp, &god->favor_min)) { free(god); return NULL; }
    if (!read_int16(fp, &god->favor_max)) { free(god); return NULL; }
    if (!read_int16(fp, &god->favor_start)) { free(god); return NULL; }
    if (!read_uint32(fp, &god->interactions)) { free(god); return NULL; }
    if (!read_bool(fp, &god->summoned)) { free(god); return NULL; }
    if (!read_bool(fp, &god->judgment_given)) { free(god); return NULL; }
    if (!read_bool(fp, &god->combat_possible)) { free(god); return NULL; }
    if (!read_uint32(fp, &god->combat_difficulty)) { free(god); return NULL; }

    /* Read dialogue trees */
    uint32_t dialogue_tree_count;
    if (!read_uint32(fp, &dialogue_tree_count)) { free(god); return NULL; }
    god->dialogue_tree_count = dialogue_tree_count < MAX_GOD_DIALOGUE_TREES ? dialogue_tree_count : MAX_GOD_DIALOGUE_TREES;
    for (size_t i = 0; i < god->dialogue_tree_count; i++) {
        if (!read_string(fp, god->dialogue_trees[i], sizeof(god->dialogue_trees[i]))) {
            free(god);
            return NULL;
        }
    }

    /* Read trials */
    uint32_t trial_count;
    if (!read_uint32(fp, &trial_count)) { free(god); return NULL; }
    god->trial_count = trial_count < MAX_GOD_TRIALS ? trial_count : MAX_GOD_TRIALS;
    for (size_t i = 0; i < god->trial_count; i++) {
        if (!read_string(fp, god->trials[i], sizeof(god->trials[i]))) {
            free(god);
            return NULL;
        }
    }

    /* Read restrictions */
    uint32_t restriction_count;
    if (!read_uint32(fp, &restriction_count)) { free(god); return NULL; }
    god->restriction_count = restriction_count < MAX_GOD_RESTRICTIONS ? restriction_count : MAX_GOD_RESTRICTIONS;
    for (size_t i = 0; i < god->restriction_count; i++) {
        if (!read_string(fp, god->restrictions[i], sizeof(god->restrictions[i]))) {
            free(god);
            return NULL;
        }
    }

    /* Read amnesty/judgment state */
    if (!read_bool(fp, &god->amnesty_granted)) { free(god); return NULL; }
    if (!read_bool(fp, &god->condemned)) { free(god); return NULL; }

    return god;
}

/* Write divine council */
static bool write_divine_council(FILE* fp, const DivineCouncil* council) {
    if (!council) {
        return write_uint32(fp, 0);
    }

    if (!write_uint32(fp, (uint32_t)council->god_count)) return false;

    /* Write all gods */
    for (size_t i = 0; i < council->god_count; i++) {
        if (!write_god(fp, council->gods[i])) return false;
    }

    /* Write council state */
    if (!write_bool(fp, council->council_summoned)) return false;
    if (!write_uint32(fp, council->summon_day)) return false;
    if (!write_bool(fp, council->judgment_complete)) return false;

    /* Write verdict */
    if (!write_uint32(fp, (uint32_t)council->verdict)) return false;
    if (!write_string(fp, council->verdict_text, sizeof(council->verdict_text))) return false;

    /* Write restrictions */
    if (!write_uint32(fp, (uint32_t)council->restriction_count)) return false;
    for (size_t i = 0; i < council->restriction_count; i++) {
        if (!write_string(fp, council->restrictions[i], sizeof(council->restrictions[i]))) return false;
    }

    /* Write vote tracking */
    if (!write_uint8(fp, council->votes_amnesty)) return false;
    if (!write_uint8(fp, council->votes_conditional)) return false;
    if (!write_uint8(fp, council->votes_purge)) return false;
    if (!write_uint8(fp, council->votes_death)) return false;

    /* Write statistics */
    if (!write_int16(fp, council->average_favor)) return false;
    if (!write_uint32(fp, council->total_interactions)) return false;

    return true;
}

/* Read divine council */
static DivineCouncil* read_divine_council(FILE* fp) {
    uint32_t god_count;
    if (!read_uint32(fp, &god_count)) return NULL;

    if (god_count == 0) return NULL;

    DivineCouncil* council = divine_council_create();
    if (!council) return NULL;

    /* Read all gods */
    for (uint32_t i = 0; i < god_count && i < MAX_COUNCIL_GODS; i++) {
        God* god = read_god(fp);
        if (!god) {
            divine_council_destroy(council);
            return NULL;
        }
        if (!divine_council_add_god(council, god)) {
            god_destroy(god);
            divine_council_destroy(council);
            return NULL;
        }
    }

    /* Read council state */
    if (!read_bool(fp, &council->council_summoned)) { divine_council_destroy(council); return NULL; }
    if (!read_uint32(fp, &council->summon_day)) { divine_council_destroy(council); return NULL; }
    if (!read_bool(fp, &council->judgment_complete)) { divine_council_destroy(council); return NULL; }

    /* Read verdict */
    uint32_t verdict_val;
    if (!read_uint32(fp, &verdict_val)) { divine_council_destroy(council); return NULL; }
    council->verdict = (DivineVerdict)verdict_val;

    if (!read_string(fp, council->verdict_text, sizeof(council->verdict_text))) {
        divine_council_destroy(council);
        return NULL;
    }

    /* Read restrictions */
    uint32_t restriction_count;
    if (!read_uint32(fp, &restriction_count)) { divine_council_destroy(council); return NULL; }
    council->restriction_count = restriction_count < MAX_COUNCIL_RESTRICTIONS ? restriction_count : MAX_COUNCIL_RESTRICTIONS;
    for (size_t i = 0; i < council->restriction_count; i++) {
        if (!read_string(fp, council->restrictions[i], sizeof(council->restrictions[i]))) {
            divine_council_destroy(council);
            return NULL;
        }
    }

    /* Read vote tracking */
    if (!read_uint8(fp, &council->votes_amnesty)) { divine_council_destroy(council); return NULL; }
    if (!read_uint8(fp, &council->votes_conditional)) { divine_council_destroy(council); return NULL; }
    if (!read_uint8(fp, &council->votes_purge)) { divine_council_destroy(council); return NULL; }
    if (!read_uint8(fp, &council->votes_death)) { divine_council_destroy(council); return NULL; }

    /* Read statistics */
    if (!read_int16(fp, &council->average_favor)) { divine_council_destroy(council); return NULL; }
    if (!read_uint32(fp, &council->total_interactions)) { divine_council_destroy(council); return NULL; }

    return council;
}

/* ==================== Thessara Relationship Serialization ==================== */

/* Write knowledge transfer */
static bool write_knowledge_transfer(FILE* fp, const KnowledgeTransfer* transfer) {
    if (!write_uint32(fp, (uint32_t)transfer->type)) return false;
    if (!write_string(fp, transfer->id, sizeof(transfer->id))) return false;
    if (!write_string(fp, transfer->description, sizeof(transfer->description))) return false;
    if (!write_uint32(fp, transfer->day_transferred)) return false;
    return true;
}

/* Read knowledge transfer */
static bool read_knowledge_transfer(FILE* fp, KnowledgeTransfer* transfer) {
    uint32_t type_val;
    if (!read_uint32(fp, &type_val)) return false;
    transfer->type = (KnowledgeType)type_val;

    if (!read_string(fp, transfer->id, sizeof(transfer->id))) return false;
    if (!read_string(fp, transfer->description, sizeof(transfer->description))) return false;
    if (!read_uint32(fp, &transfer->day_transferred)) return false;

    return true;
}

/* Write thessara relationship */
static bool write_thessara_relationship(FILE* fp, const ThessaraRelationship* thessara) {
    if (!thessara) {
        return write_bool(fp, false);
    }

    if (!write_bool(fp, true)) return false;
    if (!write_bool(fp, thessara->discovered)) return false;
    if (!write_uint32(fp, thessara->discovery_day)) return false;
    if (!write_bool(fp, thessara->severed)) return false;
    if (!write_uint32(fp, thessara->severed_day)) return false;
    if (!write_uint32(fp, thessara->meetings_count)) return false;
    if (!write_uint32(fp, thessara->last_meeting_day)) return false;

    /* Write knowledge transfers */
    if (!write_uint32(fp, (uint32_t)thessara->transfer_count)) return false;
    for (size_t i = 0; i < thessara->transfer_count; i++) {
        if (!write_knowledge_transfer(fp, &thessara->transfers[i])) return false;
    }

    /* Write trust level */
    if (!write_float(fp, thessara->trust_level)) return false;

    /* Write warnings */
    if (!write_uint32(fp, (uint32_t)thessara->warning_count)) return false;
    for (size_t i = 0; i < thessara->warning_count; i++) {
        if (!write_string(fp, thessara->warnings[i], sizeof(thessara->warnings[i]))) return false;
    }

    /* Write path revelations */
    if (!write_bool(fp, thessara->wraith_path_revealed)) return false;
    if (!write_bool(fp, thessara->morningstar_path_revealed)) return false;
    if (!write_bool(fp, thessara->archon_guidance_given)) return false;

    /* Write mentorship metrics */
    if (!write_uint32(fp, thessara->total_guidance_time)) return false;
    if (!write_uint32(fp, thessara->trials_assisted)) return false;

    return true;
}

/* Read thessara relationship */
static ThessaraRelationship* read_thessara_relationship(FILE* fp) {
    bool not_null;
    if (!read_bool(fp, &not_null)) return NULL;
    if (!not_null) return NULL;

    ThessaraRelationship* thessara = thessara_create();
    if (!thessara) return NULL;

    if (!read_bool(fp, &thessara->discovered)) { thessara_destroy(thessara); return NULL; }
    if (!read_uint32(fp, &thessara->discovery_day)) { thessara_destroy(thessara); return NULL; }
    if (!read_bool(fp, &thessara->severed)) { thessara_destroy(thessara); return NULL; }
    if (!read_uint32(fp, &thessara->severed_day)) { thessara_destroy(thessara); return NULL; }
    if (!read_uint32(fp, &thessara->meetings_count)) { thessara_destroy(thessara); return NULL; }
    if (!read_uint32(fp, &thessara->last_meeting_day)) { thessara_destroy(thessara); return NULL; }

    /* Read knowledge transfers */
    uint32_t transfer_count;
    if (!read_uint32(fp, &transfer_count)) { thessara_destroy(thessara); return NULL; }
    thessara->transfer_count = transfer_count < MAX_THESSARA_KNOWLEDGE ? transfer_count : MAX_THESSARA_KNOWLEDGE;

    for (size_t i = 0; i < thessara->transfer_count; i++) {
        if (!read_knowledge_transfer(fp, &thessara->transfers[i])) {
            thessara_destroy(thessara);
            return NULL;
        }
    }

    /* Read trust level */
    if (!read_float(fp, &thessara->trust_level)) { thessara_destroy(thessara); return NULL; }

    /* Read warnings */
    uint32_t warning_count;
    if (!read_uint32(fp, &warning_count)) { thessara_destroy(thessara); return NULL; }
    thessara->warning_count = warning_count < MAX_THESSARA_WARNINGS ? warning_count : MAX_THESSARA_WARNINGS;

    for (size_t i = 0; i < thessara->warning_count; i++) {
        if (!read_string(fp, thessara->warnings[i], sizeof(thessara->warnings[i]))) {
            thessara_destroy(thessara);
            return NULL;
        }
    }

    /* Read path revelations */
    if (!read_bool(fp, &thessara->wraith_path_revealed)) { thessara_destroy(thessara); return NULL; }
    if (!read_bool(fp, &thessara->morningstar_path_revealed)) { thessara_destroy(thessara); return NULL; }
    if (!read_bool(fp, &thessara->archon_guidance_given)) { thessara_destroy(thessara); return NULL; }

    /* Read mentorship metrics */
    if (!read_uint32(fp, &thessara->total_guidance_time)) { thessara_destroy(thessara); return NULL; }
    if (!read_uint32(fp, &thessara->trials_assisted)) { thessara_destroy(thessara); return NULL; }

    return thessara;
}

/* Main save/load functions */

bool save_game(const GameState* state, const char* filepath) {
    if (!state || !state->initialized) {
        LOG_ERROR("Cannot save uninitialized game state");
        return false;
    }

    char* path = filepath ? expand_home_directory(filepath) : get_default_save_path();
    if (!path) {
        LOG_ERROR("Failed to determine save path");
        return false;
    }

    /* Create backup of existing save */
    backup_save_file(path);

    /* Write to temporary file first */
    size_t temp_len = strlen(path) + 5;
    char* temp_path = malloc(temp_len);
    if (!temp_path) {
        free(path);
        return false;
    }
    snprintf(temp_path, temp_len, "%s.tmp", path);

    FILE* fp = fopen(temp_path, "w+b");
    if (!fp) {
        LOG_ERROR("Failed to open save file for writing: %s", strerror(errno));
        free(temp_path);
        free(path);
        return false;
    }

    /* Write placeholder header */
    SaveFileHeader header;
    memset(&header, 0, sizeof(header));
    header.magic = SAVE_MAGIC_NUMBER;
    header.version_major = SAVE_VERSION_MAJOR;
    header.version_minor = SAVE_VERSION_MINOR;
    header.version_patch = SAVE_VERSION_PATCH;
    header.reserved = 0;

    if (fwrite(&header, sizeof(header), 1, fp) != 1) {
        LOG_ERROR("Failed to write save header");
        fclose(fp);
        unlink(temp_path);
        free(temp_path);
        free(path);
        return false;
    }

    /* Remember position for data */
    long data_start = ftell(fp);

    /* Write game state data */
    bool success = true;

    /* Write managers */
    success = success && write_soul_manager(fp, state->souls);
    success = success && write_minion_manager(fp, state->minions);
    success = success && write_territory_manager(fp, state->territory);
    success = success && write_quest_manager(fp, state->quests);
    success = success && write_npc_manager(fp, state->npcs);
    success = success && write_relationship_manager(fp, state->relationships);
    success = success && write_memory_manager(fp, state->memories);
    success = success && write_divine_council(fp, state->divine_council);
    success = success && write_thessara_relationship(fp, state->thessara);

    /* Write simple structs */
    success = success && write_resources(fp, &state->resources);
    success = success && write_corruption(fp, &state->corruption);
    success = success && write_consciousness(fp, &state->consciousness);

    /* Write scalar fields */
    success = success && write_uint32(fp, state->current_location_id);
    success = success && write_uint32(fp, state->player_level);
    success = success && write_uint64(fp, state->player_experience);
    success = success && write_uint32(fp, state->next_soul_id);
    success = success && write_uint32(fp, state->next_minion_id);
    success = success && write_uint32(fp, state->civilian_kills);
    success = success && write_bool(fp, state->game_completed);
    success = success && write_uint32(fp, (uint32_t)state->ending_achieved);

    if (!success) {
        LOG_ERROR("Failed to write game state data");
        fclose(fp);
        unlink(temp_path);
        free(temp_path);
        free(path);
        return false;
    }

    /* Calculate data length and checksum */
    long data_end = ftell(fp);
    uint64_t data_length = (uint64_t)(data_end - data_start);

    /* Read data for checksum */
    fseek(fp, data_start, SEEK_SET);
    uint8_t* data_buffer = malloc((size_t)data_length);
    if (!data_buffer) {
        LOG_ERROR("Failed to allocate buffer for checksum");
        fclose(fp);
        unlink(temp_path);
        free(temp_path);
        free(path);
        return false;
    }

    if (fread(data_buffer, 1, (size_t)data_length, fp) != (size_t)data_length) {
        LOG_ERROR("Failed to read data for checksum");
        free(data_buffer);
        fclose(fp);
        unlink(temp_path);
        free(temp_path);
        free(path);
        return false;
    }

    uint32_t checksum = calculate_crc32(data_buffer, (size_t)data_length);
    free(data_buffer);

    /* Update header */
    header.checksum = checksum;
    header.data_length = data_length;

    fseek(fp, 0, SEEK_SET);
    if (fwrite(&header, sizeof(header), 1, fp) != 1) {
        LOG_ERROR("Failed to write final header");
        fclose(fp);
        unlink(temp_path);
        free(temp_path);
        free(path);
        return false;
    }

    fclose(fp);

    /* Atomic rename */
    if (rename(temp_path, path) != 0) {
        LOG_ERROR("Failed to rename temp file to save file: %s", strerror(errno));
        unlink(temp_path);
        free(temp_path);
        free(path);
        return false;
    }

    LOG_INFO("Game saved successfully to %s (%lu bytes)", path, (unsigned long)data_length);

    free(temp_path);
    free(path);
    return true;
}

GameState* load_game(const char* filepath, char* error_buffer, size_t error_size) {
    char* path = filepath ? expand_home_directory(filepath) : get_default_save_path();
    if (!path) {
        if (error_buffer) {
            snprintf(error_buffer, error_size, "Failed to determine save path");
        }
        return NULL;
    }

    FILE* fp = fopen(path, "rb");
    if (!fp) {
        if (error_buffer) {
            snprintf(error_buffer, error_size, "Failed to open save file: %s", strerror(errno));
        }
        free(path);
        return NULL;
    }

    /* Read header */
    SaveFileHeader header;
    if (fread(&header, sizeof(header), 1, fp) != 1) {
        if (error_buffer) {
            snprintf(error_buffer, error_size, "Failed to read save header");
        }
        fclose(fp);
        free(path);
        return NULL;
    }

    /* Validate magic */
    if (header.magic != SAVE_MAGIC_NUMBER) {
        if (error_buffer) {
            snprintf(error_buffer, error_size, "Invalid save file (bad magic number)");
        }
        fclose(fp);
        free(path);
        return NULL;
    }

    /* Check version compatibility */
    if (!is_version_compatible(header.version_major, header.version_minor, header.version_patch)) {
        if (error_buffer) {
            snprintf(error_buffer, error_size,
                     "Incompatible save version %u.%u.%u (current: %u.%u.%u)",
                     header.version_major, header.version_minor, header.version_patch,
                     SAVE_VERSION_MAJOR, SAVE_VERSION_MINOR, SAVE_VERSION_PATCH);
        }
        fclose(fp);
        free(path);
        return NULL;
    }

    /* Read data */
    uint8_t* data_buffer = malloc((size_t)header.data_length);
    if (!data_buffer) {
        if (error_buffer) {
            snprintf(error_buffer, error_size, "Failed to allocate buffer for data");
        }
        fclose(fp);
        free(path);
        return NULL;
    }

    if (fread(data_buffer, 1, (size_t)header.data_length, fp) != (size_t)header.data_length) {
        if (error_buffer) {
            snprintf(error_buffer, error_size, "Failed to read save data");
        }
        free(data_buffer);
        fclose(fp);
        free(path);
        return NULL;
    }

    /* Validate checksum */
    uint32_t calculated_checksum = calculate_crc32(data_buffer, (size_t)header.data_length);
    if (calculated_checksum != header.checksum) {
        if (error_buffer) {
            snprintf(error_buffer, error_size, "Checksum mismatch (file corrupted)");
        }
        free(data_buffer);
        fclose(fp);
        free(path);
        return NULL;
    }

    fclose(fp);

    /* Create a memory stream from buffer */
    FILE* mem_fp = fmemopen(data_buffer, (size_t)header.data_length, "rb");
    if (!mem_fp) {
        if (error_buffer) {
            snprintf(error_buffer, error_size, "Failed to create memory stream");
        }
        free(data_buffer);
        free(path);
        return NULL;
    }

    /* Allocate new game state */
    GameState* state = calloc(1, sizeof(GameState));
    if (!state) {
        if (error_buffer) {
            snprintf(error_buffer, error_size, "Failed to allocate game state");
        }
        fclose(mem_fp);
        free(data_buffer);
        free(path);
        return NULL;
    }

    /* Read managers */
    state->souls = read_soul_manager(mem_fp);
    state->minions = read_minion_manager(mem_fp);
    state->territory = read_territory_manager(mem_fp);
    state->quests = read_quest_manager(mem_fp);
    state->npcs = read_npc_manager(mem_fp);
    state->relationships = read_relationship_manager(mem_fp);
    state->memories = read_memory_manager(mem_fp);
    state->divine_council = read_divine_council(mem_fp);
    state->thessara = read_thessara_relationship(mem_fp);

    /* Read simple structs */
    bool success = true;
    success = success && read_resources(mem_fp, &state->resources);
    success = success && read_corruption(mem_fp, &state->corruption);
    success = success && read_consciousness(mem_fp, &state->consciousness);

    /* Read scalar fields */
    success = success && read_uint32(mem_fp, &state->current_location_id);
    success = success && read_uint32(mem_fp, &state->player_level);
    success = success && read_uint64(mem_fp, &state->player_experience);
    success = success && read_uint32(mem_fp, &state->next_soul_id);
    success = success && read_uint32(mem_fp, &state->next_minion_id);
    success = success && read_uint32(mem_fp, &state->civilian_kills);
    success = success && read_bool(mem_fp, &state->game_completed);

    uint32_t ending_u32 = 0;
    success = success && read_uint32(mem_fp, &ending_u32);
    if (success) {
        state->ending_achieved = (EndingType)ending_u32;
    }

    if (!success) {
        if (error_buffer) {
            snprintf(error_buffer, error_size, "Failed to deserialize game state");
        }
        soul_manager_destroy(state->souls);
        minion_manager_destroy(state->minions);
        territory_manager_destroy(state->territory);
        quest_manager_destroy(state->quests);
        npc_manager_destroy(state->npcs);
        relationship_manager_destroy(state->relationships);
        memory_manager_destroy(state->memories);
        divine_council_destroy(state->divine_council);
        thessara_destroy(state->thessara);
        free(state);
        fclose(mem_fp);
        free(data_buffer);
        free(path);
        return NULL;
    }

    fclose(mem_fp);
    free(data_buffer);

    /* Mark as initialized (needs further setup by caller) */
    state->initialized = false;  /* Caller must complete initialization */

    LOG_INFO("Game loaded successfully from %s", path);
    free(path);

    return state;
}

bool validate_save_file(const char* filepath) {
    char* path = filepath ? expand_home_directory(filepath) : get_default_save_path();
    if (!path) {
        return false;
    }

    FILE* fp = fopen(path, "rb");
    if (!fp) {
        free(path);
        return false;
    }

    SaveFileHeader header;
    if (fread(&header, sizeof(header), 1, fp) != 1) {
        fclose(fp);
        free(path);
        return false;
    }

    /* Validate magic */
    if (header.magic != SAVE_MAGIC_NUMBER) {
        fclose(fp);
        free(path);
        return false;
    }

    /* Check version */
    if (!is_version_compatible(header.version_major, header.version_minor, header.version_patch)) {
        fclose(fp);
        free(path);
        return false;
    }

    /* Read and validate checksum */
    uint8_t* data_buffer = malloc((size_t)header.data_length);
    if (!data_buffer) {
        fclose(fp);
        free(path);
        return false;
    }

    if (fread(data_buffer, 1, (size_t)header.data_length, fp) != (size_t)header.data_length) {
        free(data_buffer);
        fclose(fp);
        free(path);
        return false;
    }

    uint32_t calculated_checksum = calculate_crc32(data_buffer, (size_t)header.data_length);
    free(data_buffer);
    fclose(fp);
    free(path);

    return (calculated_checksum == header.checksum);
}

bool save_metadata_json(const GameState* state, const char* filepath) {
    if (!state) {
        return false;
    }

    char* path = filepath ? expand_home_directory(filepath) : NULL;
    if (!path && !filepath) {
        char* save_path = get_default_save_path();
        if (!save_path) {
            return false;
        }
        size_t len = strlen(save_path) + 6;
        path = malloc(len);
        if (!path) {
            free(save_path);
            return false;
        }
        snprintf(path, len, "%s.json", save_path);
        free(save_path);
    }

    FILE* fp = fopen(path, "w");
    if (!fp) {
        free(path);
        return false;
    }

    time_t now = time(NULL);
    struct tm* tm_info = gmtime(&now);
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%SZ", tm_info);

    Location* loc = game_state_get_current_location(state);
    const char* loc_name = loc ? loc->name : "unknown";

    fprintf(fp, "{\n");
    fprintf(fp, "  \"version\": \"%u.%u.%u\",\n",
            SAVE_VERSION_MAJOR, SAVE_VERSION_MINOR, SAVE_VERSION_PATCH);
    fprintf(fp, "  \"timestamp\": \"%s\",\n", timestamp);
    fprintf(fp, "  \"player_level\": %u,\n", state->player_level);
    fprintf(fp, "  \"corruption\": %u,\n", state->corruption.corruption);
    fprintf(fp, "  \"day_count\": %u,\n", state->resources.day_count);
    fprintf(fp, "  \"souls_collected\": %zu,\n", soul_manager_count((SoulManager*)state->souls));
    fprintf(fp, "  \"minions_raised\": %zu,\n", minion_manager_count((MinionManager*)state->minions));
    fprintf(fp, "  \"current_location\": \"%s\",\n", loc_name);
    fprintf(fp, "  \"game_completed\": %s\n", state->game_completed ? "true" : "false");
    fprintf(fp, "}\n");

    fclose(fp);
    free(path);

    return true;
}

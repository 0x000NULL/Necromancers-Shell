/**
 * @file test_checksum_debug.c
 * @brief Debug checksum validation issue
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

/* Copy structures and functions from save_load */
#define SAVE_MAGIC_NUMBER 0x5243454E

typedef struct {
    uint32_t magic;
    uint8_t version_major;
    uint8_t version_minor;
    uint8_t version_patch;
    uint8_t reserved;
    uint32_t checksum;
    uint64_t data_length;
} SaveFileHeader;

/* CRC32 table */
static uint32_t crc32_table[256];
static int crc32_table_initialized = 0;

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
    crc32_table_initialized = 1;
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

int main(void) {
    const char* test_path = "/tmp/test_checksum.dat";

    printf("=== Checksum Debug Test ===\n\n");

    /* Create a test file */
    printf("1. Creating test file...\n");
    FILE* fp = fopen(test_path, "wb");
    if (!fp) {
        printf("   FAIL: Could not create file\n");
        return 1;
    }

    /* Write header placeholder */
    SaveFileHeader header;
    memset(&header, 0, sizeof(header));
    header.magic = SAVE_MAGIC_NUMBER;
    header.version_major = 1;
    header.version_minor = 0;
    header.version_patch = 0;

    if (fwrite(&header, sizeof(header), 1, fp) != 1) {
        printf("   FAIL: Could not write header\n");
        fclose(fp);
        return 1;
    }

    printf("   Header size: %zu bytes\n", sizeof(header));
    long data_start = ftell(fp);
    printf("   Data starts at offset: %ld\n", data_start);

    /* Write some test data */
    const char* test_data = "Hello, this is test data for checksum validation!";
    size_t data_len = strlen(test_data);
    if (fwrite(test_data, 1, data_len, fp) != data_len) {
        printf("   FAIL: Could not write test data\n");
        fclose(fp);
        return 1;
    }

    printf("   Test data: \"%s\" (%zu bytes)\n", test_data, data_len);

    /* Calculate checksum */
    uint32_t checksum = calculate_crc32(test_data, data_len);
    printf("   Calculated checksum: 0x%08X\n", checksum);

    /* Update header */
    header.checksum = checksum;
    header.data_length = (uint64_t)data_len;

    fseek(fp, 0, SEEK_SET);
    if (fwrite(&header, sizeof(header), 1, fp) != 1) {
        printf("   FAIL: Could not update header\n");
        fclose(fp);
        return 1;
    }

    fclose(fp);
    printf("   SUCCESS: File created\n\n");

    /* Now validate the file */
    printf("2. Validating file...\n");
    fp = fopen(test_path, "rb");
    if (!fp) {
        printf("   FAIL: Could not open file for reading\n");
        return 1;
    }

    SaveFileHeader read_header;
    if (fread(&read_header, sizeof(read_header), 1, fp) != 1) {
        printf("   FAIL: Could not read header\n");
        fclose(fp);
        return 1;
    }

    printf("   Magic: 0x%08X (expected 0x%08X)\n", read_header.magic, SAVE_MAGIC_NUMBER);
    printf("   Version: %u.%u.%u\n", read_header.version_major, read_header.version_minor, read_header.version_patch);
    printf("   Checksum in header: 0x%08X\n", read_header.checksum);
    printf("   Data length in header: %lu\n", (unsigned long)read_header.data_length);

    long pos_after_header = ftell(fp);
    printf("   File position after reading header: %ld\n", pos_after_header);

    /* Read data */
    uint8_t* buffer = malloc((size_t)read_header.data_length);
    if (!buffer) {
        printf("   FAIL: Could not allocate buffer\n");
        fclose(fp);
        return 1;
    }

    size_t bytes_read = fread(buffer, 1, (size_t)read_header.data_length, fp);
    printf("   Bytes read: %zu (expected %lu)\n", bytes_read, (unsigned long)read_header.data_length);

    if (bytes_read != (size_t)read_header.data_length) {
        printf("   FAIL: Could not read all data\n");
        free(buffer);
        fclose(fp);
        return 1;
    }

    /* Verify data content */
    printf("   Data read: \"");
    for (size_t i = 0; i < bytes_read; i++) {
        if (buffer[i] >= 32 && buffer[i] < 127) {
            printf("%c", buffer[i]);
        } else {
            printf(".");
        }
    }
    printf("\"\n");

    /* Calculate checksum */
    uint32_t calc_checksum = calculate_crc32(buffer, (size_t)read_header.data_length);
    printf("   Calculated checksum: 0x%08X\n", calc_checksum);

    free(buffer);
    fclose(fp);

    /* Compare */
    if (calc_checksum == read_header.checksum) {
        printf("   SUCCESS: Checksums match!\n");
        unlink(test_path);
        return 0;
    } else {
        printf("   FAIL: Checksums DO NOT match\n");
        printf("   Expected: 0x%08X\n", read_header.checksum);
        printf("   Got:      0x%08X\n", calc_checksum);
        unlink(test_path);
        return 1;
    }
}

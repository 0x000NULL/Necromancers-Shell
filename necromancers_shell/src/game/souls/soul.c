#include "soul.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Counter for generating unique soul IDs */
static uint32_t g_next_soul_id = 1;

/* Soul type name strings */
static const char* SOUL_TYPE_NAMES[] = {
    "Common",
    "Warrior",
    "Mage",
    "Innocent",
    "Corrupted",
    "Ancient"
};

/* Memory templates for different soul types */
static const char* COMMON_MEMORIES[] = {
    "Memories of simple toil and daily labor",
    "Fragments of a quiet, unremarkable life",
    "Echoes of mundane routines and simple pleasures",
    "Whispers of ordinary joys and sorrows"
};

static const char* WARRIOR_MEMORIES[] = {
    "Battle cries echo through blood-soaked memories",
    "The weight of steel and the taste of victory",
    "Screams of fallen comrades haunt the edges",
    "Glory and carnage intertwined in death's embrace"
};

static const char* MAGE_MEMORIES[] = {
    "Arcane formulas dance at the edge of comprehension",
    "Libraries of lost knowledge flicker in the void",
    "The taste of raw magic lingers on spectral lips",
    "Secrets of forbidden spells whisper endlessly"
};

static const char* INNOCENT_MEMORIES[] = {
    "Laughter of children, now forever silenced",
    "Simple kindness untouched by the world's cruelty",
    "Pure hope that never knew true darkness",
    "Gentle warmth of a life cut too short"
};

static const char* CORRUPTED_MEMORIES[] = {
    "Darkness that spread from within, consuming all",
    "Twisted desires that warped the soul beyond recognition",
    "Malevolence crystallized into spectral essence",
    "Evil that persists even in death's cold grip"
};

static const char* ANCIENT_MEMORIES[] = {
    "Centuries compressed into timeless echoes",
    "Wisdom of ages mixed with the dust of empires",
    "Memories so old they predate written history",
    "Power accumulated across countless lifetimes"
};

Soul* soul_create(SoulType type, SoulQuality quality) {
    /* Validate inputs */
    if (type >= SOUL_TYPE_COUNT) {
        return NULL;
    }

    if (quality > 100) {
        quality = 100;
    }

    /* Allocate soul structure */
    Soul* soul = (Soul*)malloc(sizeof(Soul));
    if (!soul) {
        return NULL;
    }

    /* Initialize fields */
    soul->id = g_next_soul_id++;
    soul->type = type;
    soul->quality = quality;
    soul->energy = soul_calculate_energy(type, quality);
    soul->bound = false;
    soul->bound_minion_id = 0;
    soul->timestamp = time(NULL);

    /* Generate memories */
    soul_generate_memories(soul, type, quality);

    return soul;
}

void soul_destroy(Soul* soul) {
    if (soul) {
        free(soul);
    }
}

const char* soul_type_name(SoulType type) {
    if (type >= SOUL_TYPE_COUNT) {
        return "Unknown";
    }
    return SOUL_TYPE_NAMES[type];
}

uint32_t soul_calculate_energy(SoulType type, SoulQuality quality) {
    /* Clamp quality to valid range */
    if (quality > 100) {
        quality = 100;
    }

    /* Base energy ranges for each type */
    uint32_t base_min, base_max;

    switch (type) {
        case SOUL_TYPE_COMMON:
            base_min = 10;
            base_max = 20;
            break;
        case SOUL_TYPE_WARRIOR:
            base_min = 20;
            base_max = 40;
            break;
        case SOUL_TYPE_MAGE:
            base_min = 30;
            base_max = 50;
            break;
        case SOUL_TYPE_INNOCENT:
            base_min = 15;
            base_max = 25;
            break;
        case SOUL_TYPE_CORRUPTED:
            base_min = 25;
            base_max = 35;
            break;
        case SOUL_TYPE_ANCIENT:
            base_min = 50;
            base_max = 100;
            break;
        default:
            base_min = 10;
            base_max = 20;
            break;
    }

    /* Calculate energy based on quality
     * Formula: base_min + (base_max - base_min) * (quality / 100.0)
     * This gives a value between base_min (at quality 0) and base_max (at quality 100)
     */
    uint32_t range = base_max - base_min;
    uint32_t energy = base_min + (range * quality) / 100;

    return energy;
}

bool soul_bind(Soul* soul, uint32_t minion_id) {
    if (!soul) {
        return false;
    }

    if (soul->bound) {
        return false;
    }

    soul->bound = true;
    soul->bound_minion_id = minion_id;
    return true;
}

bool soul_unbind(Soul* soul) {
    if (!soul) {
        return false;
    }

    if (!soul->bound) {
        return false;
    }

    soul->bound = false;
    soul->bound_minion_id = 0;
    return true;
}

void soul_generate_memories(Soul* soul, SoulType type, SoulQuality quality) {
    if (!soul) {
        return;
    }

    /* Select memory template based on type */
    const char** templates;
    size_t template_count;

    switch (type) {
        case SOUL_TYPE_COMMON:
            templates = COMMON_MEMORIES;
            template_count = sizeof(COMMON_MEMORIES) / sizeof(COMMON_MEMORIES[0]);
            break;
        case SOUL_TYPE_WARRIOR:
            templates = WARRIOR_MEMORIES;
            template_count = sizeof(WARRIOR_MEMORIES) / sizeof(WARRIOR_MEMORIES[0]);
            break;
        case SOUL_TYPE_MAGE:
            templates = MAGE_MEMORIES;
            template_count = sizeof(MAGE_MEMORIES) / sizeof(MAGE_MEMORIES[0]);
            break;
        case SOUL_TYPE_INNOCENT:
            templates = INNOCENT_MEMORIES;
            template_count = sizeof(INNOCENT_MEMORIES) / sizeof(INNOCENT_MEMORIES[0]);
            break;
        case SOUL_TYPE_CORRUPTED:
            templates = CORRUPTED_MEMORIES;
            template_count = sizeof(CORRUPTED_MEMORIES) / sizeof(CORRUPTED_MEMORIES[0]);
            break;
        case SOUL_TYPE_ANCIENT:
            templates = ANCIENT_MEMORIES;
            template_count = sizeof(ANCIENT_MEMORIES) / sizeof(ANCIENT_MEMORIES[0]);
            break;
        default:
            templates = COMMON_MEMORIES;
            template_count = sizeof(COMMON_MEMORIES) / sizeof(COMMON_MEMORIES[0]);
            break;
    }

    /* Select random template based on quality (use quality as seed) */
    size_t index = quality % template_count;
    strncpy(soul->memories, templates[index], SOUL_MEMORY_MAX_LENGTH - 1);
    soul->memories[SOUL_MEMORY_MAX_LENGTH - 1] = '\0';

    /* For low quality souls, corrupt the memories slightly */
    if (quality < 30) {
        /* Truncate some characters to simulate degraded memories */
        size_t len = strlen(soul->memories);
        if (len > 20) {
            soul->memories[len - (30 - quality) / 5] = '.';
            soul->memories[len - (30 - quality) / 5 + 1] = '.';
            soul->memories[len - (30 - quality) / 5 + 2] = '.';
            soul->memories[len - (30 - quality) / 5 + 3] = '\0';
        }
    }
}

int soul_get_description(const Soul* soul, char* buffer, size_t buffer_size) {
    if (!soul || !buffer || buffer_size == 0) {
        return 0;
    }

    int written = 0;

    /* Write type and quality */
    written += snprintf(buffer + written, buffer_size - written,
                       "%s Soul (Quality: %u%%, Energy: %u)",
                       soul_type_name(soul->type), soul->quality, soul->energy);

    /* Write bound status */
    if (written < (int)buffer_size && soul->bound) {
        written += snprintf(buffer + written, buffer_size - written,
                           " [BOUND to minion %u]", soul->bound_minion_id);
    }

    /* Write memories */
    if (written < (int)buffer_size) {
        written += snprintf(buffer + written, buffer_size - written,
                           "\n  Memories: %s", soul->memories);
    }

    return written;
}

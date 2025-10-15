#include "encounter.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Predefined encounter templates */
static const EncounterTemplate g_encounter_templates[] = {
    /* Easy encounters */
    {
        .id = "patrol_weak",
        .description = "A weak patrol of guards",
        .enemy_types = {ENEMY_TYPE_GUARD, ENEMY_TYPE_VILLAGER},
        .enemy_counts = {1, 1},
        .enemy_type_count = 2,
        .difficulty_rating = 2
    },
    {
        .id = "lone_priest",
        .description = "A solitary priest",
        .enemy_types = {ENEMY_TYPE_PRIEST},
        .enemy_counts = {1},
        .enemy_type_count = 1,
        .difficulty_rating = 3
    },

    /* Medium encounters */
    {
        .id = "guard_patrol",
        .description = "A patrol of town guards",
        .enemy_types = {ENEMY_TYPE_GUARD},
        .enemy_counts = {3},
        .enemy_type_count = 1,
        .difficulty_rating = 5
    },
    {
        .id = "church_squad",
        .description = "A church combat squad",
        .enemy_types = {ENEMY_TYPE_PALADIN, ENEMY_TYPE_PRIEST},
        .enemy_counts = {1, 1},
        .enemy_type_count = 2,
        .difficulty_rating = 6
    },

    /* Hard encounters */
    {
        .id = "inquisition_team",
        .description = "An Inquisition strike team",
        .enemy_types = {ENEMY_TYPE_INQUISITOR, ENEMY_TYPE_PALADIN},
        .enemy_counts = {2, 1},
        .enemy_type_count = 2,
        .difficulty_rating = 8
    },
    {
        .id = "rival_necromancer",
        .description = "A rival necromancer",
        .enemy_types = {ENEMY_TYPE_RIVAL_NECROMANCER},
        .enemy_counts = {1},
        .enemy_type_count = 1,
        .difficulty_rating = 7
    },

    /* Boss encounters */
    {
        .id = "church_battalion",
        .description = "A full church battalion",
        .enemy_types = {ENEMY_TYPE_PALADIN, ENEMY_TYPE_PRIEST, ENEMY_TYPE_INQUISITOR},
        .enemy_counts = {2, 2, 1},
        .enemy_type_count = 3,
        .difficulty_rating = 10
    }
};

#define TEMPLATE_COUNT (sizeof(g_encounter_templates) / sizeof(g_encounter_templates[0]))

EncounterTemplate* encounter_generate(uint8_t difficulty, LocationType location_type) {
    /* Select templates matching difficulty range */
    uint8_t min_diff = (difficulty > 2) ? (difficulty - 2) : 1;
    uint8_t max_diff = (difficulty < 8) ? (difficulty + 2) : 10;

    /* Count matching templates */
    uint8_t matching_count = 0;
    for (size_t i = 0; i < TEMPLATE_COUNT; i++) {
        if (g_encounter_templates[i].difficulty_rating >= min_diff &&
            g_encounter_templates[i].difficulty_rating <= max_diff) {
            matching_count++;
        }
    }

    if (matching_count == 0) {
        /* Fallback: return easiest encounter */
        return encounter_create_from_template("lone_priest");
    }

    /* Pick random matching template */
    uint8_t target_index = rand() % matching_count;
    uint8_t seen = 0;

    for (size_t i = 0; i < TEMPLATE_COUNT; i++) {
        if (g_encounter_templates[i].difficulty_rating >= min_diff &&
            g_encounter_templates[i].difficulty_rating <= max_diff) {
            if (seen == target_index) {
                /* Copy this template */
                EncounterTemplate* enc = malloc(sizeof(EncounterTemplate));
                if (enc) {
                    memcpy(enc, &g_encounter_templates[i], sizeof(EncounterTemplate));

                    /* Adjust based on location type */
                    if (location_type == LOCATION_TYPE_VILLAGE) {
                        /* Villages have more villagers */
                        for (uint8_t j = 0; j < enc->enemy_type_count; j++) {
                            if (enc->enemy_types[j] == ENEMY_TYPE_GUARD) {
                                enc->enemy_types[j] = ENEMY_TYPE_VILLAGER;
                            }
                        }
                    } else if (location_type == LOCATION_TYPE_CRYPT) {
                        /* Crypts have more undead hunters */
                        if (enc->difficulty_rating > 5 && enc->enemy_type_count < MAX_ENCOUNTER_ENEMIES) {
                            enc->enemy_types[enc->enemy_type_count] = ENEMY_TYPE_INQUISITOR;
                            enc->enemy_counts[enc->enemy_type_count] = 1;
                            enc->enemy_type_count++;
                        }
                    }
                }
                return enc;
            }
            seen++;
        }
    }

    return NULL;
}

EncounterTemplate* encounter_create_from_template(const char* template_id) {
    if (!template_id) return NULL;

    for (size_t i = 0; i < TEMPLATE_COUNT; i++) {
        if (strcmp(g_encounter_templates[i].id, template_id) == 0) {
            EncounterTemplate* enc = malloc(sizeof(EncounterTemplate));
            if (enc) {
                memcpy(enc, &g_encounter_templates[i], sizeof(EncounterTemplate));
            }
            return enc;
        }
    }

    return NULL;
}

void encounter_destroy(EncounterTemplate* encounter) {
    free(encounter);
}

Enemy** encounter_spawn_enemies(const EncounterTemplate* template, uint8_t* count_out) {
    if (!template || !count_out) return NULL;

    /* Calculate total count */
    uint8_t total = encounter_get_total_enemies(template);
    if (total == 0) {
        *count_out = 0;
        return NULL;
    }

    /* Allocate enemy array */
    Enemy** enemies = calloc(total, sizeof(Enemy*));
    if (!enemies) {
        *count_out = 0;
        return NULL;
    }

    /* Spawn enemies */
    uint8_t index = 0;
    uint32_t next_id = 1;

    for (uint8_t i = 0; i < template->enemy_type_count; i++) {
        EnemyType type = template->enemy_types[i];
        uint8_t count = template->enemy_counts[i];

        for (uint8_t j = 0; j < count; j++) {
            enemies[index] = enemy_create(type, next_id++);
            if (!enemies[index]) {
                /* Cleanup on failure */
                for (uint8_t k = 0; k < index; k++) {
                    enemy_destroy(enemies[k]);
                }
                free(enemies);
                *count_out = 0;
                return NULL;
            }
            index++;
        }
    }

    *count_out = total;
    return enemies;
}

uint8_t encounter_get_total_enemies(const EncounterTemplate* template) {
    if (!template) return 0;

    uint8_t total = 0;
    for (uint8_t i = 0; i < template->enemy_type_count; i++) {
        total += template->enemy_counts[i];
    }

    return total;
}

uint32_t encounter_calculate_threat(const EncounterTemplate* template) {
    if (!template) return 0;

    uint32_t threat = 0;

    for (uint8_t i = 0; i < template->enemy_type_count; i++) {
        uint32_t hp, atk, def, spd;
        enemy_get_base_stats(template->enemy_types[i], &hp, &atk, &def, &spd);

        /* Threat = (HP + ATK + DEF + SPD) * count */
        uint32_t enemy_threat = (hp / 10) + atk + def + spd;
        threat += enemy_threat * template->enemy_counts[i];
    }

    return threat;
}

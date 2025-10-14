#include "minion.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

/**
 * @brief Base stat definitions for each minion type
 */
typedef struct {
    uint32_t health_max;
    uint32_t attack;
    uint32_t defense;
    uint32_t speed;
    uint8_t loyalty;
    uint32_t raise_cost;
} BaseStats;

static const BaseStats g_base_stats[MINION_TYPE_COUNT] = {
    [MINION_TYPE_ZOMBIE] = {
        .health_max = 100,
        .attack = 15,
        .defense = 20,
        .speed = 5,
        .loyalty = 50,
        .raise_cost = 50
    },
    [MINION_TYPE_SKELETON] = {
        .health_max = 50,
        .attack = 25,
        .defense = 10,
        .speed = 15,
        .loyalty = 40,
        .raise_cost = 75
    },
    [MINION_TYPE_GHOUL] = {
        .health_max = 80,
        .attack = 30,
        .defense = 15,
        .speed = 10,
        .loyalty = 35,
        .raise_cost = 100
    },
    [MINION_TYPE_WRAITH] = {
        .health_max = 60,
        .attack = 35,
        .defense = 8,
        .speed = 20,
        .loyalty = 30,
        .raise_cost = 150
    },
    [MINION_TYPE_WIGHT] = {
        .health_max = 120,
        .attack = 40,
        .defense = 25,
        .speed = 12,
        .loyalty = 60,
        .raise_cost = 200
    },
    [MINION_TYPE_REVENANT] = {
        .health_max = 150,
        .attack = 50,
        .defense = 30,
        .speed = 15,
        .loyalty = 80,
        .raise_cost = 300
    }
};

static const char* g_minion_type_names[MINION_TYPE_COUNT] = {
    [MINION_TYPE_ZOMBIE] = "Zombie",
    [MINION_TYPE_SKELETON] = "Skeleton",
    [MINION_TYPE_GHOUL] = "Ghoul",
    [MINION_TYPE_WRAITH] = "Wraith",
    [MINION_TYPE_WIGHT] = "Wight",
    [MINION_TYPE_REVENANT] = "Revenant"
};

Minion* minion_create(MinionType type, const char* name, uint32_t soul_id) {
    if (type >= MINION_TYPE_COUNT) {
        return NULL;
    }

    Minion* minion = (Minion*)malloc(sizeof(Minion));
    if (!minion) {
        return NULL;
    }

    /* Initialize basic fields */
    minion->id = 0;  /* Will be set by caller */
    minion->type = type;
    minion->bound_soul_id = soul_id;
    minion->location_id = 0;  /* Will be set by caller */
    minion->raised_timestamp = (uint64_t)time(NULL);
    minion->experience = 0;
    minion->level = 1;

    /* Generate name if not provided */
    if (name && name[0] != '\0') {
        strncpy(minion->name, name, sizeof(minion->name) - 1);
        minion->name[sizeof(minion->name) - 1] = '\0';
    } else {
        /* Auto-generate name: Type-XXXX */
        snprintf(minion->name, sizeof(minion->name), "%s-%04d",
                 minion_type_name(type), rand() % 10000);
    }

    /* Initialize stats from base stats */
    const BaseStats* base = &g_base_stats[type];
    minion->stats.health_max = base->health_max;
    minion->stats.health = base->health_max;  /* Start at full health */
    minion->stats.attack = base->attack;
    minion->stats.defense = base->defense;
    minion->stats.speed = base->speed;
    minion->stats.loyalty = base->loyalty;

    return minion;
}

void minion_destroy(Minion* minion) {
    if (minion) {
        free(minion);
    }
}

const char* minion_type_name(MinionType type) {
    if (type >= MINION_TYPE_COUNT) {
        return "Unknown";
    }
    return g_minion_type_names[type];
}

void minion_get_base_stats(MinionType type, MinionStats* stats) {
    if (!stats || type >= MINION_TYPE_COUNT) {
        return;
    }

    const BaseStats* base = &g_base_stats[type];
    stats->health_max = base->health_max;
    stats->health = base->health_max;
    stats->attack = base->attack;
    stats->defense = base->defense;
    stats->speed = base->speed;
    stats->loyalty = base->loyalty;
}

uint32_t minion_calculate_raise_cost(MinionType type) {
    if (type >= MINION_TYPE_COUNT) {
        return 0;
    }
    return g_base_stats[type].raise_cost;
}

bool minion_bind_soul(Minion* minion, uint32_t soul_id) {
    if (!minion || minion->bound_soul_id != 0) {
        return false;
    }

    minion->bound_soul_id = soul_id;
    return true;
}

uint32_t minion_unbind_soul(Minion* minion) {
    if (!minion || minion->bound_soul_id == 0) {
        return 0;
    }

    uint32_t soul_id = minion->bound_soul_id;
    minion->bound_soul_id = 0;
    return soul_id;
}

void minion_move_to_location(Minion* minion, uint32_t location_id) {
    if (minion) {
        minion->location_id = location_id;
    }
}

bool minion_add_experience(Minion* minion, uint32_t xp) {
    if (!minion) {
        return false;
    }

    minion->experience += xp;

    /* Calculate XP needed for next level: level * 100 */
    uint32_t xp_needed = minion->level * 100;

    if (minion->experience >= xp_needed) {
        minion->level++;
        minion->experience -= xp_needed;

        /* Level up stat bonuses: +5% to all stats per level (minimum +1) */
        uint32_t new_hp_max = (uint32_t)(minion->stats.health_max * 1.05f);
        if (new_hp_max <= minion->stats.health_max) new_hp_max = minion->stats.health_max + 1;
        minion->stats.health_max = new_hp_max;
        minion->stats.health = minion->stats.health_max;  /* Restore to full on level up */

        uint32_t new_attack = (uint32_t)(minion->stats.attack * 1.05f);
        if (new_attack <= minion->stats.attack) new_attack = minion->stats.attack + 1;
        minion->stats.attack = new_attack;

        uint32_t new_defense = (uint32_t)(minion->stats.defense * 1.05f);
        if (new_defense <= minion->stats.defense) new_defense = minion->stats.defense + 1;
        minion->stats.defense = new_defense;

        uint32_t new_speed = (uint32_t)(minion->stats.speed * 1.05f);
        if (new_speed <= minion->stats.speed) new_speed = minion->stats.speed + 1;
        minion->stats.speed = new_speed;

        return true;  /* Leveled up */
    }

    return false;  /* No level up */
}

bool minion_take_damage(Minion* minion, uint32_t damage) {
    if (!minion) {
        return false;
    }

    if (damage >= minion->stats.health) {
        minion->stats.health = 0;
        return false;  /* Dead */
    }

    minion->stats.health -= damage;
    return true;  /* Still alive */
}

void minion_heal(Minion* minion, uint32_t amount) {
    if (!minion) {
        return;
    }

    minion->stats.health += amount;
    if (minion->stats.health > minion->stats.health_max) {
        minion->stats.health = minion->stats.health_max;
    }
}

void minion_get_description(const Minion* minion, char* buffer, size_t buffer_size) {
    if (!minion || !buffer || buffer_size == 0) {
        return;
    }

    snprintf(buffer, buffer_size,
             "%s '%s' (ID: %u, Level %u)\n"
             "Type: %s | Soul: %s\n"
             "HP: %u/%u | Atk: %u | Def: %u | Spd: %u | Loyalty: %u%%\n"
             "Experience: %u/%u",
             minion_type_name(minion->type),
             minion->name,
             minion->id,
             minion->level,
             minion_type_name(minion->type),
             minion->bound_soul_id ? "Bound" : "None",
             minion->stats.health,
             minion->stats.health_max,
             minion->stats.attack,
             minion->stats.defense,
             minion->stats.speed,
             minion->stats.loyalty,
             minion->experience,
             minion->level * 100);
}

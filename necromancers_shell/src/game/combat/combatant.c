#include "combatant.h"
#include "../minions/minion.h"
#include "enemy.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Helper to get random number in range */
static uint8_t random_range(uint8_t min, uint8_t max) {
    return min + (rand() % (max - min + 1));
}

Combatant* combatant_create_from_minion(void* minion_entity, bool is_player_controlled) {
    if (!minion_entity) {
        return NULL;
    }

    Minion* minion = (Minion*)minion_entity;
    Combatant* combatant = calloc(1, sizeof(Combatant));
    if (!combatant) {
        return NULL;
    }

    /* Copy identification */
    snprintf(combatant->id, sizeof(combatant->id), "M%u", minion->id);
    strncpy(combatant->name, minion->name, sizeof(combatant->name) - 1);
    combatant->type = COMBATANT_TYPE_MINION;

    /* Copy stats from minion */
    combatant->health = minion->stats.health;
    combatant->health_max = minion->stats.health_max;
    combatant->attack = minion->stats.attack;
    combatant->defense = minion->stats.defense;
    combatant->speed = minion->stats.speed;

    /* Control settings */
    combatant->is_player_controlled = is_player_controlled;
    combatant->ai_func = NULL;  /* Player controls minions */

    /* Entity reference */
    combatant->entity = minion_entity;

    /* Combat state */
    combatant->has_acted_this_turn = false;
    combatant->is_defending = false;
    combatant->initiative = 0;

    return combatant;
}

Combatant* combatant_create_from_enemy(void* enemy_entity, AIFunction ai_func) {
    if (!enemy_entity) {
        return NULL;
    }

    Enemy* enemy = (Enemy*)enemy_entity;
    Combatant* combatant = calloc(1, sizeof(Combatant));
    if (!combatant) {
        return NULL;
    }

    /* Copy identification */
    snprintf(combatant->id, sizeof(combatant->id), "E%u", enemy->id);
    strncpy(combatant->name, enemy->name, sizeof(combatant->name) - 1);
    combatant->type = COMBATANT_TYPE_ENEMY;

    /* Copy stats from enemy */
    combatant->health = enemy->health;
    combatant->health_max = enemy->health_max;
    combatant->attack = enemy->attack;
    combatant->defense = enemy->defense;
    combatant->speed = enemy->speed;

    /* Control settings */
    combatant->is_player_controlled = false;
    combatant->ai_func = ai_func;

    /* Entity reference */
    combatant->entity = enemy_entity;

    /* Combat state */
    combatant->has_acted_this_turn = false;
    combatant->is_defending = false;
    combatant->initiative = 0;

    return combatant;
}

void combatant_destroy(Combatant* combatant) {
    if (!combatant) {
        return;
    }

    /* Note: Does NOT free the underlying entity (minion/enemy) */
    /* Those are managed by their respective managers */
    free(combatant);
}

bool combatant_take_damage(Combatant* combatant, uint32_t damage) {
    if (!combatant) {
        return false;
    }

    if (damage >= combatant->health) {
        combatant->health = 0;
        return false;  /* Dead */
    }

    combatant->health -= damage;
    return true;  /* Still alive */
}

void combatant_heal(Combatant* combatant, uint32_t amount) {
    if (!combatant) {
        return;
    }

    combatant->health += amount;
    if (combatant->health > combatant->health_max) {
        combatant->health = combatant->health_max;
    }
}

bool combatant_is_alive(const Combatant* combatant) {
    if (!combatant) {
        return false;
    }
    return combatant->health > 0;
}

void combatant_set_defending(Combatant* combatant, bool defending) {
    if (!combatant) {
        return;
    }
    combatant->is_defending = defending;
}

uint32_t combatant_get_effective_defense(const Combatant* combatant) {
    if (!combatant) {
        return 0;
    }

    uint32_t defense = combatant->defense;
    if (combatant->is_defending) {
        defense = (defense * 150) / 100;  /* +50% */
    }

    return defense;
}

void combatant_reset_turn_flags(Combatant* combatant) {
    if (!combatant) {
        return;
    }

    combatant->has_acted_this_turn = false;
    combatant->is_defending = false;
}

void combatant_roll_initiative(Combatant* combatant) {
    if (!combatant) {
        return;
    }

    /* Initiative = speed (0-255 max) + random(0-50) */
    uint8_t speed_component = (uint8_t)(combatant->speed > 255 ? 255 : combatant->speed);
    uint8_t random_component = random_range(0, 50);

    uint16_t total = speed_component + random_component;
    combatant->initiative = (uint8_t)(total > 255 ? 255 : total);
}

void combatant_sync_to_entity(Combatant* combatant) {
    if (!combatant || !combatant->entity) {
        return;
    }

    if (combatant->type == COMBATANT_TYPE_MINION) {
        Minion* minion = (Minion*)combatant->entity;
        minion->stats.health = combatant->health;
        minion->stats.attack = combatant->attack;
        minion->stats.defense = combatant->defense;
        minion->stats.speed = combatant->speed;
    } else if (combatant->type == COMBATANT_TYPE_ENEMY) {
        Enemy* enemy = (Enemy*)combatant->entity;
        enemy->health = combatant->health;
        enemy->attack = combatant->attack;
        enemy->defense = combatant->defense;
        enemy->speed = combatant->speed;
    }
}

void combatant_get_description(const Combatant* combatant, char* buffer, size_t buffer_size) {
    if (!combatant || !buffer || buffer_size == 0) {
        return;
    }

    const char* type_str = "Unknown";
    if (combatant->type == COMBATANT_TYPE_MINION) {
        type_str = "Minion";
    } else if (combatant->type == COMBATANT_TYPE_ENEMY) {
        type_str = "Enemy";
    }

    snprintf(buffer, buffer_size,
        "%s \"%s\" [%s] HP: %u/%u  ATK: %u  DEF: %u  SPD: %u  INIT: %u",
        type_str,
        combatant->name,
        combatant->id,
        combatant->health,
        combatant->health_max,
        combatant->attack,
        combatant->defense,
        combatant->speed,
        combatant->initiative
    );
}

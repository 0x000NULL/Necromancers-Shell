#include "enemy.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Enemy type definitions with base stats */
typedef struct {
    const char* name;
    const char* description;
    uint32_t hp;
    uint32_t attack;
    uint32_t defense;
    uint32_t speed;
    AIBehavior default_ai;
    uint32_t exp_reward;
    uint32_t energy_reward;
} EnemyTypeDef;

static const EnemyTypeDef g_enemy_types[ENEMY_TYPE_COUNT] = {
    [ENEMY_TYPE_PALADIN] = {
        .name = "Paladin",
        .description = "A holy warrior clad in blessed armor, sworn enemy of the undead",
        .hp = 120,
        .attack = 35,
        .defense = 40,
        .speed = 8,
        .default_ai = AI_BEHAVIOR_DEFENSIVE,
        .exp_reward = 50,
        .energy_reward = 40
    },
    [ENEMY_TYPE_PRIEST] = {
        .name = "Priest",
        .description = "A servant of the light, capable of healing and banishment",
        .hp = 60,
        .attack = 20,
        .defense = 15,
        .speed = 10,
        .default_ai = AI_BEHAVIOR_SUPPORT,
        .exp_reward = 35,
        .energy_reward = 30
    },
    [ENEMY_TYPE_INQUISITOR] = {
        .name = "Inquisitor",
        .description = "An anti-undead specialist wielding holy fire and silver blades",
        .hp = 80,
        .attack = 50,
        .defense = 20,
        .speed = 12,
        .default_ai = AI_BEHAVIOR_AGGRESSIVE,
        .exp_reward = 60,
        .energy_reward = 50
    },
    [ENEMY_TYPE_VILLAGER] = {
        .name = "Villager",
        .description = "A frightened civilian armed with improvised weapons",
        .hp = 30,
        .attack = 5,
        .defense = 5,
        .speed = 6,
        .default_ai = AI_BEHAVIOR_DEFENSIVE,
        .exp_reward = 10,
        .energy_reward = 5
    },
    [ENEMY_TYPE_GUARD] = {
        .name = "Guard",
        .description = "A town guard trained in basic combat",
        .hp = 60,
        .attack = 25,
        .defense = 15,
        .speed = 9,
        .default_ai = AI_BEHAVIOR_BALANCED,
        .exp_reward = 25,
        .energy_reward = 20
    },
    [ENEMY_TYPE_RIVAL_NECROMANCER] = {
        .name = "Rival Necromancer",
        .description = "A competing practitioner of the dark arts",
        .hp = 70,
        .attack = 40,
        .defense = 18,
        .speed = 11,
        .default_ai = AI_BEHAVIOR_TACTICAL,
        .exp_reward = 70,
        .energy_reward = 60
    }
};

Enemy* enemy_create(EnemyType type, uint32_t id) {
    if (type >= ENEMY_TYPE_COUNT) {
        return NULL;
    }

    Enemy* enemy = calloc(1, sizeof(Enemy));
    if (!enemy) {
        return NULL;
    }

    const EnemyTypeDef* def = &g_enemy_types[type];

    enemy->id = id;
    enemy->type = type;
    strncpy(enemy->name, def->name, sizeof(enemy->name) - 1);
    strncpy(enemy->description, def->description, sizeof(enemy->description) - 1);

    /* Set base stats */
    enemy->health_max = def->hp;
    enemy->health = def->hp;
    enemy->attack = def->attack;
    enemy->defense = def->defense;
    enemy->speed = def->speed;

    /* Set AI and rewards */
    enemy->ai_behavior = def->default_ai;
    enemy->experience_reward = def->exp_reward;
    enemy->soul_energy_reward = def->energy_reward;

    return enemy;
}

void enemy_destroy(Enemy* enemy) {
    free(enemy);
}

const char* enemy_type_name(EnemyType type) {
    if (type >= ENEMY_TYPE_COUNT) {
        return "Unknown";
    }
    return g_enemy_types[type].name;
}

void enemy_get_base_stats(EnemyType type, uint32_t* hp, uint32_t* atk, uint32_t* def, uint32_t* spd) {
    if (type >= ENEMY_TYPE_COUNT) {
        return;
    }

    const EnemyTypeDef* enemy_def = &g_enemy_types[type];

    if (hp) *hp = enemy_def->hp;
    if (atk) *atk = enemy_def->attack;
    if (def) *def = enemy_def->defense;
    if (spd) *spd = enemy_def->speed;
}

AIBehavior enemy_get_default_ai(EnemyType type) {
    if (type >= ENEMY_TYPE_COUNT) {
        return AI_BEHAVIOR_BALANCED;
    }
    return g_enemy_types[type].default_ai;
}

uint32_t enemy_get_experience_reward(const Enemy* enemy) {
    if (!enemy) {
        return 0;
    }
    return enemy->experience_reward;
}

uint32_t enemy_get_soul_energy_reward(const Enemy* enemy) {
    if (!enemy) {
        return 0;
    }
    return enemy->soul_energy_reward;
}

bool enemy_take_damage(Enemy* enemy, uint32_t damage) {
    if (!enemy) {
        return false;
    }

    if (damage >= enemy->health) {
        enemy->health = 0;
        return false;  /* Dead */
    }

    enemy->health -= damage;
    return true;  /* Still alive */
}

void enemy_heal(Enemy* enemy, uint32_t amount) {
    if (!enemy) {
        return;
    }

    enemy->health += amount;
    if (enemy->health > enemy->health_max) {
        enemy->health = enemy->health_max;
    }
}

bool enemy_is_alive(const Enemy* enemy) {
    if (!enemy) {
        return false;
    }
    return enemy->health > 0;
}

void enemy_get_description(const Enemy* enemy, char* buffer, size_t buffer_size) {
    if (!enemy || !buffer || buffer_size == 0) {
        return;
    }

    snprintf(buffer, buffer_size,
        "%s (Level %u) HP: %u/%u  ATK: %u  DEF: %u  SPD: %u",
        enemy->name,
        1,  /* Level - could be dynamic later */
        enemy->health,
        enemy->health_max,
        enemy->attack,
        enemy->defense,
        enemy->speed
    );
}

const char* ai_behavior_name(AIBehavior behavior) {
    switch (behavior) {
        case AI_BEHAVIOR_AGGRESSIVE:  return "Aggressive";
        case AI_BEHAVIOR_DEFENSIVE:   return "Defensive";
        case AI_BEHAVIOR_BALANCED:    return "Balanced";
        case AI_BEHAVIOR_SUPPORT:     return "Support";
        case AI_BEHAVIOR_TACTICAL:    return "Tactical";
        default:                      return "Unknown";
    }
}

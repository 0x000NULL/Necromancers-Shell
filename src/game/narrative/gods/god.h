/**
 * @file god.h
 * @brief Divine Architect entity system
 *
 * Manages the Seven Architects - god-tier characters with divine powers,
 * domains, favor tracking, and judgment mechanics.
 */

#ifndef NECROMANCERS_GOD_H
#define NECROMANCERS_GOD_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

/* Maximum values for god data */
#define MAX_GOD_DIALOGUE_TREES 8
#define MAX_GOD_TRIALS 7
#define MAX_GOD_RESTRICTIONS 5

/**
 * @brief Divine power level
 *
 * Categorizes entities by their power tier.
 */
typedef enum {
    POWER_MORTAL,              /**< Regular mortal/necromancer */
    POWER_AVATAR,              /**< Minor avatar or demigod */
    POWER_LESSER_DIVINE,       /**< Lesser deity */
    POWER_DIVINE_ARCHITECT     /**< One of the Seven Architects (max power) */
} PowerLevel;

/**
 * @brief God domain types
 *
 * Each Architect governs a specific domain of existence.
 */
typedef enum {
    DOMAIN_LIFE,               /**< Anara - Life and creation */
    DOMAIN_ORDER,              /**< Keldrin - Structure and law */
    DOMAIN_TIME,               /**< Theros - Time and eternity */
    DOMAIN_SOULS,              /**< Myrith - Consciousness and souls */
    DOMAIN_ENTROPY,            /**< Vorathos - Void and decay */
    DOMAIN_BOUNDARIES,         /**< Seraph - Protection and limits */
    DOMAIN_NETWORKS            /**< Nexus - Connections and systems */
} GodDomain;

/**
 * @brief God structure
 *
 * Represents one of the Seven Divine Architects.
 */
typedef struct {
    /* Identity */
    char id[64];
    char name[128];
    char title[128];
    char description[512];

    /* Divine attributes */
    GodDomain domain;
    PowerLevel power_level;
    char manifestation[256];  /**< How they appear/manifest */
    char personality[256];    /**< Core personality traits */

    /* Player relationship */
    int16_t favor;            /**< Divine favor (-100 to +100) */
    int16_t favor_min;        /**< Minimum possible favor */
    int16_t favor_max;        /**< Maximum possible favor */
    int16_t favor_start;      /**< Starting favor (usually 0) */

    /* Interaction tracking */
    uint32_t interactions;    /**< Number of interactions with player */
    bool summoned;            /**< Has player summoned this god? */
    bool judgment_given;      /**< Has god passed judgment on player? */

    /* Combat/trials */
    bool combat_possible;     /**< Can player fight this god? */
    uint32_t combat_difficulty; /**< Difficulty rating (if combat possible) */

    /* Dialogue trees */
    char dialogue_trees[MAX_GOD_DIALOGUE_TREES][64];
    size_t dialogue_tree_count;

    /* Trials/challenges */
    char trials[MAX_GOD_TRIALS][64];
    size_t trial_count;

    /* Restrictions imposed (after judgment) */
    char restrictions[MAX_GOD_RESTRICTIONS][256];
    size_t restriction_count;

    /* Amnesty/judgment state */
    bool amnesty_granted;     /**< Has god granted amnesty to player? */
    bool condemned;           /**< Has god condemned player? */
} God;

/**
 * @brief Create a new god
 *
 * @param id Unique identifier for the god
 * @param name Display name
 * @param domain God's domain
 * @return Newly allocated God or NULL on failure
 */
God* god_create(const char* id, const char* name, GodDomain domain);

/**
 * @brief Destroy a god and free memory
 *
 * @param god God to destroy (can be NULL)
 */
void god_destroy(God* god);

/**
 * @brief Modify god's favor toward player
 *
 * Adjusts favor by specified amount (can be negative).
 * Favor is clamped to [favor_min, favor_max].
 *
 * @param god God to modify
 * @param change Amount to change favor (positive or negative)
 * @return true on success, false if god is NULL
 */
bool god_modify_favor(God* god, int16_t change);

/**
 * @brief Set god's favor to specific value
 *
 * @param god God to modify
 * @param favor New favor value
 * @return true on success, false if god is NULL
 */
bool god_set_favor(God* god, int16_t favor);

/**
 * @brief Add a dialogue tree reference
 *
 * @param god God to modify
 * @param tree_id Dialogue tree identifier
 * @return true on success, false if god is NULL or tree list is full
 */
bool god_add_dialogue_tree(God* god, const char* tree_id);

/**
 * @brief Add a trial reference
 *
 * @param god God to modify
 * @param trial_id Trial identifier
 * @return true on success, false if god is NULL or trial list is full
 */
bool god_add_trial(God* god, const char* trial_id);

/**
 * @brief Add a restriction (after judgment)
 *
 * Gods can impose restrictions on player after judgment.
 *
 * @param god God to modify
 * @param restriction Text description of restriction
 * @return true on success, false if god is NULL or restriction list is full
 */
bool god_add_restriction(God* god, const char* restriction);

/**
 * @brief Grant amnesty to player
 *
 * Sets amnesty_granted flag and typically increases favor.
 *
 * @param god God to modify
 * @return true on success, false if god is NULL or already granted
 */
bool god_grant_amnesty(God* god);

/**
 * @brief Condemn player
 *
 * Sets condemned flag and typically decreases favor significantly.
 *
 * @param god God to modify
 * @return true on success, false if god is NULL or already condemned
 */
bool god_condemn(God* god);

/**
 * @brief Mark god as summoned
 *
 * Records that player has successfully summoned this god.
 *
 * @param god God to modify
 */
void god_mark_summoned(God* god);

/**
 * @brief Mark judgment as given
 *
 * Records that god has passed judgment on player.
 *
 * @param god God to modify
 */
void god_mark_judgment_given(God* god);

/**
 * @brief Record an interaction
 *
 * Increments interaction counter.
 *
 * @param god God to modify
 */
void god_record_interaction(God* god);

/**
 * @brief Check if god's favor is positive
 *
 * @param god God to check
 * @return true if favor > 0, false otherwise
 */
bool god_has_positive_favor(const God* god);

/**
 * @brief Check if god's favor is negative
 *
 * @param god God to check
 * @return true if favor < 0, false otherwise
 */
bool god_has_negative_favor(const God* god);

/**
 * @brief Check if god's favor is neutral
 *
 * @param god God to check
 * @return true if favor == 0, false otherwise
 */
bool god_is_neutral(const God* god);

/**
 * @brief Get favor level description
 *
 * Returns string describing favor level (e.g., "Revered", "Hated").
 *
 * @param god God to check
 * @return String describing favor level
 */
const char* god_get_favor_description(const God* god);

/**
 * @brief Get domain name
 *
 * @param domain God domain
 * @return String name (e.g., "Life", "Order")
 */
const char* god_domain_name(GodDomain domain);

/**
 * @brief Get power level name
 *
 * @param power Power level
 * @return String name (e.g., "Divine Architect")
 */
const char* god_power_level_name(PowerLevel power);

/**
 * @brief Check if god can be challenged in combat
 *
 * @param god God to check
 * @return true if combat_possible is true, false otherwise
 */
bool god_can_be_challenged(const God* god);

#endif /* NECROMANCERS_GOD_H */

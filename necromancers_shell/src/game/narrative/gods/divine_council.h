/**
 * @file divine_council.h
 * @brief Divine Council management system
 *
 * Manages the Seven Architects collectively, handling council summoning,
 * multi-god dialogue, collective judgment, and divine verdict mechanics.
 */

#ifndef NECROMANCERS_DIVINE_COUNCIL_H
#define NECROMANCERS_DIVINE_COUNCIL_H

#include "god.h"
#include <stdbool.h>
#include <stddef.h>

/* Maximum number of gods in council */
#define MAX_COUNCIL_GODS 7

/* Maximum number of collective restrictions */
#define MAX_COUNCIL_RESTRICTIONS 10

/**
 * @brief Divine verdict types
 *
 * Possible outcomes from Divine Council judgment.
 */
typedef enum {
    VERDICT_NONE,              /**< No verdict yet */
    VERDICT_AMNESTY,           /**< Full amnesty granted */
    VERDICT_CONDITIONAL,       /**< Amnesty with restrictions */
    VERDICT_PURGE,             /**< Condemned to Purge */
    VERDICT_IMMEDIATE_DEATH    /**< Immediate execution (rare) */
} DivineVerdict;

/**
 * @brief Divine Council structure
 *
 * Manages the Seven Architects as a collective entity.
 */
typedef struct {
    God* gods[MAX_COUNCIL_GODS];        /**< The Seven Architects */
    size_t god_count;                   /**< Number of gods (should be 7) */

    /* Council state */
    bool council_summoned;              /**< Has council been summoned? */
    uint32_t summon_day;                /**< Day when council was summoned */
    bool judgment_complete;             /**< Has judgment been passed? */

    /* Verdict */
    DivineVerdict verdict;              /**< Collective verdict */
    char verdict_text[1024];            /**< Text of verdict */
    char restrictions[MAX_COUNCIL_RESTRICTIONS][256]; /**< Imposed restrictions */
    size_t restriction_count;

    /* Vote tracking (for conditional verdicts) */
    uint8_t votes_amnesty;              /**< Gods voting for amnesty */
    uint8_t votes_conditional;          /**< Gods voting conditional */
    uint8_t votes_purge;                /**< Gods voting for purge */
    uint8_t votes_death;                /**< Gods voting for death */

    /* Council statistics */
    int16_t average_favor;              /**< Average favor across all gods */
    uint32_t total_interactions;        /**< Total interactions across all gods */
} DivineCouncil;

/**
 * @brief Create divine council
 *
 * @return Newly allocated DivineCouncil or NULL on failure
 */
DivineCouncil* divine_council_create(void);

/**
 * @brief Destroy divine council and all gods
 *
 * @param council Council to destroy (can be NULL)
 */
void divine_council_destroy(DivineCouncil* council);

/**
 * @brief Add a god to the council
 *
 * @param council Divine council
 * @param god God to add (council takes ownership)
 * @return true on success, false if council is full or inputs are NULL
 */
bool divine_council_add_god(DivineCouncil* council, God* god);

/**
 * @brief Find god by ID
 *
 * @param council Divine council
 * @param god_id Unique god identifier
 * @return Pointer to god, or NULL if not found
 */
God* divine_council_find_god(const DivineCouncil* council, const char* god_id);

/**
 * @brief Get god by index
 *
 * @param council Divine council
 * @param index God index (0 to god_count-1)
 * @return Pointer to god, or NULL if index out of range
 */
God* divine_council_get_god(const DivineCouncil* council, size_t index);

/**
 * @brief Summon the Divine Council
 *
 * Marks council as summoned and records the day.
 * Can only be summoned once.
 *
 * @param council Divine council
 * @param day Current game day
 * @return true on success, false if already summoned or council is NULL
 */
bool divine_council_summon(DivineCouncil* council, uint32_t day);

/**
 * @brief Pass collective judgment
 *
 * Gods vote based on their favor and player actions.
 * Verdict is determined by majority vote.
 *
 * @param council Divine council
 * @param player_corruption Player's corruption level (0-100)
 * @param player_consciousness Player's consciousness level (0-100)
 * @return true on success, false if council is NULL or judgment already complete
 */
bool divine_council_pass_judgment(DivineCouncil* council, uint8_t player_corruption,
                                  float player_consciousness);

/**
 * @brief Add restriction to verdict
 *
 * Adds a restriction to the conditional verdict.
 *
 * @param council Divine council
 * @param restriction Text description of restriction
 * @return true on success, false if council is NULL or restriction list is full
 */
bool divine_council_add_restriction(DivineCouncil* council, const char* restriction);

/**
 * @brief Calculate average favor across all gods
 *
 * @param council Divine council
 * @return Average favor (-100 to 100), or 0 if council is NULL
 */
int16_t divine_council_calculate_average_favor(const DivineCouncil* council);

/**
 * @brief Calculate total interactions across all gods
 *
 * @param council Divine council
 * @return Total interaction count
 */
uint32_t divine_council_calculate_total_interactions(const DivineCouncil* council);

/**
 * @brief Check if council can be summoned
 *
 * Requires Day 162+ and not already summoned.
 *
 * @param council Divine council
 * @param current_day Current game day
 * @return true if summonable, false otherwise
 */
bool divine_council_can_summon(const DivineCouncil* council, uint32_t current_day);

/**
 * @brief Check if majority favor amnesty
 *
 * @param council Divine council
 * @return true if average favor >= 20, false otherwise
 */
bool divine_council_favor_amnesty(const DivineCouncil* council);

/**
 * @brief Check if majority favor purge
 *
 * @param council Divine council
 * @return true if average favor <= -40, false otherwise
 */
bool divine_council_favor_purge(const DivineCouncil* council);

/**
 * @brief Get verdict name
 *
 * @param verdict Divine verdict
 * @return String name (e.g., "Amnesty", "Conditional")
 */
const char* divine_council_verdict_name(DivineVerdict verdict);

/**
 * @brief Get verdict description
 *
 * Returns full text explanation of verdict.
 *
 * @param council Divine council
 * @return String describing verdict
 */
const char* divine_council_get_verdict_description(const DivineCouncil* council);

/**
 * @brief Count gods with positive favor
 *
 * @param council Divine council
 * @return Number of gods with favor > 0
 */
size_t divine_council_count_positive_favor(const DivineCouncil* council);

/**
 * @brief Count gods with negative favor
 *
 * @param council Divine council
 * @return Number of gods with favor < 0
 */
size_t divine_council_count_negative_favor(const DivineCouncil* council);

/**
 * @brief Check if all gods have voted
 *
 * @param council Divine council
 * @return true if total votes == god_count, false otherwise
 */
bool divine_council_all_voted(const DivineCouncil* council);

#endif /* NECROMANCERS_DIVINE_COUNCIL_H */

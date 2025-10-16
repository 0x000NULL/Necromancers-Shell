/**
 * @file thessara.h
 * @brief Thessara Ghost system - 3,000-year-old mentor NPC
 *
 * Manages Thessara, the original necromancer who exists as a ghost in null space.
 * Provides consciousness-to-consciousness knowledge transfer, mentorship, and
 * serves as guide for hidden paths (Wraith, Morningstar).
 */

#ifndef NECROMANCERS_THESSARA_H
#define NECROMANCERS_THESSARA_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

/* Maximum values for Thessara data */
#define MAX_THESSARA_KNOWLEDGE 50
#define MAX_THESSARA_WARNINGS 20

/**
 * @brief Knowledge transfer types
 *
 * Types of knowledge Thessara can instantly transfer.
 */
typedef enum {
    KNOWLEDGE_RESEARCH,        /**< Complete research project instantly */
    KNOWLEDGE_SPELL,           /**< Learn spell instantly */
    KNOWLEDGE_TECHNIQUE,       /**< Master technique */
    KNOWLEDGE_HISTORY,         /**< Historical information */
    KNOWLEDGE_PATH_SECRET      /**< Hidden path revelation (Wraith, Morningstar) */
} KnowledgeType;

/**
 * @brief Knowledge transfer record
 *
 * Records a specific knowledge transfer from Thessara.
 */
typedef struct {
    KnowledgeType type;
    char id[64];               /**< Research/spell/technique ID */
    char description[256];     /**< What was learned */
    uint32_t day_transferred;  /**< Day when transfer occurred */
} KnowledgeTransfer;

/**
 * @brief Thessara relationship structure
 *
 * Tracks player's relationship with Thessara ghost.
 */
typedef struct {
    /* Discovery */
    bool discovered;           /**< Has player found Thessara? */
    uint32_t discovery_day;    /**< Day when first discovered */

    /* Connection state */
    bool severed;              /**< True after Trial 6 sacrifice */
    uint32_t severed_day;      /**< Day when connection severed */

    /* Interaction tracking */
    uint32_t meetings_count;   /**< Number of meetings in null space */
    uint32_t last_meeting_day; /**< Day of last meeting */

    /* Knowledge transfers */
    KnowledgeTransfer transfers[MAX_THESSARA_KNOWLEDGE];
    size_t transfer_count;

    /* Trust level */
    float trust_level;         /**< Trust (0-100%) */

    /* Warnings given */
    char warnings[MAX_THESSARA_WARNINGS][256];
    size_t warning_count;

    /* Path revelations */
    bool wraith_path_revealed; /**< Has she revealed Wraith path? */
    bool morningstar_path_revealed; /**< Has she revealed Morningstar path? */
    bool archon_guidance_given; /**< Has she provided Archon guidance? */

    /* Mentorship metrics */
    uint32_t total_guidance_time; /**< Total hours of guidance */
    uint32_t trials_assisted;     /**< Number of trials she helped with */
} ThessaraRelationship;

/**
 * @brief Create Thessara relationship
 *
 * @return Newly allocated ThessaraRelationship or NULL on failure
 */
ThessaraRelationship* thessara_create(void);

/**
 * @brief Destroy Thessara relationship and free memory
 *
 * @param thessara Relationship to destroy (can be NULL)
 */
void thessara_destroy(ThessaraRelationship* thessara);

/**
 * @brief Mark Thessara as discovered
 *
 * Records first contact with Thessara ghost.
 *
 * @param thessara Thessara relationship
 * @param day Current game day
 * @return true on success, false if already discovered or thessara is NULL
 */
bool thessara_discover(ThessaraRelationship* thessara, uint32_t day);

/**
 * @brief Sever connection with Thessara (Trial 6 sacrifice)
 *
 * Permanently severs connection to save distant child.
 * This is irreversible and prevents future meetings.
 *
 * @param thessara Thessara relationship
 * @param day Current game day
 * @return true on success, false if already severed or thessara is NULL
 */
bool thessara_sever_connection(ThessaraRelationship* thessara, uint32_t day);

/**
 * @brief Record a meeting with Thessara
 *
 * Increments meeting counter and updates last meeting day.
 *
 * @param thessara Thessara relationship
 * @param day Current game day
 * @return true on success, false if connection severed or thessara is NULL
 */
bool thessara_record_meeting(ThessaraRelationship* thessara, uint32_t day);

/**
 * @brief Transfer knowledge to player
 *
 * Instant consciousness-to-consciousness knowledge transfer.
 * More powerful than standard learning - bypasses time requirements.
 *
 * @param thessara Thessara relationship
 * @param type Type of knowledge
 * @param id Knowledge identifier (research ID, spell name, etc.)
 * @param description Text description of what was learned
 * @param day Current game day
 * @return true on success, false if connection severed or transfer list full
 */
bool thessara_transfer_knowledge(ThessaraRelationship* thessara, KnowledgeType type,
                                  const char* id, const char* description, uint32_t day);

/**
 * @brief Add trust
 *
 * Increases trust level (capped at 100%).
 *
 * @param thessara Thessara relationship
 * @param amount Amount to increase (0-100)
 * @return true on success, false if thessara is NULL
 */
bool thessara_add_trust(ThessaraRelationship* thessara, float amount);

/**
 * @brief Reduce trust
 *
 * Decreases trust level (minimum 0%).
 *
 * @param thessara Thessara relationship
 * @param amount Amount to decrease (0-100)
 * @return true on success, false if thessara is NULL
 */
bool thessara_reduce_trust(ThessaraRelationship* thessara, float amount);

/**
 * @brief Give a warning
 *
 * Thessara provides warning about player's path/choices.
 *
 * @param thessara Thessara relationship
 * @param warning Warning text
 * @return true on success, false if warning list full or thessara is NULL
 */
bool thessara_give_warning(ThessaraRelationship* thessara, const char* warning);

/**
 * @brief Reveal Wraith path
 *
 * Thessara explains how to achieve Wraith transformation.
 *
 * @param thessara Thessara relationship
 * @return true on success, false if already revealed or thessara is NULL
 */
bool thessara_reveal_wraith_path(ThessaraRelationship* thessara);

/**
 * @brief Reveal Morningstar path
 *
 * Thessara explains the impossible Morningstar path (only 1 success in 3,000 years).
 *
 * @param thessara Thessara relationship
 * @return true on success, false if already revealed or thessara is NULL
 */
bool thessara_reveal_morningstar_path(ThessaraRelationship* thessara);

/**
 * @brief Provide Archon guidance
 *
 * Thessara helps prepare for Archon trials.
 *
 * @param thessara Thessara relationship
 * @return true on success, false if already given or thessara is NULL
 */
bool thessara_give_archon_guidance(ThessaraRelationship* thessara);

/**
 * @brief Record trial assistance
 *
 * Thessara helped with a trial (before Trial 6).
 *
 * @param thessara Thessara relationship
 */
void thessara_assist_trial(ThessaraRelationship* thessara);

/**
 * @brief Add guidance time
 *
 * Records hours spent in mentorship.
 *
 * @param thessara Thessara relationship
 * @param hours Hours to add
 */
void thessara_add_guidance_time(ThessaraRelationship* thessara, uint32_t hours);

/**
 * @brief Check if connection is severed
 *
 * @param thessara Thessara relationship
 * @return true if severed, false otherwise
 */
bool thessara_is_severed(const ThessaraRelationship* thessara);

/**
 * @brief Check if Thessara can be met
 *
 * Connection must not be severed.
 *
 * @param thessara Thessara relationship
 * @return true if can meet, false if severed or thessara is NULL
 */
bool thessara_can_meet(const ThessaraRelationship* thessara);

/**
 * @brief Get total knowledge transfers
 *
 * @param thessara Thessara relationship
 * @return Number of knowledge transfers, or 0 if thessara is NULL
 */
size_t thessara_get_transfer_count(const ThessaraRelationship* thessara);

/**
 * @brief Get knowledge transfer by index
 *
 * @param thessara Thessara relationship
 * @param index Transfer index (0 to transfer_count-1)
 * @return Pointer to transfer, or NULL if index out of range
 */
const KnowledgeTransfer* thessara_get_transfer(const ThessaraRelationship* thessara, size_t index);

/**
 * @brief Check if trust is high
 *
 * High trust (>= 75%) unlocks special dialogue and assistance.
 *
 * @param thessara Thessara relationship
 * @return true if trust >= 75%, false otherwise
 */
bool thessara_has_high_trust(const ThessaraRelationship* thessara);

/**
 * @brief Get trust level description
 *
 * @param thessara Thessara relationship
 * @return String describing trust level
 */
const char* thessara_get_trust_description(const ThessaraRelationship* thessara);

/**
 * @brief Get knowledge type name
 *
 * @param type Knowledge type
 * @return String name (e.g., "Research", "Spell")
 */
const char* thessara_knowledge_type_name(KnowledgeType type);

#endif /* NECROMANCERS_THESSARA_H */

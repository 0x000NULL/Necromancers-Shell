/**
 * @file village_event.h
 * @brief Village raid event system
 *
 * Manages major village events like The Ashbrook Event (Day 47).
 * Handles population simulation, mass soul harvesting, child NPCs,
 * corruption consequences, and divine attention triggers.
 */

#ifndef NECROMANCERS_VILLAGE_EVENT_H
#define NECROMANCERS_VILLAGE_EVENT_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

/* Maximum values for village data */
#define MAX_VILLAGE_NAME 64
#define MAX_VILLAGE_DESCRIPTION 512
#define MAX_POPULATION_BREAKDOWN 10

/**
 * @brief Population category types
 *
 * Different population groups have different soul qualities and corruption costs.
 */
typedef enum {
    POP_CHILDREN,       /**< Children (age < 16) - highest corruption cost */
    POP_WARRIORS,       /**< Trained fighters - high soul quality */
    POP_ELDERS,         /**< Elderly (age > 60) - wisdom, lower quality */
    POP_ADULTS,         /**< Working-age adults - standard quality */
    POP_CLERGY,         /**< Priests/clerics - very high corruption cost */
    POP_CRIMINALS       /**< Prisoners/outlaws - lower corruption cost */
} PopulationCategory;

/**
 * @brief Population breakdown entry
 *
 * Tracks number of individuals in a specific category.
 */
typedef struct {
    PopulationCategory category;
    uint32_t count;
    float average_soul_quality;  /**< 0-100 quality */
    uint8_t corruption_per_soul; /**< Corruption cost per harvested soul */
} PopulationBreakdown;

/**
 * @brief Village event outcome
 *
 * Records the result of player's choice regarding the village.
 */
typedef enum {
    OUTCOME_NONE,               /**< Event not yet resolved */
    OUTCOME_SPARED,             /**< Village spared, no harvest */
    OUTCOME_PARTIAL_HARVEST,    /**< Selective harvest (warriors/criminals only) */
    OUTCOME_FULL_HARVEST,       /**< Complete harvest including children */
    OUTCOME_DEFENDED            /**< Village defended from other threat */
} VillageOutcome;

/**
 * @brief Village raid event structure
 *
 * Represents a village that can be raided for souls.
 */
typedef struct {
    /* Identity */
    char village_id[MAX_VILLAGE_NAME];
    char village_name[MAX_VILLAGE_NAME];
    char description[MAX_VILLAGE_DESCRIPTION];

    /* Population */
    uint32_t total_population;
    PopulationBreakdown breakdown[MAX_POPULATION_BREAKDOWN];
    size_t breakdown_count;

    /* Soul data */
    float average_soul_quality;     /**< Overall average (0-100) */
    uint32_t total_soul_energy;     /**< Total energy if all harvested */

    /* Corruption cost */
    uint32_t total_corruption_cost; /**< Total corruption for full harvest */
    uint32_t children_corruption;   /**< Corruption from children alone */

    /* Event state */
    bool event_triggered;           /**< Has event been presented to player? */
    uint32_t trigger_day;           /**< Day when event triggered */
    VillageOutcome outcome;         /**< Player's choice */
    uint32_t resolution_day;        /**< Day when resolved */

    /* Consequences */
    bool triggers_divine_attention; /**< Does this trigger divine scrutiny? */
    bool locks_revenant_path;       /**< Does this lock Revenant path? */
    bool locks_wraith_path;         /**< Does this lock Wraith path? */

    /* Moral consequence text */
    char moral_consequence[512];    /**< Narrative description of choice */
} VillageEvent;

/**
 * @brief Create a village event
 *
 * @param village_id Unique identifier
 * @param village_name Display name
 * @param total_population Total population count
 * @return Newly allocated VillageEvent or NULL on failure
 */
VillageEvent* village_event_create(const char* village_id, const char* village_name,
                                   uint32_t total_population);

/**
 * @brief Destroy village event and free memory
 *
 * @param event Event to destroy (can be NULL)
 */
void village_event_destroy(VillageEvent* event);

/**
 * @brief Add population breakdown category
 *
 * @param event Village event
 * @param category Population category
 * @param count Number of individuals
 * @param soul_quality Average soul quality (0-100)
 * @param corruption_per_soul Corruption cost per soul
 * @return true on success, false if breakdown list full or event is NULL
 */
bool village_event_add_population(VillageEvent* event, PopulationCategory category,
                                  uint32_t count, float soul_quality,
                                  uint8_t corruption_per_soul);

/**
 * @brief Calculate total statistics
 *
 * Computes total soul energy, corruption cost, and average quality.
 * Should be called after all population categories are added.
 *
 * @param event Village event
 * @return true on success, false if event is NULL
 */
bool village_event_calculate_totals(VillageEvent* event);

/**
 * @brief Trigger the event
 *
 * Presents the event to the player. Sets event_triggered flag.
 *
 * @param event Village event
 * @param day Current game day
 * @return true on success, false if already triggered or event is NULL
 */
bool village_event_trigger(VillageEvent* event, uint32_t day);

/**
 * @brief Resolve event - spare the village
 *
 * Player chooses not to harvest souls. No corruption gain.
 *
 * @param event Village event
 * @param day Current game day
 * @return true on success, false if event is NULL
 */
bool village_event_spare(VillageEvent* event, uint32_t day);

/**
 * @brief Resolve event - partial harvest
 *
 * Player harvests only warriors/criminals. Moderate corruption.
 *
 * @param event Village event
 * @param day Current game day
 * @param souls_harvested Output: number of souls harvested
 * @param energy_gained Output: soul energy gained
 * @param corruption_gained Output: corruption gained
 * @return true on success, false if event is NULL
 */
bool village_event_partial_harvest(VillageEvent* event, uint32_t day,
                                   uint32_t* souls_harvested, uint32_t* energy_gained,
                                   uint32_t* corruption_gained);

/**
 * @brief Resolve event - full harvest
 *
 * Player harvests entire village including children. Massive corruption spike.
 *
 * @param event Village event
 * @param day Current game day
 * @param souls_harvested Output: number of souls harvested
 * @param energy_gained Output: soul energy gained
 * @param corruption_gained Output: corruption gained
 * @return true on success, false if event is NULL
 */
bool village_event_full_harvest(VillageEvent* event, uint32_t day,
                                uint32_t* souls_harvested, uint32_t* energy_gained,
                                uint32_t* corruption_gained);

/**
 * @brief Check if event locks specific path
 *
 * Some outcomes prevent certain ending paths.
 *
 * @param event Village event
 * @param revenant_locked Output: is Revenant path locked?
 * @param wraith_locked Output: is Wraith path locked?
 * @return true on success, false if event is NULL
 */
bool village_event_check_path_locks(const VillageEvent* event, bool* revenant_locked,
                                    bool* wraith_locked);

/**
 * @brief Get population count by category
 *
 * @param event Village event
 * @param category Population category
 * @return Count of individuals in category, or 0 if not found
 */
uint32_t village_event_get_population_count(const VillageEvent* event,
                                            PopulationCategory category);

/**
 * @brief Check if event triggers divine attention
 *
 * Full harvest triggers divine scrutiny.
 *
 * @param event Village event
 * @return true if divine attention triggered, false otherwise
 */
bool village_event_triggers_divine_attention(const VillageEvent* event);

/**
 * @brief Get outcome name
 *
 * @param outcome Village outcome
 * @return String name (e.g., "Spared", "Full Harvest")
 */
const char* village_event_outcome_name(VillageOutcome outcome);

/**
 * @brief Get population category name
 *
 * @param category Population category
 * @return String name (e.g., "Children", "Warriors")
 */
const char* village_event_category_name(PopulationCategory category);

/**
 * @brief Get moral consequence description
 *
 * Returns narrative text describing the moral weight of player's choice.
 *
 * @param event Village event
 * @return String describing moral consequence
 */
const char* village_event_get_moral_consequence(const VillageEvent* event);

/**
 * @brief Calculate corruption from children only
 *
 * Used to warn player about the specific cost of harvesting children.
 *
 * @param event Village event
 * @return Corruption cost from children category only
 */
uint32_t village_event_calculate_children_corruption(const VillageEvent* event);

#endif /* NECROMANCERS_VILLAGE_EVENT_H */

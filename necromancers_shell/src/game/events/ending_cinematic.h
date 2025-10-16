/**
 * @file ending_cinematic.h
 * @brief Ending cinematic system - Displays narrative endings for all 6 paths
 *
 * Provides cinematic narrative sequences for each of the six ending paths:
 * - Revenant Route: Return to mortal life (corruption <30%)
 * - Lich Lord Route: Embrace eternal undeath (corruption 100%)
 * - Reaper Route: Serve the Death Network (corruption 40-69%)
 * - Archon Route: Reform the system from within (corruption 30-60%)
 * - Wraith Route: Distributed consciousness (corruption <40%)
 * - Morningstar Route: Become a god (corruption EXACTLY 50%)
 *
 * Each ending includes:
 * - Opening narrative text
 * - Player choices and consequences
 * - Epilogue describing long-term outcomes
 * - Achievement tracking
 */

#ifndef NECROMANCER_ENDING_CINEMATIC_H
#define NECROMANCER_ENDING_CINEMATIC_H

#include <stdint.h>
#include <stdbool.h>

/* Forward declarations */
typedef struct GameState GameState;

/**
 * @brief Ending path types
 */
typedef enum {
    ENDING_REVENANT,        /**< Redemption through resurrection */
    ENDING_LICH_LORD,       /**< Immortal tyrant of undeath */
    ENDING_REAPER,          /**< Death's administrator */
    ENDING_ARCHON,          /**< System reformer */
    ENDING_WRAITH,          /**< Distributed consciousness */
    ENDING_MORNINGSTAR      /**< Ascension to godhood */
} EndingPath;

/**
 * @brief Cinematic display state
 */
typedef enum {
    CINEMATIC_NOT_STARTED,  /**< Ending not triggered */
    CINEMATIC_PLAYING,      /**< Cinematic in progress */
    CINEMATIC_COMPLETE      /**< Ending fully displayed */
} CinematicState;

/**
 * @brief Ending cinematic data
 */
typedef struct {
    EndingPath path;           /**< Which ending was achieved */
    CinematicState state;      /**< Current cinematic state */
    uint32_t completion_day;   /**< Day ending was triggered */
    bool skip_requested;       /**< Player wants to skip */

    /* Player stats at ending */
    uint8_t final_corruption;
    float final_consciousness;
    uint32_t total_souls_harvested;
    uint32_t minions_raised;
    uint32_t civilians_killed;

    /* Path-specific data */
    bool archon_amnesty_granted;   /**< Archon: Did Council approve? */
    uint8_t trial_scores[7];       /**< Archon: Trial performance */
    bool maya_saved;               /**< Archon: Trial 6 choice */
    bool thessara_connection_severed; /**< Archon: Trial 6 sacrifice */
} EndingCinematic;

/**
 * @brief Create ending cinematic
 *
 * @return Newly allocated cinematic structure or NULL on failure
 */
EndingCinematic* ending_cinematic_create(void);

/**
 * @brief Destroy ending cinematic
 *
 * @param cinematic Cinematic to destroy (can be NULL)
 */
void ending_cinematic_destroy(EndingCinematic* cinematic);

/**
 * @brief Trigger ending cinematic for specific path
 *
 * Captures current game state and displays the appropriate ending narrative.
 *
 * @param cinematic Cinematic structure
 * @param state Game state
 * @param path Which ending to display
 * @return true on success
 */
bool ending_cinematic_trigger(EndingCinematic* cinematic,
                              const GameState* state,
                              EndingPath path);

/**
 * @brief Display Revenant ending (Redemption)
 *
 * Player resurrects and returns to mortal life. Requires corruption <30%.
 * Narrative focuses on reclaiming humanity and leaving necromancy behind.
 *
 * @param cinematic Cinematic structure
 * @param state Game state
 */
void ending_cinematic_revenant(const EndingCinematic* cinematic, const GameState* state);

/**
 * @brief Display Lich Lord ending (Apotheosis)
 *
 * Player embraces undeath and becomes immortal tyrant. Corruption 100%.
 * Narrative focuses on absolute power and eternal reign.
 *
 * @param cinematic Cinematic structure
 * @param state Game state
 */
void ending_cinematic_lich_lord(const EndingCinematic* cinematic, const GameState* state);

/**
 * @brief Display Reaper ending (Service)
 *
 * Player becomes Death Network administrator. Corruption 40-69%.
 * Narrative focuses on serving the system and maintaining balance.
 *
 * @param cinematic Cinematic structure
 * @param state Game state
 */
void ending_cinematic_reaper(const EndingCinematic* cinematic, const GameState* state);

/**
 * @brief Display Archon ending (Revolution)
 *
 * Player reforms Death Network protocols. Corruption 30-60%, all trials passed.
 * Narrative focuses on systemic change and necromancer rehabilitation.
 *
 * @param cinematic Cinematic structure
 * @param state Game state
 */
void ending_cinematic_archon(const EndingCinematic* cinematic, const GameState* state);

/**
 * @brief Display Wraith ending (Freedom)
 *
 * Player becomes distributed consciousness. Corruption <40%.
 * Narrative focuses on escaping the system and exploring infinity.
 *
 * @param cinematic Cinematic structure
 * @param state Game state
 */
void ending_cinematic_wraith(const EndingCinematic* cinematic, const GameState* state);

/**
 * @brief Display Morningstar ending (Transcendence)
 *
 * Player becomes eighth god. Corruption EXACTLY 50%.
 * Narrative focuses on impossible balance and divine ascension.
 *
 * @param cinematic Cinematic structure
 * @param state Game state
 */
void ending_cinematic_morningstar(const EndingCinematic* cinematic, const GameState* state);

/**
 * @brief Get ending path name
 *
 * @param path Ending path
 * @return String name (e.g., "Revenant Route")
 */
const char* ending_cinematic_path_name(EndingPath path);

/**
 * @brief Get ending path description
 *
 * @param path Ending path
 * @return Short description string
 */
const char* ending_cinematic_path_description(EndingPath path);

/**
 * @brief Check if cinematic is playing
 *
 * @param cinematic Cinematic structure
 * @return true if currently playing
 */
bool ending_cinematic_is_playing(const EndingCinematic* cinematic);

/**
 * @brief Check if cinematic is complete
 *
 * @param cinematic Cinematic structure
 * @return true if fully displayed
 */
bool ending_cinematic_is_complete(const EndingCinematic* cinematic);

/**
 * @brief Request skip cinematic
 *
 * Player can skip to summary if desired.
 *
 * @param cinematic Cinematic structure
 */
void ending_cinematic_request_skip(EndingCinematic* cinematic);

#endif /* NECROMANCER_ENDING_CINEMATIC_H */

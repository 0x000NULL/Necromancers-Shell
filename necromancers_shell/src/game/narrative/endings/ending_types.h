/**
 * @file ending_types.h
 * @brief Ending type enumeration
 *
 * Separated into its own header to avoid circular dependencies
 * between game_state.h and ending_system.h.
 */

#ifndef ENDING_TYPES_H
#define ENDING_TYPES_H

/**
 * @brief Ending types corresponding to the six paths
 */
typedef enum {
    ENDING_NONE = 0,            /**< No ending determined yet */
    ENDING_REVENANT,            /**< Revenant Route - Redemption through resurrection */
    ENDING_LICH_LORD,           /**< Lich Lord Route - Apotheosis as immortal tyrant */
    ENDING_REAPER,              /**< Reaper Route - Service as death administrator */
    ENDING_ARCHON,              /**< Archon Route - Revolution via protocol rewrite */
    ENDING_WRAITH,              /**< Wraith Route - Freedom through distributed consciousness */
    ENDING_MORNINGSTAR          /**< Morningstar Route - Transcendence as 8th god */
} EndingType;

#endif /* ENDING_TYPES_H */

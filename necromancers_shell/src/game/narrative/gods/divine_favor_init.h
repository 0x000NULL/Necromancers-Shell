/**
 * @file divine_favor_init.h
 * @brief Initialization of the Seven Divine Architects
 */

#ifndef NECROMANCER_DIVINE_FAVOR_INIT_H
#define NECROMANCER_DIVINE_FAVOR_INIT_H

#include "divine_council.h"

/**
 * @brief Initialize the Seven Divine Architects
 *
 * Creates and configures all 7 gods with their specific attributes:
 * - Anara (Life): Opposed to necromancy, starts at -20 favor
 * - Keldrin (Order): Mildly opposed, can be convinced, starts at -10 favor
 * - Theros (Time): Neutral, sees all paths, starts at 0 favor
 * - Myrith (Souls): Sympathetic if careful, starts at +5 favor
 * - Vorathos (Entropy): Favors death magic, starts at +15 favor
 * - Seraph (Boundaries): Dislikes boundary crossing, starts at -15 favor
 * - Nexus (Networks): Appreciates technical work, starts at +10 favor
 *
 * Average starting favor: -2.14 (slightly negative, balanced)
 *
 * @return Newly allocated DivineCouncil with all 7 gods, or NULL on failure
 */
DivineCouncil* divine_favor_initialize_council(void);

#endif /* NECROMANCER_DIVINE_FAVOR_INIT_H */

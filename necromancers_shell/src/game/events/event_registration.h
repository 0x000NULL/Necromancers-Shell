/**
 * @file event_registration.h
 * @brief Centralized event registration system
 *
 * Provides a single function to register all story events with the event scheduler.
 * This ensures proper initialization order and makes it easy to see all registered events.
 */

#ifndef NECROMANCER_EVENT_REGISTRATION_H
#define NECROMANCER_EVENT_REGISTRATION_H

#include <stdint.h>
#include <stdbool.h>

/* Forward declarations */
typedef struct EventScheduler EventScheduler;
typedef struct GameState GameState;

/**
 * @brief Register all story events with the event scheduler
 *
 * Registers all major story events:
 * - Day 47: Ashbrook massacre
 * - Day 50: Thessara contact (placeholder for future)
 * - Day 89: Soldier encounter (placeholder for future)
 * - Day 92: Regional Council meeting (placeholder for future)
 * - Day 155: Divine summons (placeholder for future)
 * - Day 162: Divine judgment (placeholder for future)
 *
 * @param scheduler Event scheduler
 * @param state Game state
 * @return Number of events successfully registered
 */
uint32_t register_all_story_events(EventScheduler* scheduler, GameState* state);

#endif /* NECROMANCER_EVENT_REGISTRATION_H */

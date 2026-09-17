#ifndef GAME_TICK_H
#define GAME_TICK_H

/**
 * Initialize and start the game tick.
 */
void game_tick_init(void);

/**
 * Record a pending game tick.
 */
void game_tick_pending(void);

/**
 * Process pending game ticks.
 */
void game_tick_process(void);

#endif

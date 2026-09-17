#ifndef INPUT_WIRED_H
#define INPUT_WIRED_H

#include "input.h"

/**
 * Initialize wired controller input.
 */
void input_wired_init(void);

/**
 * Sample and debounce the wired controllers.
 */
void input_wired_sample(void);

/**
 * Return the stable wired input state for a player.
 */
InputState input_wired_get(InputPlayer player);

#endif

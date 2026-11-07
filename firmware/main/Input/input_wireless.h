#ifndef INPUT_WIRELESS_H
#define INPUT_WIRELESS_H

#include "input.h"

/**
 * Initialize wireless controller input.
 */
void input_wireless_init(void);

/**
 * Restart UART reception if it has become inactive.
 */
void input_wireless_process(void);

/**
 * Return the stable wireless input state for a player.
 */
InputState input_wireless_get(InputPlayer player);

#endif

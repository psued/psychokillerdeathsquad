#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>
#include <stdint.h>

typedef uint8_t InputButtons;

enum {
	INPUT_BUTTON_UP    = 1U << 0,
	INPUT_BUTTON_DOWN  = 1U << 1,
	INPUT_BUTTON_LEFT  = 1U << 2,
	INPUT_BUTTON_RIGHT = 1U << 3,
	INPUT_BUTTON_A     = 1U << 4,
	INPUT_BUTTON_B     = 1U << 5
};

typedef enum {
	INPUT_PLAYER_1 = 0,
	INPUT_PLAYER_2,
	INPUT_PLAYER_COUNT
} InputPlayer;

typedef struct {
	InputButtons held;
	bool connected;
} InputState;

typedef struct {
	InputButtons held;
	InputButtons pressed;
	InputButtons released;
} ControllerState;

/**
 * Initialize the input subsystem.
 */
void input_init(void);

/**
 * Process controller input.
 */
void input_process(void);

/**
 * Freeze accumulated input for the next game update.
 */
void input_freeze(void);

/**
 * Return the current input state for a player.
 */
ControllerState input_get(InputPlayer player);

#endif

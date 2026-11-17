#ifndef PLAYER_GAME_INPUT_H
#define PLAYER_GAME_INPUT_H

#include <stdbool.h>
#include "input.h"

typedef enum {
    BUTTON_1,
    BUTTON_2,
    BUTTON_DPAD_UP,
    BUTTON_DPAD_DOWN,
    BUTTON_DPAD_LEFT,
    BUTTON_DPAD_RIGHT,
    NUM_BUTTONS,
} ButtonTypes;

typedef struct {
    /* Raw input state, true as long as they're held down */
    bool dpad_up;
    bool dpad_down;
    bool dpad_left;
    bool dpad_right;

    bool button1;
    bool button2;

    /* Derive from tick history to see if the buttons were held down or just clicked */
    bool dpad_up_pressed;
    bool dpad_down_pressed;
    bool dpad_left_pressed;
    bool dpad_right_pressed;

    bool button1_pressed;
    bool button2_pressed;

} PlayerGameInput;

/**
 * Derive the state for the '_pressed' variables in 'PlayerGameInput' based on if they were pressed in the previous tick
 */
void derive_pressed_state(PlayerGameInput *current, const PlayerGameInput *previous);

PlayerGameInput read_controller_input(ControllerState controller_state);

#endif

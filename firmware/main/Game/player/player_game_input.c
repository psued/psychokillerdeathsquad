#include "player_game_input.h"
#include "input.h"

void derive_pressed_state(PlayerGameInput *current, const PlayerGameInput *previous) {
    current->dpad_up_pressed = current->dpad_up && !previous->dpad_up;
    current->dpad_down_pressed = current->dpad_down && !previous->dpad_down;
    current->dpad_left_pressed = current->dpad_left && !previous->dpad_left;
    current->dpad_right_pressed = current->dpad_right && !previous->dpad_right;

    current->button1_pressed = current->button1 && !previous->button1;
    current->button2_pressed = current->button2 && !previous->button2;
}

PlayerGameInput read_controller_input(ControllerState controller_state){
    PlayerGameInput input = {0};

    input.dpad_up = (controller_state.held & INPUT_BUTTON_UP) != 0;
    input.dpad_down = (controller_state.held & INPUT_BUTTON_DOWN) != 0;
    input.dpad_left = (controller_state.held & INPUT_BUTTON_LEFT) != 0;
    input.dpad_right = (controller_state.held & INPUT_BUTTON_RIGHT) != 0;
    input.button2 = (controller_state.held & INPUT_BUTTON_A) != 0;
    input.button1 = (controller_state.held & INPUT_BUTTON_B) != 0;

    input.dpad_up_pressed = (controller_state.pressed & INPUT_BUTTON_UP) != 0;
    input.dpad_down_pressed = (controller_state.pressed & INPUT_BUTTON_DOWN) != 0;
    input.dpad_left_pressed = (controller_state.pressed & INPUT_BUTTON_LEFT) != 0;
    input.dpad_right_pressed = (controller_state.pressed & INPUT_BUTTON_RIGHT) != 0;
    input.button2_pressed = (controller_state.pressed & INPUT_BUTTON_A) != 0;
    input.button1_pressed = (controller_state.pressed & INPUT_BUTTON_B) != 0;

    return input;
}

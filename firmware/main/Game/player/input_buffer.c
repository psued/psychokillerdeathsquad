#include <stdbool.h>
#include "input_buffer.h"
#include "player_game_input.h"

/* Assumes only one input gets registered into a sequence at a time */
static InputDirection get_direction_from_input(const PlayerGameInput *input){
    /*if (input->dpad_up_pressed) return DIR_UP; */
    if (input->dpad_down_pressed) return DIR_DOWN;
    if (input->dpad_left_pressed) return DIR_LEFT;
    if (input->dpad_right_pressed) return DIR_RIGHT;

    return DIR_NONE;
}

/* Mirror input sequence when not facing right */
static InputDirection mirror_direction(InputDirection dir, bool facing_right){
    if (facing_right){
        return dir;
    }

    /* Only switch left and right */
    switch (dir){
        case DIR_LEFT: 
            return DIR_RIGHT;
        case DIR_RIGHT: 
            return DIR_LEFT;
        default: 
            return dir;
    }
}

void update_input_buffer(InputBuffer *buffer, const PlayerGameInput *input) {
    InputDirection pressed = get_direction_from_input(input);

    if (pressed != DIR_NONE){
        /* Add new input to buffer */
        BufferedInput *entry = &buffer->entries[buffer->write_index];
        entry->direction = pressed;
        entry->ticks_since_previous_input = buffer->ticks_since_last_write;
        entry->consumed = false;

        buffer->write_index = (buffer->write_index + 1) % INPUT_BUFFER_SIZE;
        buffer->ticks_since_last_write = 0;
    } else {
        /* Increase counter when no button is pressed */
        if (buffer->ticks_since_last_write < 255){
            buffer->ticks_since_last_write++;
        }
    }
}

/* Checks if a given sequence is present in the input buffer, with a maximum tick duration between each sequence input */
bool check_input_sequence(
    InputBuffer *buffer, 
    const InputDirection *sequence, 
    uint8_t sequence_length, 
    uint8_t max_ticks_ago,
    bool facing_right
) {
    uint8_t matched_indices[INPUT_BUFFER_SIZE];
    int seq_idx = sequence_length - 1;
    uint16_t elapsed = 0;

    for (int i = 0; i < sequence_length; i++) {
        uint8_t idx = (buffer->write_index + INPUT_BUFFER_SIZE - 1 - i) % INPUT_BUFFER_SIZE;
        BufferedInput *entry = &buffer->entries[idx];

        /* No input history for this index */
        if (entry->direction == DIR_NONE) {
            return false;
        }

        elapsed += (i == 0) ? buffer->ticks_since_last_write : entry->ticks_since_previous_input;

        /* Too slow between inputs */
        if (elapsed > max_ticks_ago) {
            return false;
        }

        /* Input has already been used in a sequence before */
        if (entry->consumed){
            return false;
        }

        /* Possibly mirror input direction */
        InputDirection mirrored_diretion = mirror_direction(sequence[seq_idx], facing_right);

        /* Wrong input for the sequence being checked */
        if (entry->direction != mirrored_diretion) {
            return false;
        }

        matched_indices[seq_idx] = idx;
        seq_idx--;
        elapsed = 0;
    }

    /* If all sequence inputs have been matched, mark them as consumed (a special has been recognized) */
    if (seq_idx < 0){
        for (int j = 0; j < sequence_length; j++){
            buffer->entries[matched_indices[j]].consumed = true;
        }
        return true;
    }

    return false;
}


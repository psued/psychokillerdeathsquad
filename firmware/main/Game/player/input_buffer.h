#ifndef INPUT_BUFFER_H
#define INPUT_BUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include "player_game_input.h"

/* the longest sequence of expected inputs to form a special */
#define INPUT_BUFFER_SIZE 8

typedef enum {
    DIR_NONE,
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT,
} InputDirection;

typedef struct {
    InputDirection direction;
    uint8_t ticks_since_previous_input;
    bool consumed; /* if this input has been used in a special move */
} BufferedInput;

typedef struct {
    BufferedInput entries[INPUT_BUFFER_SIZE];
    uint8_t write_index;

    uint8_t ticks_since_last_write;
} InputBuffer;

void update_input_buffer(InputBuffer *buffer, const PlayerGameInput *input);

bool check_input_sequence(
    InputBuffer *buffer, 
    const InputDirection *sequence, 
    uint8_t sequence_length, 
    uint8_t max_ticks_ago,
    bool facing_right
);

#endif
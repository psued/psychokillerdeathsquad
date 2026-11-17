#ifndef GAME_H
#define GAME_H

#include "game_state.h"
#include "player.h"
#include "player_game_input.h"
#include "input_buffer.h"
#include "start_screen.h"
#include "char_select.h"
#include "stage_select.h"

typedef struct {
    GameState state;

    Player p1, p2;
    PlayerGameInput p1_input_prev, p2_input_prev;
    InputBuffer p1_input_buffer, p2_input_buffer;

    StartScreenState start_state;
    CharacterSelectionState p1_char_selection, p2_char_selection;
    StageSelectionState stage_selection;
    MatchState match_state;
} Game;

/**
 * Initialize the game.
 */
void game_init(void);

/**
 * Advance the game by one simulation tick.
 */
void game_tick(void);

#endif

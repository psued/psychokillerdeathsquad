#ifndef BATTLE_H
#define BATTLE_H

#include "game_state.h"
#include "stage_def.h"

#include "player.h"
#include "player_game_input.h"
#include "input_buffer.h"

void enter_battle_state(MatchState *match_state);

void update_battle_state(
    GameState *game_state, 
    MatchState *match_state,
    const StageDef *stage_def,
    Player *p1, 
    Player *p2,
    const PlayerGameInput *p1_input, 
    const PlayerGameInput *p2_input,
    InputBuffer *p1_input_buffer,
    InputBuffer *p2_input_buffer
);

#endif
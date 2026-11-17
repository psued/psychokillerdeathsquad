#ifndef BATTLE_INPUT_H
#define BATTLE_INPUT_H

#include "player.h"
#include "input_buffer.h"
#include "player_game_input.h"
#include "character.h"

void update_player_state_from_input(
    Player *player, 
    const PlayerGameInput *player_input, 
    InputBuffer *player_buffer,
    const CharacterDef *def,
    const Player *opponent,
    const CharacterDef *opponent_def
);

#endif
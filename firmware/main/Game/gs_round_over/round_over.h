#ifndef ROUND_OVER_H
#define ROUND_OVER_H

#include "game_state.h"
#include "player.h"
#include "player_game_input.h"
#include "stage_def.h"
#include "render_builder.h"

void enter_round_over(void);

void update_round_over(
    GameState *game_state, 
    MatchState *match_state, 
    const StageDef *stage_def,
    Player *p1,
    Player *p2,
    const PlayerGameInput *p1_input, 
    const PlayerGameInput *p2_input
);

void build_round_over_render(
    RenderBuilder *builder, 
    const StageDef *stage_def,
    const MatchState *match_state,
    const Player *p1,
    const Player *p2
);

#endif
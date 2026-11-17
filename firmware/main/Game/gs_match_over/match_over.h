#ifndef MATCH_OVER_H
#define MATCH_OVER_H

#include "game_state.h"
#include "render_builder.h"
#include "player.h"
#include "player_game_input.h"

void enter_match_over(void);

void update_match_over(
    GameState *game_state,
    const PlayerGameInput *p1_input, 
    const PlayerGameInput *p2_input
);

void build_match_over_render(
    RenderBuilder *builder, 
    const MatchState *match_state,
    const Player *p1,
    const Player *p2
);

#endif

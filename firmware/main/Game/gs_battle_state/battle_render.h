#ifndef BATTLE_RENDER_H
#define BATTLE_RENDER_H

#include <stdint.h>

#include "render_builder.h"
#include "game_state.h"
#include "stage_def.h"
#include "player.h"

void render_add_player(RenderBuilder *builder,
    const Player *player,
    int camera_x,
    int camera_y,
    uint8_t z,
    uint8_t layer_index
);

void build_battle_render(
    RenderBuilder *builder, 
    const StageDef *stage_def,
    const MatchState *match_state,
    const Player *p1,
    const Player *p2
);

#endif

#ifndef START_SCREEN_H
#define START_SCREEN_H

#include "player_game_input.h"
#include "game_state.h"
#include "render_builder.h"
#include "screen_animation.h"

typedef struct {
    AnimationHandle title;
    AnimationHandle flames;
    AnimationHandle press_start;

    bool press_start_started;
    uint32_t ticks_before_click;
} StartScreenState;

void enter_start_screen(StartScreenState *state);

void update_start_screen(GameState *game_state, StartScreenState *state, const PlayerGameInput *p1_input, const PlayerGameInput *p2_input);

void build_start_screen_render(RenderBuilder *builder);

#endif
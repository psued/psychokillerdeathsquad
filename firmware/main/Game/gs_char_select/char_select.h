#ifndef CHAR_SELECT_H
#define CHAR_SELECT_H 

#include <stdint.h>
#include <stdbool.h>

#include "player_game_input.h"
#include "game_state.h"
#include "player.h"
#include "render_builder.h"
#include "screen_animation.h"

typedef struct {
    uint8_t hovered_char_id;
    bool confirmed;
    AnimationHandle ring;
} CharacterSelectionState;

void enter_char_select(CharacterSelectionState *p1_char_selection, CharacterSelectionState *p2_char_selection);

void update_char_select(
    GameState *game_state, 
    CharacterSelectionState *p1_char_selection, 
    CharacterSelectionState *p2_char_selection,
    const PlayerGameInput *p1_input, 
    const PlayerGameInput *p2_input
);

void build_char_select_render(
    RenderBuilder *builder, 
    const CharacterSelectionState *p1_char_selection,
    const CharacterSelectionState *p2_char_selection
);

#endif
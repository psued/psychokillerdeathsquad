#ifndef STAGE_H
#define STAGE_H

#include <stdbool.h>

#include "stage_def.h"
#include "game_state.h"
#include "player_game_input.h"
#include "player.h"
#include "char_select.h"
#include "render_builder.h"

#define STAGE_SELECT_NUM_FLAGS_1 3U
#define STAGE_SELECT_NUM_FLAGS_2 3U

typedef struct {
    StageID hovered_stage_id;
    bool confirmed;
} StageSelectionState;

void enter_stage_select(StageSelectionState *stage_selection);

void update_stage_select(
    GameState *game_state, 
    MatchState *match_state,
    StageSelectionState *stage_selection, 
    Player *p1,
    Player *p2,
    CharacterSelectionState *p1_char_selection,
    CharacterSelectionState *p2_char_selection,
    const PlayerGameInput *p1_input, 
    const PlayerGameInput *p2_input
);

void build_stage_select_render(RenderBuilder *builder, const StageSelectionState *stage_selection);

#endif

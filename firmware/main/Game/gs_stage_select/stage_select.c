#include <stdint.h>
#include "audio.h"

#include "stage_select.h"
#include "char_select.h"

#include "battle.h"
#include "battle_setup.h"

#include "game_state.h"
#include "game_audio.h"
#include "player_game_input.h"
#include "player.h"

#include "animations_data.h"
#include "screen_animation.h"
#include "texture.h"
#include "layer_def.h"

#define Z_STAGE_SELECT_BG 0U
#define Z_STAGE_SELECT_FLAGS 1U
#define Z_STAGE_SELECT_RING 10U

/* Where the ring should be positioned over the selected stage */
static const ScreenPos ring_positions[NUM_STAGES] = {
    [STAGE_BRAZIL] = {64, 96},
    [STAGE_USA] = {49, 44},
    [STAGE_BRITAIN] = {118, 31},
    
    [STAGE_EGYPT] = {156, 72},
    [STAGE_CHINA] = {220, 44},
    [STAGE_JAPAN] = {240, 90},
    
    [STAGE_DUNGEON] = {175, 155},
    [STAGE_VOLCANO] = {81, 155},
};

void enter_stage_select(StageSelectionState *stage_selection) {
    stage_selection->confirmed = false;
    stage_selection->hovered_stage_id = STAGE_CHINA;

    for (uint8_t i = 0; i < STAGE_SELECT_NUM_FLAGS_1; i++){
        ScreenPos flag_pos = ring_positions[i];
        animation_play(&flag_animations_1[i], flag_pos.x, flag_pos.y, Z_STAGE_SELECT_FLAGS + i, LAYER_STAGE_SELECT_FLAGS_1 + i);
    }

    for (uint8_t i = 0; i < STAGE_SELECT_NUM_FLAGS_2; i++){
        uint8_t offset = 3;
        ScreenPos flag_pos = ring_positions[i + offset];
        animation_play(&flag_animations_2[i], flag_pos.x, flag_pos.y, Z_STAGE_SELECT_FLAGS + i + offset, LAYER_STAGE_SELECT_FLAGS_4 + i);
    }

    audio_play(MUSIC_SELECT);
    audio_load(SFX_SELECT);
    audio_load(SFX_SWITCH);
}

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
) {
    /* Back to character selection  */
    if ((p1_input->button2_pressed || p2_input->button2_pressed) && !stage_selection->confirmed){
        p1_char_selection->confirmed = false;
        p2_char_selection->confirmed = false;
        *game_state = GS_CHARACTER_SELECTION;
        return;
    }

    /* Adjust volume */
	if (p1_input->dpad_up_pressed || p2_input->dpad_up_pressed) {
		increase_game_volume();
	} else if (p1_input->dpad_down_pressed || p2_input->dpad_down_pressed){
		decrease_game_volume();
	}

    if (!stage_selection->confirmed && p1_input->dpad_right_pressed){
        stage_selection->hovered_stage_id = (stage_selection->hovered_stage_id + 1) % NUM_STAGES;
        audio_play(SFX_SWITCH);
    } else if (!stage_selection->confirmed && p1_input->dpad_left_pressed){
        stage_selection->hovered_stage_id = (stage_selection->hovered_stage_id + NUM_STAGES - 1) % NUM_STAGES;
        audio_play(SFX_SWITCH);
    } else if (!stage_selection->confirmed && p2_input->dpad_right_pressed){
        stage_selection->hovered_stage_id = (stage_selection->hovered_stage_id + 1) % NUM_STAGES;
        audio_play(SFX_SWITCH);
    } else if (!stage_selection->confirmed && p2_input->dpad_left_pressed){
        stage_selection->hovered_stage_id = (stage_selection->hovered_stage_id + NUM_STAGES - 1) % NUM_STAGES;
        audio_play(SFX_SWITCH);
    }

    /* Transition to battle */
    if (p1_input->button1_pressed && stage_selection->confirmed){
        p1->character_id = p1_char_selection->hovered_char_id;
        p2->character_id = p2_char_selection->hovered_char_id;
        match_state->stage_id = stage_selection->hovered_stage_id;
        match_state->p1_round_wins = 0;
        match_state->p2_round_wins = 0;

        reset_state_for_round(p1, p2, match_state, stage_table[stage_selection->hovered_stage_id].stage_width);

        audio_play(SFX_SELECT);
        *game_state = GS_BATTLE;
        return;
    }

    /* Confirm choice */
    if (p1_input->button1_pressed){
        stage_selection->confirmed = true;
        audio_play(SFX_SELECT);
    } 

    /* Cancel choice */
    if (stage_selection->confirmed && p1_input->button2_pressed){
        stage_selection->confirmed = false;
        audio_play(SFX_SWITCH);
    }
};

void build_stage_select_render(RenderBuilder *builder, const StageSelectionState *stage_selection){
    render_add(builder, LAYER_BACKGROUND, TEX_STAGE_SELECT_BG, 0, 0, GAME_WIDTH, GAME_HEIGHT, 0, 0, Z_STAGE_SELECT_BG, false);

    ScreenPos ring_stage_pos = ring_positions[stage_selection->hovered_stage_id];
    ImageRect ring_src = {0, 0, 64, 48};

    render_add(builder, LAYER_STAGE_SELECT_HOVER_ICON, TEX_STAGE_SELECT_UI, ring_src.x, ring_src.y, ring_src.w, ring_src.h, ring_stage_pos.x, ring_stage_pos.y, Z_STAGE_SELECT_RING, false);
    animation_render(builder);
}

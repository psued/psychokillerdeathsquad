#include "audio.h"

#include "char_select.h"
#include "game_state.h"
#include "game_audio.h"
#include "player.h"
#include "player_game_input.h"
#include "character.h"
#include "layer_def.h"

#include "render_builder.h"
#include "texture.h"
#include "screen_constants.h"

#include "screen_animation.h"
#include "animations_data.h"

#define Z_CHAR_SELECT_BG 0U
#define Z_CHAR_SELECT_PORTRAIT_P1 1U
#define Z_CHAR_SELECT_PORTRAIT_P2 2U

#define Z_CHAR_SELECT_SMALL_ICONS 3U

#define Z_CHAR_SELECT_CONFIRM 6U

#define Z_CHAR_SELECT_RING_P2 4U
#define Z_CHAR_SELECT_RING_P1 5U

#define Z_CHAR_SELECT_NAME_P1 7U
#define Z_CHAR_SELECT_NAME_P2 8U


/* Where in the ui source image each character portrait should be drawn from */
static const ImageRect char_ui_table[NUM_CHARACTERS] = {
    [CHAR_FREDRIK] = {0, 0, 144, 48},
    [CHAR_JENS] = {144, 0, 144, 48},
    [CHAR_EIRIK] = {0, 96, 144, 48},
    [CHAR_PERNILLE] = {0, 48, 144, 48},
    [CHAR_SONDRE] = {144, 48, 144, 48},
    [CHAR_ARI] = {144, 96, 144, 48},
};

/* Where the rings should be positioned over the selected characters */
static const ScreenPos ring_positions[NUM_CHARACTERS] = {
    [CHAR_FREDRIK] = {10, 138},
    [CHAR_JENS] = {60, 152},
    [CHAR_EIRIK] = {110, 164},
    [CHAR_PERNILLE] = {163, 167},
    [CHAR_SONDRE] = {212, 156},
    [CHAR_ARI] = {260, 141},
};

void enter_char_select(CharacterSelectionState *p1_char_selection, CharacterSelectionState *p2_char_selection) {
    p1_char_selection->confirmed = false;
    p2_char_selection->confirmed = false;

    ScreenPos p1_ring_pos = ring_positions[p1_char_selection->hovered_char_id];
    ScreenPos p2_ring_pos = ring_positions[p2_char_selection->hovered_char_id];

    p1_char_selection->ring = animation_play(&p1hover_animation, p1_ring_pos.x, p1_ring_pos.y, Z_CHAR_SELECT_RING_P1, LAYER_CHAR_SELECT_HOVER_P1);
    p2_char_selection->ring = animation_play(&p2hover_animation, p2_ring_pos.x, p2_ring_pos.y, Z_CHAR_SELECT_RING_P2, LAYER_CHAR_SELECT_HOVER_P2);

    audio_play(MUSIC_SELECT);
    audio_load(SFX_SELECT);
    audio_load(SFX_SWITCH);
}

void update_char_select(
    GameState *game_state, 
    CharacterSelectionState *p1_char_selection, 
    CharacterSelectionState *p2_char_selection,
    const PlayerGameInput *p1_input, 
    const PlayerGameInput *p2_input
){
    /* Back to start screen */
    if (p1_input->button2_pressed || p2_input->button2_pressed){
        *game_state = GS_START_SCREEN;
        return;
    }

    /* Adjust volume */
    if (p1_input->dpad_up_pressed || p2_input->dpad_up_pressed) {
		increase_game_volume();
	} else if (p1_input->dpad_down_pressed || p2_input->dpad_down_pressed){
		decrease_game_volume();
	}

    /* Move between characters and select */
    if (!p1_char_selection->confirmed){
        if(p1_input->button1_pressed){
            p1_char_selection->confirmed = true;
            audio_play(SFX_SELECT);
        } else if (p1_input->dpad_right_pressed){
            p1_char_selection->hovered_char_id = (p1_char_selection->hovered_char_id + 1) % NUM_CHARACTERS;
            audio_play(SFX_SWITCH);
        } else if (p1_input->dpad_left_pressed){
            p1_char_selection->hovered_char_id = (p1_char_selection->hovered_char_id + NUM_CHARACTERS - 1) % NUM_CHARACTERS;
            audio_play(SFX_SWITCH);
        }
    } else {
        /* Deselect choice */
        if (p1_input->button1_pressed){
            p1_char_selection->confirmed = false;
            audio_play(SFX_SWITCH);
        }
    }

    if (!p2_char_selection->confirmed){
        if(p2_input->button1_pressed){
            p2_char_selection->confirmed = true;
            audio_play(SFX_SELECT);
        } else if (p2_input->dpad_right_pressed){
            p2_char_selection->hovered_char_id = (p2_char_selection->hovered_char_id + 1) % NUM_CHARACTERS;
            audio_play(SFX_SWITCH);
        } else if (p2_input->dpad_left_pressed){
            p2_char_selection->hovered_char_id = (p2_char_selection->hovered_char_id + NUM_CHARACTERS - 1) % NUM_CHARACTERS;
            audio_play(SFX_SWITCH);
        }
    } else {
        /* Deselect choice */
        if (p2_input->button1_pressed){
            p2_char_selection->confirmed = false;
            audio_play(SFX_SWITCH);
        }
    }

    /* If both have selected, transition to battle */
    if (p1_char_selection->confirmed && p2_char_selection->confirmed){
        audio_play(SFX_SELECT);
        *game_state = GS_STAGE_SELECT;
        return;
    }

    /* Update position of hover animations */
    ScreenPos p1_ring_pos = ring_positions[p1_char_selection->hovered_char_id];
    ScreenPos p2_ring_pos = ring_positions[p2_char_selection->hovered_char_id];
    animation_set_position(p1_char_selection->ring, p1_ring_pos.x, p1_ring_pos.y);
    animation_set_position(p2_char_selection->ring, p2_ring_pos.x, p2_ring_pos.y);
};

void build_char_select_render(
    RenderBuilder *builder, 
    const CharacterSelectionState *p1_char_selection,
    const CharacterSelectionState *p2_char_selection
) {
    /* Draw background first */
    render_add(builder, LAYER_BACKGROUND, TEX_CHAR_SELECT_BG, 0, 0, GAME_WIDTH, GAME_HEIGHT, 0, 0, Z_CHAR_SELECT_BG, false);

    /* Draw portraits of currently chosen characters */
    render_add(builder, LAYER_CHAR_SELECT_LARGE_ICONS_P1, TEX_CHAR_SELECT_LARGE_ICONS, p1_char_selection->hovered_char_id * 100, 0, 100, 176, 16, 44, Z_CHAR_SELECT_PORTRAIT_P1, false);
    render_add(builder, LAYER_CHAR_SELECT_LARGE_ICONS_P2, TEX_CHAR_SELECT_LARGE_ICONS, p2_char_selection->hovered_char_id * 100, 0, 100, 176, 204, 44, Z_CHAR_SELECT_PORTRAIT_P2, true);

    /* Draw character names */
    ImageRect p1_name = char_ui_table[p1_char_selection->hovered_char_id];
    ImageRect p2_name = char_ui_table[p2_char_selection->hovered_char_id];

    render_add(builder, LAYER_CHAR_SELECT_NAME_P1, TEX_CHAR_SELECT_UI, p1_name.x, p1_name.y, p1_name.w, p1_name.h, 0, 10, Z_CHAR_SELECT_NAME_P1, false);
    render_add(builder, LAYER_CHAR_SELECT_NAME_P2, TEX_CHAR_SELECT_UI, p2_name.x, p2_name.y, p2_name.w, p2_name.h, 176, 10, Z_CHAR_SELECT_NAME_P2, false);


    /* Add small character icons */
    render_add(builder, LAYER_CHAR_SELECT_SMALL_ICONS, TEX_CHAR_SELECT_SMALL_ICONS, 0, 0, 291, 71, 13, 143, Z_CHAR_SELECT_SMALL_ICONS, false);

    /* Draw animated hover icons */
    animation_render(builder);
}

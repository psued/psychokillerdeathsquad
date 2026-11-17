#include <stdint.h>
#include "debug.h"
#include "audio.h"
#include "game_random.h"
#include "game_audio.h"

#include "start_screen.h"
#include "game_state.h"
#include "render_builder.h"
#include "player_game_input.h"

#include "animations_data.h"
#include "screen_animation.h"
#include "layer_def.h"

#define Z_START_SCREEN_TITLE 1U
#define Z_START_SCREEN_TEXT_PROMPT 2U
#define Z_START_SCREEN_FLAMES 3U


void enter_start_screen(StartScreenState *state) {
    state->title = animation_play(&title_animation, 43, -64, Z_START_SCREEN_TITLE, LAYER_START_SCREEN_TITLE);
    state->flames = animation_play(&flame_animation, 0, 240, Z_START_SCREEN_FLAMES, LAYER_START_SCREEN_FLAMES);
    state->press_start = ANIMATION_HANDLE_NONE;
    state->press_start_started = false;

    uint8_t music_select_bit = random_bit();

    if (music_select_bit) {
    	audio_play(MUSIC_MENU_1);
    } else {
    	audio_play(MUSIC_MENU_2);
    }
    audio_load(SFX_SELECT);
}

void update_start_screen(GameState *game_state, StartScreenState *state, const PlayerGameInput *p1_input, const PlayerGameInput *p2_input){
    if (!state->press_start_started && animation_point(state->title) >= 1){
        state->press_start = animation_play(&press_start_animation, 45, 140, Z_START_SCREEN_TEXT_PROMPT, LAYER_START_SCREEN_TEXT_PROMPT);
        state->press_start_started = true;
    }
    
    if(p1_input->button1_pressed || p2_input->button1_pressed){
        audio_play(SFX_SELECT);
        *game_state = GS_CHARACTER_SELECTION;
        debug_printf("TRANSITION: START SCREEN -> CHAR SELECT\n");
        return;
    }

    if (p1_input->dpad_up_pressed || p2_input->dpad_up_pressed) {
    	increase_game_volume();
    } else if (p1_input->dpad_down_pressed || p2_input->dpad_down_pressed){
    	decrease_game_volume();
    }
}

void build_start_screen_render(RenderBuilder *builder){
    // render_add(builder, TEX_START_SCREEN_BG, 0, 0, GAME_WIDTH, GAME_HEIGHT, 0, 0, 0, false);
    animation_render(builder);
}

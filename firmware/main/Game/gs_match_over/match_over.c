#include <stdint.h>

#include "audio.h"
#include "debug.h"
#include "match_over.h"
#include "game_state.h"

#include "screen_animation.h"
#include "animations_data.h"
#include "player.h"
#include "player_game_input.h"
#include "battle_camera.h"
#include "battle_render.h"
#include "render_builder.h"
#include "texture.h"
#include "character.h"
#include "layer_def.h"

#define Z_MATCH_OVER_BG 0U
#define Z_MATCH_OVER_ICON 1U
#define Z_MATCH_OVER_FIRE_1 2U
#define Z_MATCH_OVER_FIRE_2 3U
#define Z_MATCH_OVER_NAME 4U
#define Z_MATCH_OVER_QUOTE 5U

static const ImageRect char_name_table[NUM_CHARACTERS] = {
    [CHAR_FREDRIK] = {0, 0, 144, 48},
    [CHAR_JENS] = {144, 0, 144, 48},
    [CHAR_EIRIK] = {0, 96, 144, 48},
    [CHAR_PERNILLE] = {0, 48, 144, 48},
    [CHAR_SONDRE] = {144, 48, 144, 48},
    [CHAR_ARI] = {144, 96, 144, 48},
};

void enter_match_over(){
    audio_play(MUSIC_MATCH_OVER);
    animation_play(&static_flame_animation, 0, 154, Z_MATCH_OVER_FIRE_1, LAYER_MATCH_OVER_FLAME_1);
    animation_play(&static_flame_upsidedown_animation, 0, -10, Z_MATCH_OVER_FIRE_2, LAYER_MATCH_OVER_FLAME_2);
}

void update_match_over(
    GameState *game_state,
    const PlayerGameInput *p1_input, 
    const PlayerGameInput *p2_input
) {
    bool confirm_pressed = p1_input->button1_pressed || p2_input->button1_pressed;
    if (confirm_pressed) {
        debug_printf("TRANSITION: MATCH OVER -> START SCREEN\n");
        *game_state = GS_START_SCREEN;
    }
    return;
}

void build_match_over_render(
    RenderBuilder *builder, 
    const MatchState *match_state,
    const Player *p1,
    const Player *p2
) {
    render_add(builder, LAYER_BACKGROUND, TEX_MATCH_OVER_BG, 0, 0, 320, 240, 0, 0, Z_MATCH_OVER_BG, false);
    
    if (match_state->last_round_result == ROUND_RESULT_P1_WIN) {
        uint16_t char_icon_x = p1->character_id * 100;
        render_add(builder, LAYER_MATCH_OVER_LARGE_ICON, TEX_CHAR_SELECT_LARGE_ICONS, char_icon_x, 0, 100, 176, 20, 50, Z_MATCH_OVER_ICON, false);

        ImageRect p1_name = char_name_table[p1->character_id];
        render_add(builder, LAYER_MATCH_OVER_NAME, TEX_CHAR_SELECT_UI, p1_name.x, p1_name.y, p1_name.w, p1_name.h, 134, 70, Z_MATCH_OVER_NAME, false);

        uint16_t char_quote_y = p1->character_id * 32;
        render_add(builder, LAYER_MATCH_OVER_QUOTES, TEX_MATCH_OVER_QUOTES, 0, char_quote_y, 176, 32, 134, 134, Z_MATCH_OVER_NAME, false);
    } else {
        uint16_t char_x = p2->character_id * 100;
        render_add(builder, LAYER_MATCH_OVER_LARGE_ICON, TEX_CHAR_SELECT_LARGE_ICONS, char_x, 0, 100, 176, 20, 50, Z_MATCH_OVER_ICON, false);

        ImageRect p2_name = char_name_table[p2->character_id];
        render_add(builder, LAYER_MATCH_OVER_NAME, TEX_CHAR_SELECT_UI, p2_name.x, p2_name.y, p2_name.w, p2_name.h, 134, 70, Z_MATCH_OVER_NAME, false);

        uint16_t char_quote_y = p2->character_id * 32;
        render_add(builder, LAYER_MATCH_OVER_QUOTES, TEX_MATCH_OVER_QUOTES, 0, char_quote_y, 176, 32, 134, 134, Z_MATCH_OVER_NAME, false);
    }

    animation_render(builder);
}

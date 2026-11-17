#include <stdbool.h>
#include "game_random.h"
#include "audio.h"

#include "round_over.h"
#include "game_state.h"

#include "player.h"
#include "player_game_input.h"

#include "stage_def.h"
#include "layer_def.h"
#include "battle_setup.h"
#include "battle_camera.h"
#include "screen_animation.h"
#include "texture.h"
#include "render_builder.h"
#include "battle_render.h"

#define Z_STAGE 0U
#define Z_PLAYER_BACK 10U
#define Z_PLAYER_FRONT 11U
#define Z_ROUND_STATUS 12U

static uint8_t random_text_value = 0;

void enter_round_over(void){
    random_text_value = random_bit();
    audio_play(MUSIC_ROUND_OVER);
}

void update_round_over(
    GameState *game_state, 
    MatchState *match_state, 
    const StageDef *stage_def,
    Player *p1,
    Player *p2,
    const PlayerGameInput *p1_input, 
    const PlayerGameInput *p2_input
) {
    bool match_over = (match_state->p1_round_wins == 2) || (match_state->p2_round_wins == 2);
    bool confirm_pressed = p1_input->button1_pressed || p2_input->button1_pressed;

    if (match_over && confirm_pressed) {
        *game_state = GS_MATCH_OVER;
        return;
    }

    if (!confirm_pressed) {
        return;
    }

    reset_state_for_round(p1, p2, match_state, stage_def->stage_width);
    *game_state = GS_BATTLE;
}

void build_round_over_render(
    RenderBuilder *builder, 
    const StageDef *stage_def,
    const MatchState *match_state,
    const Player *p1,
    const Player *p2
) {
    /* Nothing to render to -> no reason to render */
    if (stage_def->stage_width == 0U || stage_def->stage_height == 0U){
        return;
    }

    int camera_x = 0;
    int camera_y = 0;
    calc_battle_camera(stage_def, p1, p2, &camera_x, &camera_y);

    /* Add stage layers to snapshot */
    for (uint8_t layer = 0; layer < stage_def->layer_count; layer++){
        const StageLayerDef *layer_def = &stage_def->layers[layer];

        int dst_x = (int)layer_def->world_x - (int)((float)camera_x * layer_def->scroll_factor);
        int dst_y = (int)layer_def->world_y - (int)((float)camera_y * layer_def->scroll_factor);

        render_add(
            builder, 
            layer_def->layer_index,
            layer_def->image_id, 
            0, 
            0, 
            layer_def->width, 
            layer_def->height,
            dst_x,
            dst_y,
            (uint8_t)(Z_STAGE + layer),
            false
        );
    }


    /* Add players to snapshot */
    bool p2_in_front = (p2->state == PS_PUNCH) && (p1->state != PS_PUNCH);

    render_add_player(builder, p1, camera_x, camera_y, p2_in_front ? Z_PLAYER_BACK : Z_PLAYER_FRONT, LAYER_BATTLE_P1);
    render_add_player(builder, p2, camera_x, camera_y, p2_in_front ? Z_PLAYER_FRONT : Z_PLAYER_BACK, LAYER_BATTLE_P2);

    if (match_state->last_round_result == ROUND_RESULT_DRAW){
        render_add(builder, LAYER_ROUND_OVER_TEXT_TOP, TEX_ROUND_OVER_TEXT, 0, 256, 272, 64, 24, 88, Z_ROUND_STATUS, false);
    } else {
        if (match_state->last_round_result == ROUND_RESULT_P1_WIN) {
            render_add(builder, LAYER_ROUND_OVER_TEXT_TOP, TEX_ROUND_OVER_TEXT, 0, 64, 272, 64, 24, 56, Z_ROUND_STATUS, false);
        } else {
            render_add(builder, LAYER_ROUND_OVER_TEXT_TOP, TEX_ROUND_OVER_TEXT, 0, 0, 272, 64, 24, 56, Z_ROUND_STATUS, false);
        }
    
        if (random_text_value){
            render_add(builder, LAYER_ROUND_OVER_TEXT_BOTTOM, TEX_ROUND_OVER_TEXT, 0, 128, 272, 64, 24, 120, Z_ROUND_STATUS, false);
        } else {
            render_add(builder, LAYER_ROUND_OVER_TEXT_BOTTOM, TEX_ROUND_OVER_TEXT, 0, 192, 272, 64, 24, 120, Z_ROUND_STATUS, false);
        }
    }

    animation_render(builder);
}

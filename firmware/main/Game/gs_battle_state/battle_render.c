#include <stdint.h>

#include "battle_render.h"
#include "battle_camera.h"

#include "game_state.h"
#include "texture.h"

#include "render_builder.h"
#include "stage_def.h"
#include "player.h"

#include "character.h"
#include "character_animation.h"
#include "screen_animation.h"
#include "layer_def.h"

#define Z_STAGE 0U
#define Z_PLAYER_BACK 10U
#define Z_PLAYER_FRONT 11U
#define Z_UI_BACK 12U
#define Z_UI_FRONT 13U

#define HUD_SRC_X 0U
#define HUD_SRC_Y 0U
#define HUD_W 320U
#define HUD_H 44U
#define HUD_X 0U
#define HUD_Y 0U

#define HP_BAR_SRC_X 320U
#define HP_BAR_SRC_Y 0U
#define HP_BAR_W 122U
#define HP_BAR_H 5U

#define P1_HP_BAR_X 133U
#define P1_HP_BAR_Y 16U

#define P2_HP_BAR_X 187U
#define P2_HP_BAR_Y 16U

#define STUN_BAR_SRC_X 320U
#define STUN_BAR_SRC_Y 5U
#define STUN_BAR_W 40U
#define STUN_BAR_H 3U

#define P1_STUN_X 93U
#define P1_STUN_Y 24U

#define P2_STUN_X 227U
#define P2_STUN_Y 24U

#define DIGIT_SOURCE_X 320U
#define DIGIT_SOURCE_Y 16U
#define DIGIT_W 16U
#define DIGIT_H 32U
#define DIGIT_X 144U
#define DIGIT_Y 8U


void render_add_player(RenderBuilder *builder,
    const Player *player,
    int camera_x,
    int camera_y,
    uint8_t z,
    uint8_t layer_index
) {
    const CharacterDef *def = &character_table[player->character_id];
    AnimationFrame frame = get_current_frame(player, def);

    bool flip = !player->facing_right;
    int anchor_x = flip ? (frame.src_w - frame.anchor_x) : frame.anchor_x;

    uint16_t currrent_image_id = get_current_image_id(player, def);
    int dst_x = ((int)player->pos_x - camera_x) - anchor_x;
    int dst_y = ((int)player->pos_y - camera_y) - frame.anchor_y;

    render_add(
        builder,
        layer_index,
        currrent_image_id,
        frame.src_x,
        frame.src_y,
        frame.src_w,
        frame.src_h,
        dst_x,
        dst_y,
        z,
        flip
    );
}

static void render_add_digits(RenderBuilder *builder, uint8_t value, int x, int y, uint8_t z){
    if (value > 99U){
        value = 99U;
    }

    uint8_t tens = value / 10;
    uint8_t ones = value % 10;

    render_add(builder, LAYER_BATTLE_UI_DIGIT_1, TEX_BATTLE_UI, DIGIT_SOURCE_X + DIGIT_W * tens, DIGIT_SOURCE_Y, DIGIT_W, DIGIT_H, x, y, z, false);
    render_add(builder, LAYER_BATTLE_UI_DIGIT_2, TEX_BATTLE_UI, DIGIT_SOURCE_X + DIGIT_W * ones, DIGIT_SOURCE_Y, DIGIT_W, DIGIT_H, x + DIGIT_W, y, z, false);
}

void build_battle_render(
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
    if (match_state->ko_freeze_timer > 0U){
        /* Render only KO flash and no other stage layers on KO */
        render_add(builder, LAYER_BATTLE_KO_FLASH, TEX_KO_FLASH, 0, 0, GAME_WIDTH, GAME_HEIGHT, 0, 0, Z_STAGE, false);
    } else {
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
    }


    /* Add players to snapshot */
    /* Draw attacking player in front (on top) of the other */
    bool p2_in_front = (p2->state == PS_PUNCH) && (p1->state != PS_PUNCH);

    render_add_player(builder, p1, camera_x, camera_y, p2_in_front ? Z_PLAYER_BACK : Z_PLAYER_FRONT, LAYER_BATTLE_P1);
    render_add_player(builder, p2, camera_x, camera_y, p2_in_front ? Z_PLAYER_FRONT : Z_PLAYER_BACK, LAYER_BATTLE_P2);


    /* Add UI to snapshot */
    const CharacterDef *p1_def = &character_table[p1->character_id];
    const CharacterDef *p2_def = &character_table[p2->character_id];

    /* Static HUD */
    render_add(builder, LAYER_BATTLE_UI_BACKGROUND, TEX_BATTLE_UI, HUD_SRC_X, HUD_SRC_Y, HUD_W, HUD_H, HUD_X, HUD_Y, Z_UI_BACK, false);

    /* Health bars */
    render_add_meter(builder, LAYER_BATTLE_UI_HEALTH_BAR_P1, TEX_BATTLE_UI, HP_BAR_SRC_X, HP_BAR_SRC_Y, HP_BAR_W, HP_BAR_H, p1->hp, p1_def->max_health, P1_HP_BAR_X, P1_HP_BAR_Y, true, Z_UI_FRONT);
    render_add_meter(builder, LAYER_BATTLE_UI_HEALTH_BAR_P2, TEX_BATTLE_UI, HP_BAR_SRC_X, HP_BAR_SRC_Y, HP_BAR_W, HP_BAR_H, p2->hp, p2_def->max_health, P2_HP_BAR_X, P2_HP_BAR_Y, false, Z_UI_FRONT);

    /* Stun meters */
    render_add_meter(builder, LAYER_BATTLE_UI_STUN_BAR_P1, TEX_BATTLE_UI, STUN_BAR_SRC_X, STUN_BAR_SRC_Y, STUN_BAR_W, STUN_BAR_H, p1->stun_meter, p1_def->stun_threshold, P1_STUN_X, P1_STUN_Y, false, Z_UI_FRONT);
    render_add_meter(builder, LAYER_BATTLE_UI_STUN_BAR_P2, TEX_BATTLE_UI, STUN_BAR_SRC_X, STUN_BAR_SRC_Y, STUN_BAR_W, STUN_BAR_H, p2->stun_meter, p2_def->stun_threshold, P2_STUN_X, P2_STUN_Y, true, Z_UI_FRONT);

    render_add_digits(builder, match_state->round_timer_seconds, DIGIT_X, DIGIT_Y, Z_UI_FRONT);

    animation_render(builder);
}

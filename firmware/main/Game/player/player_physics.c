#include <stdint.h>
#include "player_physics.h"
#include "player.h"
#include "character.h"
#include "character_animation.h"

void update_player_physics(Player *player, const CharacterDef *def, int16_t ground_y) {
    bool airborne = (player->pos_y < ground_y) || (player->velocity_y != 0);
    if (!airborne) {
        return;
    }

    player->pos_x += player->velocity_x;

    player->velocity_y += def->gravity;
    int32_t combined = ((int32_t)player->pos_y << JUMP_SUBPIXEL_SHIFT) + player->pos_y_subpixel + player->velocity_y;

    player->pos_y = (int16_t)(combined >> JUMP_SUBPIXEL_SHIFT);
    player->pos_y_subpixel = (int16_t)(combined & 0xF);

    /* Resets on land */
    if (player->pos_y >= ground_y) {
        player->pos_y = ground_y;
        player->pos_y_subpixel = 0;
        player->velocity_y = 0;
        player->velocity_x = 0;
        
        /* Keep animations running when landing (except jump animation) */
        if (player->state == PS_JUMP) {
            player->state = PS_IDLE;
            player->animation_frame = 0;
            player->frame_timer = get_current_frame(player, def).duration;
        }
    }
}
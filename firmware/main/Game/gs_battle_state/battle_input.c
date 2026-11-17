#include "debug.h"
#include <stdbool.h>

#include "battle_input.h"
#include "battle_combat.h"
#include "battle_constants.h"

#include "player.h"
#include "input_buffer.h"
#include "player_game_input.h"

#include "character.h"
#include "character_animation.h"

static const InputDirection jump_left_motion[] = { DIR_LEFT };
static const InputDirection jump_right_motion[] = { DIR_RIGHT };

void update_player_state_from_input(
    Player *player, 
    const PlayerGameInput *player_input, 
    InputBuffer *player_buffer,
    const CharacterDef *def,
    const Player *opponent,
    const CharacterDef *opponent_def
){
    /* Check if state is locked (input is ignored in locked states) */
    bool is_locked = false;

    AnimationFrame current_frame = get_current_frame(player, def);
    is_locked = !current_frame.is_cancelable;

    /* Ignore input if character is locked into frame */
    if (is_locked){
        return;
    }

    /* New state only when previous ticks state is different */
    PlayerState previous_state = player->state;

    /* If player is holding the dpad direction away from the other player (decides if an action is a block or walk) */
    bool holding_away = (player->facing_right && player_input->dpad_left) || (!player->facing_right && player_input->dpad_right);

    if (player_input->button1_pressed){
        /* LIGHT PUNCH as default */
        MoveID move_to_use = MOVE_LIGHT_PUNCH;
        
        /* Check for sequence */
        for (int i = 0; i < def->special_count; i++){
            const SpecialMoveDef *special = &def->specials[i];
            
            if (special->required_button == BUTTON_1 && check_input_sequence(player_buffer, special->sequence, special->sequence_length, special->max_ticks_ago, player->facing_right)) {
                move_to_use = special->triggers_move;
                break;
            }
        }
        
        /* Set state and animation */
        player->state = PS_PUNCH;
        player->current_move_id = move_to_use;
        player->attack_has_hit = false;
        player->animation_frame = 0;
        player->frame_timer = get_current_frame(player, def).duration;
    } else if (player_input->button2_pressed){
        /* Begin THROWING state */
        player->state = PS_THROW_STARTUP;
        player->current_move_id = MOVE_NONE;
        player->attack_has_hit = false;
        player->velocity_x = 0;
        player->animation_frame = 0;
        player->frame_timer = get_current_frame(player, def).duration;
    } else if (player_input->dpad_up_pressed) {
        player->state = PS_JUMP;
        player->current_move_id = MOVE_NONE;
        player->velocity_y = def->jump_velocity;

        /* Check if player was walking in a direction when jumping */
        if (check_input_sequence(player_buffer, jump_left_motion, 1, JUMP_DIRECTION_WINDOW, true)) {
            player->velocity_x = -def->jump_horizontal_speed;
            debug_printf("left jump\n");

        } else if (check_input_sequence(player_buffer, jump_right_motion, 1, JUMP_DIRECTION_WINDOW, true)) {
            player->velocity_x = def->jump_horizontal_speed;
            debug_printf("right jump\n");
        } else {
            player->velocity_x = 0;
            debug_printf("up jump\n");
        }


    } else if (holding_away && opponent->state == PS_PUNCH && check_hit(opponent, get_current_frame(opponent, opponent_def), player, current_frame)) {
        /* BLOCK: Holding away while opponent is attacking */
        player->state = PS_BLOCK;
        player->current_move_id = MOVE_NONE;
    } else if (player_input->dpad_left || player_input->dpad_right){
        /* MOVE CHARACTER */
        player->state = PS_WALK;
        player->current_move_id = MOVE_NONE;

        if (player_input->dpad_left) {
            player->pos_x -= def->walk_speed;
        } else if (player_input->dpad_right){
            player->pos_x += def->walk_speed;
        }
    } else if (player->state == PS_WALK || player->state == PS_BLOCK) {
        player->state = PS_IDLE;
        player->current_move_id = MOVE_NONE;
    }

    /* Only reset animation on new state */
    if (player->state != previous_state){
        player->animation_frame = 0;
        player->frame_timer = get_current_frame(player, def).duration;
    }
}

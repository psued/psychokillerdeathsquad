#include <stdint.h>

#include "character_animation.h"
#include "character.h"
#include "player.h"
#include "player_physics.h"

/* Map player state to animation type */
static AnimationType get_anim_type_from_player_state(PlayerState state){
	switch (state) {
		case PS_WALK: return ANIM_WALK;
		case PS_BLOCK: return ANIM_BLOCK;
		case PS_BLOCK_STUNNED: return ANIM_BLOCK_STUNNED;
		case PS_HIT_STUNNED: return ANIM_HIT_STUNNED;
		case PS_CROUCH: return ANIM_CROUCH;
		case PS_DIZZY: return ANIM_STUNNED;
		case PS_JUMP: return ANIM_JUMP;
        case PS_THROWN: return ANIM_THROWN;
        case PS_KNOCKDOWN: return ANIM_KNOCKDOWN;
        case PS_WAKEUP: return ANIM_WAKEUP;
		case PS_DEAD: return ANIM_DEAD;

		default: return ANIM_IDLE;
	}
}

/* Get the current frame of an animation */
AnimationFrame get_current_frame(const Player *player, const CharacterDef *def){
    switch (player->state){
        case PS_PUNCH:
		    return def->moves[player->current_move_id].frames[player->animation_frame];
        
        case PS_THROW_STARTUP:
            return def->throw.startup_frames[player->animation_frame];

        case PS_THROWING:
            return def->throw.connect_frames[player->animation_frame].frame;

        default: {
            AnimationType anim_type = get_anim_type_from_player_state(player->state);
            return def->animations[anim_type][player->animation_frame];
        }
    }

}

/* Get the total amount of frames for the current animation */
uint8_t get_current_frame_count(const Player *player, const CharacterDef *def) {
    switch (player->state){
        case PS_PUNCH:
            return def->moves[player->current_move_id].frame_count;
        
        case PS_THROW_STARTUP:
            return def->throw.startup_frame_count;

        case PS_THROWING:
            return def->throw.connect_frame_count;

        default: {
            AnimationType anim_type = get_anim_type_from_player_state(player->state);
            return def->anim_frame_count[anim_type];
        }
    }
}

/* Get the image id corresponding to the current state */
uint16_t get_current_image_id(const Player *player, const CharacterDef *def) {
    switch (player->state){
        case PS_PUNCH:
            return def->moves[player->current_move_id].image_id;

        case PS_THROW_STARTUP:
            return def->throw.startup_image_id;

        case PS_THROWING:
            return def->throw.connect_image_id;

        default: {
            AnimationType anim_type = get_anim_type_from_player_state(player->state);
            return def->animation_images[anim_type];
        }
    }
}

/* Leave lockout state, become dizzy if meter is filled or go idle */
static void exit_lockout(Player *player, const CharacterDef *def){
     /* If a player has been hit to dizzy in apply hit -> next state is DIZZY */
    if (player->dizzy_pending) {
        player->dizzy_pending = false;
        player->stun_meter = 0;
        player->combo_hit_count = 0;
        player->state = PS_DIZZY;
        player->state_timer = def->dizzy_duration;
    } else {
        /* If not -> next state is IDLE */
        player->state = PS_IDLE;
        player->combo_hit_count = 0;
        player->stun_immune = false;
    }
}  

void enter_knockdown(Player *player, const CharacterDef *def, uint8_t duration) {
    player->state = PS_KNOCKDOWN;
    player->state_timer = duration;

    player->combo_hit_count = 0;
    player->stun_immune = false;

    player->current_move_id = MOVE_NONE;
    player->velocity_x = 0;

    player->animation_frame = 0;
    player->frame_timer = get_current_frame(player, def).duration;
}

/* Update animation of character */
void update_player_animation(Player *player, const CharacterDef *def){
    /* Animation of player being thrown is controlled by attacker throwing */
    if (player->state == PS_THROWN){
        return;
    }

    /* If the current frame has reached its end in duration */
    if (player->frame_timer == 0){
        uint8_t frame_count = get_current_frame_count(player, def);

        /* If there are no more frames in the animation */
        if (player->animation_frame + 1 >= frame_count){
            /* Reset attack action to idle or loop current state */
            if (player->state == PS_PUNCH) {
                player->state = PS_IDLE;
                player->current_move_id = MOVE_NONE;
            } else if (player->state == PS_WAKEUP) {
                exit_lockout(player, def);
            } else if (player->state == PS_THROW_STARTUP){
                player->state = PS_IDLE;
            }
            
            if (player->state != PS_JUMP && player->state != PS_KNOCKDOWN && player->state != PS_THROWING) {
                /* Hold the last frame of the animation until lockout timer ends */
                player->animation_frame = 0;
            }

        } else {
            /* Move to next frame */
            player->animation_frame++;
        }

        /* Update current frame countdown when switching frame */
        AnimationFrame new_frame = get_current_frame(player, def);
        player->frame_timer = new_frame.duration;
    } else {
        player->frame_timer--;
    }
}

/* Count down and reset player state when stun states are over */
void update_player_lockout_timer(Player *player, const CharacterDef *def, int16_t ground_y) {
    if (player->state != PS_HIT_STUNNED && 
        player->state != PS_BLOCK_STUNNED && 
        player->state != PS_DIZZY &&
        player->state != PS_KNOCKDOWN){
        return;
    }

    if (player->state == PS_KNOCKDOWN && player_is_airborne(player, ground_y)){
        return;
    }

    if (player->state_timer > 0){
        player->state_timer--;
    }

    /* Set dizzy state or return player to idle */
    if (player->state_timer == 0){
        if (player_is_airborne(player, ground_y)) {
            return;
        }

        switch (player->state){
            case PS_KNOCKDOWN:
                player->state = PS_WAKEUP;
                break;

            default:
                exit_lockout(player, def);
                break;
        }

        player->animation_frame = 0;
        player->frame_timer = get_current_frame(player, def).duration;
    }
}
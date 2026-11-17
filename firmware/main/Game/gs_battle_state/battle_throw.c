#include <stdbool.h>
#include <stdint.h>
#include "audio.h"

#include "battle_throw.h"
#include "battle_boxes.h"
#include "battle_constants.h"

#include "player.h"
#include "character.h"
#include "character_animation.h"
#include "move_def.h"

#include "animations_data.h"
#include "screen_animation.h"

static bool can_be_thrown(const Player *player){
    switch (player->state){
        case PS_IDLE:
        case PS_WALK:
        case PS_CROUCH:
        case PS_BLOCK:
        case PS_PUNCH:
        case PS_THROW_STARTUP:
        case PS_DIZZY:
            return true;
        default:
            return false;
    }
}

/* Check if attackers throw box overlaps with defenders pushbox */
static bool check_throw(
    const Player *attacker, 
    const CharacterDef *attacker_def,
    const Player *defender,
    const CharacterDef *defender_def
) {
    if (attacker->state != PS_THROW_STARTUP) {
        return false;
    }

    /* Throw only connects if there is an active hitbox on the attacker */
    AnimationFrame attacker_frame = get_current_frame(attacker, attacker_def);
    if (attacker_frame.hitbox_w == 0){
        return false;
    }

    if (!can_be_thrown(defender)){
        return false;
    }

    BoxRect throw_box = get_world_hitbox(attacker, attacker_frame);
    BoxRect defender_box = get_world_pushbox(defender, defender_def->pushbox);

    return boxes_overlap(throw_box, defender_box);
}

/* Place the defenders position based on the attackers throw stats when being thrown */
static void place_thrown_defender(
    Player *attacker, 
    const CharacterDef *attacker_def,
    Player *defender,
    uint16_t stage_width
) {
    /* Get current defender position offset from the attackers position */
    const ThrowDefenderPose *pose = &attacker_def->throw.connect_frames[attacker->animation_frame].pose;

    /* Clamp new defender position to stage */
    int32_t x = (int32_t)attacker->pos_x + (attacker->facing_right ? pose->offset_x : -pose->offset_x);
    int32_t min_x = STAGE_EDGE_MARGIN;
    int32_t max_x = (int32_t)stage_width - STAGE_EDGE_MARGIN;
    if (x < min_x) x = min_x;
    if (x > max_x) x = max_x;

    defender->pos_x = (uint16_t)x;
    defender->pos_y = (int16_t)(attacker->pos_y + pose->offset_y);
    defender->animation_frame = pose->defender_frame;
}

static void start_throw(
    Player *attacker, 
    const CharacterDef *attacker_def,
    Player *defender,
    uint16_t stage_width
){
    /* Attacker goes into throwing state */
    attacker->state = PS_THROWING;
    attacker->current_move_id = MOVE_NONE;
    attacker->attack_has_hit = false;
    attacker->velocity_x = 0;
    attacker->pushback_velocity = 0;
    attacker->animation_frame = 0;
    attacker->frame_timer = get_current_frame(attacker, attacker_def).duration;

    /* Defender goes into thrown state, animation and position now controlled by the attacker */
    defender->state = PS_THROWN;
    defender->current_move_id = MOVE_NONE;
    defender->state_timer = 0;
    defender->velocity_x = 0;
    defender->velocity_y = 0;
    defender->pos_y_subpixel = 0;
    defender->pushback_velocity = 0;
    defender->combo_hit_count = 0;
    defender->stun_immune = (defender->state == PS_DIZZY);
    defender->facing_right = !attacker->facing_right;

    place_thrown_defender(attacker, attacker_def, defender, stage_width);
}

bool is_in_throw(const Player *player) {
    return player->state == PS_THROWING || player->state == PS_THROWN;
}

void resolve_throws(
    Player *p1, 
    Player *p2, 
    const CharacterDef *p1_def, 
    const CharacterDef *p2_def, 
    uint16_t stage_width
) {
    bool p1_throws_p2 = check_throw(p1, p1_def, p2, p2_def);
    bool p2_throws_p1 = check_throw(p2, p2_def, p1, p1_def);

    /* Simultaneous connect cancels both */
    if (p1_throws_p2 && p2_throws_p1){
        return;
    }

    if (p1_throws_p2){
        start_throw(p1, p1_def, p2, stage_width);
    } else if (p2_throws_p1) {
        start_throw(p2, p2_def, p1, stage_width);
    }
}

void update_throw(
    Player *attacker, 
    Player *defender, 
    const CharacterDef *attacker_def, 
    const CharacterDef *defender_def, 
    uint16_t stage_width
) {
    if (attacker->state != PS_THROWING) {
        return;
    }

    const ThrowDef *throw = &attacker_def->throw;

    place_thrown_defender(attacker, attacker_def, defender, stage_width);

    /* Damage and stun happens on the damage frame of the throw */
    if (!attacker->attack_has_hit && attacker->animation_frame == throw->damage_frame){
        defender->hp -= (defender->hp > throw->damage) ? throw->damage : defender->hp;
        attacker->attack_has_hit = true;
        audio_play(SFX_SLAM);

        /* Fill stun meter */
        if (!defender->stun_immune){
            uint16_t new_stun = (uint16_t)defender->stun_meter + throw->stun;
            defender->stun_meter = (new_stun > 255) ? 255 : (uint8_t)new_stun;

            if (defender->stun_meter >= defender_def->stun_threshold){
                defender->dizzy_pending = true;
            }
        }
    }

    /* Release defender after last throw frame */
    bool on_last_frame = (attacker->animation_frame + 1 >= throw->connect_frame_count);
    if (on_last_frame && attacker->frame_timer == 0){
        attacker->state = PS_IDLE;
        attacker->animation_frame = 0;
        attacker->frame_timer = get_current_frame(attacker, attacker_def).duration;

        enter_knockdown(defender, defender_def, throw->knockdown_duration);
    }
}

#include <stdbool.h>
#include "audio.h"

#include "battle_combat.h"
#include "battle_boxes.h"
#include "battle_constants.h"
#include "character.h"
#include "move_def.h"
#include "character_animation.h"
#include "player.h"

/* Reduce damage taken based on how many hits have been laid before in a combo */
static const uint8_t combo_damage_percent[] = {
    100, 
    100, 
    90, 
    80,
    70,
    60, 
    50,
};
#define COMBO_SCALING_STEPS (sizeof(combo_damage_percent) / sizeof(combo_damage_percent[0]))

static uint8_t scale_combo_damage(uint8_t base_damage, uint8_t combo_hit_count){
    uint8_t index = combo_hit_count;
    /* Combos longer than the combo damage percent array stays at the last percent in the array */
    if (index >= COMBO_SCALING_STEPS){
        index = COMBO_SCALING_STEPS - 1;
    }

    /* Scale damage by combo percent and make sure an attack deals minimum 1 damage */
    uint16_t scaled_damage = (uint16_t)base_damage * combo_damage_percent[index] / 100;
    return (scaled_damage == 0 && base_damage > 0) ? 1 : (uint8_t)scaled_damage;
}

static bool can_be_hit(const Player *player){
    return player->state != PS_KNOCKDOWN && 
           player->state != PS_WAKEUP && 
           player->state != PS_THROWING && 
           player->state != PS_THROWN;
}

static bool is_striking(const Player *player){
    return player->state == PS_PUNCH;
}

bool check_hit(
    const Player *attacker, 
    AnimationFrame attacker_frame, 
    const Player *defender, 
    AnimationFrame defender_frame
){
    /* No hitbox on frame or frame is part of action that has already hit the defender */
    if (attacker_frame.hitbox_w == 0 || attacker->attack_has_hit){
        return false;
    }

    BoxRect attacker_hitbox = get_world_hitbox(attacker, attacker_frame);
    BoxRect defender_hurtbox = get_world_hurtbox(defender, defender_frame);

    return boxes_overlap(attacker_hitbox, defender_hurtbox);
}

void apply_hit(Player *defender, const CharacterDef * defender_def, MoveDef move, bool push_right) {
    bool was_blocking = (defender->state == PS_BLOCK || defender->state == PS_BLOCK_STUNNED);
    bool was_stunned = (defender->state == PS_HIT_STUNNED);
    bool was_dizzy = (defender->state == PS_DIZZY);

    /* Set the defender state to blockstunned or hitstunned based on if they were blocking when hit */
    if (was_blocking){
        /* Block -> no damage taken */
        defender->state = PS_BLOCK_STUNNED;
        defender->state_timer = move.blockstun_duration;
        defender->combo_hit_count = 0;
        audio_play(SFX_BLOCK);
    } else {
        defender->state = PS_HIT_STUNNED;
        defender->state_timer = move.hitstun_duration;
        defender->combo_hit_count = was_stunned ? defender->combo_hit_count + 1 : 1;

        /* Stop defender movement and make them fall when hit (if jumping upwards) */
        defender->velocity_x = 0;
        if (defender->velocity_y < 0){
            defender->velocity_y = 0;
        }

        uint8_t damage = scale_combo_damage(move.damage, defender->combo_hit_count);

        /* Either reduce by damage from move or set to 0 if move takes more damage than hp left */
        defender->hp -= (defender->hp > damage) ? damage : defender->hp;
        audio_play(SFX_HIT);

        if (was_dizzy){
            defender->stun_immune = true;
        }

        if (!defender->stun_immune){
            uint16_t new_stun = (uint16_t)defender->stun_meter + move.stun;
            defender->stun_meter = (new_stun > 255) ? 255 : (uint8_t)new_stun;

            if (defender->stun_meter >= defender_def->stun_threshold){
                defender->dizzy_pending = true;
            }
        }

        if (move.knockdown_duration > 0){
            enter_knockdown(defender, defender_def, move.knockdown_duration);
        }
    }
    defender->animation_frame = 0;
    defender->frame_timer = get_current_frame(defender, defender_def).duration;
    defender->pushback_velocity = push_right ? (int8_t)move.pushback : -(int8_t)move.pushback;
}



void resolve_hits(Player *p1, Player *p2, const CharacterDef *p1_def, const CharacterDef *p2_def) {
    AnimationFrame p1_frame = get_current_frame(p1, p1_def);
    AnimationFrame p2_frame = get_current_frame(p2, p2_def);

    /* Set if any players hit each other (trading hits is allowed) */
    bool p1_hits_p2 = is_striking(p1) && can_be_hit(p2) && check_hit(p1, p1_frame, p2, p2_frame);
    bool p2_hits_p1 = is_striking(p2) && can_be_hit(p1) && check_hit(p2, p2_frame, p1, p1_frame);

    /* Apply hit damage */
    if (p1_hits_p2) {
        apply_hit(p2, p2_def, p1_def->moves[p1->current_move_id], p1->facing_right);
        p1->attack_has_hit = true;
    }
    if (p2_hits_p1) {
        apply_hit(p1, p1_def, p2_def->moves[p2->current_move_id], p2->facing_right);
        p2->attack_has_hit = true;
    }
}

void apply_pushback(Player *pushed, Player *other, uint16_t stage_width) {
    if (pushed->pushback_velocity == 0) {
        return;
    }

    int32_t target = (int32_t)pushed->pos_x + pushed->pushback_velocity;
    int32_t min_x = STAGE_EDGE_MARGIN;
    int32_t max_x = (int32_t)stage_width - STAGE_EDGE_MARGIN;

    int32_t clamped = target;
    if (clamped < min_x) clamped = min_x;
    if (clamped > max_x) clamped = max_x;

    int32_t blocked = target - clamped;
    pushed->pos_x = (uint16_t)clamped;
    /* Push the other player in the opposite direction if at a stage edge */
    other->pos_x = (uint16_t)((int32_t)other->pos_x - blocked);

    /* Slow pushback speed each tick */
    if (pushed->pushback_velocity > 0) {
        pushed->pushback_velocity--;
    } else {
        pushed->pushback_velocity++;
    }
}

/* Decrease stun meter of player 1 point every interval (interval depending on character stun_decay_interval) */
void update_stun_decay(Player *player, const CharacterDef *def) {
    /* If there is nothing to drain or the player is dizzy */
    if (player->stun_meter == 0 || player->state == PS_DIZZY){
        player->stun_decay_counter = 0;
        return;
    }

    /* Decrease stun meter */
    player->stun_decay_counter++;

    if (player->stun_decay_counter >= def->stun_decay_interval){
        player->stun_decay_counter = 0;
        player->stun_meter--;
    }
}

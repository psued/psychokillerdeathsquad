#include <stdint.h>
#include <stdbool.h>

#include "battle_boxes.h"
#include "player.h"
#include "character.h"
#include "character_animation.h"

bool boxes_overlap(BoxRect a, BoxRect b){
    return (a.x < b.x + b.w) && (a.x + a.w > b.x) && (a.y < b.y + b.h) && (a.y + a.h > b.y);
}

BoxRect get_world_hurtbox(const Player *player, AnimationFrame frame){
    bool flip_horizontally = !player->facing_right;
    uint8_t effective_anchor_x = flip_horizontally ? (frame.src_w - frame.anchor_x) : frame.anchor_x;
    uint8_t hurtbox_x = flip_horizontally ? (frame.src_w - frame.hurtbox_x - frame.hurtbox_w): frame.hurtbox_x;

    return (BoxRect){
        (int16_t)((player->pos_x - effective_anchor_x) + hurtbox_x),
        (int16_t)((player->pos_y - frame.anchor_y) + frame.hurtbox_y),
        frame.hurtbox_w,
        frame.hurtbox_h
    };
}

BoxRect get_world_hitbox(const Player *player, AnimationFrame frame){
    bool flip_horizontally = !player->facing_right;
    uint8_t effective_anchor_x = flip_horizontally ? (frame.src_w - frame.anchor_x) : frame.anchor_x;
    uint8_t hitbox_x = flip_horizontally ? (frame.src_w - frame.hitbox_x - frame.hitbox_w): frame.hitbox_x;

    return (BoxRect){
        (int16_t)((player->pos_x - effective_anchor_x) + hitbox_x),
        (int16_t)((player->pos_y - frame.anchor_y) + frame.hitbox_y),
        frame.hitbox_w,
        frame.hitbox_h
    };
}

BoxRect get_world_pushbox(const Player *player, PushBoxDef pushbox) {
    return (BoxRect){
        (int16_t)(player->pos_x - (pushbox.width / 2)),
        (int16_t)(player->pos_y - (pushbox.height)),
        pushbox.width, 
        pushbox.height
    };
}
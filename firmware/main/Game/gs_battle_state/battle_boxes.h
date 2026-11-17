#ifndef BATTLE_BOXES_H
#define BATTLE_BOXES_H

#include <stdint.h>
#include <stdbool.h>

#include "battle_boxes.h"
#include "player.h"
#include "character.h"
#include "character_animation.h"

typedef struct {
    uint16_t x, y;
    uint8_t w, h;
} BoxRect;

bool boxes_overlap(BoxRect a, BoxRect b);

BoxRect get_world_hurtbox(const Player *player, AnimationFrame frame);

BoxRect get_world_hitbox(const Player *player, AnimationFrame frame);

BoxRect get_world_pushbox(const Player *player, PushBoxDef pushbox);

#endif
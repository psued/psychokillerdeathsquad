#ifndef PLAYER_PHYSICS_H
#define PLAYER_PHYSICS_H

#include <stdint.h>
#include "player.h"
#include "character.h"

#define JUMP_SUBPIXEL_SHIFT 4

static inline bool player_is_airborne(const Player *player, int16_t ground_y) {
    return player->pos_y < ground_y;
}

void update_player_physics(Player *player, const CharacterDef *def, int16_t ground_y);

#endif
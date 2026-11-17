#ifndef BATTLE_POSITION_CONSTRAINTS_H
#define BATTLE_POSITION_CONSTRAINTS_H

#include <stdint.h>
#include "player.h"
#include "character.h"

void clamp_player_to_stage(Player *player, uint16_t stage_width, uint8_t edge_margin);

void clamp_player_distance(Player *p1, Player *p2, uint16_t max_distance);

/* Push colliding characters away from each other */
void resolve_pushbox_collision(Player *p1, Player *p2, const CharacterDef *p1_def, const CharacterDef *p2_def, int16_t ground_y);

#endif
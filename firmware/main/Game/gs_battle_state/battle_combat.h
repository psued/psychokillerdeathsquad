#ifndef BATTLE_COMBAT_H
#define BATTLE_COMBAT_H

#include <stdint.h>
#include <stdbool.h>

#include "character.h"
#include "move_def.h"
#include "character_animation.h"
#include "player.h"

bool check_hit(
    const Player *attacker, 
    AnimationFrame attacker_frame, 
    const Player *defender, 
    AnimationFrame defender_frame
);

void apply_hit(Player *defender, const CharacterDef * defender_def, MoveDef move, bool push_right);

void resolve_hits(Player *p1, Player *p2, const CharacterDef *p1_def, const CharacterDef *p2_def);

void apply_pushback(Player *pushed, Player *other, uint16_t stage_width);

void update_stun_decay(Player *player, const CharacterDef *def);

#endif
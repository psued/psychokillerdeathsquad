#ifndef BATTLE_THROW_H
#define BATTLE_THROW_H

#include <stdbool.h>
#include <stdint.h>

#include "player.h"
#include "character.h"

bool is_in_throw(const Player *player);

void resolve_throws(
    Player *p1, 
    Player *p2, 
    const CharacterDef *p1_def, 
    const CharacterDef *p2_def, 
    uint16_t stage_width
);

void update_throw(
    Player *attacker, 
    Player *defender, 
    const CharacterDef *attacker_def, 
    const CharacterDef *defender_def, 
    uint16_t stage_width
);

#endif
#ifndef BATTLE_SETUP_H
#define BATTLE_SETUP_H

#include <stdint.h>
#include "player.h"
#include "battle.h"

/**
 * Resets players and match state before the next round
 */
void reset_state_for_round(Player *p1, Player *p2, MatchState *match_state, uint16_t stage_width);

#endif

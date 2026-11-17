#ifndef BATTLE_ROUND_H
#define BATTLE_ROUND_H

#include <stdbool.h>
#include "game_state.h"
#include "player.h"

/* Update countdown for current round */
void update_round_timer(MatchState *match_state);

/* Check who won (or draw) and update win counters */
void resolve_round_result(MatchState *match_state, bool p1_dead, bool p2_dead, Player *p1, Player *p2);

/* Checks for timer and death to transition to round over state */
void check_round_end_conditions(GameState *game_state, MatchState *match_state, Player *p1, Player *p2);

#endif
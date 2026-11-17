#include <stdbool.h>
#include "debug.h"

#include "battle_round.h"
#include "battle_constants.h"
#include "game_state.h"
#include "player.h"

void update_round_timer(MatchState *match_state) {
    match_state->tick_subcounter++;
    if (match_state->tick_subcounter >= 60) {
        match_state->tick_subcounter = 0;

        if (match_state->round_timer_seconds > 0){
            match_state->round_timer_seconds--;
        }
    }
}

void resolve_round_result(MatchState *match_state, bool p1_dead, bool p2_dead, Player *p1, Player *p2) {
    /* Both dead at the exact same time -> DRAW -> no one wins */
    if (p1_dead && p2_dead) {
        match_state->last_round_result = ROUND_RESULT_DRAW;
        return;
    } else if (p1_dead){
        match_state->p2_round_wins++;
        match_state->last_round_result = ROUND_RESULT_P2_WIN;
        return;
    } else if (p2_dead){
        match_state->p1_round_wins++;
        match_state->last_round_result = ROUND_RESULT_P1_WIN;
        return;
    } 

    /* On time expiration */
    if (p1->hp > p2->hp) {
        match_state->p1_round_wins++;
        match_state->last_round_result = ROUND_RESULT_P1_WIN;
    } else if (p2->hp > p1->hp){
        match_state->p2_round_wins++;
        match_state->last_round_result = ROUND_RESULT_P2_WIN;
    } else {
        /* Both same amount of HP -> DRAW -> no one wins */
        match_state->last_round_result = ROUND_RESULT_DRAW;
    }
}

void check_round_end_conditions(GameState *game_state, MatchState *match_state, Player *p1, Player *p2) {
    bool p1_dead = (p1->hp == 0);
    bool p2_dead = (p2->hp == 0);
    bool timer_expired = (match_state->round_timer_seconds == 0);

    /* Begin KO freeze if a player reaches 0 hp, or jump to round over if time expired */
    if (p1_dead || p2_dead) {
        match_state->ko_freeze_timer = KO_FREEZE_DURATION;
    } else if (timer_expired){
        resolve_round_result(match_state, false, false, p1, p2);
        debug_printf("TRANSITION: BATTLE -> ROUND OVER\n");
        *game_state = GS_ROUND_OVER;
        return;
    }
}

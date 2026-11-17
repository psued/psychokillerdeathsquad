#include <stdint.h>
#include "player.h"
#include "battle.h"
#include "battle_setup.h"
#include "battle_constants.h"
#include "character.h"

void reset_state_for_round(Player *p1, Player *p2, MatchState *match_state, uint16_t stage_width){
    const CharacterDef *p1_def = &character_table[p1->character_id];
    const CharacterDef *p2_def = &character_table[p2->character_id];

    /* Player positions */
    uint16_t center_x = stage_width / 2;
    p1->pos_x = center_x - STARTING_SEPARATION;
    p1->pos_y = P_START_Y;
    p2->pos_x = center_x + STARTING_SEPARATION;
    p2->pos_y = P_START_Y;

    /* Player states */
    p1->state = PS_IDLE;
    p1->state_timer = 0;
    p1->animation_frame = 0;
    p1->hp = p1_def->max_health;
    p1->stun_meter = 0;
    p1->stun_decay_counter = 0;
    p1->dizzy_pending = false;
    p1->stun_immune = false;
    p1->current_move_id = MOVE_NONE;
    p1->combo_hit_count = 0;
    p1->attack_has_hit = false;
    p1->facing_right = true;
    p1->velocity_x = 0;
    p1->velocity_y = 0;
    p1->pos_y_subpixel = 0;
    p1->pushback_velocity = 0;


    p2->state = PS_IDLE;
    p2->state_timer = 0;
    p2->animation_frame = 0;
    p2->hp = p2_def->max_health;
    p2->stun_meter = 0;
    p2->stun_decay_counter = 0;
    p2->dizzy_pending = false;
    p2->stun_immune = false;
    p2->current_move_id = MOVE_NONE;
    p2->combo_hit_count = 0;
    p2->attack_has_hit = false;
    p2->facing_right = false;
    p2->velocity_x = 0;
    p2->velocity_y = 0;
    p2->pos_y_subpixel = 0;
    p2->pushback_velocity = 0;

    /* Match state */
    match_state->ko_freeze_timer = 0;
    match_state->round_timer_seconds = ROUND_DURATION;
    match_state->tick_subcounter = 0;
}

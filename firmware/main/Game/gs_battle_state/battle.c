#include <stdbool.h>
#include "debug.h"
#include "audio.h"

#include "game_state.h"
#include "screen_constants.h"

#include "battle.h"
#include "battle_input.h"
#include "battle_constants.h"
#include "battle_combat.h"
#include "battle_round.h"
#include "battle_position_constraints.h"
#include "battle_throw.h"

#include "player.h"
#include "player_game_input.h"
#include "player_physics.h"
#include "input_buffer.h"

#include "character.h"
#include "character_animation.h"
#include "stage_def.h"

#include "animations_data.h"
#include "screen_animation.h"
#include "layer_def.h"

#define Z_BATTLE_STATE_BANNER 20U

void enter_battle_state(MatchState *match_state) {
    match_state->fight_banner = animation_play(&fight_banner_animation, 320, 88, Z_BATTLE_STATE_BANNER, LAYER_BATTLE_UI_BANNER);

    switch (match_state->stage_id){
        case STAGE_BRAZIL:
            audio_play(MUSIC_BATTLE_BRAZIL);
            break;
        case STAGE_USA:
            audio_play(MUSIC_BATTLE_USA);
            break;
        case STAGE_BRITAIN:
            audio_play(MUSIC_BATTLE_BRITAIN);
            break;
        case STAGE_EGYPT:
            audio_play(MUSIC_BATTLE_EGYPT);
            break;
        case STAGE_CHINA:
            audio_play(MUSIC_BATTLE_CHINA);
            break;
        case STAGE_JAPAN:
            audio_play(MUSIC_BATTLE_JAPAN);
            break;
        case STAGE_DUNGEON:
            audio_play(MUSIC_BATTLE_DUNGEON);
            break;
        case STAGE_VOLCANO:
            audio_play(MUSIC_BATTLE_VOLCANO);
            break;
        default:
            audio_play(MUSIC_BATTLE_BRAZIL);
            break;
    }

    audio_load(SFX_HIT_LIGHT);
    audio_load(SFX_HIT_HEAVY);
    audio_load(SFX_SLAM);
    audio_load(SFX_BLOCK);
    audio_load(SFX_GRAB);
}

void update_battle_state(
    GameState *game_state, 
    MatchState *match_state,
    const StageDef *stage_def,
    Player *p1, 
    Player *p2,
    const PlayerGameInput *p1_input, 
    const PlayerGameInput *p2_input,
    InputBuffer *p1_input_buffer,
    InputBuffer *p2_input_buffer
){
    /* Freeze game if KO timer is active and transition when timer reaches 0 */
    if (match_state->ko_freeze_timer > 0){
        match_state->ko_freeze_timer--;

        if (match_state->ko_freeze_timer == 0) {
            resolve_round_result(match_state, p1->hp == 0, p2->hp == 0, p1, p2);
            audio_play(SFX_SLAM);
            *game_state = GS_ROUND_OVER;
            debug_printf("TRANSITION: BATTLE -> ROUND OVER\n");
        }

        return;
    }

    /* Dont accept updates before battle intro has played */
    if (animation_is_playing(match_state->fight_banner)){
        return;
    }

    const CharacterDef *p1_def = &character_table[p1->character_id];
    const CharacterDef *p2_def = &character_table[p2->character_id];

    /* Freeze facing directions while a throw is active */
    bool throw_active = is_in_throw(p1) || is_in_throw(p2);

    if (!throw_active){
        /* Updating facing directions */
        p1->facing_right = (p2->pos_x >= p1->pos_x);
        p2->facing_right = (p1->pos_x >= p2->pos_x);
    }

    update_player_animation(p1, p1_def);
    update_player_animation(p2, p2_def);

    update_player_state_from_input(p1, p1_input, p1_input_buffer, p1_def, p2, p2_def);
    update_player_state_from_input(p2, p2_input, p2_input_buffer, p2_def, p1, p1_def);

    if (p1->state != PS_THROWN) update_player_physics(p1, p1_def, P_START_Y);
    if (p2->state != PS_THROWN) update_player_physics(p2, p2_def, P_START_Y);

    apply_pushback(p1, p2, stage_def->stage_width);
    apply_pushback(p2, p1, stage_def->stage_width);

    if (!throw_active){
        resolve_pushbox_collision(p1, p2, p1_def, p2_def, P_START_Y);
    }

    clamp_player_distance(p1, p2, GAME_WIDTH);
    clamp_player_to_stage(p1, stage_def->stage_width, STAGE_EDGE_MARGIN);
    clamp_player_to_stage(p2, stage_def->stage_width, STAGE_EDGE_MARGIN);

    update_throw(p1, p2, p1_def, p2_def, stage_def->stage_width);
    update_throw(p2, p1, p2_def, p1_def, stage_def->stage_width);

    update_player_lockout_timer(p1, p1_def, P_START_Y);
    update_player_lockout_timer(p2, p2_def, P_START_Y);

    update_stun_decay(p1, p1_def);
    update_stun_decay(p2, p2_def);

    resolve_hits(p1, p2, p1_def, p2_def);
    resolve_throws(p1, p2, p1_def, p2_def, stage_def->stage_width);

    update_round_timer(match_state);

    check_round_end_conditions(game_state, match_state, p1, p2);
}

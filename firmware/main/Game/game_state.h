#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <stdint.h>
#include "stage_def.h"
#include "audio.h"
#include "screen_animation.h"

/* All possible states the game can be in */ 
typedef enum {
	GS_START_SCREEN,
	GS_CHARACTER_SELECTION,
	GS_STAGE_SELECT,
	GS_BATTLE,
	GS_ROUND_OVER,
	GS_MATCH_OVER,
} GameState;

/* Result from previous round  */
typedef enum {
	ROUND_RESULT_P1_WIN,
	ROUND_RESULT_P2_WIN,
	ROUND_RESULT_DRAW,
} RoundResult;

typedef struct {
    StageID stage_id;
	AudioId match_audio;
    uint8_t p1_round_wins;
    uint8_t p2_round_wins;

    uint8_t round_timer_seconds; /* Show players and count down during round */
    uint8_t tick_subcounter; /* Tick counter to decrement round_timer_seconds */
    uint8_t ko_freeze_timer; /* Hold the KO hit for a few frames for more impact */
    RoundResult last_round_result;

	AnimationHandle fight_banner;
} MatchState;

#endif
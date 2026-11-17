#ifndef PLAYER_H
#define PLAYER_H

#include <stdint.h>
#include <stdbool.h>

typedef enum  {
	PS_IDLE,
	PS_WALK,
	PS_PUNCH,
	PS_BLOCK,
	PS_HIT_STUNNED,
	PS_BLOCK_STUNNED,
	PS_DIZZY,
	PS_JUMP,
	PS_THROW_STARTUP,
	PS_THROWING,
	PS_THROWN,
	PS_KNOCKDOWN,
	PS_WAKEUP,
	PS_CROUCH,
	PS_DEAD,
} PlayerState;

typedef struct {
	PlayerState state; /* Current active state. idle, walk etc */
	uint8_t character_id; /* Which character chosen (FREDRIK, DOG, etc) */

	uint16_t pos_x;
	int16_t pos_y;
	int16_t pos_y_subpixel;
	int16_t velocity_y;
	int16_t velocity_x;
	int8_t pushback_velocity;

	uint8_t state_timer; /* ticks left in currect state */
	uint8_t animation_frame; /* Which frame in sprite to show */
	uint8_t frame_timer; /* How many ticks left in the current frame */

	uint8_t stun_meter;
	uint8_t stun_decay_counter; /* Tick counter for stun meter decay */
	bool dizzy_pending; /* When player should go into dizzy state after hitstun is done */
	bool stun_immune; /* Combo should not increase stun meter if started on an already dizzy player */

	uint8_t hp;
	uint8_t current_move_id;
	uint8_t combo_hit_count;

	bool facing_right;
	bool attack_has_hit; /* If current punch has hit the target in this action (prevent damaging several times in same punch) */
} Player;

#endif
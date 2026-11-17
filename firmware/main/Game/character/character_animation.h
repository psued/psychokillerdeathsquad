#ifndef CHARACTER_ANIMATION_H
#define CHARACTER_ANIMATION_H

#include <stdint.h>
#include <stdbool.h>

#include "player.h"

typedef struct CharacterDef CharacterDef;

typedef enum {
	ANIM_IDLE,
	ANIM_WALK, 
	ANIM_JUMP,
	ANIM_CROUCH,
	ANIM_BLOCK,
	ANIM_BLOCK_STUNNED,
	ANIM_HIT,
	ANIM_HIT_STUNNED,
	ANIM_STUNNED,
	ANIM_THROW_STARTUP,
	ANIM_THROWING,
	ANIM_THROWN,
	ANIM_KNOCKDOWN, 
	ANIM_WAKEUP,
	ANIM_DEAD,
	NUM_ANIM_TYPES,
} AnimationType;

/* Contents of one frame of an animation */
typedef struct {
	/* Origin from spritesheet top left */
	uint16_t src_x, src_y;
	uint8_t src_w, src_h;
	
	/* Where this frame can be hit (origin is this frames top left) */
	uint8_t hurtbox_x, hurtbox_y;
	uint8_t hurtbox_w, hurtbox_h;
	
	/* Where this frame can hit (attack area) */
	/* Zero for frames with no attack */
	uint8_t hitbox_x, hitbox_y;
	uint8_t hitbox_w, hitbox_h;

	/* Where in the frame is the sprites "anchor point", ex between characters legs */
	uint8_t anchor_x, anchor_y;
	
	/* Ticks for frame */
	uint8_t duration;

	/* If the frame can be interrupted into another move */
	bool is_cancelable; 

} AnimationFrame;

AnimationFrame get_current_frame(const Player *player, const CharacterDef *def);

uint8_t get_current_frame_count(const Player *player, const CharacterDef *def);

uint16_t get_current_image_id(const Player *player, const CharacterDef *def);

void enter_knockdown(Player *player, const CharacterDef *def, uint8_t duration);

void update_player_animation(Player *player, const CharacterDef *def);

void update_player_lockout_timer(Player *player, const CharacterDef *def, int16_t ground_y);

#endif
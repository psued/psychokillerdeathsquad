#ifndef CHARACTER_H
#define CHARACTER_H

#include <stdint.h>
#include "move_def.h"
#include "character_animation.h"
#include "player.h"

#define MAX_SPECIALS_PER_CHARACTER 4

/* The different possible characters */
typedef enum {
	CHAR_FREDRIK,
	CHAR_JENS,
	CHAR_EIRIK,
	CHAR_PERNILLE,
	CHAR_SONDRE,
	CHAR_ARI,
	NUM_CHARACTERS,
} CharacterID;

/* The collision dimensions of a character */
typedef struct {
	uint8_t width;
	uint8_t height;
} PushBoxDef;

/* What each character consists of */
typedef struct CharacterDef {
	uint8_t walk_speed;
	uint8_t max_health;
	uint8_t stun_threshold; /* how larger the stun meter must fill up before getting into dizzy state */
	uint8_t stun_decay_interval; /* ticks between each 1 point decrease in stun meter */
	uint8_t dizzy_duration; /* how many ticks to spend in dizzy state */

	int16_t jump_velocity; /* initial upward jump speed */
	int16_t gravity; /* pull player down when jumping */
	int16_t jump_horizontal_speed;

	MoveDef moves[NUM_MOVES];
	SpecialMoveDef specials[MAX_SPECIALS_PER_CHARACTER];
	uint8_t special_count;

	ThrowDef throw;
	
	const AnimationFrame *animations[NUM_ANIM_TYPES];
	uint8_t anim_frame_count[NUM_ANIM_TYPES];
	uint16_t animation_images[NUM_ANIM_TYPES];
	PushBoxDef pushbox;
} CharacterDef;

extern const CharacterDef character_table[NUM_CHARACTERS];

#endif
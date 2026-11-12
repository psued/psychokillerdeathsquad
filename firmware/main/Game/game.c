#include "game.h"

#include "audio.h"
#include "input.h"
#include "storage.h"
#include "debug.h"

typedef enum {
	STAGE_MENU = 0,
	STAGE_SELECT
} Stage;

static Stage stage;

static void enter_stage(const Stage next)
{
	stage = next;

	if (!storage_available()) {
		return;
	}

	audio_free();

	(void)audio_load(SFX_HIT_LIGHT);

	if (stage == STAGE_MENU) {
		audio_play(MUSIC_MENU_2);
	} else {
		audio_play(MUSIC_SELECT);
	}

	audio_play(SFX_HIT_LIGHT);
}

void game_init(void)
{
	enter_stage(STAGE_SELECT);
}

void game_tick(void)
{
	const ControllerState player1 = input_get(INPUT_PLAYER_1);
	const ControllerState player2 = input_get(INPUT_PLAYER_2);

	const InputButtons pressed = player1.pressed | player2.pressed;

	if (stage == STAGE_MENU && (pressed & INPUT_BUTTON_A) != 0U) {
		enter_stage(STAGE_SELECT);
		debug_printf("Stage: %d\r\n", stage);
	} else if (stage == STAGE_SELECT && (pressed & INPUT_BUTTON_B) != 0U) {
		enter_stage(STAGE_MENU);
		debug_printf("Stage: %d\r\n", stage);
	}
}

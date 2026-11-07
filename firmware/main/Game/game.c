#include "game.h"

#include "audio.h"
#include "input.h"
#include "storage.h"

typedef enum {
	STAGE_MENU = 0,
	STAGE_BATTLE
} Stage;

static Stage stage;

static void enter_stage(const Stage next)
{
	stage = next;

	if (!storage_available()) {
		return;
	}

	audio_free();

	(void)audio_load(SFX_HIT);

	if (stage == STAGE_MENU) {
		audio_play(MUSIC_MENU);
	} else {
		audio_play(MUSIC_BATTLE);
	}

	audio_play(SFX_HIT);
}

void game_init(void)
{
	enter_stage(STAGE_MENU);
}

void game_tick(void)
{
	const ControllerState player1 = input_get(INPUT_PLAYER_1);
	const ControllerState player2 = input_get(INPUT_PLAYER_2);

	const InputButtons pressed = player1.pressed | player2.pressed;

	if (stage == STAGE_MENU && (pressed & INPUT_BUTTON_A) != 0U) {
		enter_stage(STAGE_BATTLE);
	} else if (stage == STAGE_BATTLE && (pressed & INPUT_BUTTON_B) != 0U) {
		enter_stage(STAGE_MENU);
	}
}

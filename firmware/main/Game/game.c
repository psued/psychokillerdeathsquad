#include "game.h"

#include "input.h"

void game_init(void)
{
}

void game_tick(void)
{
	const ControllerState player1 = input_get(INPUT_PLAYER_1);
	const ControllerState player2 = input_get(INPUT_PLAYER_2);

	(void)player1;
	(void)player2;
}

#include "input_wireless.h"

#include "error.h"

InputState input_wireless_get(const InputPlayer player)
{
	ASSERT(player < INPUT_PLAYER_COUNT);

	const InputState state = {
		.held = 0,
		.connected = false
	};

	return state;
}

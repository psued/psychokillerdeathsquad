#include "input.h"

#include "error.h"
#include "input_wired.h"
#include "input_wireless.h"

typedef enum {
	INPUT_SOURCE_NONE = 0,
	INPUT_SOURCE_WIRED,
	INPUT_SOURCE_WIRELESS
} InputSource;

typedef struct {
	InputSource source;
	ControllerState pending;
	ControllerState current;
} PlayerInput;

static PlayerInput players[INPUT_PLAYER_COUNT];

static InputSource select_source(const InputState wired, const InputState wireless)
{
	if (wired.connected) {
		return INPUT_SOURCE_WIRED;
	}

	if (wireless.connected) {
		return INPUT_SOURCE_WIRELESS;
	}

	return INPUT_SOURCE_NONE;
}

static InputButtons get_held(
	const InputSource source,
	const InputState wired,
	const InputState wireless
)
{
	switch (source) {
	case INPUT_SOURCE_NONE:
		return 0;

	case INPUT_SOURCE_WIRED:
		return wired.held;

	case INPUT_SOURCE_WIRELESS:
		return wireless.held;

	default:
		PANIC("Invalid input source");
	}
}

static void update_held(PlayerInput *input, const InputButtons held)
{
	const InputButtons changed = input->pending.held ^ held;

	input->pending.pressed |= changed & held;
	input->pending.released |= changed & input->pending.held;
	input->pending.held = held;
}

static void change_source(
	PlayerInput *input,
	const InputSource source,
	const InputButtons held
)
{
	input->source = source;

	input->pending.held = held;
	input->pending.pressed = 0;
	input->pending.released = 0;
}

void input_init(void)
{
	input_wired_init();
}

void input_process(void)
{
	for (
		InputPlayer player = INPUT_PLAYER_1;
		player < INPUT_PLAYER_COUNT;
		player++
	)
	{
		const InputState wired = input_wired_get(player);
		const InputState wireless = input_wireless_get(player);

		const InputSource source = select_source(wired, wireless);
		const InputButtons held = get_held(source, wired, wireless);

		PlayerInput *input = &players[player];

		if (source != input->source) {
			change_source(input, source, held);
		} else {
			update_held(input, held);
		}
	}
}

void input_freeze(void)
{
	for (
		InputPlayer player = INPUT_PLAYER_1;
		player < INPUT_PLAYER_COUNT;
		player++
	)
	{
		PlayerInput *input = &players[player];

		input->current = input->pending;
		input->pending.pressed = 0;
		input->pending.released = 0;
	}
}

ControllerState input_get(const InputPlayer player)
{
	ASSERT(player < INPUT_PLAYER_COUNT);
	return players[player].current;
}

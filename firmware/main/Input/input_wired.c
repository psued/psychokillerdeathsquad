#include "input_wired.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "error.h"
#include "main.h"
#include "stm32u5xx_hal.h"

extern TIM_HandleTypeDef htim6;

enum {
	WIRED_BUTTON_COUNT = 6,
	DEBOUNCE_SAMPLES = 10
};

typedef struct {
	GPIO_TypeDef *port;
	uint16_t pin;
	InputButtons button;
} ButtonMapping;

typedef struct {
	GPIO_TypeDef *port;
	uint16_t pin;
} PresenceMapping;

typedef struct {
	InputState state;
	uint8_t button_counters[WIRED_BUTTON_COUNT];
	uint8_t presence_counter;
} WiredController;

static const InputButtons buttons[WIRED_BUTTON_COUNT] = {
	INPUT_BUTTON_UP,
	INPUT_BUTTON_DOWN,
	INPUT_BUTTON_LEFT,
	INPUT_BUTTON_RIGHT,
	INPUT_BUTTON_A,
	INPUT_BUTTON_B
};

static const ButtonMapping button_mappings[INPUT_PLAYER_COUNT][WIRED_BUTTON_COUNT] = {
	[INPUT_PLAYER_1] = {
		{ GPIOE, GPIO_PIN_2, INPUT_BUTTON_UP },
		{ GPIOE, GPIO_PIN_3, INPUT_BUTTON_DOWN },
		{ GPIOE, GPIO_PIN_4, INPUT_BUTTON_LEFT },
		{ GPIOE, GPIO_PIN_5, INPUT_BUTTON_RIGHT },
		{ GPIOE, GPIO_PIN_6, INPUT_BUTTON_A },
		{ GPIOE, GPIO_PIN_7, INPUT_BUTTON_B }
	},
	[INPUT_PLAYER_2] = {
		{ GPIOE, GPIO_PIN_8, INPUT_BUTTON_UP },
		{ GPIOE, GPIO_PIN_9, INPUT_BUTTON_DOWN },
		{ GPIOE, GPIO_PIN_10, INPUT_BUTTON_LEFT },
		{ GPIOE, GPIO_PIN_11, INPUT_BUTTON_RIGHT },
		{ GPIOE, GPIO_PIN_12, INPUT_BUTTON_A },
		{ GPIOE, GPIO_PIN_13, INPUT_BUTTON_B }
	}
};

static const PresenceMapping presence_mappings[INPUT_PLAYER_COUNT] = {
	[INPUT_PLAYER_1] = {
		P1_WIRED_PRESENT_GPIO_Port,
		P1_WIRED_PRESENT_Pin
	},
	[INPUT_PLAYER_2] = {
		P2_WIRED_PRESENT_GPIO_Port,
		P2_WIRED_PRESENT_Pin
	}
};

static WiredController controllers[INPUT_PLAYER_COUNT];

static bool is_pressed(InputButtons state, InputButtons button)
{
	return (state & button) != 0;
}

static InputButtons sample_buttons(InputPlayer player)
{
	InputButtons sample = 0;

	for (size_t i = 0; i < WIRED_BUTTON_COUNT; i++) {
		const ButtonMapping *mapping = &button_mappings[player][i];

		if (HAL_GPIO_ReadPin(mapping->port, mapping->pin) == GPIO_PIN_RESET) {
			sample |= mapping->button;
		}
	}

	return sample;
}

static bool sample_presence(InputPlayer player)
{
	const PresenceMapping *mapping = &presence_mappings[player];

	return HAL_GPIO_ReadPin(mapping->port, mapping->pin) == GPIO_PIN_RESET;
}

static bool debounce_accept(
	bool stable,
	bool sample,
	uint8_t *counter
)
{
	if (sample == stable) {
		*counter = 0;
		return false;
	}

	(*counter)++;

	if (*counter < DEBOUNCE_SAMPLES) {
		return false;
	}

	*counter = 0;
	return true;
}

static void debounce_buttons(WiredController *controller, InputButtons sample)
{
	for (size_t i = 0; i < WIRED_BUTTON_COUNT; i++) {
		InputButtons button = buttons[i];

		bool sampled_pressed = is_pressed(sample, button);
		bool stable_pressed = is_pressed(controller->state.held, button);

		if (!debounce_accept(
			stable_pressed,
			sampled_pressed,
			&controller->button_counters[i]
		))
		{
			continue;
		}

		if (sampled_pressed) {
			controller->state.held |= button;
		} else {
			controller->state.held &= ~button;
		}
	}
}

static void debounce_presence(WiredController *controller, bool sample)
{
	if (debounce_accept(
		controller->state.connected,
		sample,
		&controller->presence_counter
	))
	{
		controller->state.connected = sample;
	}
}

void input_wired_init(void)
{
	if (HAL_TIM_Base_Start_IT(&htim6) != HAL_OK) {
		PANIC("Failed to start TIM6");
	}
}

void input_wired_sample(void)
{
	for (
		InputPlayer player = INPUT_PLAYER_1;
		player < INPUT_PLAYER_COUNT;
		player++
	)
	{
		WiredController *controller = &controllers[player];

		InputButtons buttons_sample = sample_buttons(player);
		bool presence_sample = sample_presence(player);

		debounce_buttons(controller, buttons_sample);
		debounce_presence(controller, presence_sample);
	}
}

InputState input_wired_get(InputPlayer player)
{
	ASSERT(player < INPUT_PLAYER_COUNT);

	uint32_t primask = __get_PRIMASK();
	__disable_irq();

	InputState state = controllers[player].state;

	__set_PRIMASK(primask);

	return state;
}

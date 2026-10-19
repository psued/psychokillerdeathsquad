#include "input_wired.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "error.h"
#include "main.h"
#include "stm32u5xx_hal.h"

extern TIM_HandleTypeDef htim6;

#define WIRED_BUTTON_COUNT 6U
#define DEBOUNCE_SAMPLES   10U

typedef struct {
	GPIO_TypeDef *port;
	uint16_t pin;
} GPIOMapping;

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

static const GPIOMapping button_mappings[INPUT_PLAYER_COUNT][WIRED_BUTTON_COUNT] = {
	[INPUT_PLAYER_1] = {
		{.port = P1_UP_GPIO_Port,    .pin = P1_UP_Pin},
		{.port = P1_DOWN_GPIO_Port,  .pin = P1_DOWN_Pin},
		{.port = P1_LEFT_GPIO_Port,  .pin = P1_LEFT_Pin},
		{.port = P1_RIGHT_GPIO_Port, .pin = P1_RIGHT_Pin},
		{.port = P1_A_GPIO_Port,     .pin = P1_A_Pin},
		{.port = P1_B_GPIO_Port,     .pin = P1_B_Pin}
	},
	[INPUT_PLAYER_2] = {
		{.port = P2_UP_GPIO_Port,    .pin = P2_UP_Pin},
		{.port = P2_DOWN_GPIO_Port,  .pin = P2_DOWN_Pin},
		{.port = P2_LEFT_GPIO_Port,  .pin = P2_LEFT_Pin},
		{.port = P2_RIGHT_GPIO_Port, .pin = P2_RIGHT_Pin},
		{.port = P2_A_GPIO_Port,     .pin = P2_A_Pin},
		{.port = P2_B_GPIO_Port,     .pin = P2_B_Pin}
	}
};

static const GPIOMapping presence_mappings[INPUT_PLAYER_COUNT] = {
	[INPUT_PLAYER_1] = {
		.port = P1_WIRED_PRESENT_GPIO_Port,
		.pin = P1_WIRED_PRESENT_Pin
	},
	[INPUT_PLAYER_2] = {
		.port = P2_WIRED_PRESENT_GPIO_Port,
		.pin = P2_WIRED_PRESENT_Pin
	}
};

static WiredController controllers[INPUT_PLAYER_COUNT];

static InputButtons sample_buttons(const InputPlayer player)
{
	InputButtons sample = 0;

	for (size_t i = 0; i < WIRED_BUTTON_COUNT; i++) {
		const GPIOMapping *mapping = &button_mappings[player][i];

		if (HAL_GPIO_ReadPin(mapping->port, mapping->pin) == GPIO_PIN_RESET) {
			sample |= buttons[i];
		}
	}

	return sample;
}

static bool sample_presence(const InputPlayer player)
{
	const GPIOMapping *mapping = &presence_mappings[player];
	return HAL_GPIO_ReadPin(mapping->port, mapping->pin) == GPIO_PIN_RESET;
}

static bool debounce_accept(
	const bool stable,
	const bool sample,
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

static void debounce_buttons(WiredController *controller, const InputButtons sample)
{
	for (size_t i = 0; i < WIRED_BUTTON_COUNT; i++) {
		const InputButtons button = buttons[i];

		const bool sampled_held = (sample & button) != 0U;
		const bool stable_held = (controller->state.held & button) != 0U;

		if (!debounce_accept(
			stable_held,
			sampled_held,
			&controller->button_counters[i]
		))
		{
			continue;
		}

		if (sampled_held) {
			controller->state.held |= button;
		} else {
			controller->state.held &= ~button;
		}
	}
}

static void debounce_presence(WiredController *controller, const bool sample)
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

		const InputButtons buttons_sample = sample_buttons(player);
		const bool presence_sample = sample_presence(player);

		debounce_buttons(controller, buttons_sample);
		debounce_presence(controller, presence_sample);
	}
}

InputState input_wired_get(const InputPlayer player)
{
	ASSERT(player < INPUT_PLAYER_COUNT);

	const uint32_t primask = __get_PRIMASK();
	__disable_irq();

	const InputState state = controllers[player].state;

	__set_PRIMASK(primask);

	return state;
}

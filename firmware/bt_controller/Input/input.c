#include "input.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "error.h"
#include "main.h"
#include "stm32wbxx_hal.h"

extern LPTIM_HandleTypeDef hlptim2;

#define INPUT_BUTTON_COUNT 6U
#define DEBOUNCE_SAMPLES   10U
#define LPTIM_PERIOD       32U

typedef struct {
	GPIO_TypeDef *port;
	uint16_t pin;
} GPIOMapping;

static const InputButtons buttons[INPUT_BUTTON_COUNT] = {
	INPUT_BUTTON_UP,
	INPUT_BUTTON_DOWN,
	INPUT_BUTTON_LEFT,
	INPUT_BUTTON_RIGHT,
	INPUT_BUTTON_A,
	INPUT_BUTTON_B
};

static const GPIOMapping button_mappings[INPUT_BUTTON_COUNT] = {
	{.port = BTN_UP_GPIO_Port,    .pin = BTN_UP_Pin},
	{.port = BTN_DOWN_GPIO_Port,  .pin = BTN_DOWN_Pin},
	{.port = BTN_LEFT_GPIO_Port,  .pin = BTN_LEFT_Pin},
	{.port = BTN_RIGHT_GPIO_Port, .pin = BTN_RIGHT_Pin},
	{.port = BTN_A_GPIO_Port,     .pin = BTN_A_Pin},
	{.port = BTN_B_GPIO_Port,     .pin = BTN_B_Pin}
};

static volatile InputButtons held;
static uint8_t button_counters[INPUT_BUTTON_COUNT];

static InputButtons sample_buttons()
{
	InputButtons sample = 0;

	for (size_t i = 0; i < INPUT_BUTTON_COUNT; i++) {
		const GPIOMapping *mapping = &button_mappings[i];

		if (HAL_GPIO_ReadPin(mapping->port, mapping->pin) == GPIO_PIN_RESET) {
			sample |= buttons[i];
		}
	}

	return sample;
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

void input_init(void)
{
	if (HAL_LPTIM_Counter_Start_IT(&hlptim2, LPTIM_PERIOD) != HAL_OK) {
		PANIC("Failed to start LPTIM2");
	}
}

void input_sample(void)
{
	const InputButtons sample = sample_buttons();

	for (size_t i = 0; i < INPUT_BUTTON_COUNT; i++) {
		const InputButtons button = buttons[i];

		const bool sampled_held = (sample & button) != 0U;
		const bool stable_held = (held & button) != 0U;

		if (!debounce_accept(
			stable_held,
			sampled_held,
			&button_counters[i]
		))
		{
			continue;
		}

		if (sampled_held) {
			held |= button;
		} else {
			held &= ~button;
		}
	}
}

InputButtons input_get()
{
	return held;
}

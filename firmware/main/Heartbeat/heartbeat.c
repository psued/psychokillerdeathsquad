#include "heartbeat.h"

#include <stdint.h>

#include "main.h"
#include "time.h"

enum {
	HEARTBEAT_INTERVAL_MS = 1000
};

static uint32_t last_toggle;

void heartbeat_init(void)
{
	last_toggle = time_ms();

	HAL_GPIO_WritePin(
		HEARTBEAT_LED_GPIO_Port,
		HEARTBEAT_LED_Pin,
		GPIO_PIN_SET
	);
}

void heartbeat_process(void)
{
	uint32_t now = time_ms();

	if (now - last_toggle < HEARTBEAT_INTERVAL_MS) {
		return;
	}

	last_toggle = now;

	HAL_GPIO_TogglePin(HEARTBEAT_LED_GPIO_Port, HEARTBEAT_LED_Pin);
}

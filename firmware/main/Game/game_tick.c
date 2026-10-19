#include "game_tick.h"

#include <stdint.h>

#include "error.h"
#include "game.h"
#include "input.h"
#include "main.h"

extern TIM_HandleTypeDef htim7;

static volatile uint32_t pending_ticks;

void game_tick_init(void)
{
	if (HAL_TIM_Base_Start_IT(&htim7) != HAL_OK) {
		PANIC("Failed to start TIM7");
	}
}

void game_tick_pending(void)
{
	pending_ticks++;
}

void game_tick_process(void)
{
	const uint32_t primask = __get_PRIMASK();
	__disable_irq();

	uint32_t ticks = pending_ticks;
	pending_ticks = 0U;

	__set_PRIMASK(primask);

	while (ticks > 0U) {
		input_freeze();
		game_tick();
		ticks--;
	}
}

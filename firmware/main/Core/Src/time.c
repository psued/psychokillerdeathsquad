#include "time.h"

#include "error.h"
#include "main.h"

extern TIM_HandleTypeDef htim2;

void time_init(void)
{
	if (HAL_TIM_Base_Start(&htim2) != HAL_OK) {
		PANIC("Failed to start TIM2");
	}
}

uint32_t time_ms(void)
{
	return HAL_GetTick();
}

uint32_t time_us(void)
{
	return __HAL_TIM_GET_COUNTER(&htim2);
}

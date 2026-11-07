#include "watchdog.h"

#include "main.h"
#include "error.h"

extern IWDG_HandleTypeDef hiwdg;

void watchdog_process(void)
{
	if (HAL_IWDG_Refresh(&hiwdg) != HAL_OK) {
		PANIC("Watchdog refresh failed");
	}
}

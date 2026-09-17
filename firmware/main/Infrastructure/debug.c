#include "debug.h"

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "stm32u5xx_hal.h"

#define DEBUG_BUFFER_SIZE 256U
#define DEBUG_TIMEOUT_MS   50U

extern UART_HandleTypeDef huart4;

void debug_printf(const char *format, ...)
{
	char buffer[DEBUG_BUFFER_SIZE];

	va_list args;
	va_start(args, format);
	int length = vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	if (length <= 0) {
		return;
	}

	size_t transmit_length = (size_t)length;

	if (transmit_length >= sizeof(buffer)) {
		transmit_length = sizeof(buffer) - 1U;
	}

	(void)HAL_UART_Transmit(
		&huart4,
		(uint8_t *)buffer,
		(uint16_t)transmit_length,
		DEBUG_TIMEOUT_MS
	);
}

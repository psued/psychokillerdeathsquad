#include "uart.h"

#include <stdint.h>

#include "app_conf.h"
#include "client.h"
#include "error.h"
#include "main.h"
#include "stm32_seq.h"

#define UART_PACKET_SIZE  6U
#define UART_PAYLOAD_SIZE 4U
#define UART_PACKET_SYNC  0xA5U

#define UART_LPTIM_PERIOD   327U
#define UART_TX_TIMEOUT_MS  5U

extern UART_HandleTypeDef huart1;
extern CRC_HandleTypeDef hcrc;
extern LPTIM_HandleTypeDef hlptim1;

static void le16_write(uint8_t *dst, const uint16_t value)
{
	dst[0] = (uint8_t)value;
	dst[1] = (uint8_t)(value >> 8);
}

static HAL_StatusTypeDef uart_send(const InputPlayer player, const InputState state)
{
	uint8_t packet[UART_PACKET_SIZE] = {
		UART_PACKET_SYNC,
		(uint8_t)player,
		state.connected ? 1U : 0U,
		state.connected ? state.held : 0U,
		0U,
		0U
	};

	const uint16_t crc = (uint16_t)HAL_CRC_Calculate(
		&hcrc,
		(uint32_t *)packet,
		UART_PAYLOAD_SIZE
	);

	le16_write(&packet[UART_PAYLOAD_SIZE], crc);

	return HAL_UART_Transmit(
		&huart1,
		packet,
		UART_PACKET_SIZE,
		UART_TX_TIMEOUT_MS
	);
}

static void uart_task(void)
{
	for (InputPlayer player = INPUT_PLAYER_1; player < INPUT_PLAYER_COUNT; player++) {
		const InputState state = client_get(player);

		if (uart_send(player, state) != HAL_OK) {
			return;
		}
	}
}

void HAL_LPTIM_AutoReloadMatchCallback(LPTIM_HandleTypeDef *hlptim)
{
	if (hlptim->Instance == LPTIM1) {
		UTIL_SEQ_SetTask(1U << CFG_TASK_UART_ID, CFG_SCH_PRIO_0);
	}
}

void uart_init(void)
{
	UTIL_SEQ_RegTask(
		1U << CFG_TASK_UART_ID,
		UTIL_SEQ_RFU,
		uart_task
	);

	if (HAL_LPTIM_Counter_Start_IT(&hlptim1, UART_LPTIM_PERIOD) != HAL_OK) {
		PANIC("UART timer start failed");
	}

	UTIL_SEQ_SetTask(1U << CFG_TASK_UART_ID, CFG_SCH_PRIO_0);
}

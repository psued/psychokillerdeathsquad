#include "input_wireless.h"

#include <stdbool.h>
#include <string.h>

#include "error.h"
#include "main.h"
#include "time.h"

#define WIRELESS_PACKET_SIZE  6U
#define WIRELESS_PAYLOAD_SIZE 4U
#define WIRELESS_PACKET_SYNC  0xA5U
#define WIRELESS_BUTTON_MASK  0x3FU
#define WIRELESS_TIMEOUT_MS   500U

extern UART_HandleTypeDef huart3;
extern CRC_HandleTypeDef hcrc;

static uint8_t rx_byte;
static uint8_t rx_window[WIRELESS_PACKET_SIZE];
static uint8_t rx_count;

static uint8_t packets[INPUT_PLAYER_COUNT][WIRELESS_PACKET_SIZE];
static uint32_t packet_times[INPUT_PLAYER_COUNT];

static InputState states[INPUT_PLAYER_COUNT];
static uint32_t state_times[INPUT_PLAYER_COUNT];

static uint16_t le16_read(const uint8_t *data)
{
	return (uint16_t)data[0] | ((uint16_t)data[1] << 8);
}

static bool packet_valid(const uint8_t *packet, const InputPlayer player)
{
	if (packet[0] != WIRELESS_PACKET_SYNC || packet[1] != (uint8_t)player || packet[2] > 1U || (packet[3] & (uint8_t)~WIRELESS_BUTTON_MASK) != 0U || (packet[2] == 0U && packet[3] != 0U)) {
		return false;
	}

	const uint16_t received_crc = le16_read(&packet[WIRELESS_PAYLOAD_SIZE]);
	const uint16_t calculated_crc = (uint16_t)HAL_CRC_Calculate(
		&hcrc,
		(uint32_t *)packet,
		WIRELESS_PAYLOAD_SIZE
	);

	return received_crc == calculated_crc;
}

static void packet_receive(const uint8_t byte)
{
	memmove(rx_window, &rx_window[1], WIRELESS_PACKET_SIZE - 1U);
	rx_window[WIRELESS_PACKET_SIZE - 1U] = byte;

	if (rx_count < WIRELESS_PACKET_SIZE) {
		rx_count++;
	}

	if (rx_count != WIRELESS_PACKET_SIZE || rx_window[0] != WIRELESS_PACKET_SYNC || rx_window[1] >= INPUT_PLAYER_COUNT || rx_window[2] > 1U || (rx_window[3] & (uint8_t)~WIRELESS_BUTTON_MASK) != 0U || (rx_window[2] == 0U && rx_window[3] != 0U)) {
		return;
	}

	const InputPlayer player = (InputPlayer)rx_window[1];

	memcpy(packets[player], rx_window, WIRELESS_PACKET_SIZE);
	packet_times[player] = time_ms();
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART3) {
		packet_receive(rx_byte);
		(void)HAL_UART_Receive_IT(&huart3, &rx_byte, 1U);
	}
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART3) {
		rx_count = 0U;

		if (huart->RxState == HAL_UART_STATE_READY) {
			(void)HAL_UART_Receive_IT(&huart3, &rx_byte, 1U);
		}
	}
}

void input_wireless_init(void)
{
	if (HAL_UART_Receive_IT(&huart3, &rx_byte, 1U) != HAL_OK) {
		PANIC("Wireless UART receive start failed");
	}
}

void input_wireless_process(void)
{
	const uint32_t primask = __get_PRIMASK();
	__disable_irq();

	if (huart3.RxState == HAL_UART_STATE_READY) {
		rx_count = 0U;
		(void)HAL_UART_Receive_IT(&huart3, &rx_byte, 1U);
	}

	__set_PRIMASK(primask);
}

InputState input_wireless_get(const InputPlayer player)
{
	ASSERT(player < INPUT_PLAYER_COUNT);

	uint8_t packet[WIRELESS_PACKET_SIZE];

	const uint32_t primask = __get_PRIMASK();
	__disable_irq();

	memcpy(packet, packets[player], WIRELESS_PACKET_SIZE);
	const uint32_t packet_time = packet_times[player];

	__set_PRIMASK(primask);

	if (packet_valid(packet, player)) {
		states[player].connected = packet[2] != 0U;
		states[player].held = packet[3];
		state_times[player] = packet_time;
	}

	if (time_ms() - state_times[player] >= WIRELESS_TIMEOUT_MS) {
		return (InputState){0};
	}

	return states[player];
}

#include "fpga.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "main.h"

extern CRC_HandleTypeDef hcrc;
extern SPI_HandleTypeDef hspi2;

#define RENDER_WIDTH    320U
#define RENDER_HEIGHT   240U
#define LAYER_WIRE_SIZE 16U
#define CRC_SIZE        2U
#define PAYLOAD_SIZE    (RENDER_LAYER_COUNT * LAYER_WIRE_SIZE)
#define PACKET_SIZE     (PAYLOAD_SIZE + CRC_SIZE)
#define SPI_TIMEOUT_MS  5U

static RenderSnapshot pending_snapshot;
static uint8_t packet[PACKET_SIZE];
static bool snapshot_pending;
static volatile uint32_t frame_count;

static bool fpga_ready(void)
{
	return HAL_GPIO_ReadPin(FPGA_READY_GPIO_Port, FPGA_READY_Pin) == GPIO_PIN_SET;
}

static bool snapshot_valid(const RenderSnapshot *snapshot)
{
	for (size_t i = 0; i < RENDER_LAYER_COUNT; i++) {
		const RenderLayer *layer = &snapshot->layers[i];

		if ((layer->flags & ~(RENDER_FLAG_ENABLED | RENDER_FLAG_FLIP_X)) != 0U) {
			return false;
		}

		if ((layer->flags & RENDER_FLAG_ENABLED) == 0U) {
			continue;
		}

		if (layer->width == 0U || layer->height == 0U) {
			return false;
		}

		if (layer->width > RENDER_WIDTH || layer->height > RENDER_HEIGHT) {
			return false;
		}

		if (layer->dst_x > RENDER_WIDTH - layer->width ||
			layer->dst_y > RENDER_HEIGHT - layer->height) {
			return false;
		}
	}

	return true;
}

static void write_u16_le(uint8_t *dst, uint16_t value)
{
	dst[0] = (uint8_t)value;
	dst[1] = (uint8_t)(value >> 8);
}

static void serialize_snapshot(uint8_t *dst, const RenderSnapshot *snapshot)
{
	for (size_t i = 0; i < RENDER_LAYER_COUNT; i++) {
		const RenderLayer *layer = &snapshot->layers[i];
		uint8_t *out = &dst[i * LAYER_WIRE_SIZE];

		if ((layer->flags & RENDER_FLAG_ENABLED) == 0U) {
			memset(out, 0, LAYER_WIRE_SIZE);
			continue;
		}

		write_u16_le(&out[0], layer->image_id);
		write_u16_le(&out[2], layer->src_x);
		write_u16_le(&out[4], layer->src_y);
		write_u16_le(&out[6], layer->width);
		write_u16_le(&out[8], layer->height);
		write_u16_le(&out[10], layer->dst_x);
		write_u16_le(&out[12], layer->dst_y);
		out[14] = layer->z;
		out[15] = layer->flags;
	}
}

static Error transmit_packet(const uint8_t *data, uint16_t size)
{
	HAL_GPIO_WritePin(FPGA_CS_GPIO_Port, FPGA_CS_Pin, GPIO_PIN_RESET);

	HAL_StatusTypeDef status = HAL_SPI_Transmit(&hspi2, data, size, SPI_TIMEOUT_MS);

	HAL_GPIO_WritePin(FPGA_CS_GPIO_Port, FPGA_CS_Pin, GPIO_PIN_SET);

	switch (status) {
	case HAL_OK:
		return ERROR_NONE;

	case HAL_TIMEOUT:
		return ERROR_TIMEOUT;

	case HAL_BUSY:
		return ERROR_BUSY;

	default:
		return ERROR_IO;
	}
}

void fpga_init(void)
{
	snapshot_pending = false;
	frame_count = 0U;
}

void fpga_frame(void)
{
	frame_count++;
}

void fpga_process(void)
{
	if (!snapshot_pending || !fpga_ready()) {
		return;
	}

	serialize_snapshot(packet, &pending_snapshot);

	uint16_t crc = (uint16_t)HAL_CRC_Calculate(
		&hcrc,
		(const uint32_t *)packet,
		PAYLOAD_SIZE
	);

	write_u16_le(&packet[PAYLOAD_SIZE], crc);

	if (transmit_packet(packet, PACKET_SIZE) == ERROR_NONE) {
		snapshot_pending = false;
	}
}

Error fpga_submit(const RenderSnapshot *snapshot)
{
	if (snapshot == NULL || !snapshot_valid(snapshot)) {
		return ERROR_INVALID_DATA;
	}

	pending_snapshot = *snapshot;
	snapshot_pending = true;

	return ERROR_NONE;
}

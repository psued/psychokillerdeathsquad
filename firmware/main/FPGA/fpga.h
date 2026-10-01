#ifndef FPGA_H
#define FPGA_H

#include <stdint.h>

#define RENDER_LAYER_COUNT 24U

typedef uint8_t RenderFlags;

#define RENDER_FLAG_ENABLED (1U << 0)
#define RENDER_FLAG_FLIP_X  (1U << 1)

typedef struct {
	uint16_t image_id;
	uint16_t src_x;
	uint16_t src_y;
	uint16_t width;
	uint16_t height;
	uint16_t dst_x;
	uint16_t dst_y;
	uint8_t z;
	RenderFlags flags;
} RenderLayer;

typedef struct {
	RenderLayer layers[RENDER_LAYER_COUNT];
} RenderSnapshot;

/**
 * Initialize the FPGA subsystem.
 */
void fpga_init(void);

/**
 * Record an FPGA video frame boundary.
 */
void fpga_frame(void);

/**
 * Process pending FPGA communication.
 */
void fpga_process(void);

/**
 * Submit the latest render snapshot.
 */
void fpga_submit(const RenderSnapshot *snapshot);

#endif

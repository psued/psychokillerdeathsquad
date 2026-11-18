#ifndef RENDER_BUILDER_H
#define RENDER_BUILDER_H

#include <stdint.h>
#include <stdbool.h>

#include "fpga.h"

typedef struct {
    uint16_t x;
    uint16_t y;
} ScreenPos;

typedef struct {
    uint16_t x;
    uint16_t y;
    uint16_t w;
    uint16_t h;
} ImageRect;

typedef struct {
    RenderSnapshot snapshot;
} RenderBuilder;

/* Clears all snapshot data */
void render_begin(RenderBuilder *builder);

/**
 * Add one layer to the snapshot
 * 
 * Cut off parts of the image that would be drawn outside of the screen, 
 * to only send draw data to the fpga that will be visible on screen
 */
void render_add(
    RenderBuilder *builder,
    uint8_t layer_index,
    uint16_t image_id,
    int src_x,
    int src_y,
    int width,
    int height,
    int dst_x,
    int dst_y,
    uint8_t z,
    bool flip_x
);

/**
 * Add a meter image to the snapshot, ex: hp or stun bars
 * 
 * Width of meter is based on amount of value left,
 * converted from value -> fill_width (width in pixels of amount).
 * 
 * Drawn from anchor point either to the left or right based on anchored_on_right
 */
void render_add_meter(
    RenderBuilder *builder, 
    uint8_t layer_index,
    uint16_t image_id,
    int src_x,
    int src_y,
    int full_width,
    int height,
    uint16_t value,
    uint16_t max_value,
    int anchor_x,
    int y,
    bool anchored_on_right,
    uint8_t z
);


#endif

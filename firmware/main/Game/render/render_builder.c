#include <string.h>
#include "render_builder.h"
#include "fpga.h"
#include "screen_constants.h"


void render_begin(RenderBuilder *builder){
    memset(&builder->snapshot, 0, sizeof(builder->snapshot));
}

bool render_add(
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
) {
    /* Don't render part of image outside left edge of screen */
    if (dst_x < 0){
        int cut = -dst_x;
        if (!flip_x){
            src_x += cut; /* Shift x src by how much is outside screen*/
        }
        width -= cut;
        dst_x = 0;
    }

    /* Don't render part of image outside right edge of screen */
    if (dst_x + width > GAME_WIDTH){
        int cut = dst_x + width - GAME_WIDTH;
        if (flip_x){
            src_x += cut;
        }
        width -= cut;
    }

    /* Dont render part of image outside of top of screen */
    if (dst_y < 0){
        int cut = -dst_y;
        src_y += cut;
        height -= cut;
        dst_y = 0;
    }

    /* Dont render part of image below the screen */
    if (dst_y + height > GAME_HEIGHT){
        height -= dst_y + height - GAME_HEIGHT;
    }

    /* Dont add layer if image is entirely off screen*/
    if (width <= 0 || height <= 0){
        return true;
    }

    /* Finally add layer with dimensions inside screen size */
    RenderLayer *layer = &builder->snapshot.layers[layer_index];
    layer->image_id = image_id;
    layer->src_x = (uint16_t)src_x;
    layer->src_y = (uint16_t)src_y;
    layer->width = (uint16_t) width;
    layer->height = (uint16_t) height;
    layer->dst_x = (uint16_t) dst_x;
    layer->dst_y = (uint16_t) dst_y;
    layer->z = z;
    layer->flags = RENDER_FLAG_ENABLED | (flip_x ? RENDER_FLAG_FLIP_X : 0U);

    return true;
}

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
){
    int fill_width = 0;
    if (max_value > 0){
        fill_width = (int)((uint32_t)value * (uint32_t)full_width / max_value);
    }

    if (fill_width > full_width){
        fill_width = full_width;
    }

    /* Always draw at least one pixel of bar if there is */ 
    if (value > 0 && fill_width == 0){
        fill_width = 1;
    }

    if (fill_width == 0){
        return;
    }

    /* Move draw destination to the left if the bar should extend leftward from anchor */
    int dst_x = anchored_on_right ? anchor_x - fill_width : anchor_x;

    render_add(
        builder,
        layer_index,
        image_id,
        src_x,
        src_y, 
        fill_width, 
        height,
        dst_x, 
        y,
        z, 
        anchored_on_right
    );
}

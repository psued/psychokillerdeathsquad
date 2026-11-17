#ifndef STAGE_DEF_H
#define STAGE_DEF_H

#include <stdint.h>
#include "screen_constants.h"

typedef enum {
    STAGE_BRAZIL,
    STAGE_USA,
    STAGE_BRITAIN,
    STAGE_EGYPT,
    STAGE_CHINA,
    STAGE_JAPAN,
    STAGE_DUNGEON,
    STAGE_VOLCANO,

    NUM_STAGES,
} StageID;

/* A whole screen layer */
typedef struct {
    uint16_t image_id;
    int16_t world_x;
    int16_t world_y;
    uint16_t width;
    uint16_t height; 
    float scroll_factor;
    uint8_t layer_index;
} StageLayerDef;

typedef struct {
    StageLayerDef layers[MAX_LAYERS_PER_STAGE];
    uint8_t layer_count;

    uint16_t stage_width;
    uint16_t stage_height;
} StageDef;

extern const StageDef stage_table[NUM_STAGES];

#endif
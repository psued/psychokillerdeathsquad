#include "stage_def.h"
#include "layer_def.h"
#include "texture.h"

const StageDef stage_table[NUM_STAGES] = {
    [STAGE_BRAZIL] = {
        .layers = {
            { TEX_STAGE_BRAZIL, 0, 0, 600, 240, 1.0f, LAYER_BACKGROUND },
        },
        .layer_count = 1,
        .stage_width = 600,
        .stage_height = 240,
    },
    [STAGE_USA] = {
        .layers = {
            { TEX_STAGE_USA_BG, 0, 0, 600, 240, 0.6f, LAYER_BACKGROUND},
            { TEX_STAGE_USA_FLOOR, 0, 164, 600, 240, 1.0f, LAYER_BATTLE_FLOOR},
        },
        .layer_count = 2,
        .stage_width = 600,
        .stage_height = 240,
    },
    [STAGE_BRITAIN] = {
        .layers = {
            {TEX_STAGE_BRITAIN, 0, 0, 600, 240, 1.0f, LAYER_BACKGROUND},
        },
        .layer_count = 1,
        .stage_width = 600,
        .stage_height = 240,
    },
    [STAGE_EGYPT] = {
        .layers = {
            {TEX_STAGE_EGYPT_BG, 0, 0, 600, 240, 0.4f, LAYER_BACKGROUND},
            {TEX_STAGE_EGYPT_FLOOR, 0, 128, 600, 240, 1.0f, LAYER_BATTLE_FLOOR},
        },
        .layer_count = 2,
        .stage_width = 600,
        .stage_height = 240,
    },
    [STAGE_CHINA] = {
        .layers = {
            {TEX_STAGE_CHINA_BG, 0, 0, 600, 240, 0.4f, LAYER_BACKGROUND},
            {TEX_STAGE_CHINA_FLOOR, 0, 160, 600, 240, 1.0f, LAYER_BATTLE_FLOOR},
        },
        .layer_count = 2,
        .stage_width = 600,
        .stage_height = 240,
    },
    [STAGE_JAPAN] = {
        .layers = {
            { TEX_STAGE_JAPAN, 0, 0, 600, 240, 1.0f, LAYER_BACKGROUND},
        },
        .layer_count = 1,
        .stage_width = 600,
        .stage_height = 240,
    },
    [STAGE_DUNGEON] = {
        .layers = {
            {TEX_STAGE_DUNGEON, 0, 0, 600, 240, 1.0f, LAYER_BACKGROUND},
        },
        .layer_count = 1,
        .stage_width = 600,
        .stage_height = 240,
    },
    [STAGE_VOLCANO] = {
        .layers = {
            {TEX_STAGE_VOLCANO, 0, 0, 600, 240, 1.0f, LAYER_BACKGROUND},
        },
        .layer_count = 1,
        .stage_width = 600,
        .stage_height = 240,
    },
};
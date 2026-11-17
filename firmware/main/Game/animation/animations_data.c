#include "screen_animation.h"
#include "texture.h"

#define FLAG_WIDTH 60
#define FLAG_HEIGHT 48

/**
 * Fight banner shown before match
 */
const SpriteFrame fight_banner_frames[1] = {
    {320, 48, 320, 64, 1},
};

const MotionPoint fight_banner_points[4] = {
    {0, 0, 15},
    {-320, 0, 50},
    {-320, 0, 30},
    {-640, 0, 0}
};

const ScreenAnimation fight_banner_animation = {
    TEX_BATTLE_UI,
    0,
    0,
    fight_banner_frames,
    1,
    ANIM_END_HOLD,

    fight_banner_points,
    4,
    ANIM_END_FINISH,
};

/**
 * Title screen title slide in
 */
const SpriteFrame title_frames[1] = {
    {0, 0, 224, 64, 1},
};

const MotionPoint title_points[2] = {
    {0, 0, 600},
    {0, 148, 1},
};

const ScreenAnimation title_animation = {
    TEX_START_SCREEN_UI,
    0,
    0,
    title_frames,
    1,
    ANIM_END_HOLD,

    title_points,
    2,
    ANIM_END_HOLD,
};

/**
 * Burning flame animation
 */
const SpriteFrame flame_frames[3] = {
    {0, 0, 320, 96, 6},
    {320, 0, 320, 96, 6},
    {640, 0, 320, 96, 6},
};

const MotionPoint flame_points[2] = {
    {0, 0, 600},
    {0, -96, 1},
};

const MotionPoint static_flame_points[1] = {
    {0, 0, 1},
};

const ScreenAnimation flame_animation = {
    TEX_FLAMES,
    0,
    0,
    flame_frames,
    3,
    ANIM_END_LOOP,

    flame_points,
    2,
    ANIM_END_HOLD,
};

const ScreenAnimation static_flame_animation = {
    TEX_FLAMES,
    0,
    0,
    flame_frames,
    3,
    ANIM_END_LOOP,

    static_flame_points,
    1,
    ANIM_END_HOLD,
};

const ScreenAnimation static_flame_upsidedown_animation = {
    TEX_FLAMES_UPSIDE_DOWN,
    0,
    0,
    flame_frames,
    3,
    ANIM_END_LOOP,

    static_flame_points,
    1,
    ANIM_END_HOLD,
};

/**
 * Start screen press start prompt text
 */
const SpriteFrame press_start_frames[1] = {
    {0, 64, 224, 32, 1},
};

const MotionPoint press_start_points[5] = {
    {0, 0, 40},
    {0, 0, 1},
    {0, 2, 1},
    {0, 2, 40},
    {0, 2, 1},
};

const ScreenAnimation press_start_animation = {
    TEX_START_SCREEN_UI,
    0,
    0,
    press_start_frames,
    1,
    ANIM_END_HOLD,

    press_start_points,
    5,
    ANIM_END_LOOP,
};

/** 
 * Hover dots in character selection
 */

const SpriteFrame phover_frames[4] = {
    {0, 0, 48, 48, 4},
    {48, 0, 48, 48, 4},
    {96, 0, 48, 48, 4},
    {48, 0, 48, 48, 4},
};

const MotionPoint phover_points[1] = {
    {0, 0, 1},
};

const ScreenAnimation p1hover_animation = {
    TEX_CHAR_SELECT_P1_HOVER_DOT,
    0,
    0,
    phover_frames,
    4,
    ANIM_END_LOOP,

    phover_points,
    1,
    ANIM_END_HOLD,
};

const ScreenAnimation p2hover_animation = {
    TEX_CHAR_SELECT_P2_HOVER_DOT,
    0,
    0,
    phover_frames,
    4,
    ANIM_END_LOOP,

    phover_points,
    1,
    ANIM_END_HOLD,
};


/**
 * Flag animations
 */

const MotionPoint flag_points[1] = {{0, 0, 1}};
const SpriteFrame flag_frames[10] = {
    {0 * FLAG_WIDTH, 0, FLAG_WIDTH, FLAG_HEIGHT, 4},
    {1 * FLAG_WIDTH, 0, FLAG_WIDTH, FLAG_HEIGHT, 4},
    {2 * FLAG_WIDTH, 0, FLAG_WIDTH, FLAG_HEIGHT, 4},
    {3 * FLAG_WIDTH, 0, FLAG_WIDTH, FLAG_HEIGHT, 4},
    {4 * FLAG_WIDTH, 0, FLAG_WIDTH, FLAG_HEIGHT, 4},
    {5 * FLAG_WIDTH, 0, FLAG_WIDTH, FLAG_HEIGHT, 4},
    {6 * FLAG_WIDTH, 0, FLAG_WIDTH, FLAG_HEIGHT, 4},
    {7 * FLAG_WIDTH, 0, FLAG_WIDTH, FLAG_HEIGHT, 4},
    {8 * FLAG_WIDTH, 0, FLAG_WIDTH, FLAG_HEIGHT, 4},
    {9 * FLAG_WIDTH, 0, FLAG_WIDTH, FLAG_HEIGHT, 4},
};

#define FLAG_ROW_ANIMATION_1(row) { \
    TEX_STAGE_FLAGS_1, 0, (row) * FLAG_HEIGHT, \
    flag_frames, 10, ANIM_END_LOOP, \
    flag_points, 1, ANIM_END_HOLD, \
}

#define FLAG_ROW_ANIMATION_2(row) { \
    TEX_STAGE_FLAGS_2, 0, (row) * FLAG_HEIGHT, \
    flag_frames, 10, ANIM_END_LOOP, \
    flag_points, 1, ANIM_END_HOLD, \
}

const ScreenAnimation flag_animations_1[3] = {
    FLAG_ROW_ANIMATION_1(0),
    FLAG_ROW_ANIMATION_1(1),
    FLAG_ROW_ANIMATION_1(2),
};

const ScreenAnimation flag_animations_2[3] = {
    FLAG_ROW_ANIMATION_2(0),
    FLAG_ROW_ANIMATION_2(1),
    FLAG_ROW_ANIMATION_2(2),
};

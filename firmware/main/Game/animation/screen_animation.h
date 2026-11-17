#ifndef SCREEN_ANIMATION_H
#define SCREEN_ANIMATION_H

#include <stdint.h>
#include <stdbool.h>
#include "render_builder.h"

/* Decides what will happen to the animation when it reaches its end */
typedef enum {
    ANIM_END_FINISH,
    ANIM_END_HOLD,
    ANIM_END_LOOP,
} AnimationEndMode;

/* The sprite data for one frame of an animation */
typedef struct {
    uint16_t src_x, src_y, width, height;
    uint16_t duration;
} SpriteFrame;

/* Position of the current frame relative to the animation origin */
typedef struct {
    int16_t offset_x, offset_y;
    uint16_t ticks_to_next;
} MotionPoint;

/* An animation defining its frames and positions to move */
typedef struct {
    uint16_t image_id;
    uint16_t offset_x, offset_y;

    const SpriteFrame *frames;
    uint8_t frame_count;
    AnimationEndMode frames_end;

    const MotionPoint *points;
    uint8_t point_count;
    AnimationEndMode motion_end;
} ScreenAnimation;

/* Identifies an animation in the animation array so that it can be stopped */
typedef struct {
    uint8_t slot;
    uint8_t generation;
} AnimationHandle;

#define ANIMATION_HANDLE_NONE ((AnimationHandle) {0xFFU, 0U})

/* An instance of a currently playing animation */
typedef struct {
    const ScreenAnimation *animation;
    int16_t start_x, start_y;
    uint8_t z;
    uint8_t generation;

    uint8_t frame;
    uint16_t frame_ticks;

    uint8_t point;
    uint16_t point_ticks;

    uint16_t time_elapsed;
    uint8_t layer_index;
} AnimationInstance;

uint16_t animation_elapsed(AnimationHandle handle);

uint8_t animation_point(AnimationHandle handle);

/* Start a specific animation instance */
void animation_instance_start(AnimationInstance *instance, const ScreenAnimation *animation, int16_t start_x, int16_t start_y, uint8_t z, uint8_t layer_index);

/* Starts a new animation if there is a free slot. Returns the handle for the animation instance, or ANIMATION_HANDLE_NONE if array is full */
AnimationHandle animation_play(const ScreenAnimation *animation, int16_t start_x, int16_t start_y, uint8_t z, uint8_t layer_index);

/* Free the animation slot, if the handle matches any active animations */
void animation_stop(AnimationHandle handle);

bool animation_is_playing(AnimationHandle handle);

/* Update a specific animation instance */
bool animation_instance_update(AnimationInstance *instance);

/* Advance every active animation instance, remove the ones that have ended */
void animation_update(void);

/* Moves the point of the animation to a new place */
void animation_set_position(AnimationHandle handle, int16_t new_x, int16_t new_y);

/* Free every slot in animation array */
void animation_clear_all(void);

void animation_instance_render(const AnimationInstance *instance, RenderBuilder *builder);

/* Add every active animation instance to the render snapshot */
void animation_render(RenderBuilder *builder);

#endif
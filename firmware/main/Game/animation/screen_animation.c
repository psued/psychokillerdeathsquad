#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "screen_animation.h"
#include "render_builder.h"

#define MAX_ANIMATIONS 8U

static AnimationInstance animations[MAX_ANIMATIONS];

static AnimationInstance *instance_from_handle(AnimationHandle handle){
    if (handle.slot >= MAX_ANIMATIONS){
        return NULL;
    }

    AnimationInstance *instance = &animations[handle.slot];

    if (instance->animation == NULL || instance->generation != handle.generation){
        return NULL;
    }

    return instance;
}

uint16_t animation_elapsed(AnimationHandle handle){
    const AnimationInstance *instance = instance_from_handle(handle);
    return (instance != NULL) ? instance->time_elapsed : 0U;
}

uint8_t animation_point(AnimationHandle handle){
    const AnimationInstance *instance = instance_from_handle(handle);
    return (instance != NULL) ? instance->point : 0U;
}

void animation_instance_start(AnimationInstance *instance, const ScreenAnimation *animation, int16_t start_x, int16_t start_y, uint8_t z, uint8_t layer_index){
    instance->animation = animation;
    instance->start_x = start_x;
    instance->start_y = start_y;
    instance->z = z;
    instance->layer_index = layer_index;

    instance->frame = 0;
    instance->frame_ticks = 0;
    instance->point = 0;
    instance->point_ticks = 0;

    instance->time_elapsed = 0;
}

AnimationHandle animation_play(const ScreenAnimation *animation, int16_t start_x, int16_t start_y, uint8_t z, uint8_t layer_index) {
    /* Loop through and find available slot */
    for (uint8_t slot = 0; slot < MAX_ANIMATIONS; slot++){
        AnimationInstance *instance = &animations[slot];

        /* Skip filled slots */
        if (instance->animation != NULL){
            continue;
        }

        /* Instantiate new animation instance in slot */
        instance->generation++;
        animation_instance_start(instance, animation, start_x, start_y, z, layer_index);

        /* Return identifier */
        return (AnimationHandle){slot, instance->generation};
    }

    /* No available slots */
    return ANIMATION_HANDLE_NONE;
}

void animation_stop(AnimationHandle handle) {
    AnimationInstance *instance = instance_from_handle(handle);

    if (instance != NULL){
        instance->animation = NULL;
    }
}

bool animation_is_playing(AnimationHandle handle) {
    return instance_from_handle(handle) != NULL;
}


void animation_clear_all(void) {
    for (uint8_t slot = 0; slot < MAX_ANIMATIONS; slot++){
        animations[slot].animation = NULL; 
    }
}

static bool advance_frames(AnimationInstance *instance){
    const ScreenAnimation *animation = instance->animation;
    const SpriteFrame *frame = &animation->frames[instance->frame];

    /* Increment progress of current frame */
    if (instance->frame_ticks < frame->duration) {
        instance->frame_ticks++;
    }

    /* Current frame still playing -> dont end animation */
    if (instance->frame_ticks < frame->duration) {
        return false;
    }

    /* Move to next frame if current is done */
    if (instance->frame + 1 < animation->frame_count) {
        instance->frame++;
        instance->frame_ticks = 0;
        return false;
    }

    /* Last frame is done: apply end rule */
    switch (animation->frames_end){
        /* Keep returning false indefinitely */
        case ANIM_END_HOLD:
            return false;

        /* Reset current point and point duration progression */
        case ANIM_END_LOOP:
            instance->frame = 0;
            instance->frame_ticks = 0;
            return false;

        /* Return true to end animation */
        case ANIM_END_FINISH:
        default:
            return true;
    }
}

static bool advance_motion(AnimationInstance *instance) {
    const ScreenAnimation *animation = instance->animation;

    /* If the motion has reached its last point, apply end rule */
    if (instance->point + 1 >= animation->point_count){
        switch (animation->motion_end) {
            /* Keep returning false indefinitely */
            case ANIM_END_HOLD:
                return false;

            /* Reset current point and point duration progression */
            case ANIM_END_LOOP:
                instance->point = 0;
                instance->point_ticks = 0;
                return false;

            /* Return true to end animation */
            case ANIM_END_FINISH:
            default:
                return true;
        }
    }

    /* Progress towards next point */
    instance->point_ticks++;

    /* Increment current point if ticks to next is reached */
    if (instance->point_ticks >= animation->points[instance->point].ticks_to_next) {
        instance->point++;
        instance->point_ticks = 0;
    }
    return false;
}

bool animation_instance_update(AnimationInstance *instance){
    if (instance->animation == NULL) {
        return true;
    }

    if (instance->time_elapsed < UINT16_MAX) {
        instance->time_elapsed++;
    }

    bool frames_finished = advance_frames(instance);
    bool motion_finished = advance_motion(instance);

    return frames_finished || motion_finished;
}

void animation_update(void) {
    for (uint8_t slot = 0; slot < MAX_ANIMATIONS; slot++){
        AnimationInstance *instance = &animations[slot];

        /* Update animation state and release from poool if finished */
        if (instance->animation != NULL && animation_instance_update(instance)){
            instance->animation = NULL;
        }
    }
}

void animation_set_position(AnimationHandle handle, int16_t new_x, int16_t new_y){
    AnimationInstance *instance = instance_from_handle(handle);
    if (instance != NULL){
        instance->start_x = new_x;
        instance->start_y = new_y;
    }
}

/* Current position on screen. Start position + distance based on remaining ticks and offset */
static void current_position(const AnimationInstance *instance, int *x, int *y){
    const ScreenAnimation *animation = instance->animation;
    const MotionPoint *from = &animation->points[instance->point];

    int offset_x = from->offset_x;
    int offset_y = from->offset_y;

    /* Move part of the way to next point */
    bool has_next = (instance->point + 1 < animation->point_count);

    if (has_next && from->ticks_to_next > 0){
        const MotionPoint *to = &animation->points[instance->point + 1];
        offset_x += ((to->offset_x - from->offset_x) * (int)instance->point_ticks) / from->ticks_to_next;
        offset_y += ((to->offset_y - from->offset_y) * (int)instance->point_ticks) / from->ticks_to_next;
    }

    *x = instance->start_x + offset_x;
    *y = instance->start_y + offset_y;
}

void animation_instance_render(const AnimationInstance *instance, RenderBuilder *builder){
    if (instance->animation == NULL){
        return;
    }

    const ScreenAnimation *animation = instance->animation;
    const SpriteFrame *frame = &animation->frames[instance->frame];

    int x = 0;
    int y = 0;
    current_position(instance, &x, &y);

    render_add(
        builder, 
        instance->layer_index,
        animation->image_id, 
        animation->offset_x + frame->src_x,
        animation->offset_y + frame->src_y,
        frame->width,
        frame->height,
        x,
        y,
        instance->z,
        false
    );
}

void animation_render(RenderBuilder *builder) {
    for (uint8_t slot = 0; slot < MAX_ANIMATIONS; slot++){
        AnimationInstance *instance = &animations[slot];
        animation_instance_render(instance, builder);
    }
}

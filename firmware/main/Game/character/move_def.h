#ifndef MOVE_DEF_H
#define MOVE_DEF_H

#include <stdint.h>
#include "character_animation.h"
#include "input_buffer.h"

typedef enum {
    MOVE_NONE,
    MOVE_LIGHT_PUNCH,
    MOVE_HEAVY_PUNCH,
    NUM_MOVES,
} MoveID;

typedef struct {
    uint8_t damage;
    uint8_t hitstun_duration;
    uint8_t blockstun_duration;
    const AnimationFrame *frames;
    uint8_t frame_count;
    uint16_t image_id;
    uint8_t pushback;
    uint8_t stun;
    uint8_t knockdown_duration; /* 0 = no knockdown */
} MoveDef;

typedef struct {
    const InputDirection *sequence;
    uint8_t sequence_length;
    uint8_t max_ticks_ago; /* How many ticks can be between each input of the sequence before the input is too slow */
    uint8_t required_button; /* Which button finishes the sequence to activate the move */
    MoveID triggers_move;
} SpecialMoveDef;

typedef enum {
    THROWN_POSE_GRABBED,
    THROWN_POSE_LIFTED,
    THROWN_POSE_AIR,
    THROWN_POSE_IMPACT,
    NUM_THROWN_POSES,
} ThrownPose;

typedef struct {
    int8_t offset_x;
    int8_t offset_y;
    uint8_t defender_frame;
} ThrowDefenderPose;

typedef struct {
    AnimationFrame frame;
    ThrowDefenderPose pose;
} ThrowConnectFrame;

typedef struct {
    /* Framedata for before the throw move connects to the defender (Plays out in full if the attacker misses) */
    const AnimationFrame *startup_frames;
    uint8_t startup_frame_count;
    uint16_t startup_image_id;

    /* Framedata when the throw connects */
    const ThrowConnectFrame *connect_frames;
    uint8_t connect_frame_count;
    uint16_t connect_image_id;

    uint8_t damage;
    uint8_t stun;
    uint8_t damage_frame;
    uint8_t knockdown_duration;

} ThrowDef;

#endif
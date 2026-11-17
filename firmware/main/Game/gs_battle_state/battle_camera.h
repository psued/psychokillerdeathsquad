#ifndef BATTLE_CAMERA_H
#define BATTLE_CAMERA_H

#include "stage_def.h"
#include "player.h"

void calc_battle_camera(
    const StageDef *stage_def, 
    const Player *p1, 
    const Player *p2,
    int *camera_x,
    int *camera_y
);


#endif
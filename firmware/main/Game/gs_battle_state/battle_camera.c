#include "battle_camera.h"
#include "stage_def.h"
#include "player.h"

void calc_battle_camera(
    const StageDef *stage_def, 
    const Player *p1, 
    const Player *p2,
    int *camera_x,
    int *camera_y
) {
    int midpoint_x = ((int)p1->pos_x + (int)p2->pos_x) / 2;
    int midpoint_y = ((int)p1->pos_y + (int)p2->pos_y) / 2;

    int new_cam_x = midpoint_x - (GAME_WIDTH / 2);
    int new_cam_y = midpoint_y - (GAME_HEIGHT / 2);

    /* Clamp camera position */
    if (new_cam_x < 0) new_cam_x = 0;
    if (new_cam_x > stage_def->stage_width - GAME_WIDTH) {
        new_cam_x = stage_def->stage_width - GAME_WIDTH;
    }

    if (new_cam_y < 0) new_cam_y = 0;
    if (new_cam_y > stage_def->stage_height - GAME_HEIGHT) {
        new_cam_y = stage_def->stage_height - GAME_HEIGHT;
    }

    *camera_x = new_cam_x;
    *camera_y = new_cam_y;
}
#include <stdint.h>

#include "battle_boxes.h"
#include "battle_position_constraints.h"
#include "player.h"
#include "player_physics.h"
#include "character.h"

/* Dont let players go outside the stage */
void clamp_player_to_stage(Player *player, uint16_t stage_width, uint8_t edge_margin){
    if (player->pos_x < edge_margin){
        player->pos_x = edge_margin;
    } 
    if (player->pos_x > stage_width - edge_margin){
        player->pos_x = stage_width - edge_margin;
    }
}

/* Players cannot walk further away from each other than the screen is wide */
void clamp_player_distance(Player *p1, Player *p2, uint16_t max_distance){
    int distance = (int)p2->pos_x - (int)p1->pos_x;

    if (distance > max_distance){
        p2->pos_x = p1->pos_x + max_distance;
    } else if (distance < -max_distance){
        p2->pos_x = p1->pos_x - max_distance;
    }
}

void resolve_pushbox_collision(Player *p1, Player *p2, const CharacterDef *p1_def, const CharacterDef *p2_def, int16_t ground_y) {
    /* Dont enforce collision when in air */
    if (player_is_airborne(p1, ground_y) || player_is_airborne(p2, ground_y)) {
        return;
    }

    BoxRect p1_pushbox = get_world_pushbox(p1, p1_def->pushbox);
    BoxRect p2_pushbox = get_world_pushbox(p2, p2_def->pushbox);

    if (!boxes_overlap(p1_pushbox, p2_pushbox)){
        return;
    }

    /* Push apart on x axis when players overlap */
    int distance = (int)p2->pos_x - (int)p1->pos_x;
    int min_separation = (p1_def->pushbox.width / 2) + (p2_def->pushbox.width / 2);

    if (distance >= 0){
        int push = (min_separation - distance) / 2;
        p1->pos_x -= push;
        p2->pos_x += push;
    } else {
        int push = (min_separation + distance) / 2;
        p1->pos_x += push;
        p2->pos_x -= push;
    }
}
#pragma once

#include "animation.h"

typedef struct animator_t {
    HeimMat4* final_bone_matrices;
    animation_t* animation;
    float current_time;
    float delta_time;
} animator_t;

animator_t* animator_init(animation_t* animation);
void animator_update(animator_t* animator, float dt);
void animator_play_animation(animator_t* animator, animation_t* animation);
void calculate_bone_transform(animator_t* animator, const struct assimp_node_data_t* node, HeimMat4 parent_transform);

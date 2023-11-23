#pragma once

#include "animation.h"

typedef struct animator_t {
    HeimMat4* final_bone_matrices;
    animation_t* current_animation;
    animation_t* blend_animation;
    float blend_factor;
    float blend_speed;
    animation_t** animations;
    float current_time;
    float blend_time;
    float delta_time;
} animator_t;

animator_t* animator_init(const char* path, model_t* model);
void animator_update(animator_t* animator, float dt);
void animator_switch_animation(animator_t* animator, size_t index);
void animator_play_animation(animator_t* animator, animation_t* animation);
void calculate_bone_transform(animator_t* animator, const struct assimp_node_data_t* node, HeimMat4 parent_transform);

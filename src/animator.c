#include "animator.h"
#include "anim_data.h"
#include "animation.h"
#include "bone.h"
#include "model.h"
#include "utils/heim_mat.h"
#include <stddef.h>

animator_t* animator_init(animation_t* animation) {
    animator_t* animator = malloc(sizeof(animator_t));
    memset(animator, 0, sizeof(animator_t));
    animator->animation = animation;
    animator->current_time = 0.0f;
    arrsetlen(animator->final_bone_matrices, arrlenu(animation->bones));
    for (size_t i = 0; i < arrlenu(animation->bones); i++) {
        animator->final_bone_matrices[i] = heim_mat4_identity();
    }
    return animator;
}

void animator_update(animator_t* animator, float dt) {
    animator->delta_time = dt;
    if (animator->animation) {
        animator->current_time += animator->animation->ticks_per_second * dt;
        animator->current_time = fmod(animator->current_time, animator->animation->duration);
        HeimMat4 identity = heim_mat4_identity();
        calculate_bone_transform(animator, &animator->animation->root_node, identity);
    }
}

void animator_play_animation(animator_t* animator, animation_t* animation) {
    animator->animation = animation;
    animator->current_time = 0.0f;
}

void calculate_bone_transform(animator_t* animator, const struct assimp_node_data_t* node, HeimMat4 parent_transform) {
    char* node_name = node->name;
    HeimMat4 node_transform = node->transformation;
    bone_t* bone = find_bone(animator->animation, node_name);
    if(bone) {
        bone_update(bone, animator->current_time);
        node_transform = bone->local_transform;
    }
    HeimMat4 global_transform = heim_mat4_multiply(parent_transform, node_transform);
    bone_info_map_t* bone_info_map = animator->animation->bone_info_map;
    if(shgeti(bone_info_map, node_name) != -1) {
        bone_info_t bone_info = shget(bone_info_map, node_name);
        size_t bone_index = bone_info.id;
        HeimMat4 offset = bone_info.offset;
        animator->final_bone_matrices[bone_index] = heim_mat4_multiply(global_transform, offset);
    }
    for(int32_t i = 0; i < node->children_count; i++) {
        calculate_bone_transform(animator, &node->children[i], global_transform);
    }
}

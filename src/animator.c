#include "animator.h"
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
    // arrsetlen(animator->final_bone_matrices, 100);
    for (size_t i = 0; i < 100; i++) {
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
    bone_t* bone = find_bone(animator->animation, node->name);
    HeimMat4 local_transform = heim_mat4_identity();
    if (bone) {
        bone_update(bone, animator->current_time);
        //memcpy(&node->transformation, &bone->local_transform, sizeof(HeimMat4));
        local_transform = bone->local_transform;
    }
    HeimMat4 global_transformation = heim_mat4_multiply(parent_transform, local_transform);
    /*
    printf("Parent_transform: %s\n", node->name);
    print_matrix(parent_transform);
    printf("Node_transform: %s\n", node->name);
    print_matrix(node->transformation);
    printf("Global_transform: %s\n", node->name);
    print_matrix(global_transformation);
    printf("Offset: %s\n", node->name);
    getchar();
    */
    ptrdiff_t bone_info_i = shgeti(animator->animation->bone_info_map, node->name);
    if (bone_info_i != -1) {
        bone_info_t bone_info = shget(animator->animation->bone_info_map, node->name);
        int32_t index = bone_info.id;
        animator->final_bone_matrices[index] = heim_mat4_multiply(global_transformation, bone_info.offset);
    }
    for (int32_t i = 0; i < node->children_count; i++) {
        calculate_bone_transform(animator, &node->children[i], global_transformation);
    }
}

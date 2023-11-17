#include "animator.h"
#include "animation.h"
#include "bone.h"
#include "model.h"
#include <cglm/mat4.h>
#include <cglm/types.h>
#include <stddef.h>

animator_t* animator_init(animation_t* animation) {
    animator_t* animator = malloc(sizeof(animator_t));
    memset(animator, 0, sizeof(animator_t));
    animator->animation = animation;
    animator->current_time = 0.0f;
    // arrsetlen(animator->final_bone_matrices, 100);
    for (size_t i = 0; i < 100; i++) {
        glm_mat4_identity(animator->final_bone_matrices[i]);
    }
    return animator;
}

void animator_update(animator_t* animator, float dt) {
    animator->delta_time = dt;
    if (animator->animation) {
        animator->current_time += animator->animation->ticks_per_second * dt;
        animator->current_time = fmod(animator->current_time, animator->animation->duration);
        mat4 identity;
        glm_mat4_identity(identity);
        calculate_bone_transform(animator, &animator->animation->root_node, identity);
    }
}

void animator_play_animation(animator_t* animator, animation_t* animation) {
    animator->animation = animation;
    animator->current_time = 0.0f;
}

void print_matrix(mat4 matrix) {
    for (size_t i = 0; i < 4; i++) {
        printf("%.2f %.2f %.2f %.2f\n", matrix[i][0], matrix[i][1], matrix[i][2], matrix[i][3]);
    }
}

void calculate_bone_transform(animator_t* animator, const struct assimp_node_data_t* node, mat4 parent_transform) {
    bone_t* bone = find_bone(animator->animation, node->name);
    if (bone) {
        bone_update(bone, animator->current_time);
        glm_mat4_copy(bone->local_transform, node->transformation);
    }
    mat4 global_transformation;
    glm_mat4_mul(parent_transform, node->transformation, global_transformation);
    printf("Parent_transform: %s\n", node->name);
    print_matrix(parent_transform);
    printf("Node_transform: %s\n", node->name);
    print_matrix(node->transformation);
    printf("Global_transform: %s\n", node->name);
    print_matrix(global_transformation);
    printf("Offset: %s\n", node->name);
    getchar();
    ptrdiff_t bone_info_i = shgeti(animator->animation->bone_info_map, node->name);
    if (bone_info_i != -1) {
        bone_info_t bone_info = shget(animator->animation->bone_info_map, node->name);
        int32_t index = bone_info.id;
        mat4 res;
        glm_mat4_mul(global_transformation, bone_info.offset, res);
        glm_mat4_copy(res, animator->final_bone_matrices[index]);
    }
    for (size_t i = 0; i < node->children_count; i++) {
        calculate_bone_transform(animator, &node->children[i], global_transformation);
    }
}

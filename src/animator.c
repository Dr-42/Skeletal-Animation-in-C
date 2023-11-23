#include "animator.h"
#include "anim_data.h"
#include "animation.h"
#include "bone.h"
#include "model.h"
#include "utils/heim_mat.h"
#include "utils/heim_vec.h"

#include <stddef.h>
#include <stdio.h>
#include <assert.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

animator_t* animator_init(const char* path, model_t* model) {
    animator_t* animator = malloc(sizeof(animator_t));
    memset(animator, 0, sizeof(animator_t));

    const struct aiScene* scene = aiImportFile(path, aiProcess_Triangulate);
    assert(scene && scene->mRootNode);
    for (uint32_t i = 0; i < scene->mNumAnimations; i++) {
        printf("%u] Animation name: %s\n",i, scene->mAnimations[i]->mName.data);
        animation_t* animation = animation_init(scene, scene->mAnimations[i], model);
        arrput(animator->animations, animation);

    }
    animator->current_animation = animator->animations[0];
    animator->current_time = 0.0f;
    animator->blend_speed = 1.0f;
    arrsetlen(animator->final_bone_matrices, arrlenu(animator->current_animation->bones));
    for (size_t i = 0; i < arrlenu(animator->current_animation->bones); i++) {
        animator->final_bone_matrices[i] = heim_mat4_identity();
    }
    aiReleaseImport(scene);
    return animator;
}

void animator_update(animator_t* animator, float dt) {
    animator->delta_time = dt;
    if (animator->current_animation) {
        animator->current_time += animator->current_animation->ticks_per_second * dt;
        animator->current_time = fmod(animator->current_time, animator->current_animation->duration);
        HeimMat4 identity = heim_mat4_identity();
        if(animator->blend_animation) {
            animator->blend_time += animator->blend_animation->ticks_per_second * dt;
            animator->blend_time = fmod(animator->blend_time, animator->blend_animation->duration);
            animator->blend_factor += dt * animator->blend_speed;
            if(animator->blend_factor >= 1.0f) {
                animator->current_animation = animator->blend_animation;
                animator->blend_animation = NULL;
                animator->blend_factor = 0.0f;
                animator->current_time = animator->blend_time;
                animator->blend_time = 0.0f;
            }
        }
        calculate_bone_transform(animator, &animator->current_animation->root_node, identity);
    }
}

void animator_play_animation(animator_t* animator, animation_t* animation) {
    animator->current_animation = animation;
    animator->current_time = 0.0f;
}

void calculate_bone_transform(animator_t* animator, const struct assimp_node_data_t* node, HeimMat4 parent_transform) {
    char* node_name = node->name;
    HeimMat4 node_transform = node->transformation;
    bone_t* bone = find_bone(animator->current_animation, node_name);
    HeimVec3f position;
    HeimVec4f rotation;
    HeimVec3f scale;
    if(bone) {
        bone_update(bone, animator->current_time);
        position = bone->local_position;
        rotation = bone->local_rotation;
        scale = bone->local_scale;
        if(animator->blend_animation) {
            bone_t* blend_bone = find_bone(animator->blend_animation, node_name);
            if(blend_bone) {
                bone_update(blend_bone, animator->blend_time);
                HeimVec3f blend_position = blend_bone->local_position;
                HeimVec4f blend_rotation = blend_bone->local_rotation;
                HeimVec3f blend_scale = blend_bone->local_scale;
                position = heim_vec3f_mix(position, blend_position, animator->blend_factor);
                rotation = heim_quat_slerp(rotation, blend_rotation, animator->blend_factor);
                scale = heim_vec3f_mix(scale, blend_scale, animator->blend_factor);
                HeimMat4 identity = heim_mat4_identity();
                HeimMat4 translation = heim_mat4_translate(identity, position);
                HeimMat4 rotation_mat = heim_mat4_from_quat(rotation);
                HeimMat4 scale_mat = heim_mat4_scale(identity, scale);
                node_transform = heim_mat4_multiply(translation, heim_mat4_multiply(rotation_mat, scale_mat));
            }
        } else {
            HeimMat4 identity = heim_mat4_identity();
            HeimMat4 translation = heim_mat4_translate(identity, position);
            HeimMat4 rotation_mat = heim_mat4_from_quat(rotation);
            HeimMat4 scale_mat = heim_mat4_scale(identity, scale);
            node_transform = heim_mat4_multiply(translation, heim_mat4_multiply(rotation_mat, scale_mat));
        }
    }
    HeimMat4 global_transform = heim_mat4_multiply(parent_transform, node_transform);
    bone_info_map_t* bone_info_map = animator->current_animation->bone_info_map;
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

void animator_switch_animation(animator_t* animator, size_t index){
    if(index < arrlenu(animator->animations)) {
        /*
        animator->current_animation = animator->animations[index];
        animator->current_time = 0.0f;
        */
        animator->blend_animation = animator->animations[index];
    }
}

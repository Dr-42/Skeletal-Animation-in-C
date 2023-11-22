#include "animation.h"

#include <assimp/anim.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stdio.h>

#include <assert.h>
#include <stdint.h>

#include "bone.h"
#include "model.h"
#include "assimp_glm_helpers.h"

void read_missing_bones(animation_t* animation, const struct aiAnimation* ai_animation, model_t* model);
void read_heirarchy_data(assimp_node_data_t* assimp_node_data, const struct aiNode* src);

animation_t* animation_init(const struct aiScene* scene, struct aiAnimation* animation, model_t* model) {
    animation_t* anim = malloc(sizeof(animation_t));
    memset(anim, 0, sizeof(animation_t));
    anim->duration = animation->mDuration;
    anim->ticks_per_second = animation->mTicksPerSecond;
    anim->name = malloc(strlen(animation->mName.data) + 1);
    strcpy(anim->name, animation->mName.data);
    read_heirarchy_data(&anim->root_node, scene->mRootNode);
    read_missing_bones(anim, animation, model);
    return anim;
}

bone_t* find_bone(animation_t* animation, const char* name) {
    for (size_t i = 0; i < arrlenu(animation->bones); i++) {
        if (strcmp(animation->bones[i].name, name) == 0) {
            return &animation->bones[i];
        }
    }
    return NULL;
}

void read_missing_bones(animation_t* animation, const struct aiAnimation* ai_animation, model_t* model) {
    int32_t size = ai_animation->mNumChannels;
    bone_info_map_t* bone_info_map = model->bone_info_map;
    int32_t bone_count = model->bone_counter;

    for (int i = 0; i < size; i++) {
        struct aiNodeAnim* channel = ai_animation->mChannels[i];
        const char* bone_name = channel->mNodeName.data;

        if (shgeti(bone_info_map, bone_name) == -1) {
            bone_info_t info = {0};
            info.id = bone_count;
            printf("Bone info: %s\n", bone_name);
            shput(bone_info_map, bone_name, info);
            bone_count++;
        }
        bone_t* bone = bone_init(channel->mNodeName.data, shget(bone_info_map, bone_name).id, channel);
        arrput(animation->bones, *bone);
    }
    animation->bone_info_map = bone_info_map;
}

void read_heirarchy_data(assimp_node_data_t* dest, const struct aiNode* src) {
    assert(src);
    dest->name = malloc(strlen(src->mName.data) + 1);
    strcpy(dest->name, src->mName.data);
    dest->transformation = convert_matrix_to_glm(&src->mTransformation);
    dest->children_count = src->mNumChildren;

    for (uint32_t i = 0; i < src->mNumChildren; i++) {
        assimp_node_data_t child = {0};
        read_heirarchy_data(&child, src->mChildren[i]);
        arrput(dest->children, child);
    }
}

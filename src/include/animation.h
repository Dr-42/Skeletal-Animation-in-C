#pragma once

#include <cglm/cglm.h>
#include <stdint.h>

#include "bone.h"
#include "model.h"

typedef struct assimp_node_data_t assimp_node_data_t;

typedef struct assimp_node_data_t {
    mat4 transformation;
    char* name;
    int32_t children_count;
    assimp_node_data_t* children;
} assimp_node_data_t;

typedef struct animation_t {
    float duration;
    int32_t ticks_per_second;
    bone_t* bones;
    assimp_node_data_t root_node;
    bone_info_map_t* bone_info_map;
} animation_t;

animation_t* animation_init(const char* path, model_t* model);
bone_t* find_bone(animation_t* animation, const char* name);

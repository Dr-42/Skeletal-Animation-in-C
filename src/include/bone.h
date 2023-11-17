#pragma once

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <stdint.h>

#include "utils/heim_vec.h"
#include "utils/heim_mat.h"

typedef struct key_position_t {
	HeimVec3f position;
	float time_stamp;
} key_position_t;

typedef struct key_rotation_t {
	HeimVec4f orientation;
	float time_stamp;
} key_rotation_t;

typedef struct key_scale_t {
	HeimVec3f scale;
	float time_stamp;
} key_scale_t;

typedef struct bone_t {
	key_position_t* positions;
	key_rotation_t* rotations;
	key_scale_t* scales;
	int32_t num_positions;
	int32_t num_rotations;
	int32_t num_scales;

	HeimMat4 local_transform;
	const char* name;
	int32_t id;
} bone_t;

bone_t* bone_init(const char* name, int32_t id, const struct aiNodeAnim* channel);
void bone_update(bone_t* bone, float animation_time);
int32_t get_poisiton_index(bone_t* bone, float animation_time);
int32_t get_rotation_index(bone_t* bone, float animation_time);
int32_t get_scale_index(bone_t* bone, float animation_time);


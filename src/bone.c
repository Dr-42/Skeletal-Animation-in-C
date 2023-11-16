#include "bone.h"
#include <cglm/mat4.h>
#include <cglm/quat.h>
#include <cglm/vec3.h>
#include <stb/stb_ds.h>
#include <assert.h>

#include "assimp_glm_helpers.h"

float get_scale_factor(float last_time_stamp, float next_time_stamp, float animation_time);
void interpolate_position(bone_t* bone, float animation_time, mat4* dest);
void interpolate_rotation(bone_t* bone, float animation_time, mat4* dest);
void interpolate_scaling(bone_t* bone, float animation_time, mat4* dest);

bone_t* bone_init(const char* name, int32_t id, const struct aiNodeAnim* channel){
	bone_t* bone = malloc(sizeof(bone_t));
	bone->name = strdup(name);
	bone->id = id;
	glm_mat4_identity(bone->local_transform);

	bone->num_positions = channel->mNumPositionKeys;

	for(int32_t position_idx = 0; position_idx < bone->num_positions; position_idx++) {
		struct aiVector3D ai_position = channel->mPositionKeys[position_idx].mValue;
		float time_stamp = channel->mPositionKeys[position_idx].mTime;
		key_position_t data;
		get_glm_vec(&ai_position, &data.position);
		data.time_stamp = time_stamp;
		arrput(bone->positions, data);
	}
	bone->num_rotations = channel->mNumRotationKeys;
	for(int32_t rotation_idx = 0; rotation_idx < bone->num_rotations; rotation_idx++) {
		struct aiQuaternion ai_rotation = channel->mRotationKeys[rotation_idx].mValue;
		float time_stamp = channel->mRotationKeys[rotation_idx].mTime;
		key_rotation_t data;
		get_glm_quat(&ai_rotation, &data.orientation);
		data.time_stamp = time_stamp;
		arrput(bone->rotations, data);
	}

	bone->num_scales = channel->mNumScalingKeys;
	for(int32_t scale_idx = 0; scale_idx < bone->num_scales; scale_idx++) {
		struct aiVector3D ai_scale = channel->mScalingKeys[scale_idx].mValue;
		float time_stamp = channel->mScalingKeys[scale_idx].mTime;
		key_scale_t data;
		get_glm_vec(&ai_scale, &data.scale);
		data.time_stamp = time_stamp;
		arrput(bone->scales, data);
	}
	return bone;
}

void bone_update(bone_t* bone, float animation_time){
	mat4 translation, rotation, scale;
	interpolate_position(bone, animation_time, &translation);
	interpolate_rotation(bone, animation_time, &rotation);
	interpolate_scaling(bone, animation_time, &scale);
	glm_mat4_mulN((mat4*[]){&translation, &rotation, &scale}, 3, bone->local_transform);
}

int32_t get_poisiton_index(bone_t* bone, float animation_time){
	if(bone->num_positions == 1){
		return 0;
	}
	for(int32_t i = 0; i < bone->num_positions - 1; i++){
		if(animation_time < bone->positions[i + 1].time_stamp){
			return i;
		}
	}
	assert(0);
	return 0;
}

int32_t get_rotation_index(bone_t* bone, float animation_time){
	if(bone->num_rotations == 1){
		return 0;
	}
	for(int32_t i = 0; i < bone->num_rotations - 1; i++){
		if(animation_time < bone->rotations[i + 1].time_stamp){
			return i;
		}
	}
	assert(0);
	return 0;
}

int32_t get_scale_index(bone_t* bone, float animation_time){
	if(bone->num_scales == 1){
		return 0;
	}
	for(int32_t i = 0; i < bone->num_scales - 1; i++){
		if(animation_time < bone->scales[i + 1].time_stamp){
			return i;
		}
	}
	assert(0);
	return 0;
}

float get_scale_factor(float last_time_stamp, float next_time_stamp, float animation_time){
	float mid_way_length = animation_time - last_time_stamp;
	float frames_length = next_time_stamp - last_time_stamp;
	return mid_way_length / frames_length;
}

void interpolate_position(bone_t* bone, float animation_time, mat4* dest){
	if(bone->num_positions == 1){
		glm_mat4_identity(*dest);
		glm_translate(*dest, bone->positions[0].position);
		return;
	}
	int32_t p0_index = get_poisiton_index(bone, animation_time);
	int32_t p1_index = p0_index + 1;
	float scale_factor = get_scale_factor(bone->positions[p0_index].time_stamp, bone->positions[p1_index].time_stamp, animation_time);
	vec3 final_position;
	glm_vec3_mix(bone->positions[p0_index].position, bone->positions[p1_index].position, scale_factor, final_position);
}

void interpolate_rotation(bone_t* bone, float animation_time, mat4* dest){
	if(bone->num_rotations == 1){
		glm_mat4_identity(*dest);
		glm_quat_rotate(*dest, bone->rotations[0].orientation, *dest);
		return;
	}
	int32_t p0_index = get_rotation_index(bone, animation_time);
	int32_t p1_index = p0_index + 1;
	float scale_factor = get_scale_factor(bone->rotations[p0_index].time_stamp, bone->rotations[p1_index].time_stamp, animation_time);
	versor final_orientation;
	glm_quat_slerp(bone->rotations[p0_index].orientation, bone->rotations[p1_index].orientation, scale_factor, final_orientation);
	glm_quat_rotate(*dest, final_orientation, *dest);
}

void interpolate_scaling(bone_t* bone, float animation_time, mat4* dest){
	if(bone->num_scales == 1){
		glm_mat4_identity(*dest);
		glm_scale(*dest, bone->scales[0].scale);
		return;
	}
	int32_t p0_index = get_scale_index(bone, animation_time);
	int32_t p1_index = p0_index + 1;
	float scale_factor = get_scale_factor(bone->scales[p0_index].time_stamp, bone->scales[p1_index].time_stamp, animation_time);
	vec3 final_scale;
	glm_vec3_mix(bone->scales[p0_index].scale, bone->scales[p1_index].scale, scale_factor, final_scale);
	glm_scale(*dest, final_scale);
}



#include "animator.h"
#include "animation.h"
#include "bone.h"
#include "model.h"
#include <cglm/mat4.h>
#include <stddef.h>

animator_t* animator_init(animation_t* animation){
	animator_t* animator = malloc(sizeof(animator_t));
	animator->animation = animation;
	animator->current_time = 0.0f;
	arrsetlen(animator->final_bone_matrices, 100);
	for(size_t i = 0; i < 100; i++){
		glm_mat4_identity(animator->final_bone_matrices[i]);
	}
	return animator;
}

void animator_update(animator_t* animator, float dt){
	animator->delta_time = dt;
	if(animator->animation) {
		animator->current_time += animator->animation->ticks_per_second * dt;
		animator->current_time = fmod(animator->current_time, animator->animation->duration);
		mat4 identity;
		glm_mat4_identity(identity);
		calculate_bone_transform(animator, &animator->animation->root_node, identity);
	}
}

void animator_play_animation(animator_t* animator, animation_t* animation){
	animator->animation = animation;
	animator->current_time = 0.0f;
}

void calculate_bone_transform(animator_t* animator, const struct assimp_node_data_t* node, mat4 parent_transform){
	char* node_name = node->name;
	mat4 node_transform;
	glm_mat4_copy(node->transformation, node_transform);
	bone_t* bone = find_bone(animator->animation, node_name);
	if(bone){
		bone_update(bone, animator->current_time);
		glm_mat4_copy(bone->local_transform, node_transform);
	}
	mat4 global_transform;
	glm_mat4_mul(parent_transform, node_transform, global_transform);
	
	bone_info_map_t* bone_info_map = &animator->animation->bone_info_map;
	if(shgeti(bone_info_map, node_name)){
		int bone_index = shget(bone_info_map, node_name).id;
		glm_mat4_mul(global_transform,
			   shget(bone_info_map, node_name).offset,
			   animator->final_bone_matrices[bone_index]
		);
	}
	for(int32_t i = 0; i < node->children_count; i++){
		calculate_bone_transform(animator, &node->children[i], global_transform);
	}
}

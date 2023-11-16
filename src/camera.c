#include "camera.h"

const float YAW         = -90.0f;
const float PITCH       =  0.0f;
const float SPEED       =  2.5f;
const float SENSITIVITY =  0.1f;
const float ZOOM        =  45.0f;


void camera_update_vectors(camera_t* camera);

camera_t* camera_init(vec3 position){
	camera_t* camera = malloc(sizeof(camera_t));
	camera->position[0] = position[0];
	camera->position[1] = position[1];
	camera->position[2] = position[2];

	camera->world_up[0] = 0.0f;
	camera->world_up[1] = 1.0f;
	camera->world_up[2] = 0.0f;

	camera->yaw = YAW;
	camera->pitch = PITCH;
	camera->movement_speed = SPEED;
	camera->mouse_sensitivity = SENSITIVITY;
	camera->zoom = ZOOM;

	camera->front[0] = 0.0f;
	camera->front[1] = 0.0f;
	camera->front[2] = -1.0f;

	camera_update_vectors(camera);
	return camera;
}

void camera_process_keyboard(camera_t* camera, camera_movement_t direction, float delta_time){
	float velocity = camera->movement_speed * delta_time;
	if(direction == FORWARD){
		glm_vec3_scale(camera->front, velocity, camera->position);
		glm_vec3_add(camera->position, camera->position, camera->position);
	}
	if(direction == BACKWARD){
		glm_vec3_scale(camera->front, velocity, camera->position);
		glm_vec3_sub(camera->position, camera->position, camera->position);
	}
	if(direction == LEFT){
		vec3 temp;
		glm_vec3_cross(camera->front, camera->world_up, temp);
		glm_vec3_normalize(temp);
		glm_vec3_scale(temp, velocity, temp);
		glm_vec3_sub(camera->position, temp, camera->position);
	}
	if(direction == RIGHT){
		vec3 temp;
		glm_vec3_cross(camera->front, camera->world_up, temp);
		glm_vec3_normalize(temp);
		glm_vec3_scale(temp, velocity, temp);
		glm_vec3_add(camera->position, temp, camera->position);
	}
}

void camera_process_mouse_movement(camera_t* camera, float x_offset, float y_offset, bool constrain_pitch){
	x_offset *= camera->mouse_sensitivity;
	y_offset *= camera->mouse_sensitivity;

	camera->yaw += x_offset;
	camera->pitch += y_offset;

	if(constrain_pitch){
		if(camera->pitch > 89.0f){
			camera->pitch = 89.0f;
		}
		if(camera->pitch < -89.0f){
			camera->pitch = -89.0f;
		}
	}

	camera_update_vectors(camera);
}

void camera_process_mouse_scroll(camera_t* camera, float y_offset){
	if(camera->zoom >= 1.0f && camera->zoom <= 45.0f){
		camera->zoom -= y_offset;
	}
	if(camera->zoom <= 1.0f){
		camera->zoom = 1.0f;
	}
	if(camera->zoom >= 45.0f){
		camera->zoom = 45.0f;
	}
}

void camera_update_vectors(camera_t* camera){
	vec3 front;
	front[0] = cos(glm_rad(camera->yaw)) * cos(glm_rad(camera->pitch));
	front[1] = sin(glm_rad(camera->pitch));
	front[2] = sin(glm_rad(camera->yaw)) * cos(glm_rad(camera->pitch));
	glm_vec3_normalize(front);
	glm_vec3_copy(front, camera->front);

	vec3 temp;
	glm_vec3_cross(camera->front, camera->world_up, temp);
	glm_vec3_normalize(temp);
	glm_vec3_copy(temp, camera->right);

	glm_vec3_cross(camera->right, camera->front, temp);
	glm_vec3_normalize(temp);
	glm_vec3_copy(temp, camera->up);
}

void camera_get_view_matrix(camera_t* camera, mat4* dest){
	vec3 center;
	glm_vec3_add(camera->position, camera->front, center);
	glm_lookat(camera->position, center, camera->up, *dest);
}

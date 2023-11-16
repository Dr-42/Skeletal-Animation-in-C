#pragma once

#include <cglm/cglm.h>

typedef enum camera_movement_t {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
} camera_movement_t;

typedef struct camera_t {
	vec3 position;
	vec3 front;
	vec3 up;
	vec3 right;
	vec3 world_up;

	float yaw;
	float pitch;

	float movement_speed;
	float mouse_sensitivity;
	float zoom;
} camera_t;

camera_t* camera_init(vec3 position);
void camera_process_keyboard(camera_t* camera, camera_movement_t direction, float delta_time);
void camera_process_mouse_movement(camera_t* camera, float x_offset, float y_offset, bool constrain_pitch);
void camera_process_mouse_scroll(camera_t* camera, float y_offset);
void camera_get_view_matrix(camera_t* camera, mat4* dest);

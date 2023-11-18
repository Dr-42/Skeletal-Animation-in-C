#pragma once

#include <stdbool.h>

#include "utils/heim_vec.h"
#include "utils/heim_mat.h"

typedef enum camera_movement_t {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
	UP,
	DOWN
} camera_movement_t;

typedef struct camera_t {
    HeimVec3f position;
    HeimVec3f front;
    HeimVec3f up;
    HeimVec3f right;
    HeimVec3f world_up;

    float yaw;
    float pitch;

    float movement_speed;
    float mouse_sensitivity;
    float zoom;
} camera_t;

camera_t* camera_init(HeimVec3f position);
void camera_process_keyboard(camera_t* camera, camera_movement_t direction, float delta_time);
void camera_process_mouse_movement(camera_t* camera, float x_offset, float y_offset, bool constrain_pitch);
void camera_process_mouse_scroll(camera_t* camera, float y_offset);
HeimMat4 camera_get_view_matrix(camera_t* camera);

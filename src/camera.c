#include "camera.h"
#include "utils/heim_mat.h"
#include "utils/heim_vec.h"

#include <string.h>
#include <stdlib.h>

const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

void camera_update_vectors(camera_t* camera);

camera_t* camera_init(HeimVec3f position) {
    camera_t* camera = malloc(sizeof(camera_t));
    memset(camera, 0, sizeof(camera_t));
    camera->position = position;
    camera->world_up = (HeimVec3f){0.0f, 1.0f, 0.0f};
    camera->front = (HeimVec3f){0.0f, 0.0f, -1.0f};

    camera->yaw = YAW;
    camera->pitch = PITCH;
    camera->movement_speed = SPEED;
    camera->mouse_sensitivity = SENSITIVITY;
    camera->zoom = ZOOM;

    camera_update_vectors(camera);
    return camera;
}

void camera_process_keyboard(camera_t* camera, camera_movement_t direction, float delta_time) {
    float velocity = camera->movement_speed * delta_time;
    if (direction == FORWARD) {
        camera->position = heim_vec3f_add(camera->position,
            heim_vec3f_mul(heim_vec3f_normalize(camera->front), velocity));
    }
    if (direction == BACKWARD) {
        camera->position = heim_vec3f_sub(camera->position,
            heim_vec3f_mul(heim_vec3f_normalize(camera->front), velocity));
    }
    if (direction == LEFT) {
        camera->position = heim_vec3f_sub(camera->position,
            heim_vec3f_mul(
            heim_vec3f_normalize(heim_vec3f_cross(camera->front, camera->up)), velocity));
    }
    if (direction == RIGHT) {
        camera->position = heim_vec3f_add(camera->position,
            heim_vec3f_mul(
            heim_vec3f_normalize(heim_vec3f_cross(camera->front, camera->up)), velocity));
    }
    if (direction == UP) {
        camera->position = heim_vec3f_add(camera->position,
            heim_vec3f_mul(heim_vec3f_normalize(camera->up), velocity));
    }
    if (direction == DOWN) {
        camera->position = heim_vec3f_sub(camera->position,
            heim_vec3f_mul(heim_vec3f_normalize(camera->up), velocity));
    }
}

void camera_process_mouse_movement(camera_t* camera, float x_offset, float y_offset, bool constrain_pitch) {
    x_offset *= camera->mouse_sensitivity;
    y_offset *= camera->mouse_sensitivity;

    camera->yaw += x_offset;
    camera->pitch += y_offset;

    if (constrain_pitch) {
        if (camera->pitch > 89.0f) {
            camera->pitch = 89.0f;
        }
        if (camera->pitch < -89.0f) {
            camera->pitch = -89.0f;
        }
    }

    camera_update_vectors(camera);
}

void camera_process_mouse_scroll(camera_t* camera, float y_offset) {
    if (camera->zoom >= 1.0f && camera->zoom <= 45.0f) {
        camera->zoom -= y_offset;
    }
    if (camera->zoom <= 1.0f) {
        camera->zoom = 1.0f;
    }
    if (camera->zoom >= 45.0f) {
        camera->zoom = 45.0f;
    }
}

void camera_update_vectors(camera_t* camera) {
    HeimVec3f front;
    front.x = cos(heim_deg_to_rad(camera->yaw)) * cos(heim_deg_to_rad(camera->pitch));
    front.y = sin(heim_deg_to_rad(camera->pitch));
    front.z = sin(heim_deg_to_rad(camera->yaw)) * cos(heim_deg_to_rad(camera->pitch));
    camera->front = heim_vec3f_normalize(front);
    camera->right = heim_vec3f_normalize(heim_vec3f_cross(camera->front, camera->world_up));
    camera->up = heim_vec3f_normalize(heim_vec3f_cross(camera->right, camera->front));
}

HeimMat4 camera_get_view_matrix(camera_t* camera) {
    HeimVec3f center = heim_vec3f_add(camera->position, camera->front);
    return heim_mat4_lookat(camera->position, center, camera->up);
}

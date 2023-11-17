#pragma once

#include <GL/glew.h>

#include <stdint.h>
#include <stdint.h>
#include <stdbool.h>

#include "utils/heim_vec.h"
#include "utils/heim_mat.h"

typedef struct shader_t {
	uint32_t id;
} shader_t;

shader_t* shader_init(const char* vertex_path, const char* fragment_path);
void shader_use(shader_t* shader);
void shader_set_bool(shader_t* shader, const char* name, bool value);
void shader_set_int(shader_t* shader, const char* name, int32_t value);
void shader_set_float(shader_t* shader, const char* name, float value);
void shader_set_vec2(shader_t* shader, const char* name, HeimVec2f value);
void shader_set_vec2_raw(shader_t* shader, const char* name, float x, float y);
void shader_set_vec3(shader_t* shader, const char* name, HeimVec3f value);
void shader_set_vec3_raw(shader_t* shader, const char* name, float x, float y, float z);
void shader_set_vec4(shader_t* shader, const char* name, HeimVec4f value);
void shader_set_vec4_raw(shader_t* shader, const char* name, float x, float y, float z, float w);
void shader_set_mat3(shader_t* shader, const char* name, HeimMat3 value);
void shader_set_mat4(shader_t* shader, const char* name, HeimMat4 value);

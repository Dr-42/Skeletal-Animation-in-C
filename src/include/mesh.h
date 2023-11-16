#pragma once

#include <GL/glew.h>
#include <cglm/cglm.h>

#include "shader.h"

#include <stb/stb_ds.h>

#define MAX_BONE_INFLUENCE 4

typedef struct vertex_t {
	vec3 position;
	vec3 normal;
	vec2 tex_coords;
	vec3 tangent;
	vec3 bitangent;
	int32_t bone_ids[MAX_BONE_INFLUENCE];
	float bone_weights[MAX_BONE_INFLUENCE];
} vertex_t;

typedef struct texture_t {
	uint32_t id;
	char* type;
	char* path;
} texture_t;

typedef struct mesh_t {
	vertex_t* vertices;
	uint32_t* indices;
	texture_t* textures;
	uint32_t vao;
	uint32_t vbo;
	uint32_t ebo;
} mesh_t;

mesh_t* mesh_init(vertex_t* vertices, uint32_t* indices, texture_t* textures);
void mesh_draw(mesh_t* mesh, shader_t* shader);

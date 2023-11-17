#pragma once

#include <GL/glew.h>

#include "shader.h"


#define MAX_BONE_INFLUENCE 4

typedef struct vertex_t {
	HeimVec3f position;
	HeimVec3f normal;
	HeimVec2f tex_coords;
	HeimVec3f tangent;
	HeimVec3f bitangent;
	int32_t bone_ids[MAX_BONE_INFLUENCE];
	float bone_weights[MAX_BONE_INFLUENCE];
} vertex_t;

typedef struct texture_t {
	uint32_t id;
	const char* type;
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

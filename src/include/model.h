#pragma once

#include <stb/stb_ds.h>

#include "anim_data.h"
#include "mesh.h"

typedef struct bone_info_map_t {
	char* key;
	bone_info_t value;
} bone_info_map_t;

typedef struct model_t {
	texture_t* textures_loaded;
	mesh_t* meshes;
	const char* directory;
	bool gamma_correction;

	bone_info_map_t* bone_info_map;
	int32_t bone_counter;
} model_t;

model_t* model_init(const char* path, bool gamma);
void model_draw(model_t* model, shader_t* shader);

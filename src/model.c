#include "model.h"

#include <assimp/cimport.h>
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stb/stb_image.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "mesh.h"
#include "assimp_glm_helpers.h"

typedef struct aiScene aiScene;
typedef struct aiNode aiNode;
typedef struct aiMesh aiMesh;
typedef struct aiMaterial aiMaterial;
typedef enum aiTextureType aiTextureType;

char* dirname(const char* path);

void load_model(model_t* model, const char* path);
void process_node(model_t* model, aiNode* node, const aiScene* scene);
void set_vertex_bone_data_to_default(vertex_t* vertex);
mesh_t* process_mesh(model_t* model, aiMesh* mesh, const aiScene* scene);
void set_vertex_bone_data(vertex_t* vertex, int bone_id, float weight);
void extract_bone_weight_for_vertices(model_t* model, vertex_t* vertices, aiMesh* mesh);

uint32_t texture_from_file(const char* path, const char* directory, bool gamma);
texture_t* load_material_textures(model_t* model, aiMaterial* mat, enum aiTextureType type, const char* type_name);

model_t* model_init(const char* path, bool gamma){
	model_t* model = malloc(sizeof(model_t));
	model->gamma_correction = gamma;
	load_model(model, path);
	return model;
}

void model_draw(model_t* model, shader_t* shader){
	for(size_t i = 0; i < arrlenu(model->meshes); i++){
		mesh_draw(&model->meshes[i], shader);
	}
}

char* dirname(const char* path){
	char* dir = strdup(path);
	char* last_slash = strrchr(dir, '/');
	if(last_slash){
		*last_slash = '\0';
	}
	return dir;
}

void load_model(model_t* model, const char* path){
	const aiScene* scene = aiImportFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);
	if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode){
		fprintf(stderr, "ERROR::ASSIMP::%s\n", aiGetErrorString());
		return;
	}
	model->directory = dirname(path);
	process_node(model, scene->mRootNode, scene);
	aiReleaseImport(scene);
}

void process_node(model_t* model, aiNode* node, const aiScene* scene){
	for(size_t i = 0; i < node->mNumMeshes; i++){
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		mesh_t* m = process_mesh(model, mesh, scene);
		arrput(model->meshes, *m);
	}
	for(size_t i = 0; i < node->mNumChildren; i++){
		process_node(model, node->mChildren[i], scene);
	}
}

void set_vertex_bone_data_to_default(vertex_t* vertex){
	for(size_t i = 0; i < MAX_BONE_INFLUENCE; i++){
		vertex->bone_ids[i] = -1;
		vertex->bone_weights[i] = 0.0f;
	}
}

mesh_t* process_mesh(model_t* model, aiMesh* mesh, const aiScene* scene){
	vertex_t* vertices = NULL;
	uint32_t* indices = NULL;
	texture_t* textures = NULL;

	for(uint32_t i = 0; i < mesh->mNumVertices; i++){
		vertex_t vertex;
		set_vertex_bone_data_to_default(&vertex);
		get_glm_vec(&mesh->mVertices[i], &vertex.position);
		get_glm_vec(&mesh->mNormals[i], &vertex.normal);
		if(mesh->mTextureCoords[0]) {
			vertex.tex_coords[0] = mesh->mTextureCoords[0][i].x;
			vertex.tex_coords[1] = mesh->mTextureCoords[0][i].y;
		} else {
			vertex.tex_coords[0] = 0.0f;
			vertex.tex_coords[1] = 0.0f;
		}
		arrput(vertices, vertex);
	}
	for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
		struct aiFace face = mesh->mFaces[i];
		for(uint32_t j = 0; j < face.mNumIndices; j++){
			arrput(indices, face.mIndices[j]);
		}
	}

	struct aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
	texture_t* diffuse_maps = load_material_textures(model, material, aiTextureType_DIFFUSE, "texture_diffuse");
	for(size_t i = 0; i < arrlenu(diffuse_maps); i++){
		arrput(textures, diffuse_maps[i]);
	}
	texture_t* specular_maps = load_material_textures(model, material, aiTextureType_SPECULAR, "texture_specular");
	for(size_t i = 0; i < arrlenu(specular_maps); i++){
		arrput(textures, specular_maps[i]);
	}
	texture_t* normal_maps = load_material_textures(model, material, aiTextureType_HEIGHT, "texture_normal");
	for(size_t i = 0; i < arrlenu(normal_maps); i++){
		arrput(textures, normal_maps[i]);
	}
	texture_t* height_maps = load_material_textures(model, material, aiTextureType_AMBIENT, "texture_height");
	for(size_t i = 0; i < arrlenu(height_maps); i++){
		arrput(textures, height_maps[i]);
	}
	
	extract_bone_weight_for_vertices(model, vertices, mesh);
	mesh_t* ret_mesh = mesh_init(vertices, indices, textures);
	return ret_mesh;
}

void set_vertex_bone_data(vertex_t* vertex, int bone_id, float weight){
	for(size_t i = 0; i < MAX_BONE_INFLUENCE; i++){
		if(vertex->bone_weights[i] < 0.0f){
			vertex->bone_ids[i] = bone_id;
			vertex->bone_weights[i] = weight;
			return;
		}
	}
}

void extract_bone_weight_for_vertices(model_t* model, vertex_t* vertices, aiMesh* mesh){
	bone_info_map_t* bone_info_map = model->bone_info_map;
	int32_t bone_count = model->bone_counter;

	for(uint32_t bone_idx = 0; bone_idx < mesh->mNumBones; bone_idx++) {
		int32_t bone_id = -1;
		char* bone_name = mesh->mBones[bone_idx]->mName.data;
		if(shgeti(bone_info_map, bone_name)){
			bone_info_t info = shget(bone_info_map, bone_name);
			info.id = bone_count;
			convert_matrix_to_glm(&mesh->mBones[bone_idx]->mOffsetMatrix, &info.offset);
			shput(bone_info_map, bone_name, info);
			bone_id = bone_count;
			bone_count++;
		} else {
			bone_id = shget(bone_info_map, bone_name).id;
		}
		assert(bone_id != -1);
		struct aiVertexWeight* weights = mesh->mBones[bone_idx]->mWeights;
		uint32_t num_weights = mesh->mBones[bone_idx]->mNumWeights;

		for(uint32_t weight_idx = 0; weight_idx < num_weights; weight_idx++){
			uint32_t vertex_id = weights[weight_idx].mVertexId;
			float weight = weights[weight_idx].mWeight;
			assert(vertex_id < arrlenu(vertices));
			set_vertex_bone_data(&vertices[vertex_id], bone_id, weight);
		}
	}
}


uint32_t texture_from_file(const char* path, const char* directory, bool gamma){
	(void)gamma;
	char* filename = malloc(strlen(path) + strlen(directory) + 1);
	strcpy(filename, directory);
	strcat(filename, path);
	filename[strlen(path) + strlen(directory)] = '\0';

	uint32_t texture_id;
	glGenTextures(1, &texture_id);
	int32_t width, height, nr_components;
	uint8_t* data = stbi_load(filename, &width, &height, &nr_components, 0);
	if(data) {
		GLenum format;
		if(nr_components == 1){
			format = GL_RED;
		} else if(nr_components == 3){
			format = GL_RGB;
		} else if(nr_components == 4){
			format = GL_RGBA;
		} else {
			fprintf(stderr, "ERROR::TEXTURE::UNKNOWN_FORMAT\n");
			exit(EXIT_FAILURE);
		}

		glBindTexture(GL_TEXTURE_2D, texture_id);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);


		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	} else {
		fprintf(stderr, "ERROR::TEXTURE::FAILED_TO_LOAD_AT_PATH: %s\n", path);
		stbi_image_free(data);
		exit(EXIT_FAILURE);
	}
	return texture_id;
}

texture_t* load_material_textures(model_t* model, aiMaterial* mat, aiTextureType type, const char* type_name){
	texture_t* textures = NULL;
	uint32_t texture_count = aiGetMaterialTextureCount(mat, type);
	for(uint32_t i = 0; i < texture_count; i++){
		struct aiString str;
		aiGetMaterialTexture(mat, type, i, &str, NULL, NULL, NULL, NULL, NULL, NULL);
		bool skip = false;
		for(uint32_t j = 0; j < arrlenu(model->textures_loaded); j++){
			if(strcmp(model->textures_loaded[j].path, str.data) == 0){
				arrput(textures, model->textures_loaded[j]);
				skip = true;
				break;
			}
		}
		if(!skip){
			texture_t texture;
			texture.id = texture_from_file(str.data, model->directory, model->gamma_correction);
			texture.type = type_name;
			texture.path = str.data;
			arrput(textures, texture);
			arrput(model->textures_loaded, texture);
		}
	}
	return textures;
}


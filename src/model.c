#include "model.h"

#include <assimp/cimport.h>
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stb/stb_image.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "anim_data.h"
#include "mesh.h"
#include "assimp_glm_helpers.h"

void load_model(model_t* model, const char* path);
void process_node(model_t* model, struct aiNode* node, const struct aiScene* scene);
void set_vertex_bone_data_to_default(vertex_t* vertex);
mesh_t* process_mesh(model_t* model, struct aiMesh* mesh);
void set_vertex_bone_data(vertex_t* vertex, int bone_id, float weight);
void extract_bone_weight_for_vertices(model_t* model, vertex_t* vertices, struct aiMesh* mesh);

texture_t* texture_from_file(const char* path, bool gamma, const char* type);

model_t* model_init(const char* path, bool gamma) {
    model_t* model = malloc(sizeof(model_t));
    model = memset(model, 0, sizeof(model_t));
    model->gamma_correction = gamma;
    model->bone_info_map = NULL;
    load_model(model, path);
    return model;
}

void model_draw(model_t* model, shader_t* shader) {
    for (size_t i = 0; i < arrlenu(model->meshes); i++) {
        mesh_draw(&model->meshes[i], shader);
    }
}

void model_set_albedo(model_t* model, const char* albedo_path){
    for(size_t i = 0; i < arrlenu(model->meshes); i++) {
        mesh_set_albedo(&model->meshes[i], texture_from_file(albedo_path, model->gamma_correction, "texture_diffuse"));
    }
}

void model_set_normal(model_t* model, const char* normal_path){
    for(size_t i = 0; i < arrlenu(model->meshes); i++) {
        mesh_set_normal(&model->meshes[i], texture_from_file(normal_path, model->gamma_correction, "texture_normal"));
    }
}

void model_set_metallic(model_t* model, const char* metallic_path){
    for(size_t i = 0; i < arrlenu(model->meshes); i++) {
        mesh_set_metallic(&model->meshes[i], texture_from_file(metallic_path, model->gamma_correction, "texture_metallic"));
    }
}

void load_model(model_t* model, const char* path) {
    const struct aiScene* scene = aiImportFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        fprintf(stderr, "ERROR::ASSIMP::%s\n", aiGetErrorString());
        return;
    }
    process_node(model, scene->mRootNode, scene);
    aiReleaseImport(scene);
}

void process_node(model_t* model, struct aiNode* node, const struct aiScene* scene) {
    for (size_t i = 0; i < node->mNumMeshes; i++) {
        struct aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        mesh_t* m = process_mesh(model, mesh);
        arrput(model->meshes, *m);
    }
    for (size_t i = 0; i < node->mNumChildren; i++) {
        process_node(model, node->mChildren[i], scene);
    }
}

void set_vertex_bone_data_to_default(vertex_t* vertex) {
    for (size_t i = 0; i < MAX_BONE_INFLUENCE; i++) {
        vertex->bone_ids[i] = -1;
        vertex->bone_weights[i] = 0.0f;
    }
}

mesh_t* process_mesh(model_t* model, struct aiMesh* mesh) {
    vertex_t* vertices = NULL;
    uint32_t* indices = NULL;
    texture_t* textures = NULL;

    for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
        vertex_t vertex;
        set_vertex_bone_data_to_default(&vertex);
        vertex.position = get_glm_vec(&mesh->mVertices[i]);
        vertex.normal = get_glm_vec(&mesh->mNormals[i]);
        if (mesh->mTextureCoords[0]) {
            vertex.tex_coords.x = mesh->mTextureCoords[0][i].x;
            vertex.tex_coords.y = mesh->mTextureCoords[0][i].y;
        } else {
            vertex.tex_coords.x = 0.0f;
            vertex.tex_coords.y = 0.0f;
        }
        arrput(vertices, vertex);
    }
    for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
        struct aiFace face = mesh->mFaces[i];
        for (uint32_t j = 0; j < face.mNumIndices; j++) {
            arrput(indices, face.mIndices[j]);
        }
    }

    extract_bone_weight_for_vertices(model, vertices, mesh);
    mesh_t* ret_mesh = mesh_init(vertices, indices, textures);
    return ret_mesh;
}

void set_vertex_bone_data(vertex_t* vertex, int bone_id, float weight) {
    for (size_t i = 0; i < MAX_BONE_INFLUENCE; i++) {
        if (vertex->bone_ids[i] < 0.0f) {
            vertex->bone_ids[i] = bone_id;
            vertex->bone_weights[i] = weight;
            return;
        }
    }
}

void extract_bone_weight_for_vertices(model_t* model, vertex_t* vertices, struct aiMesh* mesh) {
    int32_t bone_count = model->bone_counter;

    for (uint32_t bone_idx = 0; bone_idx < mesh->mNumBones; bone_idx++) {
        int32_t bone_id = -1;
        char* bone_name = strdup(mesh->mBones[bone_idx]->mName.data);
        if (shgeti(model->bone_info_map, bone_name) == -1) {
            bone_info_t info = {0};
            info.id = bone_count;
            info.offset = convert_matrix_to_glm(&mesh->mBones[bone_idx]->mOffsetMatrix);
            shput(model->bone_info_map, bone_name, info);
            bone_id = bone_count;
            bone_count++;
        } else {
            bone_id = shget(model->bone_info_map, bone_name).id;
        }
        assert(bone_id != -1);
        struct aiVertexWeight* weights = mesh->mBones[bone_idx]->mWeights;
        uint32_t num_weights = mesh->mBones[bone_idx]->mNumWeights;

        for (uint32_t weight_idx = 0; weight_idx < num_weights; weight_idx++) {
            uint32_t vertex_id = weights[weight_idx].mVertexId;
            float weight = weights[weight_idx].mWeight;
            assert(vertex_id < arrlenu(vertices));
            set_vertex_bone_data(&vertices[vertex_id], bone_id, weight);
        }
    }
}

texture_t* texture_from_file(const char* path, bool gamma, const char* type) {
    (void)gamma;

    uint32_t texture_id;
    glGenTextures(1, &texture_id);
    int32_t width, height, nr_components;
    uint8_t* data = stbi_load(path, &width, &height, &nr_components, 0);
    if (data) {
        GLenum format;
        if (nr_components == 1) {
            format = GL_RED;
        } else if (nr_components == 3) {
            format = GL_RGB;
        } else if (nr_components == 4) {
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
    texture_t* texture = malloc(sizeof(texture_t));
    texture->id = texture_id;
    texture->type = type;

    return texture;
}

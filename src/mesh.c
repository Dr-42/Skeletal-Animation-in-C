#include "mesh.h"

#include <stdint.h>
#include <stdio.h>
#include <stb/stb_ds.h>

void setup_mesh(mesh_t* mesh);

mesh_t* mesh_init(vertex_t* vertices, uint32_t* indices, texture_t* textures) {
    mesh_t* mesh = malloc(sizeof(mesh_t));
    memset(mesh, 0, sizeof(mesh_t));
    mesh->vertices = vertices;
    mesh->indices = indices;
    mesh->textures = textures;

    setup_mesh(mesh);
    return mesh;
}

void mesh_draw(mesh_t* mesh, shader_t* shader) {
    uint32_t diffuse_nr = 1;
    uint32_t specular_nr = 1;
    uint32_t normal_nr = 1;

    for (uint32_t i = 0; i < arrlenu(mesh->textures); i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        char number[2];
        const char* name = mesh->textures[i].type;
        if (strcmp(name, "texture_diffuse") == 0) {
            sprintf(number, "%d", diffuse_nr++);
        } else if (strcmp(name, "texture_specular") == 0) {
            sprintf(number, "%d", specular_nr++);
        } else if (strcmp(name, "texture_normal") == 0) {
            sprintf(number, "%d", normal_nr++);
        }
        char* final_name = malloc(strlen(name) + strlen(number) + 1);
        strcpy(final_name, name);
        strcat(final_name, number);
        glUniform1i(glGetUniformLocation(shader->id, final_name), i);
        glBindTexture(GL_TEXTURE_2D, mesh->textures[i].id);
        free(final_name);
    }
    glBindVertexArray(mesh->vao);
    glDrawElements(GL_TRIANGLES, arrlenu(mesh->indices), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glActiveTexture(GL_TEXTURE0);
}

void setup_mesh(mesh_t* mesh) {
    glGenVertexArrays(1, &mesh->vao);
    glGenBuffers(1, &mesh->vbo);
    glGenBuffers(1, &mesh->ebo);

    glBindVertexArray(mesh->vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, arrlenu(mesh->vertices) * sizeof(vertex_t), mesh->vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, arrlenu(mesh->indices) * sizeof(uint32_t), mesh->indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)offsetof(vertex_t, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)offsetof(vertex_t, tex_coords));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)offsetof(vertex_t, tangent));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)offsetof(vertex_t, bitangent));

    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, 4, GL_INT, sizeof(vertex_t), (void*)offsetof(vertex_t, bone_ids));
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)offsetof(vertex_t, bone_weights));

    glBindVertexArray(0);
}

void mesh_set_albedo(mesh_t* mesh, texture_t* albedo){
    arrput(mesh->textures, *albedo);
}

void mesh_set_normal(mesh_t* mesh, texture_t* normal){
    arrput(mesh->textures, *normal);
}

void mesh_set_metallic(mesh_t* mesh, texture_t* metallic){
    arrput(mesh->textures, *metallic);
}

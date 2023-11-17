#include "shader.h"
#include <stdio.h>
#include <stdlib.h>

shader_t* shader_init(const char* vertex_path, const char* fragment_path){
	char* vertex_code = NULL;
	char* fragment_code = NULL;

	FILE* vertex_file = fopen(vertex_path, "r");
	if(vertex_file == NULL){
		fprintf(stderr, "Failed to open vertex shader file: %s\n", vertex_path);
		return NULL;
	}
	fseek(vertex_file, 0, SEEK_END);
	size_t vertex_file_size = ftell(vertex_file);
	rewind(vertex_file);
	vertex_code = malloc(vertex_file_size + 1);
	fread(vertex_code, 1, vertex_file_size, vertex_file);
	fclose(vertex_file);
	vertex_code[vertex_file_size] = '\0';

	FILE* fragment_file = fopen(fragment_path, "r");
	if(fragment_file == NULL){
		fprintf(stderr, "Failed to open fragment shader file: %s\n", fragment_path);
		exit(EXIT_FAILURE);
	}
	fseek(fragment_file, 0, SEEK_END);
	size_t fragment_file_size = ftell(fragment_file);
	rewind(fragment_file);
	fragment_code = malloc(fragment_file_size + 1);
	fread(fragment_code, 1, fragment_file_size, fragment_file);
	fclose(fragment_file);
	fragment_code[fragment_file_size] = '\0';

	uint32_t vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_code, NULL);
	glCompileShader(vertex_shader);
	int32_t success;
	char info_log[512];
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
	if(!success){
		glGetShaderInfoLog(vertex_shader, 512, NULL, info_log);
		fprintf(stderr, "Failed to compile vertex shader: %s\n", info_log);
		exit(EXIT_FAILURE);
	}

	uint32_t fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_code, NULL);
	glCompileShader(fragment_shader);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
	if(!success){
		glGetShaderInfoLog(fragment_shader, 512, NULL, info_log);
		fprintf(stderr, "Failed to compile fragment shader: %s\n", info_log);
		exit(EXIT_FAILURE);
	}

	uint32_t shader_program = glCreateProgram();
	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, fragment_shader);
	glLinkProgram(shader_program);

	glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
	if(!success){
		glGetProgramInfoLog(shader_program, 512, NULL, info_log);
		fprintf(stderr, "Failed to link shader program: %s\n", info_log);
		exit(EXIT_FAILURE);
	}

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	shader_t* shader = malloc(sizeof(shader_t));
	shader->id = shader_program;
	return shader;

}

void shader_use(shader_t* shader){
	glUseProgram(shader->id);
}

void shader_set_bool(shader_t* shader, const char* name, bool value){
	glUniform1i(glGetUniformLocation(shader->id, name), (int32_t)value);
}

void shader_set_int(shader_t* shader, const char* name, int32_t value){
	glUniform1i(glGetUniformLocation(shader->id, name), value);
}

void shader_set_float(shader_t* shader, const char* name, float value){
	glUniform1f(glGetUniformLocation(shader->id, name), value);
}

void shader_set_vec2(shader_t* shader, const char* name, HeimVec2f value){
	glUniform2f(glGetUniformLocation(shader->id, name), value.x, value.y);
}

void shader_set_vec2_raw(shader_t* shader, const char* name, float x, float y){
	glUniform2f(glGetUniformLocation(shader->id, name), x, y);
}

void shader_set_vec3(shader_t* shader, const char* name, HeimVec3f value){
	glUniform3f(glGetUniformLocation(shader->id, name), value.x, value.y, value.z);
}

void shader_set_vec3_raw(shader_t* shader, const char* name, float x, float y, float z){
	glUniform3f(glGetUniformLocation(shader->id, name), x, y, z);
}

void shader_set_vec4(shader_t* shader, const char* name, HeimVec4f value){
	glUniform4f(glGetUniformLocation(shader->id, name), value.x, value.y, value.z, value.w);
}

void shader_set_vec4_raw(shader_t* shader, const char* name, float x, float y, float z, float w){
	glUniform4f(glGetUniformLocation(shader->id, name), x, y, z, w);
}

void shader_set_mat3(shader_t* shader, const char* name, HeimMat3 value){
    float m[9] = {
        value.m[0][0],
        value.m[0][1],
        value.m[0][2],
        value.m[1][0],
        value.m[1][1],
        value.m[1][2],
        value.m[2][0],
        value.m[2][1],
        value.m[2][2],
    };

    glUniformMatrix3fv(glGetUniformLocation(shader->id, name), 1, GL_FALSE, m);
}

void shader_set_mat4(shader_t* shader, const char* name, HeimMat4 value){
    float m[16] = {
        value.m[0][0],
        value.m[0][1],
        value.m[0][2],
        value.m[0][3],
        value.m[1][0],
        value.m[1][1],
        value.m[1][2],
        value.m[1][3],
        value.m[2][0],
        value.m[2][1],
        value.m[2][2],
        value.m[2][3],
        value.m[3][0],
        value.m[3][1],
        value.m[3][2],
        value.m[3][3],
    };

    glUniformMatrix4fv(glGetUniformLocation(shader->id, name), 1, GL_FALSE, m);
}

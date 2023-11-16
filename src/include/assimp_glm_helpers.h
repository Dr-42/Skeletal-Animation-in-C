#pragma once

#include <cglm/cglm.h>
#include <assimp/cimport.h>
#include <assimp/quaternion.h>
#include <assimp/vector3.h>
#include <assimp/matrix4x4.h>

void convert_matrix_to_glm(struct aiMatrix4x4* matrix, mat4 dest);
void get_glm_vec(struct aiVector3D* vec, vec3 dest);
void get_glm_quat(struct aiQuaternion* quat, versor dest);

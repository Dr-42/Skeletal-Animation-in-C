#include "assimp_glm_helpers.h"
#include <cglm/quat.h>

void convert_matrix_to_glm(struct aiMatrix4x4* matrix, mat4* dest){
	*(dest)[0][0] = matrix->a1;
	*(dest)[1][0] = matrix->a2;
	*(dest)[2][0] = matrix->a3;
	*(dest)[3][0] = matrix->a4;

	*(dest)[0][1] = matrix->b1;
	*(dest)[1][1] = matrix->b2;
	*(dest)[2][1] = matrix->b3;
	*(dest)[3][1] = matrix->b4;

	*(dest)[0][2] = matrix->c1;
	*(dest)[1][2] = matrix->c2;
	*(dest)[2][2] = matrix->c3;
	*(dest)[3][2] = matrix->c4;

	*(dest)[0][3] = matrix->d1;
	*(dest)[1][3] = matrix->d2;
	*(dest)[2][3] = matrix->d3;
	*(dest)[3][3] = matrix->d4;
}

void get_glm_vec(struct aiVector3D* vec, vec3* dest){
	*(dest)[0] = vec->x;
	*(dest)[1] = vec->y;
	*(dest)[2] = vec->z;
}

void get_glm_quat(struct aiQuaternion* quat, versor* dest){
	*(dest)[0] = quat->x;
	*(dest)[1] = quat->y;
	*(dest)[2] = quat->z;
	*(dest)[3] = quat->w;
}

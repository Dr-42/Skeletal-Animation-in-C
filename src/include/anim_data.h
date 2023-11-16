#pragma once

#include <cglm/cglm.h>
#include <stdint.h>

typedef struct bone_info_t {
	int32_t id;
	mat4 offset;
} bone_info_t;

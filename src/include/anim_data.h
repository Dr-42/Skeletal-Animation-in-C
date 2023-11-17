#pragma once

#include <stdint.h>

#include "utils/heim_mat.h"

typedef struct bone_info_t {
	int32_t id;
	HeimMat4 offset;
} bone_info_t;

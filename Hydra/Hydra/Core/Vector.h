#pragma once

#include <iostream>
#include <limits>
#include "glm/basic.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

typedef glm::vec4 Vector4;
typedef glm::vec3 Vector3;
typedef glm::vec2 Vector2;

typedef glm::ivec4 Vector4i;
typedef glm::ivec3 Vector3i;
typedef glm::ivec2 Vector2i;

typedef glm::mat4 Matrix4;
typedef glm::mat3 Matrix3;

typedef glm::quat Quaternion;

#define FloatMax std::numeric_limits<float>::max()
#define FloatMin std::numeric_limits<float>::min()
#define FloatInf std::numeric_limits<float>::infinity()

#define IntMax std::numeric_limits<int>::max()
#define IntMin std::numeric_limits<int>::min()
#define IntInf std::numeric_limits<int>::infinity()

static inline void PrintMatrix(const Matrix4& mat)
{
	for (int i = 0; i < 4; i++)
	{
		std::cout << mat[i].x << ", " << mat[i].y << ", " << mat[i].z << ", " << mat[i].w << std::endl;
	}
}
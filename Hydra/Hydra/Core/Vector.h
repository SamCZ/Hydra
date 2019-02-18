#pragma once

#include <iostream>
#include "glm/basic.hpp"

namespace Hydra
{
	typedef glm::vec4 Vector4;
	typedef glm::vec3 Vector3;
	typedef glm::vec2 Vector2;

	typedef glm::ivec4 Vector4i;
	typedef glm::ivec3 Vector3i;
	typedef glm::ivec2 Vector2i;

	typedef glm::mat4 Matrix4;
	typedef glm::mat3 Matrix3;

	typedef glm::quat Quaternion;

	static inline void PrintMatrix(const Matrix4& mat)
	{
		for (int i = 0; i < 4; i++)
		{
			std::cout << mat[i].x << ", " << mat[i].y << ", " << mat[i].z << ", " << mat[i].w << std::endl;
		}
	}
}
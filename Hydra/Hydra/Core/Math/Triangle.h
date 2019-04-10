#pragma once

#include "Hydra/Core/Library.h"
#include "Hydra/Core/Vector.h"

namespace Hydra
{
	struct HYDRA_EXPORT Triangle
	{
		Vector3 V0;
		Vector3 V1;
		Vector3 V2;

		static inline Vector3 ComputeTriangleNormal(const Vector3 & p1, const Vector3 & p2, const Vector3 & p3)
		{
			Vector3 U = p2 - p1;
			Vector3 V = p3 - p1;
			float x = (U.y * V.z) - (U.z * V.y);
			float y = (U.z * V.x) - (U.x * V.z);
			float z = (U.x * V.y) - (U.y * V.x);
			return glm::normalize(Vector3(x, y, z));
		}
	};
}
#pragma once

#include "Hydra/Core/Vector.h"
#include "Hydra/Physics/Collisons/Collidable.h"

class HYDRA_API Ray : public Collidable
{
public:
	Vector3 Origin;
	Vector3 Direction;

	float Limit;

	Ray();
	Ray(const Vector3& origin, const Vector3& direction);

	bool IntersectWithTriangle(const Vector3& v0, const Vector3& v1, const Vector3& v2, float& distance);
};
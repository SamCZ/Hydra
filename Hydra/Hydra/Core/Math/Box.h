#pragma once

#include "Hydra/Core/Library.h"

#include "Hydra/Core/Vector.h"
#include "Hydra/Core/String.h"

#include "Hydra/Physics/Collisons/Ray.h"
#include "Hydra/Physics/Collisons/CollisionResults.h"

#define BBMM 0,

class HYDRA_API Box
{
private:
	float _ClipTemp[3];
public:
	Vector3 Origin;
	Vector3 Extent;
public:
	Box();
	Box(const Vector3& origin, const Vector3& extent);
	Box(int mark, const Vector3& min, const Vector3& max);

	void SetMinMax(glm::vec3 min, glm::vec3 max);

	static void CheckMinMax(glm::vec3 &min, glm::vec3 &max, glm::vec3 &point);

	int CollideWithRay(const Ray& ray, CollisionResults& results);

	Box Transform(const glm::vec3& location, const glm::vec3& rotation, const glm::vec3& scale);

	glm::vec3 GetMin() const;
	glm::vec3 GetMax() const;

	glm::vec3 GetOrigin() const;
	glm::vec3 GetExtent() const;

	float GetXExtent() const;
	float GetYExtent() const;
	float GetZExtent() const;

	float GetMinX() const;
	float GetMinY() const;
	float GetMinZ() const;

	float GetMaxX() const;
	float GetMaxY() const;
	float GetMaxZ() const;

	String Print();

private:
	bool Clip(float denom, float numer, float t[]);
};
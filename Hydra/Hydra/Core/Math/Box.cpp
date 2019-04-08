#include "Hydra/Core/Math/Box.h"

namespace Hydra
{
	Box::Box()
	{
	}
	Box::Box(const Vector3 & origin, const Vector3 & extent) : Origin(origin), Extent(extent)
	{
	}
	Box::Box(int mark, const Vector3 & min, const Vector3 & max)
	{
		SetMinMax(min, max);
	}
	void Box::SetMinMax(glm::vec3 min, glm::vec3 max)
	{
		Origin = max;
		Origin += min;
		Origin *= 0.5f;
		Extent.x = glm::abs(max.x - Origin.x);
		Extent.y = glm::abs(max.y - Origin.y);
		Extent.z = glm::abs(max.z - Origin.z);
	}
	void Box::CheckMinMax(glm::vec3 & min, glm::vec3 & max, glm::vec3 & point)
	{
		if (point.x < min.x)
		{
			min.x = point.x;
		}
		if (point.x > max.x)
		{
			max.x = point.x;
		}
		if (point.y < min.y)
		{
			min.y = point.y;
		}
		if (point.y > max.y)
		{
			max.y = point.y;
		}
		if (point.z < min.z)
		{
			min.z = point.z;
		}
		if (point.z > max.z)
		{
			max.z = point.z;
		}
	}
	int Box::CollideWithRay(const Ray & ray, CollisionResults & results)
	{
		glm::vec3 diff = ray.Origin - Origin;
		glm::vec3 direction = ray.Direction;

		_ClipTemp[0] = 0;
		_ClipTemp[1] = FloatInf;

		float saveT0 = _ClipTemp[0];
		float saveT1 = _ClipTemp[1];

		bool notEntirelyClipped =
			Clip(+direction.x, -diff.x - Extent.x, _ClipTemp) &&
			Clip(-direction.x, +diff.x - Extent.x, _ClipTemp) &&

			Clip(+direction.y, -diff.y - Extent.y, _ClipTemp) &&
			Clip(-direction.y, +diff.y - Extent.y, _ClipTemp) &&

			Clip(+direction.z, -diff.z - Extent.z, _ClipTemp) &&
			Clip(-direction.z, +diff.z - Extent.z, _ClipTemp);
		if (notEntirelyClipped && (_ClipTemp[0] != saveT0 || _ClipTemp[1] != saveT1))
		{
			if (_ClipTemp[1] > _ClipTemp[0])
			{
				glm::vec3 point0 = (ray.Direction * _ClipTemp[0]) + ray.Origin;
				glm::vec3 point1 = (ray.Direction * _ClipTemp[1]) + ray.Origin;

				CollisionResult result;
				result.IsNull = false;
				result.ContactPoint = point0;
				result.Distance = _ClipTemp[0];
				results.AddCollision(result);

				CollisionResult result2;
				result2.IsNull = false;
				result2.ContactPoint = point1;
				result2.Distance = _ClipTemp[1];
				results.AddCollision(result2);
			}
			glm::vec3 point = (ray.Direction * _ClipTemp[0]) + ray.Origin;
			CollisionResult result;
			result.IsNull = false;
			result.ContactPoint = point;
			result.Distance = _ClipTemp[0];
			results.AddCollision(result);
			return 1;
		}
		return 0;
	}

	Box Box::Transform(const glm::vec3 & location, const glm::vec3 & rotation, const glm::vec3 & scale)
	{
		glm::vec3 center = Origin * scale;

		glm::mat4 rotationMat = glm::mat4();
		rotationMat *= glm::rotate(glm::radians(rotation.x), glm::vec3(1, 0, 0));
		rotationMat *= glm::rotate(glm::radians(rotation.y), glm::vec3(0, 1, 0));
		rotationMat *= glm::rotate(glm::radians(rotation.z), glm::vec3(0, 0, 1));

		//glm::quat rotationQuat = glm::toQuat(rotationMat);
		//glm::mat3 rotationMat3 = glm::mat3(rotationMat);

		glm::mat3 rot = glm::mat3(rotationMat);

		for (int i = 0; i < 3; i++)
		{
			for (int a = 0; a < 3; a++)
			{
				rot[i][a] = glm::abs(rot[i][a]);
			}
		}

		center = rot * center;
		center += location;

		glm::vec3 vect1 = glm::vec3(Extent.x * glm::abs(scale.x), Extent.y * glm::abs(scale.y), Extent.z * glm::abs(scale.z));
		glm::vec3 vect2 = rot * vect1;

		return Box(center, glm::abs(vect2));
	}

	glm::vec3 Box::GetMin() const
	{
		return Origin - Extent;
	}

	glm::vec3 Box::GetMax() const
	{
		return Origin + Extent;
	}

	glm::vec3 Box::GetOrigin() const
	{
		return Origin;
	}

	glm::vec3 Box::GetExtent() const
	{
		return Extent;
	}

	float Box::GetXExtent() const
	{
		return Extent.x;
	}

	float Box::GetYExtent() const
	{
		return Extent.y;
	}

	float Box::GetZExtent() const
	{
		return Extent.z;
	}

	float Box::GetMinX() const
	{
		return GetMin().x;
	}

	float Box::GetMinY() const
	{
		return GetMin().y;
	}

	float Box::GetMinZ() const
	{
		return GetMin().z;
	}

	float Box::GetMaxX() const
	{
		return GetMax().x;
	}

	float Box::GetMaxY() const
	{
		return GetMax().y;
	}

	float Box::GetMaxZ() const
	{
		return GetMax().z;
	}

	std::string Box::Print()
	{
		return std::string("Center(x=") + std::string(std::to_string(Origin.x)) + std::string(",y=") + std::string(std::to_string(Origin.y)) + std::string(",z=") + std::string(std::to_string(Origin.z)) + std::string("),xe=") + std::string(std::to_string(Extent.x)) + std::string(",ye=") + std::string(std::to_string(Extent.y)) + std::string(",ze=") + std::string(std::to_string(Extent.z));
	}

	bool Box::Clip(float denom, float numer, float t[])
	{
		if (denom > 0.0f)
		{
			float newT = numer / denom;
			if (newT > t[1])
			{
				return false;
			}
			if (newT > t[0])
			{
				t[0] = newT;
			}
			return true;
		}
		else if (denom < 0.0f)
		{
			float newT = numer / denom;
			if (newT < t[0])
			{
				return false;
			}
			if (newT < t[1])
			{
				t[1] = newT;
			}
			return true;
		}
		else
		{
			return numer <= 0.0f;
		}
	}
}
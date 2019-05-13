#pragma once

#include "Hydra/Core/Vector.h"

class Testing
{
public:
	inline static bool IntersectionSphereTriangle(const Vector3& sphereCenter, float sphereRadius, const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& normal)
	{
		Vector3 A = a - sphereCenter;
		Vector3 B = b - sphereCenter;
		Vector3 C = c - sphereCenter;

		float rr = sphereRadius * sphereRadius;

		Vector3 V = glm::cross(B - A, C - A);

		float d = glm::dot(A, V);
		float e = glm::dot(V, V);

		if (d * d > rr * e)
		{
			return false;
		}

		float aa = glm::dot(A, A);
		float ab = glm::dot(A, B);
		float ac = glm::dot(A, C);
		float bb = glm::dot(B, B);
		float bc = glm::dot(B, C);
		float cc = glm::dot(C, C);

		if (
			(aa > rr) & (ab > aa) & (ac > aa) ||
			(bb > rr) & (ab > bb) & (bc > bb) ||
			(cc > rr) & (ac > cc) & (bc > cc)
			)
		{
			return false;
		}

		Vector3 AB = B - A;
		Vector3 BC = B - C;
		Vector3 CA = A - C;

		float d1 = ab - aa;
		float d2 = bc - bb;
		float d3 = ac - cc;

		float e1 = glm::dot(AB, AB);
		float e2 = glm::dot(BC, BC);
		float e3 = glm::dot(CA, CA);

		Vector3 Q1 = (A * e1) - (AB * d1);
		Vector3 Q2 = (B * e2) - (CA * d2);
		Vector3 Q3 = (C * e3) - (CA * d3);

		Vector3 QC = (C * e1) - Q1;
		Vector3 QA = (A * e2) - Q2;
		Vector3 QB = (B * e3) - Q3;

		if (
			(glm::dot(Q1, Q1) > rr * e1 * e1) && (glm::dot(Q1, QC) >= 0) ||
			(glm::dot(Q2, Q2) > rr * e2 * e2) && (glm::dot(Q2, QA) >= 0) ||
			(glm::dot(Q3, Q3) > rr * e3 * e3) && (glm::dot(Q3, QB) >= 0)
			)
		{
			return false;
		}

		float distance = glm::sqrt(d * d / e) - sphereRadius;
		Vector3 contactPoint = sphereCenter + (-normal * distance);

		return true;
	}
};
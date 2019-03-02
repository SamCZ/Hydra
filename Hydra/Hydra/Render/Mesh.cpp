#include "Mesh.h"

namespace Hydra
{
	Mesh::Mesh() : PrimitiveType(NVRHI::PrimitiveType::TRIANGLE_LIST)
	{
	}

	void Mesh::UpdateBounds()
	{
		float floatMax = FloatMax;
		float floatMin = FloatMin;

		Vector3 maxBounds = Vector3(floatMin, floatMin, floatMin);
		Vector3 minBounds = Vector3(floatMax, floatMax, floatMax);

		for (Vector3& pos : Vertices)
		{
			minBounds = glm::min(minBounds, pos);
			maxBounds = glm::max(maxBounds, pos);
		}

		Bounds = {};
		Bounds.Origin = (minBounds + maxBounds) * 0.5f;
		Bounds.Extent = maxBounds - Bounds.Origin;
	}

	void Mesh::GenerateNormals()
	{
		List<Vector3> normals;
		normals.resize(Vertices.size());

		for (int i = 0; i < Indices.size() / 3; i++)
		{
			int n0 = Indices[i * 3 + 0];
			int n1 = Indices[i * 3 + 1];
			int n2 = Indices[i * 3 + 2];

			Vector3& v0 = Vertices[n0];
			Vector3& v1 = Vertices[n1];
			Vector3& v2 = Vertices[n2];

			Vector3 normal = ComputeTriangleNormal(v0, v1, v2);

			normals[n0] = normal;
			normals[n1] = normal;
			normals[n2] = normal;
		}

		Normals = normals;
	}

	Vector3 Mesh::ComputeTriangleNormal(const Vector3 & p1, const Vector3 & p2, const Vector3 & p3)
	{
		Vector3 U = p2 - p1;
		Vector3 V = p3 - p1;
		float x = (U.y * V.z) - (U.z * V.y);
		float y = (U.z * V.x) - (U.x * V.z);
		float z = (U.x * V.y) - (U.y * V.x);
		return glm::normalize(Vector3(x, y, z));
	}

}
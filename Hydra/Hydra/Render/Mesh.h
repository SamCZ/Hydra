#pragma once

#include "Hydra/Core/SmartPointer.h"
#include "Hydra/Core/Resource.h"
#include "Hydra/Core/Vector.h"
#include "Hydra/Core/Container.h"
#include "Hydra/Core/Math/Box.h"

#include "Hydra/Render/Pipeline/GFSDK_NVRHI.h"

namespace Hydra
{
	class Mesh : public Resource
	{
	private:

	public:
		Mesh();

		List<Vector3> Vertices;
		List<Vector2> TexCoords;
		List<Vector3> Normals;
		List<Vector3> Tangents;
		List<Vector3> BiNormals;
		List<unsigned int> Indices;
		Box Bounds;

		NVRHI::PrimitiveType::Enum PrimitiveType;

		void UpdateBounds();

		void GenerateNormals();

	private:
		Vector3 ComputeTriangleNormal(const Vector3& p1, const Vector3& p2, const Vector3& p3);
	};

	DEFINE_PTR(Mesh)
}
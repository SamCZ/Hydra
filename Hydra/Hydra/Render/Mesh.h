#pragma once

#include "Hydra/Core/SmartPointer.h"
#include "Hydra/Core/Resource.h"
#include "Hydra/Core/Vector.h"
#include "Hydra/Core/Container.h"
#include "Hydra/Core/Math/Box.h"

#include "Hydra/Render/VertexBuffer.h"
#include "Hydra/Render/Pipeline/GFSDK_NVRHI.h"

namespace Hydra
{
	struct PrimitiveType
	{
		enum Enum
		{
			Box, BoxNDC, Plane, Quad
		};
	};

	class Mesh : public Resource
	{
	private:
		NVRHI::BufferHandle _IndexHandle;
		NVRHI::BufferHandle _VertexBuffer;
		bool _IsIndexed;
		int _IndexCount;
	public:
		Mesh();
		~Mesh();

		List<VertexBufferEntry> VertexData;
		List<unsigned int> Indices;
		Box Bounds;

		NVRHI::PrimitiveType::Enum PrimitiveType;

		void UpdateBounds();

		void GenerateUVs();
		void GenerateNormals();
		void GenerateTangentsAndBiTangents();

		void SmoothMesh();

		void UpdateBuffers();

		static Mesh* CreatePrimitive(const PrimitiveType::Enum& type, const Vector3& scale = Vector3(1.0f, 1.0f, 1.0f));

		NVRHI::BufferHandle GetVertexBuffer();
		NVRHI::BufferHandle GetIndexBuffer();

		void SetIndexBuffer(NVRHI::BufferHandle buffer);
		void SetVertexBuffer(NVRHI::BufferHandle buffer);

		void SetIndexed(bool indexed);
		bool IsIndexed();

		void SetIndexCount(int count);
		int GetIndexCount() const;

	private:
		Vector3 ComputeTriangleNormal(const Vector3& p1, const Vector3& p2, const Vector3& p3);
		Vector3 ComputeNormalFromMultiplePoints(List<Vector3>& points);
	};

	DEFINE_PTR(Mesh)
}
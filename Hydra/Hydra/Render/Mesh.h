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

	class BIHTree;

	class EngineContext;

	class Mesh : public Resource
	{
	private:
		NVRHI::BufferHandle _IndexHandle;
		NVRHI::BufferHandle _VertexBuffer;
		bool _AutoCreateBuffers;
		bool _IsIndexed;
		int _IndexCount;

		BIHTree* _ComplexCollider;
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

		void CreateComplexCollider();
		BIHTree* GetComplexCollider();

		void UpdateBuffers(EngineContext* context);

		static Mesh* CreatePrimitive(const PrimitiveType::Enum& type, const Vector3& scale = Vector3(1.0f, 1.0f, 1.0f));

		NVRHI::BufferHandle GetVertexBuffer();
		NVRHI::BufferHandle GetIndexBuffer();

		void SetIndexBuffer(NVRHI::BufferHandle buffer);
		void SetVertexBuffer(NVRHI::BufferHandle buffer);

		bool CanAutoCreateBuffers() const;
		void SetAutoCreateBuffers(bool state);

		void SetIndexed(bool indexed);
		bool IsIndexed();

		void SetIndexCount(int count);
		int GetIndexCount() const;

	private:
		Vector3 ComputeNormalFromMultiplePoints(List<Vector3>& points);
	};

	DEFINE_PTR(Mesh)
}
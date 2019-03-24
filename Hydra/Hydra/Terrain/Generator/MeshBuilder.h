#pragma once

#include "Hydra/Core/Common.h"
#include "Hydra/Render/VertexBuffer.h"

namespace Hydra
{
	class Spatial;

	class MeshBuilder
	{
	private:
		List<VertexBufferEntry> _VertexData;
		List<unsigned int> _Indices;
		int squareCount;
	public:
		void AddQuad(const Vector3& v0, const Vector3& v1, const Vector3& v2, const Vector3& v3, const Vector2& uv0, const Vector2& uv1, const Vector2& uv2, const Vector2& uv3);

		void Apply(SharedPtr<Spatial> obj);
	};
}
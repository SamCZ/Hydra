#pragma once

#include "Hydra/Core/Container.h"
#include "Hydra/Render/VertexBuffer.h"

namespace Hydra
{
	class Mesh;

	class MeshData
	{
	private:
		List<VertexBufferEntry> _Vertices;
		List<unsigned int> _Indices;

		List<Vector3> _OutOfMeshVertices;
		List<int> _OutOfMeshTriangles;

		int _TriangleIndex;
		int _OutOfMeshTriangleIndex;

		bool _UseFlatShading;
	public:
		MeshData(int numVertsPerLine, int skipIncrement, bool useFlatShading);

		void AddVertex(const Vector3& vertexPosition, const Vector2& uv, const Vector2& uv2, int vertexIndex);
		void AddTriangle(int a, int b, int c);
		
		void ProcessMesh();

		Mesh* CreateMesh();
	private:
		Vector3 SurfaceNormalFromIndices(int indexA, int indexB, int indexC);

		void CalculateNormals();
		void BakeNormals();
		void FlatShading();
	};
}
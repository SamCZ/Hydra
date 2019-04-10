#include "Hydra/Terrain/Generator/MeshData.h"
#include "Hydra/Render/Mesh.h"

namespace Hydra
{
	MeshData::MeshData(int numVertsPerLine, int skipIncrement, bool useFlatShading) : _UseFlatShading(useFlatShading)
	{
		int numMeshEdgeVertices = (numVertsPerLine - 2) * 4 - 4;
		int numEdgeConnectionVertices = (skipIncrement - 1) * (numVertsPerLine - 5) / skipIncrement * 4;
		int numMainVerticesPerLine = (numVertsPerLine - 5) / skipIncrement + 1;
		int numMainVertices = numMainVerticesPerLine * numMainVerticesPerLine;

		_Vertices = List<VertexBufferEntry>(numMeshEdgeVertices + numEdgeConnectionVertices + numMainVertices);


		int numMeshEdgeTriangles = 8 * (numVertsPerLine - 4);
		int numMainTriangles = (numMainVerticesPerLine - 1) * (numMainVerticesPerLine - 1) * 2;

		_Indices = List<unsigned int>((numMeshEdgeTriangles + numMainTriangles) * 3);

		_OutOfMeshVertices = List<Vector3>(numVertsPerLine * 4 - 4);
		_OutOfMeshTriangles = List<int>(24 * (numVertsPerLine - 2));
	}

	void MeshData::AddVertex(const Vector3& vertexPosition, const Vector2& uv, const Vector2& uv2, int vertexIndex)
	{
		if (vertexIndex < 0)
		{
			_OutOfMeshVertices[-vertexIndex - 1] = vertexPosition;
		}
		else
		{
			_Vertices[vertexIndex].position = vertexPosition;
			_Vertices[vertexIndex].texCoord = uv;
			_Vertices[vertexIndex].texCoord2 = uv2;
		}
	}

	void MeshData::AddTriangle(int a, int b, int c)
	{
		if (a < 0 || b < 0 || c < 0)
		{
			_OutOfMeshTriangles[_OutOfMeshTriangleIndex] = a;
			_OutOfMeshTriangles[_OutOfMeshTriangleIndex + 1] = b;
			_OutOfMeshTriangles[_OutOfMeshTriangleIndex + 2] = c;
			_OutOfMeshTriangleIndex += 3;
		}
		else
		{
			_Indices[_TriangleIndex] = a;
			_Indices[_TriangleIndex + 1] = b;
			_Indices[_TriangleIndex + 2] = c;
			_TriangleIndex += 3;
		}
	}
	void MeshData::CalculateNormals()
	{
		int triangleCount = (int)_Indices.size() / 3;
		for (int i = 0; i < triangleCount; i++)
		{
			int normalTriangleIndex = i * 3;
			unsigned int vertexIndexA = _Indices[normalTriangleIndex];
			unsigned int vertexIndexB = _Indices[normalTriangleIndex + 1];
			unsigned int vertexIndexC = _Indices[normalTriangleIndex + 2];

			Vector3 triangleNormal = SurfaceNormalFromIndices(vertexIndexA, vertexIndexB, vertexIndexC);
			_Vertices[vertexIndexA].normal += triangleNormal;
			_Vertices[vertexIndexB].normal += triangleNormal;
			_Vertices[vertexIndexC].normal += triangleNormal;
		}

		int borderTriangleCount = (int)_OutOfMeshTriangles.size() / 3;
		for (int i = 0; i < borderTriangleCount; i++)
		{
			int normalTriangleIndex = i * 3;
			int vertexIndexA = _OutOfMeshTriangles[normalTriangleIndex];
			int vertexIndexB = _OutOfMeshTriangles[normalTriangleIndex + 1];
			int vertexIndexC = _OutOfMeshTriangles[normalTriangleIndex + 2];

			Vector3 triangleNormal = SurfaceNormalFromIndices(vertexIndexA, vertexIndexB, vertexIndexC);
			if (vertexIndexA >= 0)
			{
				_Vertices[vertexIndexA].normal += triangleNormal;
			}
			if (vertexIndexB >= 0)
			{
				_Vertices[vertexIndexB].normal += triangleNormal;
			}
			if (vertexIndexC >= 0)
			{
				_Vertices[vertexIndexC].normal += triangleNormal;
			}
		}


		for (int i = 0; i < _Vertices.size(); i++)
		{
			_Vertices[i].normal = glm::normalize(_Vertices[i].normal);
		}
	}

	void MeshData::ProcessMesh()
	{
		if (_UseFlatShading)
		{
			FlatShading();
		}
		else
		{
			BakeNormals();
		}
	}

	Mesh* MeshData::CreateMesh()
	{
		Mesh* mesh = new Mesh();

		mesh->VertexData = _Vertices;
		mesh->Indices = _Indices;

		mesh->GenerateTangentsAndBiTangents();

		return mesh;
	}

	Vector3 MeshData::SurfaceNormalFromIndices(int indexA, int indexB, int indexC)
	{
		Vector3 pointA = (indexA < 0) ? _OutOfMeshVertices[-indexA - 1] : _Vertices[indexA].position;
		Vector3 pointB = (indexB < 0) ? _OutOfMeshVertices[-indexB - 1] : _Vertices[indexB].position;
		Vector3 pointC = (indexC < 0) ? _OutOfMeshVertices[-indexC - 1] : _Vertices[indexC].position;

		Vector3 sideAB = pointB - pointA;
		Vector3 sideAC = pointC - pointA;
		return glm::normalize(glm::cross(sideAB, sideAC));
	}

	void MeshData::BakeNormals()
	{
		CalculateNormals();
	}

	void MeshData::FlatShading()
	{
		
	}
}
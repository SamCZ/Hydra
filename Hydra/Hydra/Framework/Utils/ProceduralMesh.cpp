#include "ProceduralMesh.h"

ProceduralMesh::ProceduralMesh() : quadCount(0)
{
	mesh = new HStaticMesh();
	mesh->RenderData->LODResources.push_back({ });
	lod = &mesh->RenderData->LODResources[0];
}

void ProceduralMesh::AddVertex(const Vector3 & position, const Vector2 & texCoord0, const Vector2 & texCoord1, const Vector3 & normal, const Vector3 & color, const Vector3 & tangent, const Vector3 & bitangent)
{
	lod->VertexData.push_back(
		{ position, texCoord0, texCoord1, color, normal, tangent, bitangent }
	);
}

void ProceduralMesh::AddNextIndex()
{
	lod->Indices.push_back(lod->VertexData.size() - 1);
}

void ProceduralMesh::AddIndex(int index)
{
	lod->Indices.push_back(index);
}

void ProceduralMesh::AddQuadIndices()
{
	AddIndex((quadCount * 4) + 0);
	AddIndex((quadCount * 4) + 1);
	AddIndex((quadCount * 4) + 3);

	AddIndex((quadCount * 4) + 1);
	AddIndex((quadCount * 4) + 2);
	AddIndex((quadCount * 4) + 3);

	quadCount++;
}

HStaticMesh* ProceduralMesh::Create()
{
	FStaticMeshSection section = {};
	section.EnableCollision = true;
	section.NumTriangles = lod->Indices.size();
	lod->Sections.push_back(section);
	return mesh;
}

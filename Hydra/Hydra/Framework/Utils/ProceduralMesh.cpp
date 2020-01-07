#include "ProceduralMesh.h"

ProceduralMesh::ProceduralMesh()
{
	mesh = new HStaticMesh();
	mesh->RenderData->LODResources.push_back({ });
	lod = &mesh->RenderData->LODResources[0];
}

void ProceduralMesh::addTriangle(const Vector3 & position, const Vector2 & texCoord0, const Vector2 & texCoord1, const Vector3 & normal, const Vector3 & color, const Vector3 & tangent, const Vector3 & bitangent)
{
	lod->VertexData.push_back(
		{ position, texCoord0, texCoord1, color, normal, tangent, bitangent }
	);

	lod->Indices.push_back(lod->VertexData.size() - 1);
}

HStaticMesh* ProceduralMesh::create()
{
	FStaticMeshSection section = {};
	section.EnableCollision = true;
	section.NumTriangles = lod->Indices.size();
	lod->Sections.push_back(section);
	return mesh;
}

#pragma once

#include "Hydra/Framework/StaticMesh.h"
#include "Hydra/Framework/StaticMeshResources.h"

class HYDRA_API ProceduralMesh
{
private:
	HStaticMesh* mesh;
	FStaticMeshLODResources* lod;
public:
	ProceduralMesh();

	FORCEINLINE void addTriangle(const Vector3& position, const Vector2& texCoord, const Vector3& normal, const Vector3& color = Vector3(1, 1, 1))
	{
		addTriangle(position, texCoord, Vector2(), normal, color, Vector3(), Vector3());
	}

	FORCEINLINE void addTriangle(const Vector3& position, const Vector2& texCoord, const Vector3& normal, const Vector3& color, const Vector3& tangent, const Vector3& bitangent)
	{
		addTriangle(position, texCoord, Vector2(), normal, color, tangent, bitangent);
	}

	FORCEINLINE void addTriangle(const Vector3& position, const Vector2& texCoord0, const Vector2& texCoord1, const Vector3& normal, const Vector3& color = Vector3(1, 1, 1))
	{
		addTriangle(position, texCoord0, texCoord1, normal, color, Vector3(), Vector3());
	}

	void addTriangle(const Vector3& position, const Vector2& texCoord0, const Vector2& texCoord1, const Vector3& normal, const Vector3& color, const Vector3& tangent, const Vector3& bitangent);

	HStaticMesh* create();
};
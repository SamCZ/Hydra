#pragma once

#include "Hydra/Render/VertexBuffer.h"

struct FMeshBufferDataInternal;

struct FStaticMeshSection
{
	/** The index of the material with which to render this section. */
	int32 MaterialIndex;

	/** Range of vertices and indices used when rendering this section. */
	uint32 FirstIndex;
	uint32 NumTriangles;
	uint32 MinVertexIndex;
	uint32 MaxVertexIndex;

	/** If true, collision is enabled for this section. */
	bool EnableCollision;
	/** If true, this section will cast a shadow. */
	bool CastShadow;


	/** Constructor. */
	FStaticMeshSection()
		: MaterialIndex(0)
		, FirstIndex(0)
		, NumTriangles(0)
		, MinVertexIndex(0)
		, MaxVertexIndex(0)
		, EnableCollision(false)
		, CastShadow(true)
	{
	}
};

struct FStaticMeshLODResources
{
	List<VertexBufferEntry> VertexData;
	List<uint32> Indices;

	uint32 LastIndex;

	List<FStaticMeshSection> Sections;

	FMeshBufferDataInternal* InternalBufferData;
};

class FStaticMeshRenderData
{
public:
	List<FStaticMeshLODResources> LODResources;
};
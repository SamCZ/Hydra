#include "StaticMesh.h"
#include "StaticMeshResources.h"


HStaticMesh::HStaticMesh()
{
	RenderData = new FStaticMeshRenderData();
}

HStaticMesh::~HStaticMesh()
{
	delete RenderData;
}

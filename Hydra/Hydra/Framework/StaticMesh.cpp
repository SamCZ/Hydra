#include "StaticMesh.h"
#include "StaticMeshResources.h"


HStaticMesh::HStaticMesh()
{
	RenderData = new FStaticMeshRenderData();

	Log("new HStaticMesh");
}

HStaticMesh::~HStaticMesh()
{
	delete RenderData;

	Log("deleted HStaticMesh");
}

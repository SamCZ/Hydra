#include "CubeActor.h"

void ACubeActor::InitializeComponents()
{
	CubeComponent = AddComponent<HStaticMeshComponent>("Cube");
	CubeComponent->StaticMesh = nullptr;
}
#include "CubeActor.h"

namespace Hydra
{
	void ACubeActor::InitializeComponents()
	{
		CubeComponent = AddComponent<HStaticMeshComponent>("Cube");
		CubeComponent->StaticMesh = nullptr;
	}
}

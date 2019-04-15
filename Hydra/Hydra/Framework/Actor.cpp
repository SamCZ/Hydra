#include "Hydra/Framework/Actor.h"

namespace Hydra
{
	FWorld* AActor::GetWorld()
	{
		return _World;
	}

	void AActor::SetWorld(FWorld* world)
	{
		_World = world;
	}

	void AActor::BeginPlay()
	{

	}

	void AActor::Tick(float DeltaTime)
	{

	}
}
#include "Hydra/Framework/World.h"

namespace Hydra
{
	FWorld::FWorld(EngineContext* context)
	{
		_Engine = context;
	}

	void FWorld::FinishSpawningActor(AActor* actor)
	{
		if (actor)
		{
			actor->BeginPlay();
		}
	}

	void FWorld::DestroyActor(AActor* actor)
	{
		if (actor && !actor->IsIndestructible)
		{
			auto iter = Find(_Actors, actor);

			if (iter != _Actors.end())
			{
				actor->BeginDestroy();

				_Actors.erase(iter);

				delete actor;
			}
		}
	}
}
#include "Hydra/Framework/World.h"

FWorld::FWorld(EngineContext* context)
{
	_Engine = context;
}

FWorld::~FWorld()
{
	for (AActor* actor : _Actors)
	{
		DestroyActor(actor);
	}

	DestroyActor((AActor*)_GameMode);
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
		//Destroy components of actor


		// Destroy actor
		auto iter = Find(_Actors, actor);

		if (iter != _Actors.end())
		{
			actor->BeginDestroy();

			_Actors.erase(iter);

			delete actor;
		}
	}
}
List<AActor*>& FWorld::GetActors()
{
	return _Actors;
}
List<HPrimitiveComponent*>& FWorld::GetPrimitiveComponents()
{
	return _PrimitiveComponents;
}
HGameModeBase* FWorld::GetGameMode()
{
	return _GameMode;
}
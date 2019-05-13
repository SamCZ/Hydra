#include "Hydra/Framework/World.h"

#include "Hydra/Framework/Components/PrimitiveComponent.h"
#include "Hydra/Framework/Components/CameraComponent.h"

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
		for (int i = actor->Components.size() - 1; i >= 0; i--)
		{
			HSceneComponent* cmp = actor->Components[i];

			actor->DestroyComponent(cmp);
		}

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

void FWorld::RegisterComponent(HSceneComponent* component)
{
	if (HPrimitiveComponent* cmp = component->SafeCast<HPrimitiveComponent>())
	{
		_PrimitiveComponents.push_back(cmp);
	}

	if (HCameraComponent* cmp = component->SafeCast<HCameraComponent>())
	{
		_CameraComponents.push_back(cmp);

		OnCameraComponentAdded.Invoke(cmp);
	}
}

void FWorld::UnregisterComponent(HSceneComponent* component)
{
	if (HPrimitiveComponent* cmp = component->SafeCast<HPrimitiveComponent>())
	{
		List_Remove(_PrimitiveComponents, cmp);
	}

	if (HCameraComponent* cmp = component->SafeCast<HCameraComponent>())
	{
		List_Remove(_CameraComponents, cmp);

		OnCameraComponentRemoved.Invoke(cmp);
	}
}

List<AActor*>& FWorld::GetActors()
{
	return _Actors;
}

const List<HPrimitiveComponent*>& FWorld::GetPrimitiveComponents()
{
	return _PrimitiveComponents;
}

const List<HCameraComponent*>& FWorld::GetCameraComponents()
{
	return _CameraComponents;
}

HGameModeBase* FWorld::GetGameMode()
{
	return _GameMode;
}
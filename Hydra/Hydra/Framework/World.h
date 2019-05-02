#pragma once

#include "Hydra/Core/Library.h"
#include "Hydra/Core/String.h"
#include "Hydra/Core/Vector.h"

#include "Hydra/Framework/Actor.h"

class EngineContext;
class HGameModeBase;
class HPrimitiveComponent;

HCLASS()
class HYDRA_API FWorld : public HObject
{
	HCLASS_GENERATED_BODY()
private:
	EngineContext* _Engine;
	List<AActor*> _Actors;
	List<HPrimitiveComponent*> _PrimitiveComponents;
	HGameModeBase* _GameMode;
public:
	FWorld(EngineContext* context);
	~FWorld();

	template<class AActor>
	AActor* SpawnActor(const String& Name, const Vector3& Position, const Vector3& Rotation, const Vector3& Scale = Vector3(0.0f))
	{
		AActor* actor = BeginSpawnActor<AActor>(Name, Position, Rotation, Scale);

		FinishSpawningActor(actor);

		return actor;
	}

	template<class T>
	T* BeginSpawnActor(const String& Name, const Vector3& Position, const Vector3& Rotation, const Vector3& Scale = Vector3(0.0f))
	{
		T* actorTemplated = new T();
		AActor* actor = static_cast<AActor*>(actorTemplated);
		actor->Engine = _Engine;
		actor->World = this;
		actor->Name = Name;
			
		actor->RootComponent = actor->AddComponent<HSceneComponent>("SceneRoot");
		actor->Components.push_back(actor->RootComponent);

		actor->SetLocation(Position);
		actor->SetRotation(Rotation);
		actor->SetScale(Scale);

		actor->InitializeComponents();

		_Actors.push_back(actor);

		return actorTemplated;
	}

	void FinishSpawningActor(AActor* actor);
	void DestroyActor(AActor* actor);

	FORCEINLINE void RegisterPrimitiveComponent(HPrimitiveComponent* component) { _PrimitiveComponents.push_back(component); }
	FORCEINLINE void UnregisterPrimitiveComponent(HPrimitiveComponent* component) { List_Remove(_PrimitiveComponents, component); }

	template<class T>
	void OverrideGameMode()
	{
		if (_GameMode != nullptr)
		{
			DestroyActor(_GameMode);
		}

		T* actorTemplated = new T();
		AActor* actor = static_cast<AActor*>(actorTemplated);
		actor->Engine = _Engine;
		actor->World = this;

		_GameMode = static_cast<HGameModeBase*>(actorTemplated);

		FinishSpawningActor(actor);
	}

	List<AActor*>& GetActors();
	List<HPrimitiveComponent*>& GetPrimitiveComponents();

	HGameModeBase* GetGameMode();
};
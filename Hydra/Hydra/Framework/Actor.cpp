#include "Hydra/Framework/Actor.h"
#include "Hydra/Framework/World.h"
#include "Hydra/Framework/Components/PrimitiveComponent.h"

namespace Hydra
{
	AActor::AActor() : HObject(), Engine(nullptr), World(nullptr), RootComponent(nullptr), IsIndestructible(false), IsActive(true), IsEditorOnly(false)
	{

	}
	void AActor::InitializeComponents()
	{

	}

	void AActor::DestroyComponent(HSceneComponent*& component)
	{
		if (component)
		{
			if (HPrimitiveComponent* pComponent = dynamic_cast<HPrimitiveComponent*>(component))
			{
				World->UnregisterPrimitiveComponent(pComponent);
			}

			List_Remove(Components, component);
			
			delete component;
			component = nullptr;
		}
	}

	void AActor::BeginPlay()
	{

	}

	void AActor::BeginDestroy()
	{
	}

	void AActor::Tick(float DeltaTime)
	{

	}

	void AActor::SetActive(bool newActive)
	{
		IsActive = newActive;
	}

	void AActor::ToggleActive()
	{
		IsActive = !IsActive;
	}

	void AActor::Destroy()
	{
		if (!IsIndestructible)
		{
			World->DestroyActor(this);
		}
	}

	void AActor::InitilizeComponent(HSceneComponent* component)
	{
		component->World = World;
		component->Engine = Engine;
		component->Owner = this;

		if (HPrimitiveComponent* pComponent = dynamic_cast<HPrimitiveComponent*>(component))
		{
			World->RegisterPrimitiveComponent(pComponent);
		}
	}
}
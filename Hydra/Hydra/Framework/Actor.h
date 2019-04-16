#pragma once

#include "Hydra/Framework/Object.h"
#include "Hydra/Framework/Components/SceneComponent.h"

namespace Hydra
{
	class FWorld;
	class EngineContext;

	class HYDRA_API AActor : public HObject
	{
	public:
		EngineContext* Engine;
		FWorld* World;

		uint8 IsIndestructible : 1;
		uint8 IsActive : 1;
		uint8 IsEditorOnly : 1;

		List<HSceneComponent*> Components;

		HSceneComponent* RootComponent;
	public:
		AActor();

		virtual void InitializeComponents();

		template<class T>
		FORCEINLINE T* AddComponent(const String& name)
		{
			T* componentRaw = new T();

			HSceneComponent* component = reinterpret_cast<HSceneComponent*>(componentRaw);

			InitilizeComponent(component);
			Components.push_back(component);
			component->AttachToComponent(RootComponent);

			return componentRaw;
		}

		void DestroyComponent(HSceneComponent*& component);

		virtual void BeginPlay();
		virtual void BeginDestroy();
		virtual void Tick(float DeltaTime);

		virtual void SetActive(bool newActive);
		virtual void ToggleActive();

		virtual void Destroy();


		/*Vector3 GetForwardVector() const;
		Vector3 GetUpVector() const;
		Vector3 GetRightVector() const;*/

	private:
		void InitilizeComponent(HSceneComponent* component);
	};
}
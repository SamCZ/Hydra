#pragma once

#include "Hydra/Framework/Object.h"
#include "Hydra/Core/Container.h"
#include "Hydra/Core/String.h"


namespace Hydra
{
	class AActor;
	class FWorld;

	namespace EEndPlayReason
	{
		enum Type
		{
			/** When the Actor or Component is explicitly destroyed. */
			Destroyed,
			/** When the world is being unloaded for a level transition. */
			LevelTransition,
			/** When the world is being unloaded because PIE is ending. */
			EndPlayInEditor,
			/** When the level it is a member of is streamed out. */
			RemovedFromWorld,
			/** When the application is being exited. */
			Quit,
		};

	}

	class HYDRA_API HActorComponent : public HObject
	{
	private:
		uint8 RenderTransformDirty : 1;
	public:
		List<String> Tags;

		uint8 Registered : 1;
		uint8 AutoActivate : 1;
		uint8 IsActive : 1;

		uint8 IsEditorOnly : 1;

		AActor* GetOwner() const;
		FWorld* GetWorld() const;

		virtual void Activate();
		virtual void Deactivate();
		virtual void SetActive(bool newActive);
		virtual void ToggleActive();

		virtual bool IsActive() const { return IsActive; }

		virtual bool IsEditorOnly() const { return IsEditorOnly; }

		virtual void MarkAsEditorOnlySubobject()
		{
			IsEditorOnly = true;
		}

		virtual void OnRegister();
		virtual void OnUnregister();

		virtual void BeginPlay();
		virtual void BeginDestroy();

		virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);

		FORCEINLINE bool IsRegistered() const { return Registered; }

		void RegisterComponent();
		void UnregisterComponent();


	};
}
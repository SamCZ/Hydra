#pragma once

#include "Hydra/Core/Library.h"
#include "Hydra/Core/String.h"
#include "Hydra/Core/Vector.h"

#include "Hydra/Framework/Actor.h"

namespace Hydra
{
	class HYDRA_API FWorld
	{
	private:
		List<AActor*> _Actors;
	public:

		template<class T>
		T* SpawnActor(const String& Name, const Vector3& Position, const Vector3& Rotation, const Vector3& Scale = Vector3(0.0f))
		{
			T* actor = BeginSpawnActor<T>(Name, Position, Rotation, Scale);

			FinishSpawningActor(actor);

			return actor;
		}

		template<class T>
		T* BeginSpawnActor(const String& Name, const Vector3& Position, const Vector3& Rotation, const Vector3& Scale = Vector3(0.0f))
		{
			T* actorTemplated = new T();
			AActor* actor = static_cast<AActor*>(actorTemplated);

			actor->SetWorld(this);

			return actorTemplated;
		}

		void FinishSpawningActor(AActor* actor);

		void DestroyActor(AActor* actor);

		template<class T>
		void OverrideGameMode()
		{

		}
	};
}
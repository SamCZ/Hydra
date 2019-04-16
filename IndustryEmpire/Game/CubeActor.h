#pragma once

#include "Hydra/Framework/Actor.h"
#include "Hydra/Framework/Components/StaticMeshComponent.h"

namespace Hydra
{
	class ACubeActor : public AActor
	{
	public:
		HStaticMeshComponent* CubeComponent;
	public:
		void InitializeComponents();
	};
}
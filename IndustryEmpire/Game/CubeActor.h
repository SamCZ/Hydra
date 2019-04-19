#pragma once

#include "Hydra/Framework/Actor.h"
#include "Hydra/Framework/Components/StaticMeshComponent.h"

class ACubeActor : public AActor
{
public:
	HStaticMeshComponent* CubeComponent;
public:
	void InitializeComponents();
};
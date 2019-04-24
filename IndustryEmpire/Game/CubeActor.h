#pragma once

#include "Hydra/Framework/Actor.h"
#include "Hydra/Framework/Components/StaticMeshComponent.h"

#include "Hydra/Core/Random.h"

class ACubeActor : public AActor
{
public:
	HStaticMeshComponent* CubeComponent;

	Vector2 Velocity;
	Vector2 Acceleration;
	Random Rnd;
public:
	void InitializeComponents();

	void BeginPlay();
	void Tick(float DeltaTime);
};
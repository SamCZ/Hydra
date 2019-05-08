#pragma once

#include "Hydra/Framework/Actor.h"
#include "Hydra/Framework/Components/StaticMeshComponent.h"

#include "Hydra/Core/Random.h"
#include "CubeActor.generated.h"


class UIRenderer;

HCLASS()
class ACubeActor : public AActor
{
	HCLASS_GENERATED_BODY()
public:
	HStaticMeshComponent* CubeComponent;

	Vector2 Velocity;
	Vector2 Acceleration;
	Random Rnd;
public:
	void InitializeComponents();

	void BeginPlay();
	void Tick(float DeltaTime);

	void OnHud(UIRenderer* renderer);
};

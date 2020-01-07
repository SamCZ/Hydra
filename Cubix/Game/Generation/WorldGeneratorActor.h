#pragma once

#include "Hydra/Framework/Actor.h"
#include "WorldGeneratorActor.generated.h"


HCLASS()
class WorldGeneratorActor : public AActor
{
	HCLASS_GENERATED_BODY()
public:
	void InitializeComponents();

	void BeginPlay();
	void BeginDestroy();
	void Tick(float DeltaTime);
};

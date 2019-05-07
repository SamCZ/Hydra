#pragma once

#include "Hydra/Framework/Actor.h"
#include "Pawn.generated.h"


class InputManager;

HCLASS()
class HYDRA_API APawn : public AActor
{
	HCLASS_GENERATED_BODY()
private:

public:
	APawn();

	virtual void SetupPlayerInput(InputManager* inputManager);

	void AddMovementInput(Vector3 WorldDirection, float ScaleValue);
	void AddControllerPitchInput(float Val);
	void AddControllerYawInput(float Val);
	void AddControllerRollInput(float Val);
};

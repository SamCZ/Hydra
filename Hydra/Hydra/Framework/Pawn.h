#pragma once

#include "Hydra/Framework/Actor.h"

class InputManager;

class HYDRA_API APawn : public AActor
{
	HCLASS_BODY(APawn)
private:

public:
	APawn();

	virtual void SetupPlayerInput(InputManager* inputManager);

	void AddMovementInput(Vector3 WorldDirection, float ScaleValue);
	void AddControllerPitchInput(float Val);
	void AddControllerYawInput(float Val);
	void AddControllerRollInput(float Val);
};
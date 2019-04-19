#pragma once

#include "Hydra/Framework/Character.h"

class FirstPersonCharacter : public ACharacter
{
	class HCameraComponent* FirstPersonCameraComponent;

public:


	void InitializeComponents();

	void SetupPlayerInput(InputManager* inputManager) override;

	void BeginPlay();

	void MoveForwardBackward(float val);
	void MoveLeftRight(float val);
};
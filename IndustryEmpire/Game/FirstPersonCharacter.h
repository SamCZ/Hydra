#pragma once

#include "Hydra/Framework/Character.h"
#include "FirstPersonCharacter.generated.h"


HCLASS()
class FirstPersonCharacter : public ACharacter
{
	HCLASS_GENERATED_BODY()
	class HCameraComponent* FirstPersonCameraComponent;

public:


	void InitializeComponents();

	void SetupPlayerInput(InputManager* inputManager) override;

	void BeginPlay();

	void MoveForwardBackward(float val);
	void MoveLeftRight(float val);
};

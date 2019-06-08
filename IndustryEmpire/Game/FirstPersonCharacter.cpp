#include "FirstPersonCharacter.h"
#include "Hydra/Input/InputManager.h"

#include "Hydra/Framework/Components/CameraComponent.h"
#include "Hydra/Framework/Components/StaticMeshComponent.h"

void FirstPersonCharacter::InitializeComponents()
{
	FirstPersonCameraComponent = AddComponent<HCameraComponent>("MainCamera");

	FirstPersonCameraComponent->CameraMode = FCameraMode::Orthographic;

	//AddComponent<HStaticMeshComponent>("asd");
}

void FirstPersonCharacter::SetupPlayerInput(InputManager* inputManager)
{

}

void FirstPersonCharacter::BeginPlay()
{

}

void FirstPersonCharacter::MoveForwardBackward(float val)
{

}

void FirstPersonCharacter::MoveLeftRight(float val)
{

}
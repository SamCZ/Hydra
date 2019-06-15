#include "FirstPersonCharacter.h"
#include "Hydra/Input/InputManager.h"

#include "Hydra/Framework/Components/CameraComponent.h"
#include "Hydra/Framework/Components/StaticMeshComponent.h"

void FirstPersonCharacter::InitializeComponents()
{
	FirstPersonCameraComponent = AddComponent<HCameraComponent>("MainCamera");

	FirstPersonCameraComponent->CameraMode = FCameraMode::Perspective;

	RootComponent = FirstPersonCameraComponent;

	//AddComponent<HStaticMeshComponent>("asd");
}

void FirstPersonCharacter::SetupPlayerInput(InputManager* inputManager)
{
	_InputManager = inputManager;

	inputManager->AddAxisMapping("MoveForwardBackward", Keys::W, 1.0f);
	inputManager->AddAxisMapping("MoveForwardBackward", Keys::S, -1.0f);
	inputManager->AddAxisMapping("MoveLeftRight", Keys::A, 1.0f);
	inputManager->AddAxisMapping("MoveLeftRight", Keys::D, -1.0f);

	inputManager->AddAxisMapping("LookUpDown", Keys::MouseY, -0.1f);
	inputManager->AddAxisMapping("LookLeftRight", Keys::MouseX, -0.1f);


	inputManager->BindAxis("MoveForwardBackward", this, &FirstPersonCharacter::MoveForwardBackward);
	inputManager->BindAxis("MoveLeftRight", this, &FirstPersonCharacter::MoveLeftRight);
	inputManager->BindAxis("LookUpDown", this, &FirstPersonCharacter::LookUpDown);
	inputManager->BindAxis("LookLeftRight", this, &FirstPersonCharacter::LookLeftRight);

	_InputManager->AddActionMapping("Esc", Keys::Escape);
	_InputManager->BindAction("Esc", IE_Pressed, this, &FirstPersonCharacter::Escape);

	inputManager->SetMouseCapture(true);
}

void FirstPersonCharacter::BeginPlay()
{

}

static float moveSpeed = 0.5f;

void FirstPersonCharacter::MoveForwardBackward(float val)
{
	AddLocation(GetForwardVector() * val * -moveSpeed);
}

void FirstPersonCharacter::MoveLeftRight(float val)
{
	AddLocation(GetLeftVector() * val * -moveSpeed);
}

void FirstPersonCharacter::LookUpDown(float val)
{
	float& RotX = GetRotation().x;

	RotX += val;

	if (RotX <= -90.0f)
	{
		RotX = -90.0f;
	}
	else if (RotX >= 90.0f)
	{
		RotX = 90.0f;
	}
}

void FirstPersonCharacter::LookLeftRight(float val)
{
	float& RotY = GetRotation().y;

	RotY += val;

	if (RotY >= 360.0f || RotY <= -360.0f)
	{
		RotY = glm::mod(RotY, 360.0f);
	}
}

void FirstPersonCharacter::Escape()
{
	_InputManager->ToggleMouseCapture();
}

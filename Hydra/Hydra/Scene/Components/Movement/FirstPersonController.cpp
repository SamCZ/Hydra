#include "Hydra/Scene/Components/Movement/FirstPersonController.h"
#include "Hydra/Scene/Spatial.h"

#include "Hydra/Engine.h"

namespace Hydra
{
	void FirstPersonController::Start()
	{
		_Camera = Parent->GetComponent<Camera>();

		if (_Camera.expired())
		{
			Log("FirstPersonController", "Controller needs to be on gameobject where is Camera component !");
			return;
		}

		InputManagerPtr inputManager = Engine::GetInputManager();

		if (inputManager)
		{
			inputManager->BindAxis("MoveForwardBackward", this, &FirstPersonController::MoveForwardBackward);
			inputManager->BindAxis("MoveLeftRight", this, &FirstPersonController::MoveLeftRight);
			inputManager->BindAxis("LookUpDown", this, &FirstPersonController::LookUpDown);
			inputManager->BindAxis("LookLeftRight", this, &FirstPersonController::LookLeftRight);

			_MouseCaptured = inputManager->IsMouseCaptured();
		}
	}

	void FirstPersonController::Update()
	{
		if (!Enabled) return;

		InputManagerPtr inputManager = Engine::GetInputManager();
		_MouseCaptured = inputManager->IsMouseCaptured();
	}

	void FirstPersonController::MoveForwardBackward(float val)
	{
		if (!Enabled || !_MouseCaptured) return;

		if (auto camera = _Camera.lock())
		{
			camera->Parent->Position += camera->GetForward() * val * -0.01f;
		}
	}

	void FirstPersonController::MoveLeftRight(float val)
	{
		if (!Enabled || !_MouseCaptured) return;

		if (auto camera = _Camera.lock())
		{
			camera->Parent->Position += camera->GetLeft() * val * -0.01f;
		}
	}

	void FirstPersonController::LookUpDown(float val)
	{
		if (!Enabled || !_MouseCaptured) return;

		if (auto camera = _Camera.lock())
		{
			camera->Parent->Rotation.x += val;
		}
	}

	void FirstPersonController::LookLeftRight(float val)
	{
		if (!Enabled || !_MouseCaptured) return;

		if (auto camera = _Camera.lock())
		{
			camera->Parent->Rotation.y += val;
		}
	}
}
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
			float& RotX = camera->Parent->Rotation.x;

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
	}

	void FirstPersonController::LookLeftRight(float val)
	{
		if (!Enabled || !_MouseCaptured) return;

		if (auto camera = _Camera.lock())
		{
			float& RotY = camera->Parent->Rotation.y;

			RotY += val;

			if (RotY >= 360.0f || RotY <= -360.0f)
			{
				RotY = glm::mod(RotY, 360.0f);
			}
		}
	}
}
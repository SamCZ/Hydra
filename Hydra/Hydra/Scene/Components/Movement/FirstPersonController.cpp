#include "Hydra/Scene/Components/Movement/FirstPersonController.h"
#include "Hydra/Scene/Spatial.h"

namespace Hydra
{
	void FirstPersonController::Start()
	{
		_Camera = GameObject->GetComponent<Camera>();

		if (_Camera.expired())
		{
			Log("FirstPersonController", "Controller needs to be on gameobject where is Camera component !");
			return;
		}

		/*InputManagerPtr inputManager = Engine::GetInputManager();

		if (inputManager)
		{
			inputManager->BindAxis("MoveForwardBackward", this, &FirstPersonController::MoveForwardBackward);
			inputManager->BindAxis("MoveLeftRight", this, &FirstPersonController::MoveLeftRight);
			inputManager->BindAxis("LookUpDown", this, &FirstPersonController::LookUpDown);
			inputManager->BindAxis("LookLeftRight", this, &FirstPersonController::LookLeftRight);

			_MouseCaptured = inputManager->IsMouseCaptured();
		}*/
	}

	void FirstPersonController::Update()
	{
		if (!Enabled) return;

		//InputManagerPtr inputManager = Engine::GetInputManager();
		//_MouseCaptured = inputManager->IsMouseCaptured();
	}

	static float moveSpeed = 0.5f;

	void FirstPersonController::MoveForwardBackward(float val)
	{
		if (!Enabled || !_MouseCaptured) return;

		if (auto camera = _Camera.lock())
		{
			camera->GameObject->Position += camera->GetForward() * val * -moveSpeed;
		}
	}

	void FirstPersonController::MoveLeftRight(float val)
	{
		if (!Enabled || !_MouseCaptured) return;

		if (auto camera = _Camera.lock())
		{
			camera->GameObject->Position += camera->GetLeft() * val * -moveSpeed;
		}
	}

	void FirstPersonController::LookUpDown(float val)
	{
		if (!Enabled || !_MouseCaptured) return;

		if (auto camera = _Camera.lock())
		{
			float& RotX = camera->GameObject->Rotation.x;

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
			float& RotY = camera->GameObject->Rotation.y;

			RotY += val;

			if (RotY >= 360.0f || RotY <= -360.0f)
			{
				RotY = glm::mod(RotY, 360.0f);
			}
		}
	}
}
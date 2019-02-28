#pragma once

#include "Hydra/Scene/Component.h"
#include "Hydra/Scene/Components/Camera.h"

namespace Hydra
{
	class FirstPersonController : public Component
	{
	private:
		CameraWeakPtr _Camera;
		bool _MouseCaptured;
	public:
		virtual void Start() override;
		virtual void Update() override;

	private:
		void MoveForwardBackward(float val);
		void MoveLeftRight(float val);

		void LookUpDown(float val);
		void LookLeftRight(float val);
	};

	DEFINE_PTR(FirstPersonController)
}
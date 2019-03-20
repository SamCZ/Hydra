#pragma once

#include "Hydra/Core/SmartPointer.h"

namespace Hydra
{
	class Spatial;

	class Component
	{
	public:
		Spatial* GameObject;
		bool Enabled;

		Component();

		virtual void Start() = 0;
		virtual void Update() = 0;
	};

	DEFINE_PTR(Component)
}
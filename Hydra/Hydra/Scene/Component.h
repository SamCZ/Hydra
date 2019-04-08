#pragma once

#include "Hydra/Core/SmartPointer.h"

#include "Hydra/Physics/Collisons/Ray.h"
#include "Hydra/Physics/Collisons/CollisionResults.h"

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

		virtual int CollideWith(const Collidable& c, CollisionResults& results);
	};

	DEFINE_PTR(Component)
}
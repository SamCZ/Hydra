#pragma once

#include "Hydra/Core/Vector.h"

namespace Hydra
{
	class Transformable
	{
	public:
		Vector3 Position;
		Vector3 Rotation;
		Vector3 Scale;

		virtual Matrix4 GetModelMatrix();
	};
}
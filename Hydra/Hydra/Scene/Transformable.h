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

		static Vector3 GetRotationColumn(Matrix4& mat, int i);

		static Vector3 GetForward(Matrix4& mat);
		static Vector3 GetLeft(Matrix4& mat);
		static Vector3 GetUp(Matrix4& mat);
	};
}
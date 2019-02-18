#pragma once

#include "Hydra/Core/Vector.h"

namespace Hydra
{
	struct ColorRGBA
	{
		float r, g, b, a;

		inline Vector4 toVec4() const
		{
			return Vector4(r, g, b, a);
		}
		inline Vector3 toVec3() const
		{
			return Vector3(r, g, b);
		}
	};

	ColorRGBA MakeRGB(int r, int g, int b);
	ColorRGBA MakeRGBA(int r, int g, int b, int a);
	ColorRGBA MakeRGBf(float r, float g, float b);
	ColorRGBA MakeRGBAf(float r, float g, float b, float a);
	ColorRGBA MakeHEX(const char* hex);
}
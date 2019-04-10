#pragma once

#include "Hydra/Core/Library.h"
#include "Hydra/Core/Vector.h"

namespace Hydra
{
	struct HYDRA_EXPORT ColorRGBA
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

	HYDRA_EXPORT ColorRGBA MakeRGB(int r, int g, int b);
	HYDRA_EXPORT ColorRGBA MakeRGBA(int r, int g, int b, int a);
	HYDRA_EXPORT ColorRGBA MakeRGBf(float r, float g, float b);
	HYDRA_EXPORT ColorRGBA MakeRGBAf(float r, float g, float b, float a);
	HYDRA_EXPORT ColorRGBA MakeHEX(const char* hex);

	struct HYDRA_EXPORT Colors
	{
		static ColorRGBA White;
		static ColorRGBA Black;

		static ColorRGBA Red;
		static ColorRGBA Green;
		static ColorRGBA Blue;
	};
}
#pragma once

#include "Hydra/Core/Library.h"
#include "Hydra/Core/Vector.h"

struct HYDRA_API ColorRGBA
{
	static const ColorRGBA White;
	static const ColorRGBA Black;

	static const ColorRGBA Red;
	static const ColorRGBA Green;
	static const ColorRGBA Blue;

	float r, g, b, a;

	inline Vector4 toVec4() const
	{
		return Vector4(r, g, b, a);
	}
	inline Vector3 toVec3() const
	{
		return Vector3(r, g, b);
	}

	inline ColorRGBA operator+=(const ColorRGBA& other)
	{
		r += other.r;
		g += other.g;
		b += other.b;
		a += other.a;
		return *this;
	}

	inline ColorRGBA operator+(const ColorRGBA& other)
	{
		r += other.r;
		g += other.g;
		b += other.b;
		a += other.a;
		return *this;
	}
};

HYDRA_API ColorRGBA MakeRGB(int r, int g, int b);
HYDRA_API ColorRGBA MakeRGBA(int r, int g, int b, int a);
HYDRA_API ColorRGBA MakeRGBf(float r, float g, float b);
HYDRA_API ColorRGBA MakeRGBAf(float r, float g, float b, float a);
HYDRA_API ColorRGBA MakeHEX(const char* hex);
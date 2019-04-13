#pragma once

#include <random>
#include "Hydra/Core/Library.h"
#include "Hydra/Core/Vector.h"

namespace Hydra
{
	class HYDRA_API Random
	{
	private:
		unsigned int _Seed;
		std::mt19937 _RandomGenerator;
	public:
		Random(unsigned int seed);
		Random();

		int GetInt(int min, int max);
		float GetFloat(float min, float max);
		bool GetByPercent(float percent);
		Vector3 GetRandomUnitVector();
	};
}
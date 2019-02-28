#include "Hydra/Core/Random.h"


namespace Hydra
{
	Random::Random(unsigned int seed) : _Seed(seed)
	{
		
	}

	Random::Random() : _Seed(std::random_device()())
	{

	}

	int Random::GetInt(int min, int max)
	{
		_RandomGenerator.seed(_Seed++);

		std::uniform_int_distribution<std::mt19937::result_type> dist6(min, max);

		return dist6(_RandomGenerator);
	}

	float Random::GetFloat(float min, float max)
	{
		_RandomGenerator.seed(_Seed++);

		std::uniform_real_distribution<float> dist6(min, max);

		return dist6(_RandomGenerator);
	}

	bool Random::GetByPercent(float percent)
	{
		return GetFloat(0, 100) >= percent;
	}

	Vector3 Random::GetRandomUnitVector()
	{
		return glm::normalize(Vector3(GetFloat(0.0f, 1.0f), GetFloat(0.0f, 1.0f), GetFloat(0.0f, 1.0f)));
	}
}
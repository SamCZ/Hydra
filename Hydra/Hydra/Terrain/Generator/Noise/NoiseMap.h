#pragma once

#include "Hydra/Core/Vector.h"
#include "Hydra/Core/FastNoise.h"

#include "Hydra/Terrain/Generator/HeightMap.h"
#include "Hydra/Terrain/Generator/MeshSettings.h"

namespace Hydra
{
	class NoiseMap
	{
	public:
		static float Noise(FastNoise& generator, float x, float y, float scale, int octaves, float persistance, float lacunarity);

		static float* GenerateNoiseMap(int width, int height, int seed, float scale, int octaves, float persistance, float lacunarity);

		static HeightMap* GenerateHeightMap(const MeshSettings& meshSettings);
	};
}
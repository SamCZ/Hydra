#include "Hydra/Terrain/Generator/Noise/NoiseMap.h"
#include "Hydra/Core/Random.h"

#include "Hydra/Terrain/Erosion.h"

namespace Hydra
{
	float NoiseMap::Noise(FastNoise & generator, float x, float y, float scale, int octaves, float persistance, float lacunarity)
	{
		if (scale <= 0.0f)
		{
			scale = 0.00001f;
		}

		float amplitude = 1.0f;
		float frequency = 1.0f;
		float noiseHeight = 0.0f;

		for (int i = 0; i < octaves; i++)
		{
			float sampleX = x * scale * frequency;
			float sampleY = y * scale * frequency;

			float noiseval = generator.GetNoise(sampleX, sampleY) * 2.0 - 1.0;

			noiseHeight += noiseval * amplitude;

			amplitude *= persistance;
			frequency *= lacunarity;
		}

		return noiseHeight;
	}

	float* NoiseMap::GenerateNoiseMap(int width, int height, int seed, const Vector2& offset, float scale, int octaves, float persistance, float lacunarity)
	{
		float* map = new float[width * height];

		float minValue = 999999.999f;
		float maxValue = -999999.999f;

		FastNoise noise;
		noise.SetNoiseType(FastNoise::NoiseType::Perlin);

		Random prng = Random(seed);

		Vector2 rndOffset = Vector2(prng.GetFloat(-1000, 1000), prng.GetFloat(-1000, 1000)) + offset;

		rndOffset = offset;

		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				float noiseValue = Noise(noise, x + rndOffset.x, y + rndOffset.y, scale, octaves, persistance, lacunarity);

				map[x + y * width] = noiseValue * 0.1;

				minValue = glm::min(noiseValue, minValue);
				maxValue = glm::max(noiseValue, maxValue);
			}
		}

		// Normalize Local
		if (maxValue != minValue)
		{
			for (int i = 0; i < width * height; i++)
			{
				map[i] = (map[i] - minValue) / (maxValue - minValue);
			}
		}

		return map;
	}

	HeightMap* NoiseMap::GenerateHeightMap(const MeshSettings & meshSettings, const Vector2& offset)
	{
		int size = meshSettings.GetNumVertsPerLine();

		HeightMap* heightMap = new HeightMap();
		heightMap->Width = size;
		heightMap->Height = size;

		int numOctaves = 7;
		float persistence = .5f;
		float lacunarity = 2;
		float initialScale = 0.5f;

		heightMap->Data = GenerateNoiseMap(size, size, 0, offset, initialScale, numOctaves, persistence, lacunarity);

		int numErosionIterations = 2000;

		//Erosion erosion;
		//erosion.Erode(heightMap->Data, size, numErosionIterations);

		for (int i = 0; i < size * size; i++)
		{
			heightMap->Data[i] = heightMap->Data[i];
		}

		return heightMap;
	}
}
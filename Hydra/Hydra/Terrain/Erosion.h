#pragma once

#include "Hydra/Core/Random.h"

namespace Hydra
{
	struct ErosionOptions
	{
		int Seed;
		int ErosionRadius;
		float Inertia; // At zero, water will instantly change direction to flow downhill. At 1, water will never change direction. 
		float SedimentCapacityFactor; // Multiplier for how much sediment a droplet can carry
		float MinSedimentCapacity; // Used to prevent carry capacity getting too close to zero on flatter terrain
		float ErodeSpeed;
		float DepositSpeed;
		float EvaporateSpeed;
		float Gravity;
		int MaxDropletLifetime;

		ErosionOptions() 
			: Seed(1), ErosionRadius(3), Inertia(0.05f), SedimentCapacityFactor(4), MinSedimentCapacity(0.01f), ErodeSpeed(0.3f), DepositSpeed(0.3f), EvaporateSpeed(0.01f), Gravity(4), MaxDropletLifetime(30)
		{

		}
	};

	struct HeightAndGradient
	{
		float Height;
		float GradientX;
		float GradientY;
	};

	class Erosion
	{
	private:
		ErosionOptions _Options;
		int** _ErosionBrushIndices;
		float** _ErosionBrushWeights;
		int* _ErosionBrushCounts;
		Random _Random;
		int _MapSize;

		float _InitialWaterVolume = 1;
		float _InitialSpeed = 1;
	public:
		Erosion(const ErosionOptions& options);
		Erosion();
		~Erosion();

		void Erode(float* map, int mapSize, int numIterations = 1);

	private:
		void Initialize(int mapSize);
		void InitializeBrushIndices(int mapSize, int radius);

		HeightAndGradient CalculateHeightAndGradient(float* nodes, int mapSize, float posX, float posY);
	};
}
#include "Hydra/Terrain/Erosion.h"

namespace Hydra
{
	Erosion::Erosion(const ErosionOptions & options) : _Options(options), _Random(_Options.Seed), _ErosionBrushIndices(nullptr), _ErosionBrushWeights(nullptr)
	{
	}

	Erosion::Erosion() : Erosion(ErosionOptions())
	{
	}

	Erosion::~Erosion()
	{
		for (int i = 0; i < _MapSize * _MapSize; i++)
		{
			delete[] _ErosionBrushIndices[i];
			delete[] _ErosionBrushWeights[i];
		}

		delete[] _ErosionBrushIndices;
		delete[] _ErosionBrushWeights;
		delete[] _ErosionBrushCounts;
	}

	void Erosion::Erode(float* map, int mapSize, int numIterations)
	{
		Initialize(mapSize);

		ErosionOptions& options = _Options;

		for (int iteration = 0; iteration < numIterations; iteration++)
		{
			// Create water droplet at random point on map
			float posX = static_cast<float>(_Random.GetInt(0, mapSize - 1));
			float posY = static_cast<float>(_Random.GetInt(0, mapSize - 1));
			float dirX = 0;
			float dirY = 0;
			float speed = _InitialSpeed;
			float water = _InitialWaterVolume;
			float sediment = 0;

			for (int lifetime = 0; lifetime < options.MaxDropletLifetime; lifetime++)
			{
				int nodeX = (int)posX;
				int nodeY = (int)posY;
				int dropletIndex = nodeY * mapSize + nodeX;
				// Calculate droplet's offset inside the cell (0,0) = at NW node, (1,1) = at SE node
				float cellOffsetX = posX - nodeX;
				float cellOffsetY = posY - nodeY;

				// Calculate droplet's height and direction of flow with bilinear interpolation of surrounding heights
				HeightAndGradient heightAndGradient = CalculateHeightAndGradient(map, mapSize, posX, posY);

				// Update the droplet's direction and position (move position 1 unit regardless of speed)
				dirX = (dirX * options.Inertia - heightAndGradient.GradientX * (1 - options.Inertia));
				dirY = (dirY * options.Inertia - heightAndGradient.GradientY * (1 - options.Inertia));
				// Normalize direction
				float len = glm::sqrt(dirX * dirX + dirY * dirY);
				if (len != 0)
				{
					dirX /= len;
					dirY /= len;
				}
				posX += dirX;
				posY += dirY;

				// Stop simulating droplet if it's not moving or has flowed over edge of map
				if ((dirX == 0 && dirY == 0) || posX < 0 || posX >= mapSize - 1 || posY < 0 || posY >= mapSize - 1)
				{
					break;
				}

				// Find the droplet's new height and calculate the deltaHeight
				float newHeight = CalculateHeightAndGradient(map, mapSize, posX, posY).Height;
				float deltaHeight = newHeight - heightAndGradient.Height;
				
				// Calculate the droplet's sediment capacity (higher when moving fast down a slope and contains lots of water)
				float sedimentCapacity = glm::max(-deltaHeight * speed * water * options.SedimentCapacityFactor, options.MinSedimentCapacity);

				// If carrying more sediment than capacity, or if flowing uphill:
				if (sediment > sedimentCapacity || deltaHeight > 0)
				{
					// If moving uphill (deltaHeight > 0) try fill up to the current height, otherwise deposit a fraction of the excess sediment
					float amountToDeposit = (deltaHeight > 0) ? glm::min(deltaHeight, sediment) : (sediment - sedimentCapacity) * options.DepositSpeed;
					sediment -= amountToDeposit;

					// Add the sediment to the four nodes of the current cell using bilinear interpolation
					// Deposition is not distributed over a radius (like erosion) so that it can fill small pits
					map[dropletIndex] += amountToDeposit * (1 - cellOffsetX) * (1 - cellOffsetY);
					map[dropletIndex + 1] += amountToDeposit * cellOffsetX * (1 - cellOffsetY);
					map[dropletIndex + mapSize] += amountToDeposit * (1 - cellOffsetX) * cellOffsetY;
					map[dropletIndex + mapSize + 1] += amountToDeposit * cellOffsetX * cellOffsetY;

				}
				else
				{
					// Erode a fraction of the droplet's current carry capacity.
					// Clamp the erosion to the change in height so that it doesn't dig a hole in the terrain behind the droplet
					float amountToErode = glm::min((sedimentCapacity - sediment) * options.ErodeSpeed, -deltaHeight);

					// Use erosion brush to erode from all nodes inside the droplet's erosion radius
					for (int brushPointIndex = 0; brushPointIndex < _ErosionBrushCounts[dropletIndex]; brushPointIndex++)
					{
						int nodeIndex = _ErosionBrushIndices[dropletIndex][brushPointIndex];
						float weighedErodeAmount = amountToErode * _ErosionBrushWeights[dropletIndex][brushPointIndex];
						float deltaSediment = (map[nodeIndex] < weighedErodeAmount) ? map[nodeIndex] : weighedErodeAmount;
						map[nodeIndex] -= deltaSediment;
						sediment += deltaSediment;
					}
				}

				// Update droplet's speed and water content
				speed = glm::sqrt(speed * speed + deltaHeight * options.Gravity);
				water *= (1 - options.EvaporateSpeed);
			}
		}

	}

	HeightAndGradient Erosion::CalculateHeightAndGradient(float* nodes, int mapSize, float posX, float posY)
	{
		int coordX = (int)posX;
		int coordY = (int)posY;

		// Calculate droplet's offset inside the cell (0,0) = at NW node, (1,1) = at SE node
		float x = posX - coordX;
		float y = posY - coordY;

		// Calculate heights of the four nodes of the droplet's cell
		int nodeIndexNW = coordY * mapSize + coordX;
		float heightNW = nodes[nodeIndexNW];
		float heightNE = nodes[nodeIndexNW + 1];
		float heightSW = nodes[nodeIndexNW + mapSize];
		float heightSE = nodes[nodeIndexNW + mapSize + 1];

		// Calculate droplet's direction of flow with bilinear interpolation of height difference along the edges
		float gradientX = (heightNE - heightNW) * (1 - y) + (heightSE - heightSW) * y;
		float gradientY = (heightSW - heightNW) * (1 - x) + (heightSE - heightNE) * x;

		// Calculate height with bilinear interpolation of the heights of the nodes of the cell
		float height = heightNW * (1 - x) * (1 - y) + heightNE * x * (1 - y) + heightSW * (1 - x) * y + heightSE * x * y;

		return { height, gradientX, gradientY };
	}

	void Erosion::Initialize(int mapSize)
	{
		if (_ErosionBrushIndices == nullptr)
		{
			_MapSize = mapSize;
			InitializeBrushIndices(mapSize, _Options.ErosionRadius);
		}
	}

	void Erosion::InitializeBrushIndices(int mapSize, int radius)
	{
		int size = mapSize * mapSize;

		_ErosionBrushIndices = new int*[size];
		_ErosionBrushWeights = new float*[size];
		_ErosionBrushCounts = new int[size];

		int* xOffsets = new int[radius * radius * 4];
		int* yOffsets = new int[radius * radius * 4];
		float* weights = new float[radius * radius * 4];
		float weightSum = 0;
		int addIndex = 0;

		for (int i = 0; i < size; i++)
		{
			int centreX = i % mapSize;
			int centreY = i / mapSize;

			if (centreY <= radius || centreY >= mapSize - radius || centreX <= radius + 1 || centreX >= mapSize - radius)
			{
				weightSum = 0;
				addIndex = 0;
				for (int y = -radius; y <= radius; y++)
				{
					for (int x = -radius; x <= radius; x++)
					{
						float sqrDst = static_cast<float>(x * x + y * y);
						if (sqrDst < radius * radius)
						{
							int coordX = centreX + x;
							int coordY = centreY + y;

							if (coordX >= 0 && coordX < mapSize && coordY >= 0 && coordY < mapSize)
							{
								float weight = 1 - glm::sqrt(sqrDst) / radius;
								weightSum += weight;
								weights[addIndex] = weight;
								xOffsets[addIndex] = x;
								yOffsets[addIndex] = y;
								addIndex++;
							}
						}
					}
				}
			}

			int numEntries = addIndex;
			_ErosionBrushCounts[i] = numEntries;
			_ErosionBrushIndices[i] = new int[numEntries];
			_ErosionBrushWeights[i] = new float[numEntries];

			for (int j = 0; j < numEntries; j++)
			{
				_ErosionBrushIndices[i][j] = (yOffsets[j] + centreY) * mapSize + xOffsets[j] + centreX;
				_ErosionBrushWeights[i][j] = weights[j] / weightSum;
			}

		}

		delete[] xOffsets;
		delete[] yOffsets;
		delete[] weights;
	}
}
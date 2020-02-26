#include "Chunk.h"

#include <iostream>

#include "Hydra/Core/Container.h"
#include "Hydra/Core/Vector.h"
#include "BlockType.h"

#include <algorithm>

Chunk::Chunk(int x, int z) : m_X(x), m_Z(z)
{
	m_Blocks = new Block[ChunkTall * ChunkWide * ChunkDepth];
	std::fill(m_Blocks, m_Blocks + (ChunkTall * ChunkWide * ChunkDepth), AirBlock);
}

Chunk::~Chunk()
{
	delete[] m_Blocks;
}

float distanceSquared(int x1, int y1, int z1, int x2, int y2, int z2)
{
	return glm::sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1) + (z2 - z1) * (z2 - z1));
}

void Chunk::PropagateLight(int ox, int oy, int oz, int x, int y, int z, uint8_t lightValue, uint8_t lightStage)
{
	if (!IsInside(x, y, z))
	{
		return;
	}

	Block& block = GetBlock(x, y, z);

	if (block.LightStage != lightStage && block.LightStage != 0)
	{
		return;
	}

	if (!IsAir(block))
	{
		return;
	}

	float distance = distanceSquared(ox, oy, oz, x, y, z);

	float lightTravelDistance = 10;

	if (distance >= lightTravelDistance)
	{
		distance = lightTravelDistance;
	}

	float distanceVal = 1.0f - (distance / lightTravelDistance);

	if (distanceVal < 0.15f)
	{
		distanceVal = 0.0f;
	}

	if (distanceVal > 1.0f)
	{
		distanceVal = 1.0f;
	}

	if (lightStage > 1 && block.LightLevel > 0)
	{
		block.LightLevel = glm::max<float>(block.LightLevel, floor((float)lightValue * distanceVal));
	}
	else
	{
		block.LightLevel = floor((float)lightValue * distanceVal);
	}

	block.LightStage++;

	if (block.LightLevel <= 0)
	{
		return;
	}

	PropagateLight(ox, oy, oz, x, y + 1, z, lightValue, lightStage);
	PropagateLight(ox, oy, oz, x, y - 1, z, lightValue, lightStage);
	PropagateLight(ox, oy, oz, x + 1, y, z, lightValue, lightStage);
	PropagateLight(ox, oy, oz, x - 1, y, z, lightValue, lightStage);
	PropagateLight(ox, oy, oz, x, y, z + 1, lightValue, lightStage);
	PropagateLight(ox, oy, oz, x, y, z - 1, lightValue, lightStage);
}

void Chunk::UpdateLighting()
{
	for (int x = 0; x < Chunk::ChunkWide; x++)
	{
		for (int z = 0; z < Chunk::ChunkDepth; z++)
		{
			for (int y = 0; y < Chunk::ChunkTall; y++)
			{
				Block& block = GetBlock(x, y, z);

				if (!IsAir(block))
				{
					continue;
				}

				block.LightStage = 0;
				block.LightLevel = 0;
			}
		}
	}

	uint8_t lightStage = 1;

	for (int x = 0; x < Chunk::ChunkWide; x++)
	{
		for (int z = 0; z < Chunk::ChunkDepth; z++)
		{
			for (int y = 0; y < Chunk::ChunkTall; y++)
			{
				Block& block = GetBlock(x, y, z);

				if (IsAir(block))
				{
					continue;
				}

				if (block.LightLevel > 0)
				{
					PropagateLight(x, y, z, x, y + 1, z, block.LightLevel, lightStage);
					PropagateLight(x, y, z, x, y - 1, z, block.LightLevel, lightStage);
					PropagateLight(x, y, z, x + 1, y, z, block.LightLevel, lightStage);
					PropagateLight(x, y, z, x - 1, y, z, block.LightLevel, lightStage);
					PropagateLight(x, y, z, x, y, z + 1, block.LightLevel, lightStage);
					PropagateLight(x, y, z, x, y, z - 1, block.LightLevel, lightStage);

					lightStage++;
				}
			}
		}
	}
}

bool Chunk::IsInside(int x, int y, int z)
{
	return !(x < 0 || x > Chunk::ChunkWide - 1 || y < 0 || y > Chunk::ChunkTall - 1 || z < 0 || z > Chunk::ChunkDepth - 1);
}

Block & Chunk::GetBlock(int x, int y, int z)
{
	int index = (x * Chunk::ChunkDepth + z) * Chunk::ChunkTall + y;

	if (x < 0 || x > Chunk::ChunkWide - 1 || y < 0 || y > Chunk::ChunkTall - 1 || z < 0 || z > Chunk::ChunkDepth - 1)
	{
		return AirBlock;
	}

	return m_Blocks[index];
}

void Chunk::SetBlock(int x, int y, int z, const Block & block)
{
	int index = (x * Chunk::ChunkDepth + z) * Chunk::ChunkTall + y;

	if (x < 0 || x > Chunk::ChunkWide - 1 || y < 0 || y > Chunk::ChunkTall - 1 || z < 0 || z > Chunk::ChunkDepth - 1)
	{
		return;
	}

	m_Blocks[index] = block;
}

void Chunk::SetBlock(int x, int y, int z, uint32_t blockId)
{
	if (BlockType::Types.find(blockId) != BlockType::Types.end())
	{
		SetBlock(x, y, z, Block(BlockType::Types[blockId]));
	}
	else
	{
		SetBlock(x, y, z, Block(AirBlock));
	}
}

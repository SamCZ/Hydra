#include "Chunk.h"

#include <iostream>

#include "Hydra/Core/Container.h"
#include "Hydra/Core/Vector.h"

#include <algorithm>

Chunk::Chunk()
{
	m_Blocks = new Block[ChunkTall * ChunkWide * ChunkDepth];
	std::fill(m_Blocks, m_Blocks + (ChunkTall * ChunkWide * ChunkDepth), AirBlock);
}

Chunk::~Chunk()
{
	delete[] m_Blocks;
}

float distanceSquared(int x1, int y1, int z1, int x2, int y2, int z2) {
	return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1) + (z2 - z1) * (z2 - z1));
}

void PropagateLight(List<Vector3i>& checkedList, Chunk& chunk, int ox, int oy, int oz, int x, int y, int z, uint8_t lightValue) {
	if (Find(checkedList, { x, y, z }) != checkedList.end()) {
		return;
	}

	Block& block = chunk.GetBlock(x, y, z);

	if (!IsAir(block)) {
		return;
	}

	if (lightValue <= 0) {
		return;
	}

	float distance = distanceSquared(ox, oy, oz, x, y, z);

	int lightTravelDistance = 10;

	if (distance >= lightTravelDistance) {
		distance = lightTravelDistance;
	}

	float distanceVal = 1.0f / distance;

	if (distanceVal < 0.15f) {
		distanceVal = 0.0f;
	}

	block.LightLevel = floor((float)lightValue * distanceVal);

	if (block.LightLevel <= 0) {
		return;
	}

	checkedList.push_back({ x, y ,z });

	PropagateLight(checkedList, chunk, ox, oy, oz, x, y + 1, z, lightValue);
	PropagateLight(checkedList, chunk, ox, oy, oz, x, y - 1, z, lightValue);
	PropagateLight(checkedList, chunk, ox, oy, oz, x + 1, y, z, lightValue);
	PropagateLight(checkedList, chunk, ox, oy, oz, x - 1, y, z, lightValue);
	PropagateLight(checkedList, chunk, ox, oy, oz, x, y, z + 1, lightValue);
	PropagateLight(checkedList, chunk, ox, oy, oz, x, y, z - 1, lightValue);
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

				List<Vector3i> checkedPropagationList;

				if (IsAir(block)) {
					continue;
				}

				if (block.LightLevel > 0) {
					PropagateLight(checkedPropagationList, *this, x, y, z, x, y + 1, z, block.LightLevel);
					//PropagateLight(checkedPropagationList, *this, x, y - 1, z, block.LightLevel);
					//PropagateLight(checkedPropagationList, *this, x + 1, y, z, block.LightLevel);
					//PropagateLight(checkedPropagationList, *this, x - 1, y, z, block.LightLevel);
					//PropagateLight(checkedPropagationList, *this, x, y, z + 1, block.LightLevel);
					//PropagateLight(checkedPropagationList, *this, x, y, z - 1, block.LightLevel);
					return;
				}
			}
		}
	}
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

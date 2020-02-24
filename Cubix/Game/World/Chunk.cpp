#include "Chunk.h"

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

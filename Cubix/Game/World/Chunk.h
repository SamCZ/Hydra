#pragma once

#include "Block.h"

class Chunk
{
public:
	static constexpr int ChunkTall = 64;
	static constexpr int ChunkWide = 32;
	static constexpr int ChunkDepth = 32;
private:
	Block* m_Blocks;
public:
	Chunk();
	~Chunk();

	void UpdateLighting();

	Block& GetBlock(int x, int y, int z);
	void SetBlock(int x, int y, int z, const Block& block);
};
#pragma once

#include "Block.h"

class Chunk
{
public:
	static constexpr int ChunkTall = 128;
	static constexpr int ChunkWide = 256;
	static constexpr int ChunkDepth = 256;
private:
	Block* m_Blocks;
public:
	Chunk();
	~Chunk();

	Block& GetBlock(int x, int y, int z);
	void SetBlock(int x, int y, int z, const Block& block);
};
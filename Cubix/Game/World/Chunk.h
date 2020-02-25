#pragma once

#include "Block.h"

class Chunk
{
public:
	static constexpr int ChunkTall = 64;
	static constexpr int ChunkWide = 16;
	static constexpr int ChunkDepth = 16;
private:
	Block* m_Blocks;
public:
	Chunk();
	~Chunk();

	void UpdateLighting();

	bool IsInside(int x, int y, int z);

	Block& GetBlock(int x, int y, int z);
	void SetBlock(int x, int y, int z, const Block& block);

private:
	void PropagateLight(int ox, int oy, int oz, int x, int y, int z, uint8_t lightValue, uint8_t lightStage);
};
#pragma once

#include "Block.h"

class Chunk
{
public:
	static constexpr int ChunkTall = 16;
	static constexpr int ChunkWide = 16;
	static constexpr int ChunkDepth = 16;
private:
	Block* m_Blocks;
	int m_X;
	int m_Z;
public:
	Chunk(int x, int z);
	~Chunk();

	void UpdateLighting();

	bool IsInside(int x, int y, int z);

	Block& GetBlock(int x, int y, int z);
	void SetBlock(int x, int y, int z, const Block& block);
	void SetBlock(int x, int y, int z, uint32_t blockId);

private:
	void PropagateLight(int ox, int oy, int oz, int x, int y, int z, uint8_t lightValue, uint8_t lightStage);
};
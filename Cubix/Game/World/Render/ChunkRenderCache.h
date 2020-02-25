#pragma once

#include "Game/World/World.h"
#include "Game/World/Chunk.h"

class ChunkRenderCache
{
public:
	static constexpr int CENTER = 0;
	static constexpr int LEFT = 1;
	static constexpr int RIGHT = 2;
	static constexpr int FRONT = 3;
	static constexpr int BACK = 4;
private:
	Chunk* m_CachedChunks[5];
	int m_ChunkY;
public:
	ChunkRenderCache(WorldPtr world, int chunkX, int chunkY, int chunkZ);
	~ChunkRenderCache();

	Block& GetBlock(int x, int y, int z);
};
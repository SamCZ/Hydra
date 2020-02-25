#pragma once

#include "Hydra/Core/SmartPointer.h"
#include "Game/World/Chunk.h"
#include "Game/World/BlockType.h"

class World
{
private:

public:
	World();
	~World();

	Chunk* GetChunk(int x, int z);
	Chunk* GetChunkFromWorldCoords(int x, int z);

	Block& GetBlock(int x, int y, int z);
	void SetBlock(int x, int y, int z, const Block& block);
};

DEFINE_PTR(World)
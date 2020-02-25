#include "World.h"

World::World()
{
}

World::~World()
{
}

Chunk* World::GetChunk(int x, int z)
{
	return nullptr;
}

Chunk* World::GetChunkFromWorldCoords(int x, int z)
{
	return GetChunk(x >> 4, z >> 4);
}

Block& World::GetBlock(int x, int y, int z)
{
	Chunk* chunk = GetChunkFromWorldCoords(x, z);

	return chunk->GetBlock(x & Chunk::ChunkWide, y, z & Chunk::ChunkDepth);
}

void World::SetBlock(int x, int y, int z, const Block& block)
{
}

#include "ChunkRenderCache.h"
#include "ChunkRenderer.h"

ChunkRenderCache::ChunkRenderCache(WorldPtr world, int chunkX, int chunkY, int chunkZ) : m_ChunkY(chunkY)
{
    m_CachedChunks[CENTER] = world->GetChunk(chunkX, chunkZ);
    m_CachedChunks[LEFT] = world->GetChunk(chunkX - 1, chunkZ);
    m_CachedChunks[RIGHT] = world->GetChunk(chunkX + 1, chunkZ);
    m_CachedChunks[FRONT] = world->GetChunk(chunkX, chunkZ - 1);
    m_CachedChunks[BACK] = world->GetChunk(chunkX, chunkZ + 1);
}

ChunkRenderCache::~ChunkRenderCache()
{

}

Block& ChunkRenderCache::GetBlock(int x, int y, int z)
{
    Chunk* chunk = nullptr;

    if (x < 0)
    {
        chunk = m_CachedChunks[LEFT];
        x = 16 + x;
    }
    else if (x > 15)
    {
        chunk = m_CachedChunks[RIGHT];
        x = 16 - x;
    }
    else if (z < 0)
    {
        chunk = m_CachedChunks[FRONT];
        z = 16 + z;
    }
    else if (z > 15)
    {
        chunk = m_CachedChunks[BACK];
        z = 16 - z;
    }
    else
    {
        chunk = m_CachedChunks[CENTER];
    }

    return chunk->GetBlock(x, m_ChunkY * ChunkRenderer::ChunkRenderSize + y, z);
}
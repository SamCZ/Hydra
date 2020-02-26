#include "World.h"

#include "Hydra/Core/FastNoise.h"

World::World()
{
}

World::~World()
{
	ITER(m_GeneratedChunks, it)
	{
		delete it->second;
	}
}

void PopulateChunk(Chunk* chunk, int chunkX, int chunkZ)
{
	FastNoise noise;
	noise.SetNoiseType(FastNoise::PerlinFractal);

	for (int x = 0; x < Chunk::ChunkWide; x++)
	{
		for (int z = 0; z < Chunk::ChunkDepth; z++)
		{
			float xx = chunkX * Chunk::ChunkWide + x;
			float zz = chunkZ * Chunk::ChunkDepth + z;

			float scale = 1.0f;
			float val = (noise.GetNoise(xx * scale, zz * scale) + 1.0f) / 2.0f;
			int maxY = round(val * (float)Chunk::ChunkTall);

			float dirtVal = (noise.GetNoise(xx * 10.0f, zz * 10.0f) + 1.0f) / 2.0f;

			for (int y = 0; y < maxY; y++)
			{
				if ((maxY - y) < round((1.0 - dirtVal) * 5.0f))
				{
					chunk->SetBlock(x, y, z, 1);
				}
				else
				{
					chunk->SetBlock(x, y, z, 2);
				}
			}

			chunk->SetBlock(x, maxY, z, 3);

			if (x == 5 && z == 5)
			{
				chunk->SetBlock(x, maxY + 1, z, 4);
				chunk->GetBlock(x, maxY + 1, z).LightLevel = 255;

				chunk->SetBlock(x, maxY + 3, z, 3);
			}

			if (x == 16 && z == 16)
			{
				chunk->SetBlock(x, maxY + 1, z, 4);
				chunk->GetBlock(x, maxY + 1, z).LightLevel = 255;
			}

			/*for (int y = 0; y < Chunk::ChunkTall; y++)
			{
				float val = (noise.GetNoise(x * scale, y * scale, z * scale) + 1.0f) / 2.0f;
				//val *= 1.0f - pow((float)y / (float)Chunk::ChunkTall, 1);

				int state = round(val * (float)Chunk::ChunkTall);

				if (state > y + 1) {
					chunk->SetBlock(x, y, z, Block(DirtType));
				} else if (state > y)
				{
					chunk->SetBlock(x, y, z, Block(GrassType));
				}
			}*/
		}
	}

	chunk->UpdateLighting();
}

Chunk* World::GetChunk(int x, int z)
{
	Vector2i cpos = Vector2i(x, z);

	if (m_GeneratedChunks.find(cpos) != m_GeneratedChunks.end())
	{
		return m_GeneratedChunks[cpos];
	}
	else
	{
		Chunk* chunk = new Chunk(x, z);
		PopulateChunk(chunk, x, z);
		m_GeneratedChunks[cpos] = chunk;
		return chunk;
	}
}

Chunk* World::GetChunkFromWorldCoords(int x, int z)
{
	return GetChunk(x >> 4, z >> 4);
}

Block& World::GetBlock(int x, int y, int z)
{
	Chunk* chunk = GetChunkFromWorldCoords(x, z);

	return chunk->GetBlock(x & (Chunk::ChunkWide - 1), y, z & (Chunk::ChunkDepth - 1));
}

Block& World::GetBlock(int chunkXrelative, int chunkZrelative, int x, int y, int z)
{
	return GetBlock((chunkXrelative * Chunk::ChunkWide) + x, y, (chunkZrelative * Chunk::ChunkDepth) + z);
}

void World::SetBlock(int x, int y, int z, const Block& block)
{
	Chunk* chunk = GetChunkFromWorldCoords(x, z);

	chunk->SetBlock(x & (Chunk::ChunkWide - 1), y, z & (Chunk::ChunkDepth - 1), block);
}

#include "CubixGameMode.h"

CubixGameMode::CubixGameMode()
{
	m_World = MakeShared<World>();

	BlockType* StoneType = new BlockType(1);
	StoneType->Texture[0].x = 1;

	BlockType* DirtType = new BlockType(2);
	DirtType->Texture[0].x = 2;

	BlockType* GrassType = new BlockType(3);
	GrassType->HasMultipleTextureFaces = true;
	GrassType->Texture[(int)BlockFace::Bottom].x = 2;
	GrassType->Texture[(int)BlockFace::Left].x = 3;
	GrassType->Texture[(int)BlockFace::Right].x = 3;
	GrassType->Texture[(int)BlockFace::Front].x = 3;
	GrassType->Texture[(int)BlockFace::Back].x = 3;

	BlockType* LightType = new BlockType(4);
	LightType->Texture[0].y = 4;

	LOG("Pre loading chunks...");

	for (int x = -3; x <= 3; x++)
	{
		for (int z = -3; z <= 3; z++)
		{
			m_World->GetChunk(x, z);
		}
	}
	LOG("Done loading chunks!");
}

CubixGameMode::~CubixGameMode()
{
	ITER(BlockType::Types, it)
	{
		delete it->second;
	}
}

WorldPtr CubixGameMode::GetWorld()
{
	return m_World;
}

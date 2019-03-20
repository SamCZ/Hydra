#include "Hydra/Terrain/Terrain.h"
#include "Hydra/Terrain/TerrainNode.h"

namespace Hydra
{
	Terrain::Terrain(SharedPtr<Material> material) : Spatial("Terrain")
	{
		int terrainSize = 1 << 8;
		int halfSize = terrainSize / 2;

		AddChild(MakeShared<TerrainNode>(material, Vector2i(0, 0), halfSize, 0));
		AddChild(MakeShared<TerrainNode>(material, Vector2i(0, halfSize), halfSize, 0));
		AddChild(MakeShared<TerrainNode>(material, Vector2i(halfSize, halfSize), halfSize, 0));
		AddChild(MakeShared<TerrainNode>(material, Vector2i(halfSize, 0), halfSize, 0));
	}

	Terrain::~Terrain()
	{
	}

	void Terrain::Start()
	{
		Spatial::Start();
	}

	void Terrain::Update()
	{
		Spatial::Update();
	}
}
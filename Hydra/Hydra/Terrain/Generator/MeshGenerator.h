#pragma once

#include "Hydra/Terrain/Generator/MeshData.h"
#include "Hydra/Terrain/Generator/MeshSettings.h"
#include "Hydra/Terrain/Generator/HeightMap.h"

namespace Hydra
{
	class MeshGenerator
	{
	public:
		static MeshData* GenerateTerrainMesh(HeightMap* heightMapDef, const MeshSettings& meshSettings, int levelOfDetail);
	};
}
#pragma once

#include "Hydra/Core/Common.h"
#include "Hydra/Scene/Spatial.h"
#include "Hydra/Terrain/Voxel/World/VoxelWorld.h"

namespace Hydra
{
	class VoxelTerrain : public Spatial
	{
	private:
		List<Vector2i> _Chunks;

	public:
		VoxelTerrain();
		~VoxelTerrain();

		virtual void Start();
		virtual void Update();
	};

	DEFINE_PTR(VoxelTerrain)
}
#pragma once

namespace Hydra
{
	static int MeshSettings_SupportedChunkSizes[]{ 48,72,96,120,144,168,192,216,240 };

	struct MeshSettings
	{
		float MeshScale;
		bool UseFlatShading;
		int ChunkSizeIndex;
		int FlatshadedChunkSizeIndex;
	};
}
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
		bool UseGPUTexturing;

		inline int GetNumVertsPerLine() const
		{
			return MeshSettings_SupportedChunkSizes[(UseFlatShading) ? FlatshadedChunkSizeIndex : ChunkSizeIndex] + 5;
		}

		inline float GetMeshWorldSize() const
		{
			return (GetNumVertsPerLine() - 3) * MeshScale;
		}
	};
}
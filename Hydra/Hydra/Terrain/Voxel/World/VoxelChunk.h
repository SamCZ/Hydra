#pragma once
namespace Hydra
{
	class VoxelChunk
	{
	private:
		int _Width;
		int _Height;
		int _Depth;

		int* _VoxelData;
		int* _Metadata;
		float* _MaxHeightData;
	public:
		VoxelChunk(int width, int height, int depth);
		~VoxelChunk();

		int GetVoxel(int x, int y, int z);
		void SetVoxel(int x, int y, int z, int data);

		int GetMeta(int x, int y, int z);
		void SetMeta(int x, int y, int z, int meta);

		float GetMaxHeight(int x, int z);

		int* GetVoxelData();
		int* GetMetaData();

		int GetWidth();
		int GetHeight();
		int GetDepth();

	private:
		void GenerateHeightData();

		int GetIndex(int x, int y, int z);
		bool InRange(int x, int y, int z);
	};
}
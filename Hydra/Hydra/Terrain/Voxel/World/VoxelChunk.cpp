#include "Hydra/Terrain/Voxel/World/VoxelChunk.h"

namespace Hydra
{
	VoxelChunk::VoxelChunk(int width, int height, int depth) : _Width(width), _Height(height), _Depth(depth)
	{
		_VoxelData = new float[width * height * depth];
		_Metadata = new int[width * height * depth];
		_MaxHeightData = new float[width * height];
	}

	VoxelChunk::~VoxelChunk()
	{
		delete[] _VoxelData;
		delete[] _Metadata;
		delete[] _MaxHeightData;
	}

	float VoxelChunk::GetVoxel(int x, int y, int z)
	{
		if (InRange(x, y, z))
		{
			return _VoxelData[GetIndex(x, y, z)];
		}

		return 0.0f;
	}

	void VoxelChunk::SetVoxel(int x, int y, int z, float data)
	{
		if (InRange(x, y, z))
		{
			_VoxelData[GetIndex(x, y, z)] = data;
		}
	}

	int VoxelChunk::GetMeta(int x, int y, int z)
	{
		if (InRange(x, y, z))
		{
			return _Metadata[GetIndex(x, y, z)];
		}

		return 0;
	}

	void VoxelChunk::SetMeta(int x, int y, int z, int meta)
	{
		if (InRange(x, y, z))
		{
			_Metadata[GetIndex(x, y, z)] = meta;
		}
	}

	float VoxelChunk::GetMaxHeight(int x, int z)
	{
		return _MaxHeightData[x + z * _Width];
	}

	float* VoxelChunk::GetVoxelData()
	{
		return _VoxelData;
	}

	int* VoxelChunk::GetMetaData()
	{
		return _Metadata;
	}

	int VoxelChunk::GetWidth()
	{
		return _Width;
	}

	int VoxelChunk::GetHeight()
	{
		return _Height;
	}

	int VoxelChunk::GetDepth()
	{
		return _Depth;
	}

	void VoxelChunk::GenerateHeightData()
	{
		//TODO: Generate height data in VoxelChunk
	}

	int VoxelChunk::GetIndex(int x, int y, int z)
	{
		return x + y * _Width + z * _Width * _Height;
	}

	bool VoxelChunk::InRange(int x, int y, int z)
	{
		return x >= 0 && y >= 0 && z >= 0 && x <= _Width-1 && y <= _Height && z <= _Depth-1;
	}
}
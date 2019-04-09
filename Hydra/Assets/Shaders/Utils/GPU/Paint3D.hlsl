#pragma hydra cmp:CSMain

cbuffer Params
{
	float3 _ArrPos;
};

RWStructuredBuffer<float> _Voxels;

int GetIndex(int3 pos)
{
	return pos.x + pos.y * 64 + pos.z * 64 * 64;
}

[numthreads(16, 16, 1)]
void CSMain(uint3 groupId : SV_GroupID, uint3 groupThreadId : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex)
{

	int3 pos = (int3)_ArrPos;

	int size = 3;

	for (int x = -size; x <= size; x++)
	{
		for (int y = -size; y <= size; y++)
		{
			for (int z = -size; z <= size; z++)
			{

				_Voxels[GetIndex(int3(pos.x + x, pos.y + y, pos.z + z))] = 1;

				/*float dist = clamp(abs(distance(int3(x, y, z), int3(0, 0, 0))) / 5, 0, 1);

				if (dist > 0.5)
				{
					_Voxels[GetIndex(int3(pos.x + x, pos.y + y, pos.z + z))] = 0;
				}
				else
				{
					_Voxels[GetIndex(int3(pos.x + x, pos.y + y, pos.z + z))] = 1;
				}*/
			}
		}
	}
}
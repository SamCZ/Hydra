#pragma hydra cmp:CSMain

cbuffer Params
{
	float3 _ArrPos;
	float _Mode;
	float _Size;
};

RWStructuredBuffer<float> _Voxels;

int GetIndex(int3 pos)
{
	return pos.x + pos.y * 64 + pos.z * 64 * 64;
}

bool CheckIndex(int3 pos)
{
	if (pos.x < 0 || pos.y < 0 || pos.z < 0 || pos.x >= 63 || pos.y >= 63 || pos.z >= 63)
	{
		return false;
	}

	return true;
}

[numthreads(1, 1, 1)]
void CSMain(uint3 groupId : SV_GroupID, uint3 groupThreadId : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex)
{

	int3 pos = (int3)_ArrPos;

	int size = 32;

	for (int x = -size; x <= size; x++)
	{
		for (int y = -size; y <= size; y++)
		{
			for (int z = -size; z <= size; z++)
			{
				int3 index = int3(pos.x + x, pos.y + y, pos.z + z);

				if (!CheckIndex(index)) continue;

				int dist = abs(distance(int3(x, y, z), int3(0, 0, 0)));

				if (dist < _Size) {
					_Voxels[GetIndex(index)] = _Mode;
				}

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
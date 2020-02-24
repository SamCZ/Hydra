#pragma hydra cmp:CSMain

struct VoxelBuffer
{
	float4 Position;
	float3 Normal;
};

struct CollisionBuffer
{
	int Result;
	float3 V0;
	float3 V1;
	float3 V2;
	float3 Normal;
};

cbuffer Params
{
	float3 _Position;
	float _Radius;

	float3 _Origin;
	float3 _Direction;
};

StructuredBuffer<VoxelBuffer> _Buffer;
RWStructuredBuffer<CollisionBuffer> _Collisions;
StructuredBuffer<float> _Voxels;

int GetIndex(int x, int y, int z)
{
	//y = 32 - y;

	return x + y * 64 + z * 64 * 64;
}

[numthreads(1, 1, 1)]
void CSMain(uint3 groupId : SV_GroupID, uint3 groupThreadId : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex)
{
	_Collisions[0].Result = 0;

	float stepSize = 0.01;

	for (int i = 0; i < 5000; i++)
	{
		float3 pos = _Origin + (i * _Direction);

		int3 posInt = (int3)floor(pos);

		posInt += int3(32, 0, 32);

		if (posInt.x >= 0 && posInt.y >= 0 && posInt.z >= 0 && posInt.x <= 63 && posInt.y <= 63 && posInt.z <= 63)
		{
			if (_Voxels[GetIndex(posInt.x, posInt.y, posInt.z)] < 0.5)
			{
				_Collisions[0].Result = 1;

				_Collisions[0].V0 = pos;
				_Collisions[0].V1 = posInt;

				return;
			}
		}
	}

}
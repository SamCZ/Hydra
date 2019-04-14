#pragma hydra cmp:CSMain

struct VoxelBuffer
{
	float4 Position;
	float3 Normal;
};

cbuffer Params
{
	int _Width;
	int _Height;
	int _Depth;
};

RWStructuredBuffer<VoxelBuffer> _Buffer;

[numthreads(8, 8, 8)]
void CSMain(uint3 groupId : SV_GroupID, uint3 groupThreadId : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex)
{
	int idx = dispatchThreadId.x + dispatchThreadId.y * _Width + dispatchThreadId.z * _Width * _Height;

	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			VoxelBuffer vert;
			vert.Position = float4(-1, -1, -1, -1);
			vert.Normal = float3(0, 0, 0);

			_Buffer[idx * 15 + (3 * i + j)] = vert;
		}
	}
}
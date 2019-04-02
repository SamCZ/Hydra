#pragma hydra cmp:CSMain

struct VoxelBuffer
{
	float4 Position;
	float3 Normal;
};

cbuffer Params
{
	float a;
};

RWStructuredBuffer<VoxelBuffer> _Buffer;

VoxelBuffer CreateVertex(float3 pos)
{
	VoxelBuffer v;

	v.Position = float4(pos, 1);
	v.Normal = float3(0, 1, 0);

	return v;
}

[numthreads(8, 8, 8)]
void CSMain(uint3 groupId : SV_GroupID, uint3 groupThreadId : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex)
{
	_Buffer[0] = CreateVertex(float3(a, 0, 0));
	_Buffer[1] = CreateVertex(float3(0, 10, 0));
	_Buffer[2] = CreateVertex(float3(10, 10, 0));
}
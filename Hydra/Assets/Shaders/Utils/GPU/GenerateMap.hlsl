#pragma hydra cmp:CSMain

RWStructuredBuffer<float> _VoxelMap;

[numthreads(8, 8, 8)]
void CSMain(uint3 groupId : SV_GroupID, uint3 groupThreadId : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex)
{
	_VoxelMap[dispatchThreadId.x] = 1;
}
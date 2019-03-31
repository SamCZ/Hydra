#pragma hydra cmp:CSMain

RWTexture2D<float> _HeightMap : register(u0);

cbuffer Params
{
	float2 _Position;
	float _Radius;
};

[numthreads(16, 16, 1)]
void CSMain(uint3 groupId : SV_GroupID, uint3 groupThreadId : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex)
{
	float val = _HeightMap[dispatchThreadId.xy];

	float dist = (float)distance((float2)dispatchThreadId.xy, _Position);
	dist /= 122.0;

	_HeightMap[dispatchThreadId.xy] = val + (1.0 - smoothstep(0, (_Radius * 2) / 122.0, dist)) * 0.01;
}
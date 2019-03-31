#pragma hydra cmp:CSMain

RWTexture2D<float4> _OutNormalMap : register(u0);

Texture2D _HeightMap : register(t0);

cbuffer Params
{
	float _Strength;
};

[numthreads(16, 16, 1)]
void CSMain(uint3 groupId : SV_GroupID, uint3 groupThreadId : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex)
{

	uint x = dispatchThreadId.x + 1;
	uint y = dispatchThreadId.y + 1;

	float z0 = _HeightMap[uint2(x - 1, y - 1)].r;
	float z1 = _HeightMap[uint2(x, y - 1)].r;
	float z2 = _HeightMap[uint2(x + 1, y - 1)].r;
	float z3 = _HeightMap[uint2(x - 1, y)].r;
	float z4 = _HeightMap[uint2(x + 1, y)].r;
	float z5 = _HeightMap[uint2(x - 1, y + 1)].r;
	float z6 = _HeightMap[uint2(x, y + 1)].r;
	float z7 = _HeightMap[uint2(x + 1, y + 1)].r;

	float3 normal;

	// Sobel Filter
	normal.y = 1.0 / _Strength;
	normal.x = z0 + 2 * z3 + z5 - z2 - 2 * z4 - z7;
	normal.z = z0 + 2 * z1 + z2 - z5 - 2 * z6 - z7;

	normal = normalize(normal);

	_OutNormalMap[uint2(dispatchThreadId.x, dispatchThreadId.y)] = float4(normal, 1.0);
}
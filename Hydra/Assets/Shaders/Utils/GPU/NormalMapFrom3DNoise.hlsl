#pragma hydra cmp:CSMain

cbuffer Params
{
	int _Width;
	int _Height;
};

StructuredBuffer<float> _Noise;
RWTexture3D<float4> _Result;

[numthreads(8, 8, 8)]
void CSMain(uint3 groupId : SV_GroupID, uint3 groupThreadId : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex)
{
	uint3 id = dispatchThreadId;

	float v = _Noise[id.x + id.y * _Width + id.z * _Width * _Height];

	float dx = v - _Noise[(id.x + 1) + id.y * _Width + id.z * _Width * _Height];

	float dy = v - _Noise[id.x + (id.y + 1) * _Width + id.z * _Width * _Height];

	float dz = v - _Noise[id.x + id.y * _Width + (id.z + 1) * _Width * _Height];


	_Result[id] = float4(normalize(float3(dx, dy, dz)), 1);
}
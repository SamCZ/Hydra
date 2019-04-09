#pragma hydra cmp:CSMain

cbuffer Params
{
	int _Width;
	int _Height;
};

StructuredBuffer<float> _Noise;
RWTexture3D<float4> _Result;

float GetDensity(int x, int y, int z)
{
	return _Noise[x + y * _Width + z * _Width * _Height];
}

float GetVal(int3 index, int xOff, int yOff, int zOff)
{
	return GetDensity(index.x + xOff, index.y + yOff, index.z + zOff);
}

float3 FindNormal(int3 id, int offset)
{
	float3 normal;
	normal.x = (GetVal(id, offset, 0, 0) - GetVal(id, -offset, 0, 0)) * 0.5;
	normal.y = (GetVal(id, 0, offset, 0) - GetVal(id, 0, -offset, 0)) * 0.5;
	normal.z = (GetVal(id, 0, 0, offset) - GetVal(id, 0, 0, -offset)) * 0.5;
	return normal;
}

[numthreads(8, 8, 8)]
void CSMain(uint3 groupId : SV_GroupID, uint3 groupThreadId : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex)
{
	uint3 id = dispatchThreadId;

	float3 normal;
	int offset = 1;

	do
	{
		normal = FindNormal(id, offset);
		offset++;
	} while (normal.x == 0 && normal.y == 0 && normal.z == 0);

	_Result[id] = float4(normalize(normal), 1);
}
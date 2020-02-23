#pragma hydra vert:MainVS pixel:MainPS

#include "Assets/Shaders/Input/Default.hlsli"

struct PS_Input
{
	float4 position   : SV_Position;
	float3 positionLS : WSPOSITION1;
};

cbuffer Globals
{
	float4x4 _ProjectionMatrix;
	float4x4 _ViewMatrix;
	float4x4 _ModelMatrix;
};

PS_Input MainVS(in VS_Input input, uint id : SV_VertexID)
{
	PS_Input OUT;

	OUT.position = mul(mul(mul(_ProjectionMatrix, _ViewMatrix), _ModelMatrix), float4(input.position.xyz, 1.0));

	OUT.positionLS = input.position.xyz;

	return OUT;
}

float4 MainPS(PS_Input IN) : SV_Target
{
	return float4(1, 1, 1, 1);
}
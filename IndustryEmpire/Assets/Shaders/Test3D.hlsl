#pragma hydra vert:MainVS pixel:MainPS

#include "Assets/Shaders/Input/Default.hlsli"

struct FullScreenQuadOutput
{
	float4 position     : SV_Position;
};

cbuffer Globals
{
	float4x4 _ProjectionMatrix;
	float4x4 _ViewMatrix;
	float4x4 _ModelMatrix;
};

FullScreenQuadOutput MainVS(in VS_Input input, uint id : SV_VertexID)
{
	FullScreenQuadOutput OUT;

	OUT.position = mul(mul(mul(_ProjectionMatrix, _ViewMatrix), _ModelMatrix), float4(input.position.xyz, 1.0));

	return OUT;
}

float4 MainPS(FullScreenQuadOutput IN) : SV_Target
{
	return float4(1, 0, 0, 1);
}
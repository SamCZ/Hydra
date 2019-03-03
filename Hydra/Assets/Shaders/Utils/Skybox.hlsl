#pragma hydra vert:MainVS pixel:MainPS

#include "Assets/Shaders/Input/Default.hlsli"

struct PS_Input
{
	float4 position		: SV_POSITION;
	float3 uvw			: TEXCOORD;
};

cbuffer externalData : register(b0)
{
	float4x4 g_ProjectionMatrix;
	float4x4 g_ViewMatrix;
};

PS_Input MainVS(VS_Input input)
{
	PS_Input output;


	output.position = mul(g_ProjectionMatrix, mul(g_ViewMatrix, float4(input.position, 1.0f)));
	output.position.z = output.position.w;
	output.uvw = input.position;

	return output;
}

TextureCube Sky			: register(t0);
SamplerState basicSampler	: register(s0);

float4 MainPS(PS_Input input) : SV_TARGET
{
	return Sky.Sample(basicSampler, normalize(input.uvw));
	//return float4(normalize(input.uvw), 1.0);
	//return float4(1.0, 1.0, 1.0, 1.0);
}
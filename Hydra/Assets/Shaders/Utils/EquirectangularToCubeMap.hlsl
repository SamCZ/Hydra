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

Texture2D EquirectangularMap			: register(t0);
SamplerState BasicSampler	: register(s0);

static float2 invAtan = float2(0.1591, 0.3183);
float2 SampleSphericalMap(float3 v)
{
	float2 uv = float2(atan2(v.z, v.x), asin(v.y));
	uv *= invAtan;
	uv += 0.5;
	return uv;
}

float4 MainPS(PS_Input input) : SV_TARGET
{
	float2 uv = SampleSphericalMap(normalize(input.uvw));

	return EquirectangularMap.Sample(BasicSampler, uv);
}
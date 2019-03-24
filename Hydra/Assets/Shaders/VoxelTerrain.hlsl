#include "Assets/Shaders/RenderStage/Deffered.hlsli"
#include "Assets/Shaders/Utils/Texturing.hlsli"

#pragma hydra vert:MainVS pixel:MainPS
#pragma hydra rs:Deffered

//Define keywords
#pragma hydra kw:pixel:USE_ALBEDO_TEX
#pragma hydra kw:pixel:USE_NORMAL_TEX
#pragma hydra kw:pixel:USE_ROUGHNESS_TEX
#pragma hydra kw:pixel:USE_METALLIC_TEX
#pragma hydra kw:pixel:USE_AO_TEX
#pragma hydra kw:pixel:USE_EMISSION_TEX

//Texture2D _AlbedoMap : register(t0);

SamplerState DefaultSampler	: register(s0);

cbuffer ModelData : register(b0)
{
	float3 _Color;
}

PS_Input OnMainVS(in VS_Input input, in PS_Input output)
{
	//float4 clipPlane = float4(0, 10, 0, -1);
	//output.clip = dot(mul(g_ModelMatrix, input.position), clipPlane);

	return output;
}

PBROutput OnMainPS(in PS_Input input)
{
	PBROutput output;

	output.Albedo = _Color;
	output.Normal = input.normal;
	output.Metallic = 0.0;
	output.Roughness = 1.0;
	output.AO = 0.0;

	return output;
}
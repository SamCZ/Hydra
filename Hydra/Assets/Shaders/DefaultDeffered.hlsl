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

Texture2D _AlbedoMap : register(t0);
Texture2D _NormalMap : register(t1);
Texture2D _RoughnessMap : register(t2);
Texture2D _MetallicMap : register(t3);
Texture2D _AOMap : register(t4);
Texture2D _EmissionMap : register(t5);

SamplerState DefaultSampler	: register(s0);

cbuffer ModelData : register(b0)
{
	float3 _Color;
}

PS_Input OnMainVS(in VS_Input input, in PS_Input output)
{
	return output;
}

PBROutput OnMainPS(in PS_Input input)
{
	PBROutput output;

	float3 pixelNormal = _NormalMap.Sample(DefaultSampler, input.texCoord).xyz;

	float3 normal = normalize(input.normal);
	if (pixelNormal.z)
	{
		float3 tangent = normalize(input.tangent);
		float3 binormal = normalize(input.binormal);
		float3x3 TangentMatrix = float3x3(tangent, binormal, normal);
		normal = normalize(mul(pixelNormal * 2 - 1, TangentMatrix));
	}

	float4 albedo = _AlbedoMap.Sample(DefaultSampler, input.texCoord);
	//float4 albedo = TriplanarTexturing(_AlbedoMap, DefaultSampler, input.positionLS, input.normal, 1.0);
	//float4 albedo = float4(_Color, 1.0);

	output.Albedo = albedo.rgb + _Color;
	output.Normal = normal;
	output.Metallic = _MetallicMap.Sample(DefaultSampler, input.texCoord).r;
	output.Roughness = _RoughnessMap.Sample(DefaultSampler, input.texCoord).r;
	output.AO = 0.5;

	//output.Albedo = (1.0).xxx;
	//output.Metallic = 0.0;
	//output.Roughness = 0.8;
	//output.Emission = ((1.0).xxx - albedo.r) * 2;

	//output.Emission = float3(0.0, 0.0, 0.0);

	if (albedo.a < 0.5)
	{
		//output.Emission = (1.0).xxx;
	}

	//clip(albedo.a - 0.5);

	return output;
}
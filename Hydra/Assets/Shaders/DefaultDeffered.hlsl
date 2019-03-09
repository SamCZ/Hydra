#include "Assets/Shaders/RenderStage/Deffered.hlsli"

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

PS_Input OnMainVS(in VS_Input input, in PS_Input output)
{
	return output;
}

float3 getTriaplanarBlend(float3 normal)
{
	float3 blending = abs(normal) - 0.2679f;
	blending = normalize(max(blending, 0.0));
	return blending / (blending.x + blending.y + blending.z).xxx;
}

float4 TriplanarTexturing(Texture2D tex, float3 worldPos, float3 normal, float scale)
{
	float3 blendAxes = getTriaplanarBlend(normalize(normal));
	float3 scaledWorldPos = worldPos / scale;
	float4 xProjection = tex.Sample(DefaultSampler, scaledWorldPos.yz) * blendAxes.x;
	float4 yProjection = tex.Sample(DefaultSampler, scaledWorldPos.xz) * blendAxes.y;
	float4 zProjection = tex.Sample(DefaultSampler, scaledWorldPos.xy) * blendAxes.z;
	float4 color = xProjection + yProjection + zProjection;
	color.a /= 3.0;
	return color;
}

PBROutput OnMainPS(in PS_Input input)
{
	PBROutput output;

#ifdef USE_ALBEDO_TEX
	output.Albedo = TriplanarTexturing(_AlbedoMap, input.positionLS, input.normal, 1.0).rgb;
#else
	output.Albedo = float3(0.5, 0.5, 0.5);
#endif


	output.Normal = input.normal;
	output.Metallic = 0.0;
	output.Roughness = 1.0;
	output.AO = 0.5;

	/*float3 pixelNormal = T_Normal.Sample(basicSampler, input.texCoord).xyz;

	float3 normal = normalize(input.normal);
	if (pixelNormal.z)
	{
		float3 tangent = normalize(input.tangent);
		float3 binormal = normalize(input.binormal);
		float3x3 TangentMatrix = float3x3(tangent, binormal, normal);
		normal = normalize(mul(pixelNormal * 2 - 1, TangentMatrix));
	}

	float4 albedo = T_Albedo.Sample(basicSampler, input.texCoord);

	output.Albedo = albedo.rgb;
	output.Normal = normal;
	output.Metallic = T_Metallic.Sample(basicSampler, input.texCoord).r;
	output.Roughness = T_Roughness.Sample(basicSampler, input.texCoord).r;
	output.AO = 0.5;
	output.Emission = float3(0.0, 0.0, 0.0);

	clip(albedo.a - 0.5);*/

	return output;
}
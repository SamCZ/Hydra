#include "Assets/Shaders/RenderStage/Deffered.hlsli"

#pragma hydra vert:MainVS pixel:MainPS
#pragma hydra rs:Deffered

Texture2D T_Albedo : register(t0);
Texture2D T_Normal : register(t1);
Texture2D T_Roughness : register(t2);
Texture2D T_Metallic : register(t3);
Texture2D T_AO : register(t4);

SamplerState basicSampler	: register(s0);

PS_Input OnMainVS(in VS_Input input, in PS_Input output)
{
	return output;
}

PBROutput OnMainPS(in PS_Input input)
{
	PBROutput output;

	float3 pixelNormal = T_Normal.Sample(basicSampler, input.texCoord).xyz;

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

	clip(albedo.a - 0.5);

	return output;
}
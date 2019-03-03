#include "Assets/Shaders/RenderStage/Deffered.hlsli"

#pragma hydra vert:MainVS pixel:MainPS
#pragma hydra rs:Deffered

Texture2D T_Albedo : register(t0);
Texture2D T_Normal : register(t1);
Texture2D T_Roughness : register(t2);
Texture2D T_Metallic : register(t3);
Texture2D T_AO : register(t4);

SamplerState basicSampler	: register(s0);

SamplerState SamplerAnisotropic0
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 16;
	AddressU = Wrap;
	AddressV = Wrap;
};

SamplerState SamplerAnisotropic1
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 16;
	AddressU = Wrap;
	AddressV = Wrap;
};

SamplerState SamplerAnisotropic2
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 16;
	AddressU = Wrap;
	AddressV = Wrap;
};

SamplerState SamplerAnisotropic3
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 16;
	AddressU = Wrap;
	AddressV = Wrap;
};

SamplerState SamplerAnisotropic4
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 16;
	AddressU = Wrap;
	AddressV = Wrap;
};

PS_Input OnMainVS(in VS_Input input, in PS_Input output)
{
	return output;
}

PBROutput OnMainPS(in PS_Input input)
{
	PBROutput output;

	output.Albedo = T_Albedo.Sample(basicSampler, input.texCoord).rgb;
	output.Normal = normalize(T_Normal.Sample(basicSampler, input.texCoord).rgb * 2.0 - 1.0);
	output.Metallic = 1.0 - T_Metallic.Sample(basicSampler, input.texCoord).r;
	output.Roughness = T_Roughness.Sample(basicSampler, input.texCoord).r;
	output.AO = T_AO.Sample(basicSampler, input.texCoord).r;
	output.Emission = float3(0.0, 0.0, 0.0);

	return output;
}